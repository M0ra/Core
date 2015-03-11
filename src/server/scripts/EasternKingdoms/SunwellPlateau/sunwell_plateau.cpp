/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Sunwell_Plateau
SD%Complete: Create Mora
SDComment: Placeholder, Epilogue after Kil'jaeden, Captain Selana Gossips
EndScriptData */

/* ContentData
npc_prophet_velen
npc_captain_selana
EndContentData */

#include "ScriptPCH.h"
#include "sunwell_plateau.h"

/*######
## npc_prophet_velen
######*/

enum ProphetSpeeches
{
    PROPHET_SAY1 = 1,
    PROPHET_SAY2 = 2,
    PROPHET_SAY3 = 3,
    PROPHET_SAY4 = 4,
    PROPHET_SAY5 = 5,
    PROPHET_SAY6 = 6,
    PROPHET_SAY7 = 7,
    PROPHET_SAY8 = 8
};

enum LiadrinnSpeeches
{
    LIADRIN_SAY1 = 1,
    LIADRIN_SAY2 = 2,
    LIADRIN_SAY3 = 3
};

/*######
## npc_captain_selana
######*/

#define CS_GOSSIP1 "Дайте мне оперативную сводку, Капитан."
#define CS_GOSSIP2 "Что пошло не так, как надо?"
#define CS_GOSSIP3 "Почему они останавливались?"
#define CS_GOSSIP4 "Ваше понимание ценится."
#define CS_GOSSIP5 "Я принес Кель'Делар."

enum Yells
{
    SAY_QUELDELAR_1           = 1,
    SAY_QUELDELAR_2           = 2,
    SAY_QUELDELAR_3           = 3,  
    SAY_QUELDELAR_4           = 4,
    SAY_QUELDELAR_5           = 5, 
    SAY_QUELDELAR_6           = 6,
    SAY_QUELDELAR_7           = 7,
    SAY_QUELDELAR_8           = 8,
    SAY_QUELDELAR_9           = 9,
    SAY_QUELDELAR_10          = 10,
    SAY_QUELDELAR_11          = 11,
    SAY_QUELDELAR_12          = 12
};

enum QuelDelarEvents
{
    EVENT_QUEST_STEP_1        = 1,
    EVENT_QUEST_STEP_2        = 2,
    EVENT_QUEST_STEP_3        = 3,
    EVENT_QUEST_STEP_4        = 4,
    EVENT_QUEST_STEP_5        = 5,
    EVENT_QUEST_STEP_6        = 6,
    EVENT_QUEST_STEP_7        = 7,
    EVENT_QUEST_STEP_8        = 8,
    EVENT_QUEST_STEP_9        = 9,
    EVENT_QUEST_STEP_10       = 10,
    EVENT_QUEST_STEP_11       = 11,
    EVENT_QUEST_STEP_12       = 12,
    EVENT_QUEST_STEP_13       = 13,
    EVENT_QUEST_STEP_14       = 14,
    EVENT_QUEST_STEP_15       = 15,
    EVENT_QUEST_STEP_16       = 16
};

enum QuelDelarActions
{
    ACTION_START_EVENT        = 1
};

enum QuelDelarCreatures
{
    NPC_ROMMATH               = 37763,
    NPC_THERON                = 37764,
    NPC_AURIC                 = 37765,
    NPC_QUEL_GUARD            = 37781,
    NPC_CASTER_BUNNY          = 37746
};

enum QuelDelarGameobjects
{
    GO_QUEL_DANAR             = 201794
};
enum QuelDelarMisc
{
    ITEM_TAINTED_QUELDANAR_1  = 49879,
    ITEM_TAINTED_QUELDANAR_2  = 49889,
    SPELL_WRATH_QUEL_DANAR    = 70493,
    SPELL_ICY_PRISON          = 70540
};

/*######
## npc_queldelar_sp
######*/

class npc_queldelar_sp : public CreatureScript
{
public:
    npc_queldelar_sp() : CreatureScript("npc_queldelar_sp") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PrepareGossipMenu(creature, 0);

