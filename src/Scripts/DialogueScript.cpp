/**___________________________________________________________________________/
@file       DialogueScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       25/02/2026   (DD/MM/YYYY)
@brief      Script that handles the player character's reactive dialogue system.
            Loads dialogue lines from a JSON file, selects a random line based
            on the current game event, displays it as text above the player,
            and plays the corresponding voice line based on the dialogue's vibe.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "DialogueScript.h"
#include "PlayerControllerScript.h"

namespace {
#ifdef PLATFORM_WINDOWS
	const std::string basePath("Assets\\");
#endif
#ifdef PLATFORM_ANDROID
	const std::string basePath("");
#endif
}

void DialogueScript::OnStart(Registry &)
{
    if (Init()) initialized = true; // load dialogue data and mark as initialized if successful
};


void DialogueScript::OnUpdate(Registry & registry, float dt, bool)
{
    if (!initialized) return;

    // follow the player position, offset upward to appear above the character
    TransformComponent* tc = GetComponent<TransformComponent>(registry);
    TransformComponent* ptc = player.GetComponent<TransformComponent>();
    Vec2 newPos = { ptc->translate.x, ptc->translate.y + 200.0f };
    tc->translate = newPos;

    if (!showingText)
    {
        // clear text and reset timer when not showing dialogue
        GetComponent<TextRendererComponent>(registry)->text = "";
        timer = 0;
        return;
    }

    timer += dt;

    // hide dialogue once the display duration has elapsed
    if (timer >= textDuration)
    {
        showingText = false;
    }
};

void DialogueScript::OnFixedUpdate(Registry&, float, bool)
{
};
#ifdef PLATFORM_ANDROID
static void AndroidMakesMeMad(std::string& s) {
    std::replace(s.begin(),s.end(),'\\','/');
}
#endif
bool DialogueScript::Init()
{
    rapidjson::Document doc;
    std::string path = basePath + dialogueFilePath;
#ifdef PLATFORM_ANDROID
    AndroidMakesMeMad(path);
#endif
    std::stringstream ifs{ CEO::Instance().GetManager<FileManager>()->ReadFile(path) };
    if (!ifs) {
        LOGE("Error in reading %s dialogueData", path.c_str());
        return false;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    doc.Parse(buffer.str().c_str());
    if (doc.HasParseError()) {
        LOGE("DialogueData json is corrupted or invalid!");
        return false;
    }
    else if (!doc.HasMember("Dialogue") || !doc["Dialogue"].IsArray()) {
        LOGE("DialogueData json does not have Dialogue array!");
        return false;
    }
    const rapidjson::Value& dlg{ doc["Dialogue"] };
    for (rapidjson::SizeType i{}; i < dlg.Size(); ++i)
    {
        const rapidjson::Value& t{ dlg[i] };
        Dialogue dialogue;

        dialogue.dialogueLine = t["dialogueLine"].GetString();
        dialogue.item = t["item"].GetString();

        // parse vibe type string into enum
        std::string vibeType = t["vibe"].GetString();
        if (vibeType == "Hopeful")       dialogue.vibe = Vibe::HOPEFUL;
        else if (vibeType == "Dejected") dialogue.vibe = Vibe::DEJECTED;
        else if (vibeType == "Scared")   dialogue.vibe = Vibe::SCARED;
        else if (vibeType == "Death")    dialogue.vibe = Vibe::DEATH;

        // parse event type string into enum
        std::string eventType = t["eventType"].GetString();
        dialogue.eventType = GetEventType(eventType);

        dialogueLines.push_back(std::move(dialogue));
    }
    return true;
};

void DialogueScript::Trigger(std::string eventType, std::string itemName)
{
    EventType currentEvent = GetEventType(eventType);
    auto registry = CEO::Instance().GetManager<Registry>();
    auto tc = registry->GetComponent<TextRendererComponent>(entity);
    
    Dialogue currentDialogue;

    // item dialogues are looked up by item name, all others by event type
    if (currentEvent == EventType::ITEM)
    {
        currentDialogue = GetDialogue(itemName);
    }
    else
    {
        currentDialogue = GetDialogue(currentEvent);
    }

    // display the dialogue text and play the matching voice line
    tc->text = currentDialogue.dialogueLine;
    PlayVoiceLine(currentDialogue.vibe);
    showingText = true;
    timer = 0.0f;
};

EventType DialogueScript::GetEventType(std::string eventType)
{
    if (eventType == "ENTERDUNGEON")   return EventType::ENTERDUNGEON;
    else if (eventType == "COMBAT_S")  return EventType::COMBAT_S;
    else if (eventType == "COMBAT_M")  return EventType::COMBAT_M;
    else if (eventType == "COMBAT_L")  return EventType::COMBAT_L;
    else if (eventType == "COMBAT_SM") return EventType::COMBAT_SM;
    else if (eventType == "COMBAT_SL") return EventType::COMBAT_SL;
    else if (eventType == "COMBAT_ML") return EventType::COMBAT_ML;
    else if (eventType == "COMBAT_E")  return EventType::COMBAT_E;
    else if (eventType == "PUNISH_B")  return EventType::PUNISH_B;
    else if (eventType == "PUNISH_S")  return EventType::PUNISH_S;
    else if (eventType == "PUNISH_F")  return EventType::PUNISH_F;
    else if (eventType == "TREASURE")  return EventType::TREASURE;
    else if (eventType == "IDLE")      return EventType::IDLE;
    else if (eventType == "EXIT")      return EventType::EXIT;
    else if (eventType == "DEATH")     return EventType::DEATH;
    else if (eventType == "ITEM")      return EventType::ITEM;
	else if (eventType == "WAVE1")     return EventType::WAVE1;
	else if (eventType == "WAVE2")     return EventType::WAVE2;
	else if (eventType == "WAVE3")     return EventType::WAVE3;
	else if (eventType == "WIN")       return EventType::WIN;
    else
    {
        LOGE("Unknown eventType: %s", eventType.c_str());
        return EventType::IDLE; // default fallback
    }
};

Dialogue DialogueScript::GetDialogue(EventType currentEvent)
{
    Dialogue dialogue;
    std::vector<Dialogue> currentEventDialogues;

    // collect all dialogue lines matching the requested event type
    for (auto it : dialogueLines)
    {
        if (it.eventType == currentEvent)
        {
            currentEventDialogues.emplace_back(it);
        }
    }

    // keep picking randomly until a different line from the previous one is selected
    do {
        dialogue = currentEventDialogues[std::rand() % currentEventDialogues.size()];
    } while (dialogue.dialogueLine == prevDialogue.dialogueLine);
    return dialogue;
};

Dialogue DialogueScript::GetDialogue(std::string itemName)
{
    Dialogue dialogue;

    // find the dialogue line that matches the given item name
    for (auto it : dialogueLines)
    {
        if (it.item == itemName)
        {
            dialogue = it;
        }
    }

    return dialogue;
};

void DialogueScript::PlayVoiceLine(Vibe currentVibe)
{
    std::string dialoguePath = "";
    float vol = 1.0f;

    // select a random voice line file and volume based on the vibe
    if (currentVibe == Vibe::HOPEFUL)
    {
        std::string rand = std::to_string(std::rand() % 6 + 1);
        dialoguePath = "Hopeful" + rand + ".wav";
        vol = 0.75f;
    }
    else if (currentVibe == Vibe::DEJECTED)
    {
        std::string rand = std::to_string(std::rand() % 5 + 1);
        dialoguePath = "Dejected" + rand + ".wav";
        vol = 0.25f;
    }
    else if (currentVibe == Vibe::SCARED)
    {
        std::string rand = std::to_string(std::rand() % 5 + 1);
        dialoguePath = "Scared" + rand + ".wav";
        vol = 0.5f;
    }
    else if (currentVibe == Vibe::DEATH)
    {
        dialoguePath = "PlayerDeath.wav";   // death has a single dedicated voice line
    }

    std::string path = dialogueVoicePath + dialoguePath;
    CEO::Instance().GetManager<ResourceManager>()->GetAudio(path).Play(vol, 1, 0);
};
