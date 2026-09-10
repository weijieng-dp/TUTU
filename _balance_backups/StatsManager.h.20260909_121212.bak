/*!
@file       StatsManager.h
@author     Ng Wei Jie (weijie.ng) (10%)
@author     Ou Yukang (yukang.ng) (30%)
@author     Kaeden Tan (kaedenjiawei.tan) (60%)
@date		03/02/2026

This file defines a flexible gameplay statistics and resource management
system used to manage player attributes, resources, and status effects.

The Stat<T> template class represents a modifiable statistic built from 
a base value, additive offsets, and multiplicative modifiers, with optional 
minimum and maximum bounds. It automatically recalculates the final value 
whenever modifiers are applied.

The Resource<T> class represents consumable or regenerating resources 
(such as health) that track a current value and enforce limits using 
linked minimum and maximum Stat objects.

A common IValue interface allows different stat types (stats, resources, 
special flags, and status effects) to be modified through a unified 
interface using additive and multiplicative operators.

The system also includes status effect classes (Poison, Slow, Freeze) 
built on the StatusEffect and StatusEffectStat abstractions, enabling timed 
gameplay effects that modify entity behavior over time.

Finally, the StatsManager class centralizes all player-related statistics, 
resources, projectile modifiers, and status effects. It provides lookup by 
name, reset functionality, and management utilities such as luck and item rarity tracking.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
*/
/*____________________________________________________________________________*/


#pragma once
#include <string>
#include <type_traits>
#include <cassert>
#include <map>
#include <unordered_map>
#include <functional>

#include "Platform.h"
#include "refl.hpp"
#include "MathLib.h"

class IValue {
public:
	virtual IValue& operator+= (double other) = 0;
	virtual IValue& operator-= (double other) = 0;
	virtual IValue& operator*= (float other) = 0;

	virtual void Reset() = 0;
	virtual double GetNetValue() = 0;
};

template <typename T>
class Stat : public IValue
{
	static_assert(std::is_arithmetic_v<T>, "Stat can only be used with arithmetic types!");

	T base{};
	float multiplier{ 1.f };
	T offset{};
	T netValue{};

	std::optional<T> floor;
	std::optional<T> ceiling;

	/*!
	* \brief
	*    Recomputes the final statistic value using the base value,
	*    accumulated offset, and multiplier. Applies floor and ceiling
	*    constraints if they are defined.
	*/
	void Recalculate() { 
		netValue = static_cast<T>((base + offset) * multiplier);
		if (floor.has_value() && netValue < floor.value()) {
			offset += floor.value() - netValue;
			netValue += floor.value()- netValue;
		}
		if (ceiling.has_value() && netValue > ceiling.value()) {
			offset -= netValue - ceiling.value();
			netValue -= netValue - ceiling.value();
		}
	}

public:
	T Max() {
		return ceiling.has_value() ? ceiling.value() : 0;
	}
	T Min() {
		return floor.has_value() ? floor.value() : 0;
	}
	/*!
	* \brief
	*    Constructs a Stat with a specified base value.
	*
	* \param
	*    _baseValue - The initial base value of the statistic.
	*/
	Stat(T _baseValue) :base{ _baseValue }, netValue{ base }, floor{ std::nullopt }, ceiling{std::nullopt} {}
	Stat(T _baseValue, T _floor, T _ceiling) :base{ _baseValue }, netValue{ base }, floor{}, ceiling{} {
		if (_floor > _ceiling) {
			ceiling = floor = _floor;
		}
	}
	Stat(T _baseValue, T _val, bool setMax) :base{ _baseValue }, netValue{ base }, floor{}, ceiling{} {
		if (setMax) {
			ceiling = _val;
		}
		else {
			floor = _val;
		}
	}

	/*!
	* \brief
	*    Retrieves the final computed value of the statistic after all
	*    offsets and multipliers have been applied.
	*
	* \return
	*    [T] The current computed statistic value.
	*/
	T Get() { return netValue; } const

