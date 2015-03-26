/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Netherstorm
SD%Complete: 80
SDComment: Quest support: 10337, 10438, 10652 (special flight paths),  10198, 10191
SDCategory: Netherstorm
EndScriptData */

/* ContentData
npc_commander_dawnforge
npc_bessy
npc_maxx_a_million
go_captain_tyralius_prison
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Player.h"

/*######
## npc_commander_dawnforge
######*/

// The Speech of Dawnforge, Ardonis & Pathaleon
enum CommanderDawnforgeData
{
    SAY_COMMANDER_DAWNFORGE_1       = 0,
    SAY_COMMANDER_DAWNFORGE_2       = 1,
    SAY_COMMANDER_DAWNFORGE_3       = 2,
    SAY_COMMANDER_DAWNFORGE_4       = 3,
    SAY_COMMANDER_DAWNFORGE_5       = 4,

    SAY_ARCANIST_ARDONIS_1          = 0,
    SAY_ARCANIST_ARDONIS_2          = 1,

    SAY_PATHALEON_CULATOR_IMAGE_1   = 0,
    SAY_PATHALEON_CULATOR_IMAGE_2   = 1,
    SAY_PATHALEON_CULATOR_IMAGE_2_1 = 2,
    SAY_PATHALEON_CULATOR_IMAGE_2_2 = 3,

    QUEST_INFO_GATHERING            = 10198,
    SPELL_SUNFURY_DISGUISE          = 34603,
};

// Entries of Arcanist Ardonis, Commander Dawnforge, Pathaleon the Curators Image
const uint32 CreatureEntry[3] =
{
    19830,                                                // Ardonis
    19831,                                                // Dawnforge
    21504                                                 // Pathaleon
};

class npc_commander_dawnforge : public CreatureScript
{
public:
    npc_commander_dawnforge() : CreatureScript("npc_commander_dawnforge") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_commander_dawnforgeAI(creature);
    }

    struct npc_commander_dawnforgeAI : public ScriptedAI
    {
        npc_commander_dawnforgeAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            PlayerGUID.Clear();
            ardonisGUID.Clear();
            pathaleonGUID.Clear();

            Phase = 1;
            PhaseSubphase = 0;
            Phase_Timer = 4000;
            isEvent = false;
        }

        ObjectGuid PlayerGUID;
        ObjectGuid ardonisGUID;
        ObjectGuid pathaleonGUID;

        uint32 Phase;
        uint32 PhaseSubphase;
        uint32 Phase_Timer;
        bool isEvent;

        void Reset() override
        {
            Initialize();
        }

        void EnterCombat(Unit* /*who*/) override { }

        void JustSummoned(Creature* summoned) override
        {
            pathaleonGUID = summoned->GetGUID();
        }

        // Emote Ardonis and Pathaleon
        void Turn_to_Pathaleons_Image()
        {
            Creature* ardonis = ObjectAccessor::GetCreature(*me, ardonisGUID);
            Creature* pathaleon = ObjectAccessor::GetCreature(*me, pathaleonGUID);

            if (!ardonis || !pathaleon)
                return;

            // Turn Dawnforge
            me->SetFacingToObject(pathaleon);

            // Turn Ardonis
            ardonis->SetFacingToObject(pathaleon);

            //Set them to kneel
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            ardonis->SetStandState(UNIT_STAND_STATE_KNEEL);
        }

        //Set them back to each other
        void Turn_to_eachother()
        {
            if (Unit* ardonis = ObjectAccessor::GetUnit(*me, ardonisGUID))
            {
                // Turn Dawnforge
                me->SetFacingToObject(ardonis);

                // Turn Ardonis
                ardonis->SetFacingToObject(me);

                //Set state
                me->SetStandState(UNIT_STAND_STATE_STAND);
                ardonis->SetStandState(UNIT_STAND_STATE_STAND);
            }
        }

        bool CanStartEvent(Player* player)
        {
            if (!isEvent)
            {
                Creature* ardonis = me->FindNearestCreature(CreatureEntry[0], 10.0f);
                if (!ardonis)
                    return false;

                ardonisGUID = ardonis->GetGUID();
                PlayerGUID = player->GetGUID();

                isEvent = true;

                Turn_to_eachother();
                return true;
            }

            TC_LOG_DEBUG("scripts", "npc_commander_dawnforge event already in progress, need to wait.");
            return false;
        }

        void UpdateAI(uint32 diff) override
        {
            //Is event even running?
            if (!isEvent)
                return;

            //Phase timing
            if (Phase_Timer >= diff)
            {
                Phase_Timer -= diff;
                return;
            }

            Creature* ardonis = ObjectAccessor::GetCreature(*me, ardonisGUID);
            Creature* pathaleon = ObjectAccessor::GetCreature(*me, pathaleonGUID);
            Player* player = ObjectAccessor::GetPlayer(*me, PlayerGUID);

            if (!ardonis || !player)
            {
                Reset();
                return;
            }

            if (Phase > 4 && !pathaleon)
            {
                Reset();
                return;
            }

            //Phase 1 Dawnforge say
            switch (Phase)
            {
            case 1:
                Talk(SAY_COMMANDER_DAWNFORGE_1);
                ++Phase;
                Phase_Timer = 16000;
                break;
                //Phase 2 Ardonis say
            case 2:
                ardonis->AI()->Talk(SAY_ARCANIST_ARDONIS_1);
                ++Phase;
                Phase_Timer = 16000;
                break;
                //Phase 3 Dawnforge say
            case 3:
                Talk(SAY_COMMANDER_DAWNFORGE_2);
                ++Phase;
                Phase_Timer = 16000;
                break;
                //Phase 4 Pathaleon spawns up to phase 9
            case 4:
                //spawn pathaleon's image
                me->SummonCreature(CreatureEntry[2], 2325.851563f, 2799.534668f, 133.084229f, 6.038996f, TEMPSUMMON_TIMED_DESPAWN, 90000);
                ++Phase;
                Phase_Timer = 500;
                break;
                //Phase 5 Pathaleon say
            case 5:
                pathaleon->AI()->Talk(SAY_PATHALEON_CULATOR_IMAGE_1);
                ++Phase;
                Phase_Timer = 6000;
                break;
                //Phase 6
            case 6:
                switch (PhaseSubphase)
                {
                    //Subphase 1: Turn Dawnforge and Ardonis
                case 0:
                    Turn_to_Pathaleons_Image();
                    ++PhaseSubphase;
                    Phase_Timer = 8000;
                    break;
                    //Subphase 2 Dawnforge say
                case 1:
                    Talk(SAY_COMMANDER_DAWNFORGE_3);
                    PhaseSubphase = 0;
                    ++Phase;
                    Phase_Timer = 8000;
                    break;
                }
                break;
                //Phase 7 Pathaleons say 3 Sentence, every sentence need a subphase
            case 7:
                switch (PhaseSubphase)
                {
                    //Subphase 1
                case 0:
                    pathaleon->AI()->Talk(SAY_PATHALEON_CULATOR_IMAGE_2);
                    ++PhaseSubphase;
                    Phase_Timer = 12000;
                    break;
                    //Subphase 2
                case 1:
                    pathaleon->AI()->Talk(SAY_PATHALEON_CULATOR_IMAGE_2_1);
                    ++PhaseSubphase;
                    Phase_Timer = 16000;
                    break;
                    //Subphase 3
                case 2:
                    pathaleon->AI()->Talk(SAY_PATHALEON_CULATOR_IMAGE_2_2);
                    PhaseSubphase = 0;
                    ++Phase;
                    Phase_Timer = 10000;
                    break;
                }
                break;
                //Phase 8 Dawnforge & Ardonis say
            case 8:
                Talk(SAY_COMMANDER_DAWNFORGE_4);
                ardonis->AI()->Talk(SAY_ARCANIST_ARDONIS_2);
                ++Phase;
                Phase_Timer = 4000;
                break;
                //Phase 9 Pathaleons Despawn, Reset Dawnforge & Ardonis angle
            case 9:
                Turn_to_eachother();
                //hide pathaleon, unit will despawn shortly
                pathaleon->SetVisible(false);
                PhaseSubphase = 0;
                ++Phase;
                Phase_Timer = 3000;
                break;
                //Phase 10 Dawnforge say
            case 10:
                Talk(SAY_COMMANDER_DAWNFORGE_5);
                player->AreaExploredOrEventHappens(QUEST_INFO_GATHERING);
                Reset();
                break;
            }
         }
    };
};

