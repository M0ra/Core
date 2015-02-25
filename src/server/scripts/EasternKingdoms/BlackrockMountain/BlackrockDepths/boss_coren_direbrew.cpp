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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "blackrock_depths.h"
#include "LFGMgr.h"

#define GOSSIP_SELECT_INSULT      "Insult Coren Direbrew's brew."
#define GOSSIP_SELECT_FIGHT       "Fight."
#define GOSSIP_SELECT_APOLOGIZE   "Apologize."

enum Texts
{
    // Coren Direbrew
    SAY_COREN_RANT_1                = 0,
    SAY_COREN_RANT_2                = 1,
    SAY_COREN_RANT_3                = 2,
    SAY_COREN_INTRO_1               = 3,
    SAY_COREN_INTRO_2               = 4,

    // Dark Iron Antagonist
    SAY_DARK_IRON_CHEER             = 0,
    SAY_DARK_IRON_NO                = 1,
    SAY_DARK_IRON_AGGRO             = 2
};

enum Spells
{
    // Coren
    SPELL_DIREBREW_DISARM           = 47310,

    // Brewmaiden
    SPELL_CHUG_MUG                  = 50276,
    SPELL_DARKMAIDEN_BREW           = 47345,
    SPELL_BREWMAIDEN_STUN           = 47340,
    SPELL_BREWMAIDEN_BARREL         = 47442,

    // Helper
    SPELL_MOLE_MACHINE              = 50313 // unused
};

enum EventTypes
{
    EVENT_COREN_RANT_1                  = 1,
    EVENT_COREN_RANT_2                  = 2,
    EVENT_COREN_RANT_3                  = 3,
    EVENT_COREN_RANT_4                  = 4,
    EVENT_COREN_RANT_5                  = 5,
    EVENT_COREN_INTRO_1                 = 6,
    EVENT_COREN_INTRO_2                 = 7,
    EVENT_COREN_DISARM                  = 8
};

enum Actions
{
    ACTION_START_INTRO
};

class boss_coren_direbrew : public CreatureScript
{
public:
    boss_coren_direbrew() : CreatureScript("boss_coren_direbrew") { }
        
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_FIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_APOLOGIZE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                player->SEND_GOSSIP_MENU(15859, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->CLOSE_GOSSIP_MENU();
                if (creature->AI())
                    creature->AI()->DoAction(ACTION_START_INTRO);
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_INSULT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
                player->SEND_GOSSIP_MENU(15858, creature->GetGUID());
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT_INSULT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(15858, creature->GetGUID());

        return true;
    }

    struct boss_coren_direbrewAI : public ScriptedAI
    {
        boss_coren_direbrewAI(Creature* creature) : ScriptedAI(creature)  
        {
            instance = creature->GetInstanceScript();
            _introDone = false;
        }

        InstanceScript* instance;

        void Reset()
        {
            _events.Reset(); // todo, fix timers
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!_introDone && me->IsWithinDistInMap(who, 30.0f))
            {
                _events.ScheduleEvent(EVENT_COREN_RANT_1, 10000);
                _introDone = true;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            _events.ScheduleEvent(EVENT_COREN_DISARM, urand(12000, 20000));
        }

        void DoAction(const int32 actionId)
        {
            if (actionId == ACTION_START_INTRO)
            {
                _events.ScheduleEvent(EVENT_COREN_INTRO_1, 0);
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->SetData(DATA_COREN, DONE);

            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            if (!players.isEmpty())
                for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                    if (Player* player = i->GetSource())
                        if (player->IsAtGroupRewardDistance(me))
                            sLFGMgr->LoadRewards(287, player);
        }
        
        void UpdateAI(uint32 const diff)
        {
            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_COREN_RANT_1:
                        Talk(SAY_COREN_RANT_1);
                        _events.ScheduleEvent(EVENT_COREN_RANT_2, 5000);
                        break;
                    case EVENT_COREN_RANT_3:
                        Talk(SAY_COREN_RANT_2);
                        _events.ScheduleEvent(EVENT_COREN_RANT_4, 5000);
                        break;
                    case EVENT_COREN_RANT_5:
                        Talk(SAY_COREN_RANT_3);
                        break;
                    case EVENT_COREN_INTRO_1:
                        Talk(SAY_COREN_INTRO_1);
                        _events.ScheduleEvent(EVENT_COREN_INTRO_2, 2500);
                        break;
                    case EVENT_COREN_INTRO_2:
                        me->setFaction(FACTION_HOSTILE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        DoZoneInCombat();
                        break;
                    case EVENT_COREN_DISARM:
                        DoCastVictim(SPELL_DIREBREW_DISARM);
                        _events.ScheduleEvent(EVENT_COREN_DISARM, urand(12000, 20000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    private:
        EventMap _events;
        bool _introDone;
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_coren_direbrewAI (creature);
    }
};

class npc_dark_iron_antagonist : public CreatureScript
{
    public:
        npc_dark_iron_antagonist() : CreatureScript("npc_dark_iron_antagonist") { }
        struct npc_dark_iron_antagonistAI : public ScriptedAI
        {
            npc_dark_iron_antagonistAI(Creature* creature) : ScriptedAI(creature)  {  }

            void UpdateAI(uint32 const diff)
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_COREN_RANT_2:
                            Talk(SAY_DARK_IRON_CHEER);
                            _events.ScheduleEvent(EVENT_COREN_RANT_2, 5000);
                            break;
                        case EVENT_COREN_RANT_4:
                            Talk(SAY_DARK_IRON_NO);
                            _events.ScheduleEvent(EVENT_COREN_RANT_4, 5000);
                            break;
                        case EVENT_COREN_INTRO_1:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                            break;
                        case EVENT_COREN_INTRO_2:
                            Talk(SAY_DARK_IRON_AGGRO);
                            me->setFaction(FACTION_HOSTILE);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            EventMap _events;
        };


        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dark_iron_antagonistAI (creature);
        }
};

void AddSC_boss_coren_direbrew()
{
    new boss_coren_direbrew();
    new npc_dark_iron_antagonist();
}