	T Base() { return base; } const

	/*!
	* \brief
	*    Applies an additive offset to the statistic and recalculates
	*    the final value.
	*
	* \param
	*    _offset - The value to add to the current offset.
	*/
	void AddOffset(T _offset){
		offset += _offset;
		Recalculate();
	}

	/*!
* \brief
*    Applies a multiplicative modifier to the statistic and
*    recalculates the final value.
*
* \param
*    _multiplier - The multiplier value to add to the current multiplier.
*/
	void AddMultiplier(float _multiplier) {
		multiplier += _multiplier;
		Recalculate();
	}

	/*!
* \brief
*    Combines two Stat objects by summing their base values and offsets,
*    then applying their respective multipliers.
*
* \param
*    other - Another Stat object to combine with.
*
* \return
*    [T] The computed combined value of both statistics.
*/
	template <typename U>
	T operator+(Stat<U> other)
	{
		return (base + other.base + offset + other.offset) * multiplier * other.multiplier;
	}

	/*!
	* \brief
	*    Resets the statistic modifiers to their default values.
	*
	*    Offset is set to 0 and multiplier is reset to 1.
	*/
	void Reset() override {
		multiplier = 1.f;
		offset = 0;
		Recalculate();
	}

	/*!
	* \brief
	*    Applies an additive increase through the IValue interface.
	*
	* \param
	*    val - Value added to the stat offset.
	*
	* \return
	*    [IValue&] Reference to the modified stat.
	*/
	IValue& operator+= (double val) override {
		AddOffset(static_cast<T>(val));
		return *this;
	};

	/*!
	* \brief
	*    Applies an additive decrease through the IValue interface.
	*
	* \param
	*    val - Value subtracted from the stat offset.
	*
	* \return
	*    [IValue&] Reference to the modified stat.
	*/
	IValue& operator-= (double val) override {
		AddOffset(-static_cast<T>(val));
		return *this;
	};

	/*!
	* \brief
	*    Applies a multiplicative modifier through the IValue interface.
	*
	* \param
	*    val - Multiplier value added to the current multiplier.
	*
	* \return
	*    [IValue&] Reference to the modified stat.
	*/
	IValue& operator*= (float val) override {
		AddMultiplier(val);
		return *this;
	};

	/*!
	* \brief
	*    Retrieves the computed statistic value as a double.
	*	 To be compliant with IValue interface
	*
	* \return
	*    [double] The net value of the statistic.
	*/
	double GetNetValue() {
		return static_cast<double>(Get());
	}
};

template <typename T>
class Resource: public IValue
{
	static_assert(std::is_arithmetic_v<T>, "Resource can only be used with arithmetic types!");
	T start{};
	T current{};
	Stat<T>* minimum{};
	Stat<T>* maximum{};

public:
	Resource(T _startVal) :
		start{ _startVal }, current{ start }, minimum{}, maximum{} {
	}
	Resource(T _startVal, Stat<T>& _minVal, Stat<T>& _maxVal) :
		start{ _startVal }, current{ start }, minimum{ &_minVal }, maximum{ &_maxVal } {
	}

	Resource(T _startVal, Stat<T>& val, bool setMax) :
		start{ _startVal }, current{ start }, minimum{}, maximum{} {
		if (setMax)
			maximum = &val;
		else
			minimum = &val;
	}

	/*!
	* \brief
	*    Retrieves the current resource value.
	*
	* \return
	*    [T const&] The current value of the resource.
	*/
	T const& Curr() const { return current; }

	/*!
	* \brief
	*    Sets the current resource value while enforcing minimum
	*    and maximum limits if they exist.
	*
	* \param
	*    val - The new resource value.
	*/
	void Curr(T val) {
		if (maximum && minimum) {
			if (maximum->Get() < minimum->Get()) {
				maximum->Reset();
				maximum->AddOffset(-maximum->Get());
			}
			current = std::clamp(val, minimum->Get(), maximum->Get());
		}
		else if (maximum) {
			current = val > maximum->Get() ? maximum->Get() : val;
		}
		else if (minimum) {
			current = val < minimum->Get() ? minimum->Get() : val;
		}
		else {
			current = val;
		}
	}

