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
    SAY_QUELDELAR_1  = 1,  // Дамы и господа, я представляю $N, носителем Кель'Делара.
    SAY_QUELDELAR_2  = 2,  // Неужели это Кель'Делар?
    SAY_QUELDELAR_3  = 3,  // Мы увидим!
    SAY_QUELDELAR_4  = 4,  // Лор'темар Смотри! Определенно Кель'Делар.
    SAY_QUELDELAR_5  = 5,  // Так оно и будет. У вас есть моя благодарность $N, Кель'Делар покажет законного владельца.
    SAY_QUELDELAR_6  = 6,  // Что? Это предательство?
    SAY_QUELDELAR_7  = 7,  // Бросай оружие и сдавайся, предатель.
    SAY_QUELDELAR_8  = 8,  // Это не моя вина, Роммат. Там нет предательство.
    SAY_QUELDELAR_9  = 9,  // Убери своих людей. Глупость самого Лор'темара, причина его повреждения. Кель'Делар не выбрали, он выбирает своего хозяина.
    SAY_QUELDELAR_10 = 10,  // Охрана, вернитесь на свои места.
    SAY_QUELDELAR_11 = 11,  // Вы будете иметь то, что вы ищете , $N. Возьмите меч и ступайте. Будьте осторожны, что вы говорите в этом священном месте.
    SAY_QUELDELAR_12 = 12  // Возьмите меч и идите через портал в Даларан, $N. Вы сделали то, что многие квел'дореи мечтали в течение многих лет. Мы, наконец восстановили Кель'Делар.
};

enum QuelDelarEvents
{
    EVENT_QUEST_STEP_1  = 1,
    EVENT_QUEST_STEP_2  = 2,
    EVENT_QUEST_STEP_3  = 3,
    EVENT_QUEST_STEP_4  = 4,
    EVENT_QUEST_STEP_5  = 5,
    EVENT_QUEST_STEP_6  = 6,
    EVENT_QUEST_STEP_7  = 7,
    EVENT_QUEST_STEP_8  = 8,
    EVENT_QUEST_STEP_9  = 9,
    EVENT_QUEST_STEP_10 = 10,
    EVENT_QUEST_STEP_11 = 11,
    EVENT_QUEST_STEP_12 = 12,
    EVENT_QUEST_STEP_13 = 13,
    EVENT_QUEST_STEP_14 = 14,
    EVENT_QUEST_STEP_15 = 15,
    EVENT_QUEST_STEP_16 = 16
};

enum QuelDelarActions
{
    ACTION_START_EVENT  = 1
};

enum QuelDelarCreatures
{
    NPC_ROMMATH         = 37763,
    NPC_THERON          = 37764,
    NPC_AURIC           = 37765,
    NPC_QUEL_GUARD      = 37781,
    NPC_CASTER_BUNNY    = 37746
};

enum QuelDelarGameobjects
{
    GO_QUEL_DANAR       = 201794
};
enum QuelDelarMisc
{
    ITEM_TAINTED_QUELDANAR_1 = 49879,
    ITEM_TAINTED_QUELDANAR_2 = 49889,
    SPELL_WRATH_QUEL_DANAR   = 70493,
    SPELL_ICY_PRISON         = 70540,
	
	// Quest
	QUEST_PURIFICATION_QUELDELAR_A = 24553,
	QUEST_PURIFICATION_QUELDELAR_H = 24564
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

            if (player->HasItemCount(ITEM_TAINTED_QUELDANAR_1, 1) || player->HasItemCount(ITEM_TAINTED_QUELDANAR_2, 1));
			   ((player->GetQuestStatus(QUEST_PURIFICATION_QUELDELAR_H) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(QUEST_PURIFICATION_QUELDELAR_A) == QUEST_STATUS_INCOMPLETE));
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
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                events.Reset();
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_START_EVENT:
                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        events.ScheduleEvent(EVENT_QUEST_STEP_1, 0);
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_QUEST_STEP_1:
                        if (Creature* rommath = me->FindNearestCreature(NPC_ROMMATH, 100.0f, true))
                            uiRommath = rommath->GetGUID();

                        if (Creature* theron = me->FindNearestCreature(NPC_THERON, 100.0f, true))
                           uiTheron = theron->GetGUID();

                        if (Creature* auric = me->FindNearestCreature(NPC_AURIC, 100.0f, true))
                            uiAuric = auric->GetGUID();

                        if (GameObject* quelDelar = me->SummonGameObject(GO_QUEL_DANAR, 1683.99f, 620.231f, 29.3599f, 0.410932f, 0, 0, 0, 0, 0))
                        {
                            uiQuelDelar = quelDelar->GetGUID();
                            quelDelar->SetFlag(GAMEOBJECT_FLAGS, 5);
                        }