class at_commander_dawnforge : public AreaTriggerScript
{
public:
    at_commander_dawnforge() : AreaTriggerScript("at_commander_dawnforge") { }

    bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/) override
    {
        //if player lost aura or not have at all, we should not try start event.
        if (!player->HasAura(SPELL_SUNFURY_DISGUISE))
            return false;

        if (player->IsAlive() && player->GetQuestStatus(QUEST_INFO_GATHERING) == QUEST_STATUS_INCOMPLETE)
        {
            Creature* Dawnforge = player->FindNearestCreature(CreatureEntry[1], 30.0f);
            if (!Dawnforge)
                return false;

            if (ENSURE_AI(npc_commander_dawnforge::npc_commander_dawnforgeAI, Dawnforge->AI())->CanStartEvent(player))
                return true;
        }
        return false;
    }
};

/*######
## npc_professor_dabiri
######*/
enum ProfessorDabiriData
{
    SPELL_PHASE_DISTRUPTOR  = 35780,

  //WHISPER_DABIRI          = 0, not existing in database

    QUEST_DIMENSIUS         = 10439,
    QUEST_ON_NETHERY_WINGS  = 10438,
};

#define GOSSIP_ITEM "I need a new phase distruptor, Professor"

class npc_professor_dabiri : public CreatureScript
{
public:
    npc_professor_dabiri() : CreatureScript("npc_professor_dabiri") { }

    //OnQuestAccept:
    //if (quest->GetQuestId() == QUEST_DIMENSIUS)
        //creature->AI()->Talk(WHISPER_DABIRI, player);

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            creature->CastSpell(player, SPELL_PHASE_DISTRUPTOR, false);
            player->CLOSE_GOSSIP_MENU();
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_ON_NETHERY_WINGS) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(29778))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};

/*######
## npc_phase_hunter
######*/

enum PhaseHunterData
{
    QUEST_RECHARGING_THE_BATTERIES  = 10190,

    NPC_PHASE_HUNTER_ENTRY          = 18879,
    NPC_DRAINED_PHASE_HUNTER_ENTRY  = 19595,

    EMOTE_WEAK                      = 0,

    // Spells
    SPELL_RECHARGING_BATTERY        = 34219,
    SPELL_PHASE_SLIP                = 36574,
    SPELL_MANA_BURN                 = 13321,
    SPELL_MATERIALIZE               = 34804,
    SPELL_DE_MATERIALIZE            = 34814,
};