	/*!
	* \brief
	*    Retrieves the starting value of the resource.
	*
	* \return
	*    [T const&] The initial resource value.
	*/
	T const& Start() const { return start; }

	/*!
	* \brief
	*    Retrieves the maximum limit of the resource.
	*
	* \return
	*    [Stat<T>] The stat representing the maximum resource value.
	*/
	Stat<T> const Max() const { return *maximum; }

	/*!
	* \brief
	*    Retrieves a modifiable reference to the maximum limit stat.
	*
	* \return
	*    [Stat<T>&] Reference to the maximum stat.
	*/
	void Max(Stat<T>* _maximum) { maximum = _maximum; }

	/*!
	* \brief
	*    Removes the maximum constraint from the resource.
	*/
	void ClearMax() {
		maximum = nullptr;
	}

	/*!
	* \brief
	*    Retrieves the minimum limit of the resource.
	*
	* \return
	*    [Stat<T> const&] The stat representing the minimum resource value.
	*/
	Stat<T> const& Min() const { return *minimum; }

	/*!
	* \brief
	*    Retrieves a modifiable reference to the minimum limit stat.
	*
	* \return
	*    [Stat<T>&] Reference to the minimum stat.
	*/
	void Min(Stat<T>* _minimum) { return minimum = _minimum; }

	/*!
	* \brief
	*    Removes the minimum constraint from the resource.
	*/
	void ClearMin() {
		minimum = nullptr;
	}

	/*!
	* \brief
	*    Resets the resource to its starting value.
	*/
	void Reset() override { Curr(start); }
	
	/*!
	* \brief
	* Increases the resource value through the IValue interface.
	*
	* \param
	* val - Amount added to the current value.
	*
	*\return
	*[IValue&] Reference to the resource.
	*/	
	IValue& operator+= (double val) override {
		Curr(current + static_cast<T>(val));
		return *this;
	};
	/*!
	* \brief
	*    Decreases the resource value through the IValue interface.
	*
	* \param
	*    val - Amount subtracted from the current value.
	*
	* \return
	*    [IValue&] Reference to the resource.
	*/
	IValue& operator-= (double val) override {
		Curr(current - static_cast<T>(val));
		return *this;
	};
	IValue& operator*= (float) override {
		return *this;
		// nothing happens to the resource when multiplied
	};

	/*!
	* \brief
	*    Retrieves the current resource value as a double.
	*
	* \return
	*    [double] The current resource value.
	*/
	double GetNetValue() override {
		return static_cast<double>(Curr());
	}
};

struct StatusEffect {
	StatusEffect(float _duration, 
				float _effectiveness): 
				duration(_duration), effectiveness(_effectiveness), timeElapsed(0){}

	/*!
	* \brief
	*    Resets internal timing variables when a status effect begins.
	*/
	void Init() {
		timeElapsed = 0;
	}

	float duration;
	float effectiveness;
	float timeElapsed;

	/*!
	* \brief
	*    Updates the status effect each frame.
	*
	* \param
	*    dt - Delta time since last update.
	* \param
	*    value - The affected value (e.g., health or movement speed).
	*
	* \return
	*    [double] The modified value after applying the effect.
	*/
	virtual double UpdateCallback(float dt, double) = 0;
};