        if (player->HasItemCount(ITEM_TAINTED_QUELDANAR_1, 1) || player->HasItemCount(ITEM_TAINTED_QUELDANAR_2, 1))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, CS_GOSSIP5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SendPreparedGossip(creature);

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 Action) override
    {
        player->PlayerTalkClass->ClearMenus();
        
        switch (Action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                creature->AI()->SetGUID(player->GetGUID());
                creature->AI()->DoAction(ACTION_START_EVENT);
                break;
            default:
                return false;                                   
        }
        return true;                                          
    }

    struct npc_queldelar_spAI : public ScriptedAI
    {
        npc_queldelar_spAI(Creature* creature) : ScriptedAI(creature) { }	

        void Reset() override
        {
            _events.Reset();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
                case EVENT_QUEST_STEP_1:
                    if (Creature* rommath = me->FindNearestCreature(NPC_ROMMATH, 100.0f, true))
                    {
                        rommath->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        uiRommath = rommath->GetGUID();
                    }

                    if (Creature* theron = me->FindNearestCreature(NPC_THERON, 100.0f, true))
                    {
                        theron->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        uiTheron = theron->GetGUID();
                    }

                    if (Creature* auric = me->FindNearestCreature(NPC_AURIC, 100.0f, true))
                    {
                        auric->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        uiAuric = auric->GetGUID();
                    }
					
                    if (GameObject* quelDelar = me->SummonGameObject(GO_QUEL_DANAR, 1683.99f, 620.231f, 29.3599f, 0.410932f, 0, 0, 0, 0, 0))
                    {
                        uiQuelDelarGUID = quelDelar->GetGUID();
                        quelDelar->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }

                    if (Player* player = ObjectAccessor::GetPlayer(*me, uiPlayerGUID))
                    {
                        player->DestroyItemCount(ITEM_TAINTED_QUELDANAR_1, 1, true);
                        player->DestroyItemCount(ITEM_TAINTED_QUELDANAR_2, 1, true);
                        Talk(SAY_QUELDELAR_1);
                    }
                    _events.ScheduleEvent(EVENT_QUEST_STEP_2, 3000);
                    break;
                case EVENT_QUEST_STEP_2:
                    if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 100.0f, true))
                        guard->AI()->Talk(SAY_QUELDELAR_2);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_3, 2000);
                    break;
                case EVENT_QUEST_STEP_3:
                    if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                        theron->AI()->Talk(SAY_QUELDELAR_3);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_4, 8000);
                    break;
                case EVENT_QUEST_STEP_4:
                    if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                        rommath->GetMotionMaster()->MovePoint(1, 1675.8f, 617.19f, 28.0504f);
                    if (Creature*auric = ObjectAccessor::GetCreature(*me, uiAuric))
                        auric->GetMotionMaster()->MovePoint(1, 1681.77f, 612.084f, 28.4409f);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_5, 10000);
                    break;
                case EVENT_QUEST_STEP_5:
                    if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                    {
                        rommath->SetOrientation(0.3308f);
                        rommath->AI()->Talk(SAY_QUELDELAR_4);
                    }
                    if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                        auric->SetOrientation(1.29057f);
                    if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                        theron->GetMotionMaster()->MovePoint(1, 1677.07f, 613.122f, 28.0504f);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_6, 13000);
                    break;
                case EVENT_QUEST_STEP_6:
                    if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                    {
                        theron->AI()->Talk(SAY_QUELDELAR_5);
                        theron->GetMotionMaster()->MovePoint(1, 1682.3f, 618.459f, 27.9581f);
                    }
                    _events.ScheduleEvent(EVENT_QUEST_STEP_7, 7000);
                    break;
                case EVENT_QUEST_STEP_7:
                    if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                        theron->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_8, 800);
                    break;
                case EVENT_QUEST_STEP_8:
                    if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                        theron->CastSpell(theron, SPELL_WRATH_QUEL_DANAR, true);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_9, 2000);
                    break;
                case EVENT_QUEST_STEP_9:
                    if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                    {
                        if (Player* player = ObjectAccessor::GetPlayer(*me, uiPlayerGUID))
                        // if (Player* player = me->FindNearestCreature(player, 200.0f, true))
                        rommath->AddAura(SPELL_ICY_PRISON, player);
                        rommath->AI()->Talk(SAY_QUELDELAR_6);
                    }

                    if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 200.0f))
                    {
                        guard->GetMotionMaster()->MovePoint(0, 1681.1f, 614.955f, 28.4983f);
                        guard->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                    }
                    _events.ScheduleEvent(EVENT_QUEST_STEP_10, 5000);
                    break;
                case EVENT_QUEST_STEP_10:
                    if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 200.0f))
                        guard->AI()->Talk(SAY_QUELDELAR_7);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_11, 4000);
                    break;
                case EVENT_QUEST_STEP_11:
                    if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                        auric->AI()->Talk(SAY_QUELDELAR_8);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_12, 9000);
                    break;
                case EVENT_QUEST_STEP_12:
                    if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                        auric->AI()->Talk(SAY_QUELDELAR_9);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_13, 8000);
                    break;
                case EVENT_QUEST_STEP_13:
                    if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                        rommath->AI()->Talk(SAY_QUELDELAR_10);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_14, 4000);
                    break;
                case EVENT_QUEST_STEP_14:
                    if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 200.0f))
                    {
                        guard->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                        guard->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        guard->GetMotionMaster()->MovePoint(0, guard->GetHomePosition());
                    }

                    if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                    {
                        rommath->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, uiPlayerGUID))
                            rommath->AI()->Talk(SAY_QUELDELAR_11);
                    }
                    _events.ScheduleEvent(EVENT_QUEST_STEP_15, 11000);
                    break;
                case EVENT_QUEST_STEP_15:
                    if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                    {
                        auric->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, uiPlayerGUID))
                            auric->AI()->Talk(SAY_QUELDELAR_12);
                        if (GameObject* quelDelar = me->FindNearestGameObject(GO_QUEL_DANAR, 100.0f))
                            quelDelar->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    _events.ScheduleEvent(EVENT_QUEST_STEP_16, 4000);
                    break;
                case EVENT_QUEST_STEP_16:
                    if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                        auric->GetMotionMaster()->MovePoint(0, auric->GetHomePosition());
                    if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                        rommath->GetMotionMaster()->MovePoint(0, rommath->GetHomePosition());
                    if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                        theron->DespawnOrUnsummon(5000);
                    break;
                default:
                    break;
            }
        }
		
        void SetGUID(ObjectGuid uiGuid, int32 /*iId*/) override
        {
            uiPlayerGUID = uiGuid;
        }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_START_EVENT:
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    _events.ScheduleEvent(EVENT_QUEST_STEP_1, 0);
                    break;
            }
        }
		
        private:
            EventMap _events;
            ObjectGuid uiRommath;
            ObjectGuid uiTheron;
            ObjectGuid uiAuric;
            ObjectGuid uiQuelDelarGUID;
            ObjectGuid uiPlayerGUID;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_queldelar_spAI(creature);
    }
};

class go_dalaran_portal : public GameObjectScript
{
    public:
        go_dalaran_portal() : GameObjectScript("go_dalaran_portal_sunwell") { }

        bool OnGossipHello(Player* player, GameObject* /*go*/) override
        {
            player->SetPhaseMask(1, true);
            player->TeleportTo(571, 5804.15f, 624.771f, 647.767f, 1.64f);
            return false;
        }
};

class item_tainted_queldelar : public ItemScript
{
    public:
        item_tainted_queldelar() : ItemScript("item_tainted_queldelar") { }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/)
        {
            InstanceScript* instance = creature->GetInstanceScript();

            if (Creature* introducer = player->FindNearestCreature(NPC_CASTER_BUNNY, 200.0f, true))
            {
                introducer = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_QUELDELAR_INTRODUCER));
                introducer->AI()->SetGUID(player->GetGUID());
                introducer->AI()->DoAction(ACTION_START_EVENT);
                return true;
            }                    
            else
            return false;
        }
};

void AddSC_sunwell_plateau()
{
    new npc_queldelar_sp();
    new go_dalaran_portal();
    new item_tainted_queldelar();
}