class npc_phase_hunter : public CreatureScript
{
public:
    npc_phase_hunter() : CreatureScript("npc_phase_hunter") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_phase_hunterAI(creature);
    }

    struct npc_phase_hunterAI : public ScriptedAI
    {
        npc_phase_hunterAI(Creature* creature) : ScriptedAI(creature)
        {
            Weak = false;
            Materialize = false;
            Drained = false;
            WeakPercent = 25;
            ManaBurnTimer = 5000;
        }

        bool Weak;
        bool Materialize;
        bool Drained;
        uint8 WeakPercent;

        ObjectGuid PlayerGUID;

        uint32 ManaBurnTimer;

        void Reset() override
        {
            Weak = false;
            Materialize = false;
            Drained = false;
            WeakPercent = 25 + (rand32() % 16); // 25-40

            PlayerGUID.Clear();

            ManaBurnTimer = 5000 + (rand32() % 3 * 1000); // 5-8 sec cd

            if (me->GetEntry() == NPC_DRAINED_PHASE_HUNTER_ENTRY)
                me->UpdateEntry(NPC_PHASE_HUNTER_ENTRY);
        }

        void EnterCombat(Unit* who) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                PlayerGUID = who->GetGUID();
        }

        //void SpellHit(Unit* /*caster*/, const SpellInfo* /*spell*/) override
        //{
        //    DoCast(me, SPELL_DE_MATERIALIZE);
        //}

        void UpdateAI(uint32 diff) override
        {
            if (!Materialize)
            {
                DoCast(me, SPELL_MATERIALIZE);
                Materialize = true;
            }

            if (me->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) || me->HasUnitState(UNIT_STATE_ROOT)) // if the mob is rooted/slowed by spells eg.: Entangling Roots, Frost Nova, Hamstring, Crippling Poison, etc. => remove it
                DoCast(me, SPELL_PHASE_SLIP);

            if (!UpdateVictim())
                return;

            // some code to cast spell Mana Burn on random target which has mana
            if (ManaBurnTimer <= diff)
            {
                std::list<HostileReference*> AggroList = me->getThreatManager().getThreatList();
                std::list<Unit*> UnitsWithMana;

                for (std::list<HostileReference*>::const_iterator itr = AggroList.begin(); itr != AggroList.end(); ++itr)
                {
                    if (Unit* unit = ObjectAccessor::GetUnit(*me, (*itr)->getUnitGuid()))
                    {
                        if (unit->GetCreateMana() > 0)
                            UnitsWithMana.push_back(unit);
                    }
                }
                if (!UnitsWithMana.empty())
                {
                    DoCast(Trinity::Containers::SelectRandomContainerElement(UnitsWithMana), SPELL_MANA_BURN);
                    ManaBurnTimer = 8000 + (rand32() % 10 * 1000); // 8-18 sec cd
                }
                else
                    ManaBurnTimer = 3500;
            } else ManaBurnTimer -= diff;

            if (Player* player = ObjectAccessor::GetPlayer(*me, PlayerGUID)) // start: support for quest 10190
            {
                if (!Weak && HealthBelowPct(WeakPercent)
                    && player->GetQuestStatus(QUEST_RECHARGING_THE_BATTERIES) == QUEST_STATUS_INCOMPLETE)
                {
                    Talk(EMOTE_WEAK);
                    Weak = true;
                }
                if (Weak && !Drained && me->HasAura(SPELL_RECHARGING_BATTERY))
                {
                    Drained = true;
                    int32 uHpPct = int32(me->GetHealthPct());

                    me->UpdateEntry(NPC_DRAINED_PHASE_HUNTER_ENTRY);

                    me->SetHealth(me->CountPctFromMaxHealth(uHpPct));
                    me->LowerPlayerDamageReq(me->GetMaxHealth() - me->GetHealth());
                    me->SetInCombatWith(player);
                }
            } // end: support for quest 10190

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_bessy
######*/
enum BessyData
{
    Q_ALMABTRIEB    = 10337,
    N_THADELL       = 20464,
    SPAWN_FIRST     = 20512,
    SPAWN_SECOND    = 19881,
    SAY_THADELL_1   = 0,
    SAY_THADELL_2   = 1,
};