class StatusEffectStat : public IValue {
public:
	/*!
	* \brief
	*    Increases the strength of the status effect.
	*
	* \param
	*    other - Value added to the current effect strength.
	*
	* \return
	*    [IValue&] Reference to the modified status effect stat.
	*/
	IValue& operator+= (double other) override {
		strength += other;
		Recalculate();
		return *this;
	}
	/*!
	* \brief
	*    Decreases the strength of the status effect.
	*
	* \param
	*    other - Value subtracted from the current effect strength.
	*
	* \return
	*    [IValue&] Reference to the modified status effect stat.
	*/
	IValue& operator-= (double other) override {
		strength -= other;
		Recalculate();
		return *this;
	}
	/*!
	* \brief
	*    Multiplies the strength of the status effect.
	*
	* \param
	*    other - Multiplier applied to the effect strength.
	*
	* \return
	*    [IValue&] Reference to the modified status effect stat.
	*/
	IValue& operator*= (float other) override {
		strength *= other;
		Recalculate();
		return *this;
	}
	/*!
	* \brief
	*    Resets the status effect strength to zero.
	*/
	void Reset() override {
		strength = 0;
		Recalculate();
	}
	/*!
	* \brief
	*    Retrieves the current strength of the status effect.
	*
	* \return
	*    [double] The current effect strength.
	*/
	double GetNetValue() override {
		return strength;
	}
	/*!
	* \brief
	*    Determines whether the status effect is currently active.
	*
	* \return
	*    [bool] True if the effect strength is non-zero.
	*/
	bool IsActive() {
		return static_cast<bool>(strength);
	}

protected:
	double strength = 0;
	virtual void Recalculate() = 0;
};

struct PoisonEffect : StatusEffect {

	PoisonEffect(float _duration, float _effectiveness) : StatusEffect(_duration, _effectiveness){}
	PoisonEffect(PoisonEffect const& other) : StatusEffect(other.duration, other.effectiveness) {}

	double tickTimeElapsed = 0.f;

	/*!
	* \brief
	*    Updates the poison effect over time.
	*
	*    Applies periodic damage to the target based on the effect's
	*    effectiveness and elapsed time.
	*
	* \param
	*    dt - Time elapsed since the last update.
	* \param
	*    hp - Current health value of the target.
	*
	* \return
	*    [double] Updated health value after poison damage.
	*/
	double UpdateCallback(float dt, double hp) override;
};

class PoisonStat : public StatusEffectStat {
	public:
		PoisonStat() :effect{ 0,0 } {};
		/*!
		* \brief
		*    Generates a runtime poison effect instance using the
		*    current stat parameters.
		*
		* \return
		*    [PoisonEffect] A poison effect configured with the
		*    current strength values.
		*/
		PoisonEffect GenerateEffectInstance() {
			return effect;
		}
	private:
		/*!
		* \brief
		*    Recalculates the internal poison effect parameters
		*    every +1 to strength increases the number of ticks per second
		*/
		void Recalculate() override;
		PoisonEffect effect;
};

struct SlowEffect : StatusEffect {
	SlowEffect(float _duration, float _effectiveness) : StatusEffect(_duration, _effectiveness) {}
	SlowEffect(SlowEffect const& other) : StatusEffect(other.duration, other.effectiveness) {}
	/*!
	* \brief
	*    Updates the slow effect over time.
	*
	*    Reduces the movement speed of the affected entity
	*    according to the effect's effectiveness.
	*
	* \param
	*    dt - Time elapsed since the last update.
	* \param
	*    movespeed - Current movement speed of the entity.
	*
	* \return
	*    [double] Modified movement speed after applying slow.
	*/
	double UpdateCallback(float dt, double movespeed) override;
};

class SlowStat : public StatusEffectStat {
public:
	SlowStat() :effect{ 0,0 } {};
	/*!
	* \brief
	*    Generates a runtime slow effect instance using the
	*    current stat parameters.
	*
	* \return
	*    [SlowEffect] A slow effect configured with the
	*    current strength values.
	*/
	SlowEffect GenerateEffectInstance() {
		return effect;
	}
private:
	/*!
	* \brief
	*    Recalculates the internal slow effect parameters
	*    every +1 to strength increases the duration slow by 30%
	*/
	void Recalculate() override;
	SlowEffect effect;
};

struct FreezeEffect : StatusEffect {

