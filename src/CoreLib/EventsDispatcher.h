#pragma once
/**___________________________________________________________________________/
@file          EventsDispatcher.h
@author        Tan Jun Jie (t.junjie) (100%)
@date          06/12/2025 (DD/MM/YYYY)
@brief         A events dispatcher implemented using templates. Supports event
	           queuing as well.		

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include <functional>
#include <map>
#include <vector>
#include <queue>
#include "Events.h"

class EventsDispatcher {
	public:
		template <typename Event>
		using FuncCallback = std::function<void(const Event&)>;
		using ListenerID = size_t;

		/*!
		* \brief
		*	Subscribe a listener to an Event.
		* \param[in] callback
		*	- A function object for the listener to call when the specified
		*	event is dispatched / triggered.
		* \param[in] priority
		*	- The priority of this function, defaulted to 0. Higher the priority number means
		*	it will run first.
		* \return
		*	The listener ID for the newly subscribed listener.
		*/
		template <typename Event>
		ListenerID Subscribe(FuncCallback<Event> callback, int priority = 0) {
			std::type_index type{ typeid(Event)};		// get the type index for the Event
			ListenerID id{ GetUniqueListenerId() };			// generate the listener ID
			// create the new listener entry
			ListenerEntry entry{ id, [callback](const void* event) { callback(*static_cast<const Event*>(event)); }, priority, true };

			auto& vec{ listeners[type] };	// get the listener entry vector for this Event
			auto pos{ std::find_if(vec.begin(), vec.end(), [priority](const ListenerEntry& le) { return le.priority < priority; })};
			vec.insert(pos, entry);			// insert into vec based on priority

			return id;						// return the listener ID
		}

		/*!
		* \brief
		*	Calls all the listeners' function callback for the specified Event type.
		* \param[in] e
		*	- The Event object that the listeners belong to, const reference.
		*/
		template <typename Event>
		void Dispatch(const Event& e) {
			std::type_index type{ typeid(Event) };	// get the type index of the event
			auto it{ listeners.find(type) };		// check that the Event has been subscribed
			if (it == listeners.end()) return;

			for (auto& le : it->second) {			// loop through all listeners registed under this event
				if (le.valid) le.fn(&e);			// call the func with the event object
			}
		}

		/*!
		* \brief 
		*	Adding an event to the event queue.
		* \param[in] e 
		*	- The event to enqueue. Will enqueue all functions subscribed under
		*	this event.
		*/
		template <typename Event>
		void QueueEvent(const Event& e) {
			auto ptr{ std::make_shared<Event>(e) };
			QueueEvent(ptr);
		}

		/*!
		* \brief
		*	Adding an event to the event queue. Overload with shared_pointer.
		* \param[in] e
		*	- The shared_ptr to the event to enqueue. Will enqueue all 
		*	functions subscribed under this event.
		*/
		template <typename Event>
		void QueueEvent(const std::shared_ptr<Event>& e) {
			std::type_index type{ typeid(Event) };		// get the eventTypeId of the event
			eventQueue.push([this, e, type]() {
				auto it {listeners.find(type)};
				if (it != listeners.end()) {
					for (auto& le : it->second) {
						if (le.valid) le.fn(e.get());
					}
				}
			});
		}

		/*!
		* \brief
		*	Enqueues the specific function for this event into event queue. Nothing happens
		*	if no listener with the specified id was found.
		* \param[in] e
		*	- A const reference to the Event that func belongs to.
		* \param[in] id
		*	- The listener id to queue.
		*/
		template <typename Event>
		void QueueEvent(const Event& e, ListenerID id) {
			auto ptr{ std::make_shared<Event>(e) };
			QueueEvent(ptr, id);
		}

		/*!
		* \brief 
		*	Enqueues the specific function for this event into event queue. Nothing happens
		*	if no listener with the specified id was found.
		* \param[in] e
		*	- A reference to a const shared pointer to the Event that func belongs to.
		* \param[in] id
		*	- The listener id to queue.
		*/
		template <typename Event>
		void QueueEvent(const std::shared_ptr<Event>& e, ListenerID id) {
			std::type_index type{ typeid(Event) };	// get the type index for Event
			auto it{ listeners.find(type)};			// get the vector of listeners for this event type
			if (it == listeners.end()) return;
			
			// find the specific listener from the vector
			auto func{ std::find_if(it->second.begin(), it->second.end(), [id](const ListenerEntry& le) { return le.id == id; }) };
			if (func == it->second.end() || !func->valid) return;	// check that specific listener func exists and is valid 

			// Push the event function into eventQueue
			eventQueue.push([e, fn = func->fn]() { fn(e.get()); });
		}

		/*!
		* \brief Dispatches all events in queue in FIFO manner.
		*/
		void DispatchQueue() {
			while (!eventQueue.empty()) {	// keep looping while event queue is not empty
				eventQueue.front()();		// call the function of the event at the front of the queue
				eventQueue.pop();			// pop the event queue
			}

			// erase subscribed events when event queue is empty
			for (auto& [type, vec] : listeners) {
				vec.erase(std::remove_if(vec.begin(), vec.end(),
					[](auto& le) { return !le.valid; }), vec.end());
			}
		}

		/*!
		* \brief Unsubscribes all listeners under specified Event.
		*/
		template <typename Event>
		void UnsubscribeAll() {
			std::type_index type{ typeid(Event) };
			auto it{ listeners.find(type) };
			if (it != listeners.end()) {
				for (auto& le : it->second) { le.valid = false; }
			}
		}

		/*!
		* \brief Unsubscribes a specific listener from specified Event.
		* \param[in] id - The listener id to remove.
		*/
		template <typename Event>
		void Unsubscribe(ListenerID id) {
			std::type_index type{ typeid(Event) };
			auto it{ listeners.find(type) };
			if (it != listeners.end()) {
				for (auto& le : it->second) {
					if (le.id == id) {
						le.valid = false;
						break;
					}
				}
			}
		}

		/*!
		* \brief Prints information regarding events and its listeners.
		*/
		void DumpListeners() const {
#ifdef _DEBUG
			LOGI("Dumping Event Dispatcher Information:");
			for (const auto& [type, vec] : listeners) {
				LOGI("Event type: %s | No. of Listeners: %zu", type.name(), vec.size());

				for (const auto& le : vec) {
					LOGI("   ID: %zu | Priority: %d | Valid: %d", le.id, le.priority, le.valid);
				}
			}
#endif
		}

		/*!
		* \brief Clears event queue by swapping with an empty queue.
		*/
		void ClearEventQueue() {
			std::queue<std::function<void()>> empty;
			std::swap(eventQueue, empty);
		}
	private:
		/*!
		* \brief Generate a unique ID for each listener.
		*/
		inline ListenerID GetUniqueListenerId() { return nextListenerId++; }
	private:
		using Func = std::function<void(const void*)>;
		struct ListenerEntry {
			ListenerID id;				// the listener id
			Func fn;				// the function to call
			int priority{};			// higher = run first
			bool valid{ true };		// whether listener is valid (not been marked for removal)
		};
		std::unordered_map<std::type_index, std::vector<ListenerEntry>> listeners;
		std::queue<std::function<void()>> eventQueue;
		ListenerID nextListenerId{1}; // start from 1, 0 is invalid
};