                        if (Player* player = ObjectAccessor::GetPlayer(*me, uiPlayer))
                        {
                            player->DestroyItemCount(ITEM_TAINTED_QUELDANAR_1, 1, true);
                            player->DestroyItemCount(ITEM_TAINTED_QUELDANAR_2, 1, true);
                        }
                        events.ScheduleEvent(EVENT_QUEST_STEP_2, 2 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_2:
                        if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 100.0f, true))
                            guard->AI()->Talk(SAY_QUELDELAR_2);
                        events.ScheduleEvent(EVENT_QUEST_STEP_3, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_3:
                        if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                            theron->AI()->Talk(SAY_QUELDELAR_3);
                        events.ScheduleEvent(EVENT_QUEST_STEP_4, 4 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_4:
                        if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                            rommath->GetMotionMaster()->MovePoint(1, 1675.8f, 617.19f, 28.0504f);
                        if (Creature*auric = ObjectAccessor::GetCreature(*me, uiAuric))
                            auric->GetMotionMaster()->MovePoint(1, 1681.77f, 612.084f, 28.4409f);
                        events.ScheduleEvent(EVENT_QUEST_STEP_5, 6 * IN_MILLISECONDS);
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
                        events.ScheduleEvent(EVENT_QUEST_STEP_6, 10 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_6:
                        if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                        {
                            theron->AI()->Talk(SAY_QUELDELAR_5);
                            theron->GetMotionMaster()->MovePoint(1, 1682.3f, 618.459f, 27.9581f);
                        }
                        events.ScheduleEvent(EVENT_QUEST_STEP_7, 4 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_7:
                        if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                            theron->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                        events.ScheduleEvent(EVENT_QUEST_STEP_8, 0.8 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_8:
                        if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                            theron->CastSpell(theron, SPELL_WRATH_QUEL_DANAR, true);
                        events.ScheduleEvent(EVENT_QUEST_STEP_9, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_9:
                        if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                        {
                            if (Player* player = ObjectAccessor::GetPlayer(*me, uiPlayer))
							// if (Player* player = me->FindNearestCreature(player, 200.0f, true))
                            rommath->AddAura(SPELL_ICY_PRISON, player);
                            rommath->AI()->Talk(SAY_QUELDELAR_6);
                        }
                        if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 200.0f))
                        {
                            guard->GetMotionMaster()->MovePoint(0, 1681.1f, 614.955f, 28.4983f);
                            guard->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                        }
                        events.ScheduleEvent(EVENT_QUEST_STEP_10, 3 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_10:
                        if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 200.0f))
                            guard->AI()->Talk(SAY_QUELDELAR_7);
                        events.ScheduleEvent(EVENT_QUEST_STEP_11, 2 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_11:
                        if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                            auric->AI()->Talk(SAY_QUELDELAR_8);
                        events.ScheduleEvent(EVENT_QUEST_STEP_12, 6 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_12:
                        if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                            auric->AI()->Talk(SAY_QUELDELAR_9);
                        events.ScheduleEvent(EVENT_QUEST_STEP_13, 5 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_13:
                        if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                            rommath->AI()->Talk(SAY_QUELDELAR_10);
                        events.ScheduleEvent(EVENT_QUEST_STEP_14, 2 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_14:
                        if (Creature* guard = me->FindNearestCreature(NPC_QUEL_GUARD, 200.0f))
                        {
                            guard->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                            guard->GetMotionMaster()->MovePoint(0, guard->GetHomePosition());
                        }
                        if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                        {
                            rommath->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                            rommath->AI()->Talk(SAY_QUELDELAR_11);
                        }
                        events.ScheduleEvent(EVENT_QUEST_STEP_15, 7 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_15:
                        if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                        {
                            auric->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                            auric->AI()->Talk(SAY_QUELDELAR_12);
                            if (GameObject* quelDelar = me->FindNearestGameObject(GO_QUEL_DANAR, 100.0f))
                                quelDelar->RemoveFlag(GAMEOBJECT_FLAGS, 5);
                        }
                        events.ScheduleEvent(EVENT_QUEST_STEP_16, 2 * IN_MILLISECONDS);
                        break;
                    case EVENT_QUEST_STEP_16:
                        if (Creature* auric = ObjectAccessor::GetCreature(*me, uiAuric))
                            auric->GetMotionMaster()->MovePoint(0, auric->GetHomePosition());
                        if (Creature* rommath = ObjectAccessor::GetCreature(*me, uiRommath))
                            rommath->GetMotionMaster()->MovePoint(0, rommath->GetHomePosition());
                        if (Creature* theron = ObjectAccessor::GetCreature(*me, uiTheron))
                            theron->DespawnOrUnsummon(5 * IN_MILLISECONDS);
                        break;
                    default:
                        break;
            }
        }       
        private:
            EventMap events;
            ObjectGuid uiRommath;
            ObjectGuid uiTheron;
            ObjectGuid uiAuric;
            ObjectGuid uiQuelDelar;
			ObjectGuid uiPlayer;
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
		
        bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
        {
            InstanceScript *instance = player->GetInstanceScript();

            if (instance && player->FindNearestCreature(NPC_CASTER_BUNNY, 200.0f, true))
            {
                Creature *Introducer = NULL;
                Introducer = ObjectAccessor::GetCreature((*player), instance->GetGuidData(DATA_QUELDELAR_INTRODUCER));
                Introducer->AI()->SetGUID(player->GetGUID());
                Introducer->AI()->DoAction(ACTION_START_EVENT);
                return true;
            }
            else
                return false;
        }
};

class spell_cleanse_queldelar : public SpellScriptLoader
{
	public:
		spell_cleanse_queldelar() : SpellScriptLoader("spell_cleanse_queldelar") { }

		class spell_cleanse_queldelar_SpellScript : public SpellScript
		{
			PrepareSpellScript(spell_cleanse_queldelar_SpellScript);

			void HandleStartScript(SpellEffIndex effIndex)
			{
				if (Creature* theron = GetCaster()->FindNearestCreature(NPC_THERON, 100.0f, true))
					if (theron->AI())
						theron->AI()->SetGUID(GetCaster()->GetGUID());
				
			}

			void Register()
			{
				OnEffectHit += SpellEffectFn(spell_cleanse_queldelar_SpellScript::HandleStartScript, EFFECT_0, SPELL_EFFECT_SEND_EVENT);
			}
		};

		SpellScript* GetSpellScript() const
		{
			return new spell_cleanse_queldelar_SpellScript();
		}
};

void AddSC_sunwell_plateau()
{
    new npc_queldelar_sp();
    new go_dalaran_portal();
    new item_tainted_queldelar();
	new spell_cleanse_queldelar();
}