class npc_bessy : public CreatureScript
{
public:
    npc_bessy() : CreatureScript("npc_bessy") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == Q_ALMABTRIEB)
        {
            creature->setFaction(113);
            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            ENSURE_AI(npc_escortAI, (creature->AI()))->Start(true, false, player->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bessyAI(creature);
    }

    struct npc_bessyAI : public npc_escortAI
    {
        npc_bessyAI(Creature* creature) : npc_escortAI(creature) { }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(Q_ALMABTRIEB);
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 3: //first spawn
                    me->SummonCreature(SPAWN_FIRST, 2449.67f, 2183.11f, 96.85f, 6.20f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(SPAWN_FIRST, 2449.53f, 2184.43f, 96.36f, 6.27f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(SPAWN_FIRST, 2449.85f, 2186.34f, 97.57f, 6.08f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    break;
                case 7:
                    me->SummonCreature(SPAWN_SECOND, 2309.64f, 2186.24f, 92.25f, 6.06f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(SPAWN_SECOND, 2309.25f, 2183.46f, 91.75f, 6.22f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    break;
                case 12:
                    player->GroupEventHappens(Q_ALMABTRIEB, me);
                    if (me->FindNearestCreature(N_THADELL, 30))
                        Talk(SAY_THADELL_1);
                    break;
                case 13:
                    if (me->FindNearestCreature(N_THADELL, 30))
                        Talk(SAY_THADELL_2, player);
                    break;
            }
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(me);
        }

        void Reset() override
        {
            me->RestoreFaction();
        }
    };
};

/*######
## npc_maxx_a_million
######*/

enum MaxxAMillion
{
    QUEST_MARK_V_IS_ALIVE   = 10191,
    GO_DRAENEI_MACHINE      = 183771
};

class npc_maxx_a_million_escort : public CreatureScript
{
public:
    npc_maxx_a_million_escort() : CreatureScript("npc_maxx_a_million_escort") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_maxx_a_million_escortAI(creature);
    }

    struct npc_maxx_a_million_escortAI : public npc_escortAI
    {
        npc_maxx_a_million_escortAI(Creature* creature) : npc_escortAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            bTake = false;
            uiTakeTimer = 3000;
        }

        bool bTake;
        uint32 uiTakeTimer;

        void Reset() override
        {
            Initialize();
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 7:
                case 17:
                case 29:
                    //Find Object and "work"
                    if (GetClosestGameObjectWithEntry(me, GO_DRAENEI_MACHINE, INTERACTION_DISTANCE))
                    {
                        // take the GO -> animation
                        me->HandleEmoteCommand(EMOTE_STATE_LOOT);
                        SetEscortPaused(true);
                        bTake=true;
                    }
                    break;
                case 36: //return and quest_complete
                    player->CompleteQuest(QUEST_MARK_V_IS_ALIVE);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_MARK_V_IS_ALIVE);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (bTake)
            {
                if (uiTakeTimer < uiDiff)
                {
                    me->HandleEmoteCommand(EMOTE_STATE_NONE);
                    if (GameObject* go = GetClosestGameObjectWithEntry(me, GO_DRAENEI_MACHINE, INTERACTION_DISTANCE))
                    {
                        SetEscortPaused(false);
                        bTake=false;
                        uiTakeTimer = 3000;
                        go->Delete();
                    }
                }
                else
                    uiTakeTimer -= uiDiff;
            }
            DoMeleeAttackIfReady();
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest) override
    {
        if (quest->GetQuestId() == QUEST_MARK_V_IS_ALIVE)
        {
            if (npc_maxx_a_million_escortAI* pEscortAI = CAST_AI(npc_maxx_a_million_escort::npc_maxx_a_million_escortAI, creature->AI()))
            {
                creature->setFaction(113);
                pEscortAI->Start(false, false, player->GetGUID());
            }
        }
        return true;
    }
};

/*######
## go_captain_tyralius_prison
######*/

enum CaptainTyralius
{
    NPC_CAPTAIN_TYRALIUS    = 20787,
    SAY_FREE                = 0
};

class go_captain_tyralius_prison : public GameObjectScript
{
    public:
        go_captain_tyralius_prison() : GameObjectScript("go_captain_tyralius_prison") { }

        bool OnGossipHello(Player* player, GameObject* go) override
        {
            go->UseDoorOrButton();
            if (Creature* tyralius = go->FindNearestCreature(NPC_CAPTAIN_TYRALIUS, 1.0f))
            {
                player->KilledMonsterCredit(NPC_CAPTAIN_TYRALIUS);
                tyralius->AI()->Talk(SAY_FREE);
                tyralius->DespawnOrUnsummon(8000);
            }
            return true;
        }
};

enum TextDimensius
{
    SAY_EVENT_1         = 1,
    SAY_EVENT_2         = 2,
    SAY_EVENT_3         = 3,
    SAY_EVENT_4         = 4
};

enum NpcDimensius
{
    NPC_DEFENS_DIMES    = 20984,
    NPC_REG_DIMES       = 21783,
    NPC_AVENG_DIMES     = 21805,
    NPC_DEV_DIMES       = 19554,
    NPC_CAP_SAEED       = 20985,
    NPC_SPAWN_DIMES     = 21780
};

enum SpellDimensius
{
    SPELL_DIM_FEED      = 37450,
    SPELL_MIND_FLAY     = 23953,
    SPELL_HOLY_BOLT     = 34232,
    SPELL_THROW         = 38560,
    SPELL_SHADOW_RAIN   = 37409,
    SPELL_SHADOW_RAIN_2 = 37399,
    SPELL_SHADOW_RAIN_3 = 37396,
    SPELL_SHADOW_RAIN_4 = 37397,
    SPELL_SHADOW_RAIN_5 = 37405,
    SPELL_SHADOW_SPIRAL = 37500,
    SPELL_SHADOW_VAULT  = 37412
};

#define GOSSIP_TEXT1    "Я готов пойти на Пространствуса."

class npc_defensores_dimens : public CreatureScript
{
public:
    npc_defensores_dimens() : CreatureScript("npc_defensores_dimens") { }
	
    struct npc_defensores_dimensAI : public ScriptedAI
    {
        npc_defensores_dimensAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            bNoMove = false;
            uiTorturaMental = urand(3000, 5000);
            uiChoqueSagr = 3000;
            uiGuja = 3000;
        }

        uint32 uiTorturaMental;
        uint32 uiChoqueSagr;
        uint32 uiGuja;

        bool bNoMove;

        void Reset() override
        {
            Initialize(); 
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;
				
            switch (me->GetEntry())
            {
                case NPC_DEFENS_DIMES:
                    if (uiTorturaMental <= uiDiff)
                    {
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), SPELL_MIND_FLAY, true);
                        uiTorturaMental = urand(16000, 19000);
                    } else uiTorturaMental -= uiDiff;
                    break;
                case NPC_REG_DIMES:
                    if (uiChoqueSagr <= uiDiff)
                    {
                        if (me->GetVictim())
                            DoCast(me->GetVictim(), SPELL_HOLY_BOLT);
                        uiChoqueSagr = urand(2700, 3000);
                    } else uiChoqueSagr -= uiDiff;
                    break;
                case NPC_AVENG_DIMES:
                    if (uiGuja <= uiDiff)
                    {
                        if (me->GetVictim())
                            me->CastSpell(me->GetVictim(), SPELL_THROW, true);
                        uiGuja = urand(3000, 5000);
                    } else uiGuja -= uiDiff;
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_defensores_dimensAI (creature);
    }
};