	static constexpr int threshold{ 3 };
	int buildUp{ 0 };
	/*!
	* \brief
	*    Updates the freeze effect over time.
	*
	*    Temporarily immobilizes or greatly reduces the movement
	*    speed of the affected entity.
	*
	* \param
	*    dt - Time elapsed since the last update.
	* \param
	*    movespeed - Current movement speed of the entity.
	*
	* \return
	*    [double] Modified movement speed after applying freeze.
	*/
	FreezeEffect(float _duration, float _effectiveness) : StatusEffect(_duration, _effectiveness), buildUp(0){}
	FreezeEffect(FreezeEffect const& other) : StatusEffect(other.duration, other.effectiveness), buildUp(0) {}

	double UpdateCallback(float dt, double movespeed) override ;
};

class FreezeStat : public StatusEffectStat {
public:
	FreezeStat() :effect{ 0,0 } {};
	/*!
	* \brief
	*    Generates a runtime freeze effect instance using the
	*    current stat parameters.
	*
	* \return
	*    [FreezeEffect] A freeze effect configured with the
	*    current strength values.
	*/
	FreezeEffect GenerateEffectInstance() {
		return effect;
	}
private:
	/*!
	* \brief
	*    Recalculates the internal freeze effect parameters
	*    every +1 to strength increases the duration by 0.5 seconds
	*/
	void Recalculate() override;
	FreezeEffect effect;
};

struct KnockbackEffect : StatusEffect {
	Vec2 direction;
	
	KnockbackEffect(float _duration, float _effectiveness, Vec2 _direction) : StatusEffect(_duration, _effectiveness), direction(_direction){}
	KnockbackEffect(KnockbackEffect const& other) : StatusEffect(other.duration, other.effectiveness), direction(other.direction){}

	double UpdateCallback(float dt, double) override;
};

class KnockbackStat : public StatusEffectStat {
public:
	KnockbackStat() :effect( 0, 0, Vec2{0.f,0.f} ) {};

	KnockbackEffect GenerateEffectInstance(Vec2 _direction) {
		effect.direction = _direction;
		return effect;
	}
private:
	void Recalculate() override;
	KnockbackEffect effect;
};
class Special : public IValue {
	bool activated{ false };
public:
	Special(bool val) : activated(val){}

	IValue& operator+= (double) {
		activated = true;
		return *this;
	}
	IValue& operator-= (double) {
		activated = false;
		return *this;
	}
	IValue& operator*= (float) {
		return *this;
	}


	void Reset() {
		activated = false;
	}
	double GetNetValue() {
		return activated;
	}
};

class StatsManager
{
public:
	StatsManager();
	void ResetPlayer();
	void ResetPlayerStats();
	void ResetPlayerResources();

	/*!
	* \brief
	*    Assigns a strength value to a projectile type.
	*
	* \param
	*    Projectiletype - The identifier string of the projectile type.
	* \param
	*    strength - The strength value associated with the projectile type.
	*/
	void SetProjectileType(std::string Projectiletype, float strength);

	/*!
	* \brief
	*    Retrieves the strength value of a specified projectile type.
	*
	* \param
	*    projectiletype - The identifier string of the projectile type.
	*
	* \return
	*    [float] The strength value of the projectile type.
	*/
	float GetProjectileType(std::string projectiletype);

	/*!
	* \brief
	*    Retrieves a stat or resource using its string identifier.
	*
	* \param
	*    statName - Name of the stat to retrieve.
	*
	* \return
	*    [IValue*] Pointer to the requested stat value.
	*/
	IValue* GetByName(std::string const& statName);
	std::unordered_map<std::string, StatusEffectStat*> const& GetStatusEffects() const { return statusEffects; }

