/**___________________________________________________________________________/
@file       DialogueScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       25/02/2026   (DD/MM/YYYY)
@brief      Script that handles the player character's reactive dialogue system.
            Loads dialogue lines from a JSON file, selects a random line based
            on the current game event, displays it as text above the player,
            and plays the corresponding voice line based on the dialogue's vibe.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"
#include <list>

// Emotional tone of a dialogue line, used to select the correct voice line
enum class Vibe {
    HOPEFUL,
    DEJECTED,
    SCARED,
    DEATH
};

// In-game events that can trigger a dialogue line
enum class EventType {
    ENTERDUNGEON,   // player enters the start tile
    COMBAT_S,       // combat with small enemies only
    COMBAT_M,       // combat with medium enemies only
    COMBAT_L,       // combat with large enemies only
    COMBAT_SM,      // combat with small and medium enemies
    COMBAT_SL,      // combat with small and large enemies
    COMBAT_ML,      // combat with medium and large enemies
    COMBAT_E,       // combat with all enemy types
    PUNISH_B,       // blind gimmick punishment
    PUNISH_S,       // swarm gimmick punishment
    PUNISH_F,       // fast enemies gimmick punishment
    TREASURE,       // player enters a treasure tile
    IDLE,           // player is idle and not progressing
    EXIT,           // player enters the exit tile
    DEATH,          // player dies
    ITEM,           // player picks up an item
    WAVE1,          // start of wave 1 in wave mode
    WAVE2,          // start of wave 2 in wave mode
    WAVE3,          // start of wave 3 in wave mode
    WIN             // player wins the run
};

// A single dialogue entry loaded from DialogueData.json
struct Dialogue {
    std::string dialogueLine{};  // the text to display
    std::string item{};          // item name this line is associated with (only for ITEM event)
    EventType eventType{};       // the event that triggers this line
    Vibe vibe{};                 // emotional tone used to select the voice line
};

class DialogueScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<DialogueScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<DialogueScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<DialogueScript>(*CEO::Get<Registry>()) = *this; };

    /*!
     * \brief Called once when the script starts. Loads dialogue data from JSON.
     * \param[in] registry - The ECS registry.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Follows the player position, displays active
     *        dialogue text and hides it once the display duration has elapsed.
     * \param[in] registry   - The ECS registry.
     * \param[in] dt         - Delta time in seconds.
     * \param[in] firstframe - Whether this is the first frame of the update.
     */
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    /*!
     * \brief Called every fixed timestep. Currently unused.
     * \param[in] registry   - The ECS registry.
     * \param[in] dt         - Fixed delta time in seconds.
     * \param[in] firstframe - Whether this is the first frame of the fixed update.
     */
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief Reads DialogueData.json and populates the dialogueLines list.
    * \return True if loading succeeded, false otherwise.
    */
    bool Init();

    /*!
     * \brief Triggers a dialogue line for the given event. For ITEM events,
     *        pass the item name to find the correct line.
     * \param[in] eventType - String name of the event to trigger.
     * \param[in] itemName  - Item name for ITEM events (empty by default).
     */
    void Trigger(std::string eventType, std::string itemName = "");

    /*!
    * \brief Converts an event type string to its corresponding EventType enum value.
    * \param[in] eventType - String representation of the event type.
    * \return Corresponding EventType enum value, defaults to IDLE if unknown.
    */
    EventType GetEventType(std::string eventType);

    /*!
     * \brief Returns a random dialogue line for the given event type,
     *        avoiding repeating the previous dialogue line.
     * \param[in] currentEvent - The event type to find a dialogue line for.
     * \return A randomly selected Dialogue matching the event.
     */
    Dialogue GetDialogue(EventType currentEvent);

    /*!
     * \brief Returns the dialogue line associated with the given item name.
     * \param[in] itemName - The item name to look up.
     * \return The matching Dialogue entry.
     */
    Dialogue GetDialogue(std::string itemName);

    /*!
     * \brief Plays a random voice line audio file matching the given vibe.
     * \param[in] currentVibe - The emotional tone to select a voice line for.
     */
    void PlayVoiceLine(Vibe currentVibe);
   
    GameObject player;           // the player GameObject used to position the dialogue text
    Dialogue prevDialogue;       // the last dialogue shown, used to avoid repeating lines
    bool showingText{};          // whether a dialogue line is currently being displayed
    float timer = 0;             // tracks how long the current dialogue has been shown
    float textDuration = 2.5f;   // how long in seconds a dialogue line stays visible
    
    std::string dialogueFilePath = "GameData\\DialogueData.json";   // path to the dialogue JSON data file
    std::string dialogueVoicePath = "SFX\\Player\\Dialogue\\";      // base path for voice line audio files
    std::list<Dialogue> dialogueLines;                              // all dialogue lines loaded from JSON

    bool initialized = false;   // whether Init() has completed successfully

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(DialogueScript),
    field(player)
)