class npc_q_dimensius : public CreatureScript
{
public:
    npc_q_dimensius() : CreatureScript("npc_q_dimensius") { }

    struct npc_q_dimensiusAI : public ScriptedAI
    {
        npc_q_dimensiusAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
            bComienzoEv = false;
            uiShadowSpiral = urand(8000, 12000);
            uiShadowVault = 10000;
            uiSummonTiempo = 30000;
            uiTimerPhase = 1;
            uiTimerCount = 2500;
            SetCombatMovement(false);
            bBerkMode = false;
        }

        uint32 uiShadowSpiral;
        uint32 uiShadowVault;
        uint32 uiSummonTiempo;
        uint32 uiTimerCount;
        uint32 uiTimerPhase;

        bool bComienzoEv;
        bool bBerkMode;

        void Reset() override
        {
            Initialize();
        }

        void DespawnDefs(uint32 entry, uint32 segs)
        {
            std::list<Creature*> DefList;
            me->GetCreatureListWithEntryInGrid(DefList, entry, 40.0f);
            if (!DefList.empty())
            {
                for (std::list<Creature*>::iterator itr = DefList.begin(); itr != DefList.end(); itr++)
                {
                    if (Creature* dimens = *itr)
                    {
                        dimens->DespawnOrUnsummon(segs);
                    }
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            DespawnDefs(NPC_CAP_SAEED, 15000);
            DespawnDefs(NPC_DEFENS_DIMES, 10000);
            DespawnDefs(NPC_AVENG_DIMES, 10000);
            DespawnDefs(NPC_REG_DIMES, 10000);
        }

        void AgroDefensores(uint32 entry, uint32 /*mudf*/)
        {
            std::list<Creature*> DefList;
            me->GetCreatureListWithEntryInGrid(DefList, entry, 40.0f);
            if (!DefList.empty())
            {
                for (std::list<Creature*>::iterator itr = DefList.begin(); itr != DefList.end(); itr++)
                {
                    if (Creature* dimens = *itr)
                    {
                        switch (dimens->GetEntry())
                        {
                            case NPC_CAP_SAEED:
                                if (Creature* creat = me->SummonCreature(NPC_CAP_SAEED, dimens->GetPositionX(), dimens->GetPositionY(), dimens->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0))
                                {
                                    dimens->DespawnOrUnsummon();
                                    creat->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                                    creat->CombatStart(me);
                                    creat->AddThreat(me, 1000.0f);
                                    creat->DespawnOrUnsummon(300000);
                                    // creat->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                    me->AddThreat(creat, 100.0f);
                                }
                                break;
                            case NPC_DEFENS_DIMES:
                                if (Creature* creat = me->SummonCreature(NPC_DEFENS_DIMES, dimens->GetPositionX(), dimens->GetPositionY(), dimens->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0))
                                {
                                    dimens->DespawnOrUnsummon();
                                    creat->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                    creat->CombatStart(me);
                                    creat->AddThreat(me, 1000.0f);
                                    creat->DespawnOrUnsummon(300000);
                                    me->AddThreat(creat, 1000.0f);
                                }
                                break;
                            case NPC_AVENG_DIMES:
                                if (Creature* creat = me->SummonCreature(NPC_AVENG_DIMES, dimens->GetPositionX(), dimens->GetPositionY(), dimens->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0))
                                {
                                    dimens->DespawnOrUnsummon();
                                    creat->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                    creat->CombatStart(me);
                                    creat->AddThreat(me, 1000.0f);
                                    creat->DespawnOrUnsummon(300000);
                                    me->AddThreat(creat, 1000.0f);
                                }
                                break;
                            case NPC_REG_DIMES:
                                if (Creature* creat = me->SummonCreature(NPC_REG_DIMES, dimens->GetPositionX(), dimens->GetPositionY(), dimens->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0))
                                {
                                    dimens->DespawnOrUnsummon();
                                    creat->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                                    creat->CombatStart(me);
                                    creat->AddThreat(me, 1000.0f);
                                    creat->DespawnOrUnsummon(300000);
                                    me->AddThreat(creat, 1000.0f);
                                }
                                break;
                        }
                    }
                }
            }
        }

        void Battle()
        {
            AgroDefensores(NPC_CAP_SAEED, 1);
            AgroDefensores(NPC_DEFENS_DIMES, 1);
            AgroDefensores(NPC_AVENG_DIMES, 2);
            AgroDefensores(NPC_REG_DIMES, 2);
            bComienzoEv = true;
            me->DespawnOrUnsummon(300000);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bComienzoEv)
            {
                if (me->IsInCombat())
                {
                    if (!me->HasAura(SPELL_DIM_FEED))
                    {
                        if (uiShadowSpiral <= uiDiff)
                        {
                            if (me->GetVictim())
                                DoCast(me->GetVictim(), SPELL_SHADOW_SPIRAL);
                            uiShadowSpiral = urand(4000, 8000);
                        } else uiShadowSpiral -= uiDiff;

                        if (uiShadowVault <= uiDiff)
                        {
                            if (me->GetVictim())
                                DoCast(me->GetVictim(), SPELL_SHADOW_VAULT);
                            uiShadowVault = urand(5000, 14000);
                        } else uiShadowVault -= uiDiff;

                        if (uiSummonTiempo <= uiDiff)
                        {
                            uiTimerCount = 4000;
                            uiTimerPhase = 1;
                            me->SummonCreature(NPC_SPAWN_DIMES, 3904.895f, 2013.1589f, 257.812f, TEMPSUMMON_MANUAL_DESPAWN);
                            me->SummonCreature(NPC_SPAWN_DIMES, 3923.9f, 1972.766f, 257.812f, TEMPSUMMON_MANUAL_DESPAWN);
                            me->SummonCreature(NPC_SPAWN_DIMES, 3966.4f, 1988.784f, 257.812f, TEMPSUMMON_MANUAL_DESPAWN);
                            me->SummonCreature(NPC_SPAWN_DIMES, 3948.91f, 2034.351f, 257.812f, TEMPSUMMON_MANUAL_DESPAWN);
                            uiSummonTiempo = 60000;
                        } else uiSummonTiempo -= uiDiff;

                        if (me->HasAura(SPELL_SHADOW_RAIN_2))
                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_2);
                        if (me->HasAura(SPELL_SHADOW_RAIN))
                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN);
                        if (me->HasAura(SPELL_SHADOW_RAIN_3))
                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_3);
                        if (me->HasAura(SPELL_SHADOW_RAIN_4))
                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_4);
                        if (me->HasAura(SPELL_SHADOW_RAIN_5))
                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_5);
                    }
                    else{

                            if (uiTimerCount <= uiDiff)
                            {
                                switch (uiTimerPhase)
                                {
                                    case 1:
                                        if (me->HasAura(SPELL_SHADOW_RAIN_2))
                                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_2);
                                        me->CastSpell(me, SPELL_SHADOW_RAIN, true);
                                        uiSummonTiempo = 78000;
                                        uiTimerCount = 6500;
                                        uiTimerPhase = 2;
                                        break;
                                    case 2:
                                        if (me->HasAura(SPELL_SHADOW_RAIN))
                                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN);
                                        me->CastSpell(me, SPELL_SHADOW_RAIN_3, true);
                                        uiSummonTiempo = 78000;
                                        uiTimerCount = 5000;
                                        uiTimerPhase = 3;
                                        break;
                                    case 3:
                                        if (me->HasAura(SPELL_SHADOW_RAIN_3))
                                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_3);
                                        me->CastSpell(me, SPELL_SHADOW_RAIN_4, true);
                                        uiSummonTiempo = 78000;
                                        uiTimerCount = 4000;
                                        uiTimerPhase = 4;
                                        break;
                                    case 4:
                                        if (me->HasAura(SPELL_SHADOW_RAIN_4))
                                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_4);
                                        me->CastSpell(me, SPELL_SHADOW_RAIN_5, true);
                                        uiSummonTiempo = 78000;
                                        uiTimerCount = 4000;
                                        uiTimerPhase = 5;
                                        break;
                                    case 5:
                                        if (me->HasAura(SPELL_SHADOW_RAIN_5))
                                            me->RemoveAurasDueToSpell(SPELL_SHADOW_RAIN_5);
                                        me->CastSpell(me, SPELL_SHADOW_RAIN_2, true);
                                        uiSummonTiempo = 78000;
                                        uiTimerCount = 2500;
                                        uiTimerPhase = 1;
                                        break;
                                }

                            } else uiTimerCount -= uiDiff;
                        }
                } else
                me->DespawnOrUnsummon();

            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q_dimensiusAI (creature);
    }
};