	/*!
	* \brief
	*    Sets the current item rarity value used in gameplay logic.
	*
	* \param
	*    rarity - New rarity level.
	*/
	void SetRarity(int);
	/*!
	* \brief
	*    Adds luck points to the player's current luck pool.
	*
	* \param
	*    amount - Luck value to add.
	*/
	void DepositLuck(int);
	/*!
	* \brief
	*    Resets the accumulated luck value.
	*/
	void ResetLuck();
	/*!
	* \brief
	*    Retrieves the current item rarity value.
	*
	* \return
	*    [int] Current rarity level.
	*/
	int GetRarity() const;
	/*!
	* \brief
	*    Retrieves the current accumulated luck value.
	*
	* \return
	*    [int] Player luck value.
	*/
	int GetLuck() const;

	/*!
	* \brief
	*    Checks whether the player has collected a treasure.
	*
	* \return
	*    [bool] True if treasure has been collected.
	*/
	bool GetTreasureCollected() const;
	/*!
	* \brief
	*    Marks treasure as collected.
	*/
	void SetTreasureCollected();
	/*!
	* \brief
	*    Resets the treasure collected flag.
	*/
	void ResetTreasureCollected();
private:

	std::map<std::string, float> projectiletype;

	std::unordered_map<std::string, IValue*> memberLookup;
	std::unordered_map<std::string, StatusEffectStat*> statusEffects;

	//--------------------------------------------------
	// Touching these stats make you a pdf
	//--------------------------------------------------
	Stat<int> minHealth{ 0 };
	int luck = 0;
	int currentItemRarity = 0;
	bool treasureCollected;
	bool godMode = false;
public:
	bool GodMode();

	//--------------------------------------------------
	// Player stats
	//--------------------------------------------------q
	Stat<float> movementSpeed{ 1000.f , 0.f, false};
	Stat<float> attackSpeed{ 2.0f , 1.f, false };

	//--------------------------------------------------
	// Player projectile stats
	//--------------------------------------------------
	Stat<float> projectileDamage{ 6.f , 3.f, false };
	Stat<int  > projectileMultishot{ 1 , 1, false };
	Stat<float> projectileSpread{ 30.f , 30.f, 360.f};
	Stat<float> projectileLifetime{ 1.f , 0.f, false};
	Stat<float> projectileSpeed{300.f, 0.f, false};
	Stat<float> projectileSize{50.f, 1.f , false};
	Stat<int>	projectileSplit { 0 , 0, false };
	Stat<int>	burst{ 1 , 1, false };

	//--------------------------------------------------
	// Player projectile special effects
	//--------------------------------------------------
	Special homing{ false };
	Special pierce{ false };
	Special bounce{ false };

	Special aura{ false };
	Special auraBuff{ false };

	//--------------------------------------------------
	// Player status effect stats
	//--------------------------------------------------
	PoisonStat poison{};
	SlowStat slow{};
	FreezeStat freeze{};
	KnockbackStat knockback{};

	//--------------------------------------------------
	// Player resources
	//--------------------------------------------------
	Stat<int> maxHealth{ 6 , 12, true};
	Resource<int> health{ 6, minHealth, maxHealth };

	void Debug() {
		for(auto& [name, member] : memberLookup) {
			LOGI("%s: %f", name.c_str(), member->GetNetValue());
		}
	}

	std::vector<std::pair<std::string,std::string>> StatsView() {
		std::vector<std::pair<std::string, std::string>> retVal;
		for (auto& [name, member] : memberLookup) {
			retVal.push_back({ name, std::to_string(member->GetNetValue()) });
		}
		return retVal;
	}
};

REFL_AUTO(
	type(StatsManager),
	field(movementSpeed),
	field(attackSpeed),

	field(projectileDamage),
	field(projectileMultishot),
	field(projectileSpread),
	field(projectileLifetime),
	field(projectileSpeed),
	field(projectileSize),
	field(projectileSplit),
	field(burst),
	field(aura),
	field(auraBuff),

	field(poison),
	field(slow),
	field(freeze),
	field(knockback),

	field(homing),
	field(pierce),
	field(bounce),

	field(maxHealth),
	field(health)
)