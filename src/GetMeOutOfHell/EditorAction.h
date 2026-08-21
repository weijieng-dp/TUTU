/*!
@file       EditorActions.h
@author     Ou Yukang (yukang.ou) 100%
@date       15/11/2025
@brief		Handles editor action history for undo redo functionalities


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "imgui.h"
#include <list>
#include <memory>
#include <type_traits>
#include "MathLib.h"
#include "Platform.h"
#include <any>

class EditorActions {
public:
	struct ActionBase {
		virtual ~ActionBase() = default;
		virtual void Execute() = 0;
		/*
		*\brief get the inverted action "add" action will return a "remove" action,
		* "set" action will return a "unset" action
		*/
		virtual std::unique_ptr<ActionBase> GetInverse() = 0;
	};

	/*!
	* \brief
	*	custom undo redo with specified callables
	*/
	template <typename CallableUndo, typename CallableRedo>
	struct ActionAddRemove :ActionBase {
		ActionAddRemove() = delete;
		ActionAddRemove(CallableUndo _undo, CallableRedo _redo) :undo{ _undo }, redo{ _redo } {}
		void Execute() override
		{
			undo();
		}

		/*!
		* \brief
		*	return a action with undo and redo flipped
		*/
		std::unique_ptr<ActionBase> GetInverse() override
		{
			return std::make_unique<ActionAddRemove<CallableRedo, CallableUndo>>(redo, undo);
		}

		CallableUndo undo; // function call when undo is triggered
		CallableRedo redo; // function call when redo is triggered
	};
private:

	template <typename T>
	struct ActionSetValue :ActionBase {
		ActionSetValue() = delete;
		ActionSetValue(T& target, T value) :target{ target }, setValue{ value } {}

		/*!
		* \brief
		*	restore target's value with old value
		*/
		void Execute() override
		{
			target = setValue;
		}

		/*!
		* \brief
		*	return an action that sets target to modified value
		*/
		std::unique_ptr<ActionBase> GetInverse() override
		{
			return std::make_unique<ActionSetValue<T>>(target, target);
		}


		T setValue; // value to set when action is triggered
		T& target;
	};
public:
	EditorActions() = default;
	EditorActions(EditorActions const& rhs) = delete;
	~EditorActions() = default;
	
	/*!
	* \brief
	*	clear undo and redo stack
	*/
	void Clear();

	/*!
	* \brief
	*	save value of ref to cache
	* \param
	*	ref - target to save value of
	*/
	template <typename T>
	void Cache(T& ref)
	{
		// enforce caching of non pointer and non const values ONLY
		static_assert(!std::is_pointer<T>::value);
		static_assert(!std::is_const<T>::value);

		//LOGI("Caching");
		//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
		//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";

		// save value to cache
		if(!frontCache.has_value())
			frontCache.emplace<T>(ref);
		else if (!backCache.has_value())//cache already taken, save to backCache instead
			backCache.emplace<T>(ref);
		else // this shouldn't happen...
		{
			LOGI("Unexpected amount of cache calls without push!, check EditorActions.h");
#ifdef DEBUG
			assert(false && "Unexpected amount of cache calls without push!");
#endif
		}
	}

	/*!
	* \brief
	*	remove oldest value from cache
	*/
	void PopCache();

	/*!
	* \brief
	*	remove oldest value from cache
	*/
	void FlushCache();

	/*!
	* \brief
	*	push oldest value in cache for undo redo
	* \param
	*	ref - target to set value for when undo is triggered
	*/
	template <typename T>
	void Push(T& ref)
	{
		// enforce caching of non pointer and non const values ONLY
		static_assert(!std::is_pointer<T>::value);
		static_assert(!std::is_const<T>::value);

		//LOGI("Pushing");
		//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
		//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";

		// push oldest cached value
		if (frontCache.has_value())
		{
			undoStack.push_back(std::make_unique<ActionSetValue<T>>(ref, std::any_cast<T>(frontCache)));
			frontCache.reset(); // destroy value in cache
			frontCache.swap(backCache); // bring back cache to the front
		}
		else if (backCache.has_value())
		{
			undoStack.push_back(std::make_unique<ActionSetValue<T>>(ref, std::any_cast<T>(backCache)));
			backCache.reset(); // destroy value in cache
		}
		else // this shouldn't happen...
		{
			LOGI("Tried to push with no values in cache!, check EditorActions.h");
#ifdef DEBUG
			assert(false && "Tried to push with no values in cache!");
#endif
		}

		// ensure stack doesn't exceed max stack size
		if (undoStack.size() > maxStackSize)
			undoStack.pop_front();

		// clear redo stack when a new actions is pushed
		redoStack.clear();
	}

	/*!
	* \brief
	*	push generic undo acition into undo stack
	* \param
	*	undo - undo action to be pushed
	*/
	void Push(std::unique_ptr<ActionBase> undo);

	enum CachePushResult_ {
		CachePushResult_None = 0,
		CachePushResult_Pushed = 1 << 0, // user began edit, initial value saved
		CachePushResult_Cached = 1 << 1, // user ended edit, push cache to stack
	};
	typedef int CachePushResultFlags_;
	/*!
	* \brief
	*	Cache value if imgui edit started
	*	Push value  if imgui edit ended
	* \param
	*	undo - undo action to be pushed
	*/
	template <typename T>
	CachePushResultFlags_ TryCacheOrPush(T& ref, T& cacheValue)
	{
		// enforce caching of non pointer and non const values ONLY
		static_assert(!std::is_pointer<T>::value);
		static_assert(!std::is_const<T>::value);
		CachePushResultFlags_ resultBits = CachePushResult_None;

		if (ImGui::IsItemDeactivatedAfterEdit()) // edit finalized
		{
			//LOGI("Pushing");
			if (frontCache.has_value())
			{
				Push<T>(ref);
				resultBits |= CachePushResult_Pushed;
				//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
				//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";
			}
			else
			{
				//LOGI("Abort Push");
				//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
				//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";
			}
		}
		else if (ImGui::IsItemDeactivated()) // no edits made
		{
			//LOGI("Popping");
			PopCache();
			//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
			//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";
		}
		if (ImGui::IsItemActivated()) // imgui activated this frame
		{
			//LOGI("Caching");
			Cache<T>(cacheValue);
			//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
			//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";
			resultBits |= CachePushResult_Cached;
		}

		return resultBits;
	}
	template <typename T>
	CachePushResultFlags_ TryCacheOrPush(T& ref)
	{
		return TryCacheOrPush<T>(ref, ref);
	}

	/*!
	* \brief
	*	trigger the most recent action on the undo stack
	*/
	void Undo()
	{
		if (!undoStack.empty())
		{
			redoStack.push_back(undoStack.back()->GetInverse());
			undoStack.back()->Execute();
			undoStack.pop_back();
		}
	}

	/*!
	* \brief
	*	trigger the most recent action on the redo stack
	*/
	void Redo()
	{
		if (!redoStack.empty())
		{
			undoStack.push_back(redoStack.back()->GetInverse());
			redoStack.back()->Execute();
			redoStack.pop_back();
		}
	}

	// max stack size of undo redo before oldest actions are overriden
	const int maxStackSize{ 30 };
private:
	// used to store any kind of unknown data types temporarily to be pushed into undo/redo stack
	// sometimes a new cache call is made before the push is made so backcache is necessary
	std::any frontCache{}, backCache{};

	std::list<std::unique_ptr<ActionBase>> undoStack;
	std::list<std::unique_ptr<ActionBase>> redoStack;
};