class npc_capitan_saeed : public CreatureScript
{
public:
    npc_capitan_saeed() : CreatureScript("npc_capitan_saeed") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    { 
        if (player->GetQuestStatus(QUEST_DIMENSIUS) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch(action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1: 
                player->CLOSE_GOSSIP_MENU();
                CAST_AI(npc_capitan_saeed::npc_capitan_saeedAI, creature->AI())->Initiation();
                break;                
        }
        return true;
    }

    struct npc_capitan_saeedAI : public ScriptedAI
    {
        npc_capitan_saeedAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void Initialize()
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            uiPhaseTimer = 2000;
            uiPhase = 0;
            bBattle = false;
            bSldbt = false;
            bEvent = false;
        }

        uint8 uiIsm;
        uint32 uiPhaseTimer;
        uint32 uiPhase;
        uint32 uiFtar;

        bool bEvent;
        bool bBattle;
        bool bSldbt;

        void Reset() override
        {
            Initialize();
        }

        void Initiation()
        {
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            bEvent = true;
            uiPhaseTimer = 2000;
            uiIsm = 0;
            uiPhase = 1;
        }

        void SeguirDefensores(uint32 entry, uint32 mult)
        {
            std::list<Creature*> DefList;
            me->GetCreatureListWithEntryInGrid(DefList, entry, 40.0f);
            if (!DefList.empty())
            {
                for (std::list<Creature*>::iterator itr = DefList.begin(); itr != DefList.end(); itr++)
                {
                    if (Creature* dimens = *itr)
                    {
                        dimens->SetWalk(false);
                        dimens->SetSpeed(MOVE_RUN, 1.2f, true);
                        dimens->setFaction(35);

                        uiIsm++;

                        switch (uiIsm)
                        {
                            case 1:
                                dimens->GetMotionMaster()->MoveFollow(me, 1.0f + mult, M_PI);
                                break;
                            case 2:
                                dimens->GetMotionMaster()->MoveFollow(me, 2.0f + 2*mult, M_PI);
                                break;
                            case 3:
                                dimens->GetMotionMaster()->MoveFollow(me, 3.0f + 3*mult, M_PI);
                                break;
                            case 4:
                                dimens->GetMotionMaster()->MoveFollow(me, 1.0f + mult, M_PI / 2);
                                break;
                            case 5:
                                dimens->GetMotionMaster()->MoveFollow(me, 2.0f + 2*mult, M_PI / 2);
                                break;
                            case 6:
                                dimens->GetMotionMaster()->MoveFollow(me, 3.0f + 3*mult, M_PI / 2);
                                break;
                            case 7:
                                dimens->GetMotionMaster()->MoveFollow(me, 1.0f + mult, M_PI / 2 + M_PI);
                                break;
                            case 8:
                                dimens->GetMotionMaster()->MoveFollow(me, 2.0f + 2*mult, M_PI / 2 + M_PI);
                                break;
                            case 9:
                                dimens->GetMotionMaster()->MoveFollow(me, 3.0f + 3*mult, M_PI / 2 + M_PI);
                                break;
                            case 10:
                                dimens->GetMotionMaster()->MoveFollow(me, 5.0f + 2*mult, M_PI);
                                break;
                            case 11:
                                dimens->GetMotionMaster()->MoveFollow(me, 7.0f, M_PI);
                                break;
                        }	
                    }
                }
            }
        }

        void LinealDefenser(uint32 entry, bool Mass)
        {
            std::list<Creature*> DefList;
            me->GetCreatureListWithEntryInGrid(DefList, entry, 40.0f);
            if (!DefList.empty())
            {
                for (std::list<Creature*>::iterator itr = DefList.begin(); itr != DefList.end(); itr++)
                {
                    if (Creature* dimens = *itr)
                    {
                        if (!Mass)
                        {
                            uiIsm++;

                            switch (uiIsm)
                            {
                                case 1:
                                    dimens->GetMotionMaster()->MoveFollow(me, 1.0f, M_PI);
                                    break;
                                case 2:
                                    dimens->GetMotionMaster()->MoveFollow(me, 2.0f, M_PI);
                                    break;
                                case 3:
                                    dimens->GetMotionMaster()->MoveFollow(me, 3.0f, M_PI);
                                    break;
                                case 4:
                                    dimens->GetMotionMaster()->MoveFollow(me, 4.0f, M_PI);
                                    break;
                                case 5:
                                    dimens->GetMotionMaster()->MoveFollow(me, 5.0f, M_PI);
                                    break;
                                case 6:
                                    dimens->GetMotionMaster()->MoveFollow(me, 6.0f, M_PI);
                                    break;
                                case 7:
                                    dimens->GetMotionMaster()->MoveFollow(me, 7.0f, M_PI); 
                                    break;
                                case 8:
                                    dimens->GetMotionMaster()->MoveFollow(me, 8.0f, M_PI);
                                    break;
                                case 9:
                                    dimens->GetMotionMaster()->MoveFollow(me, 9.0f, M_PI);
                                    break;
                                case 10:
                                    dimens->GetMotionMaster()->MoveFollow(me, 10.0f, M_PI);
                                    break;
                                case 11:
                                    dimens->GetMotionMaster()->MoveFollow(me, 11.0f, M_PI);
                                    break;
                            }	
                        }
                    }
                }
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {		
            if (bEvent)
            {

                if (!bSldbt && !bBattle && me->IsInCombat())
                { 
                    uiFtar = uiPhase;
                    bSldbt = true;
                }

                if (bSldbt && !bBattle && !me->IsInCombat())
                {
                    uiPhase = uiFtar;
                    uiPhaseTimer = 7000;
                }

                if (uiPhaseTimer <= uiDiff)
                {
                    switch (uiPhase)
                    {
                        case 1:
                            Talk(SAY_EVENT_1);
                            me->setFaction(35);
                            uiPhaseTimer = 8000;
                            uiPhase = 2;
                            break;
                        case 2:
                            me->SetWalk(false);
                            me->SetSpeed(MOVE_RUN, 1.2f, true);
                            me->GetMotionMaster()->MovePoint(0, 4242.35f, 2124.63f, 145.41f);
                            uiPhaseTimer = 1000;
                            uiPhase = 3;
                            break;
                        case 3:
                            uiIsm = 0;
                            SeguirDefensores(NPC_DEFENS_DIMES, 0);
                            SeguirDefensores(NPC_AVENG_DIMES, 0);
                            SeguirDefensores(NPC_REG_DIMES, 0);
                            uiPhaseTimer = 6000;
                            uiPhase = 4;
                            break;
                        case 4:
                            me->GetMotionMaster()->MovePoint(0, 4201.36f, 2172.486f, 151.077f);
                            uiPhaseTimer = 8000;
                            uiPhase = 5;
                            break;
                        case 5:
                            me->GetMotionMaster()->MovePoint(0, 4187.68f, 2137.94f, 155.111f);
                            uiPhaseTimer = 7000;
                            uiPhase = 6;
                            break;	  
                        case 6:
                            me->GetMotionMaster()->MovePoint(0, 4187.95f, 2092.789f, 159.177f);
                            uiPhaseTimer = 7000;
                            uiPhase = 7;
                            break;	 
                        case 7:
                            me->GetMotionMaster()->MovePoint(0, 4172.53f, 2062.655f, 167.302f);
                            uiPhaseTimer = 6000;
                            uiPhase = 8;
                            break;
                        case 8:
                            me->GetMotionMaster()->MovePoint(0, 4187.749f, 2028.315f, 182.627f);
                            uiPhaseTimer = 5000;
                            uiPhase = 9;
                            break;	
                        case 9:
                            me->GetMotionMaster()->MovePoint(0, 4173.355f, 1990.116f, 205.659f);
                            uiPhaseTimer = 7000;
                            uiPhase = 10;
                            break;	
                        case 10:
                            me->GetMotionMaster()->MovePoint(0, 4129.799f, 1969.9f, 221.14f);
                            uiPhaseTimer = 8000;
                            uiPhase = 11;
                            break;
                        case 11:
                            me->GetMotionMaster()->MovePoint(0, 4093.86f, 2026.225f, 236.116f);
                            uiPhaseTimer = 8000;
                            uiPhase = 12;
                            break;
                        case 12:
                            me->GetMotionMaster()->MovePoint(0, 4054.147f, 2060.93f, 251.453f);
                            uiPhaseTimer = 12000;
                            uiPhase = 13;
                            break;
                        case 13:
                            me->GetMotionMaster()->MovePoint(0, 4008.409f, 2096.97f, 254.302f);
                            uiPhaseTimer = 7000;
                            uiPhase = 14;
                            break;
                        case 14:
                            me->GetMotionMaster()->MovePoint(0, 3988.82f, 2083.117f, 256.404f);
                            uiIsm = 0;
                            LinealDefenser(NPC_DEFENS_DIMES, false);
                            LinealDefenser(NPC_AVENG_DIMES, false);
                            LinealDefenser(NPC_REG_DIMES, false);
                            uiPhaseTimer = 4000;
                            uiPhase = 15;
                            break;
                        case 15:
                            me->GetMotionMaster()->MovePoint(0, 3949.692f, 2023.207f, 256.697f);
                            uiPhaseTimer = 7700;
                            Phase = 16;
                            break;
                        case 16:
                            uiIsm = 0;
                            SeguirDefensores(NPC_DEFENS_DIMES, 1);
                            SeguirDefensores(NPC_AVENG_DIMES, 1);
                            SeguirDefensores(NPC_REG_DIMES, 1);
                            uiPhaseTimer = 6000;
                            uiPhase = 17;
                            break;
                        case 17:
                            Talk(SAY_EVENT_2);
                            uiPhaseTimer = 6000;
                            uiPhase = 18;
                            break;
                        case 18:
                            if (Creature* dimensius = me->FindNearestCreature(NPC_DEV_DIMES, 60.0f))
                                Talk(SAY_EVENT_3);
                            uiPhaseTimer = 5000;
                            uiPhase = 19;
                            break;
                        case 19:
                            Talk(SAY_EVENT_4);
                            if (Creature* dimensius = me->FindNearestCreature(NPC_DEV_DIMES, 60.0f))
                               dimensius->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                            uiPhaseTimer = 2000;
                            uiPhase = 20;
                            break;
                        case 20:
                            bBattle = true;
                            me->setFaction(1806);
                            if (Creature* dimensius = me->FindNearestCreature(NPC_DEV_DIMES, 60.0f))
                                CAST_AI(npc_q_dimensius::npc_q_dimensiusAI, dimensius->AI())->Battle();
                            uiPhaseTimer = 100;
                            uiPhase = 21;
                            break;
                    }
                } else uiPhaseTimer -= uiDiff;
            }
            DoMeleeAttackIfReady();

        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_capitan_saeedAI (creature);
    }
};

class npc_spawn_dimensius : public CreatureScript
{
public:
    npc_spawn_dimensius() : CreatureScript("npc_spawn_dimensius") { }

    struct npc_spawn_dimensiusAI : public ScriptedAI
    {
        npc_spawn_dimensiusAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            SetCombatMovement(false);
            uiChannel = 500;
            bChannel = false;
        }

        uint32 uiChannel;

        bool bChannel;

        void Reset() override
        {
            Initialize();
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (uiChannel <= uiDiff)
            {
                if (Creature* dimensius = me->FindNearestCreature(NPC_DEV_DIMES, 60.0f))
                { 
                    if (!bChannel)
                    {
                        me->CastSpell(dimensius, SPELL_DIM_FEED, true);
                        bChannel = true;
                    }
                } else

                me->DespawnOrUnsummon();
                uiChannel = 5000;
            } else uiChannel -= uiDiff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_spawn_dimensiusAI (creature);
    }
};

void AddSC_netherstorm()
{
    new npc_commander_dawnforge();
    new at_commander_dawnforge();
    new npc_professor_dabiri();
    new npc_phase_hunter();
    new npc_bessy();
    new npc_maxx_a_million_escort();
    new go_captain_tyralius_prison();
    new npc_defensores_dimens();
    new npc_q_dimensius();
    new npc_capitan_saeed();
    new npc_spawn_dimensius();
}