/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

// TODO: Intro

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "trial_of_the_champion.h"

enum Enums
{
    // Yell
    SAY_DEATH_3             = 1,
    SAY_AGGRO               = 2,
    SAY_AGGRO_2             = 3,
    SAY_SLAY                = 4,
    SAY_DEATH_1             = 5,
    SAY_DEATH               = 6,
    SAY_START5              = 7,
    SAY_START6              = 8,
    SAY_START7              = 9,
    SAY_START8              = 10,
    SAY_START9              = 11,
    SAY_START10             = 12,
    SAY_START11             = 13,
    SAY_KILL                = 14,
    SAY_KILL1               = 15
};

enum Spells
{
    // Phase 1
    SPELL_FEIGN_DEATH       = 66804, // 1
    SPELL_RAISE_BRIGHTSTAR  = 67705, // 46
    SPELL_RAISE_SUNSWORN    = 67715, // 46
  //SPELL_PLAGUE_STRIKE_H   = 67884,
    SPELL_PLAGUE_STRIKE     = 67724,
  //SPELL_ICY_TOUCH_H       = 67881,
    SPELL_ICY_TOUCH         = 67718,
    SPELL_DEATH_RESPITE     = 67745,
  //SPELL_DEATH_RESPITE_H   = 68306,
    SPELL_DEATH_RESPITE_3   = 66798,
    SPELL_OBLITERATE_H      = 67883,
    SPELL_OBLITERATE        = 67725,

    // Phase 2
    SPELL_ARMY_DEAD         = 67761,
  //SPELL_ARMY_DEAD_H       = 67874,
    SPELL_DESECRATION       = 67778,
  //SPELL_DESECRATION_H     = 67877,
    SPELL_GHOUL_EXPLODE     = 67751,

    // Phase 3
  //SPELL_DEATH_BITE_H      = 67875,
    SPELL_DEATH_BITE        = 67808,
  //SPELL_MARKED_DEATH_H    = 67882,
    SPELL_MARKED_DEATH      = 67823,

    SPELL_BLACK_KNIGHT_RES  = 67693,

    SPELL_LEAP              = 67749,
  //SPELL_LEAP_H            = 67880,
    SPELL_CLAW              = 67774,
  //SPELL_CLAW_H            = 67879,
    SPELL_EXPLODE           = 67729,
    SPELL_EXPLODE_H         = 67886
};

enum Models
{
    MODEL_SKELETON          = 29846,
    MODEL_GHOST             = 21300
};

enum Eqip
{
    EQUIP_SWORD             = 40343
};

enum Phases
{
    PHASE_UNDEAD = 1,
    PHASE_SKELETON,
    PHASE_GHOST
};

enum Events
{
    EVENT_CLAW = 1,
    EVENT_LEAP
};

enum Misc
{
    NPC_RISEN_CHAMPION      = 35590,
    NPC_RISEN_BRIGHTSTAR    = 35564,
    NPC_RISEN_SUNSWORN      = 35545,
    DATA_I_VE_HAD_WORSE     = 1
};

class boss_black_knight : public CreatureScript
{
    public:
        boss_black_knight() : CreatureScript("boss_black_knight") { }

        struct boss_black_knightAI : public ScriptedAI
        {
            boss_black_knightAI(Creature* creature) : ScriptedAI(creature), summons(creature)
            {
                _instance = creature->GetInstanceScript();
            }

            InstanceScript* _instance;
            SummonList summons;

            bool isResurrecting;
            bool isSummoningArmy;
            bool isDeathArmySummoned;
            bool isAttacked;
            bool iveHadWorse;

            uint8 phase;

            uint32 plagueStrikeTimer;
            uint32 icyTouchTimer;
            uint32 deathRespiteTimer;
            uint32 obliterateTimer;
            uint32 desecrationTimer;
            uint32 resurrectTimer;
            uint32 deathArmyCheckTimer;
            uint32 ghoulExplodeTimer;
            uint32 deathBiteTimer;
            uint32 markedDeathTimer;

            void Reset()
            {
                summons.DespawnAll();

                me->SetDisplayId(me->GetNativeDisplayId());
                me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);

                isResurrecting = false;
                isSummoningArmy = false;
                isDeathArmySummoned = false;
                iveHadWorse = true;

                if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_MAIN_GATE)))
                    _instance->HandleGameObject(go->GetGUID(), true);
                if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_PORTCULLIS)))
                    _instance->HandleGameObject(go->GetGUID(), false);

                if (isAttacked)
                    me->GetMotionMaster()->MovePoint(1, 743.396f, 635.411f, 411.575f);

                phase = PHASE_UNDEAD;

                icyTouchTimer = urand(3000, 5000);
                plagueStrikeTimer = urand(4000, 7000);
                deathRespiteTimer = urand(5000, 8000);
                obliterateTimer = urand(10000, 12000);
                desecrationTimer = urand(12000, 15000);
                deathArmyCheckTimer = 1000;
                resurrectTimer = 4000;
                ghoulExplodeTimer = 6000;
                deathBiteTimer = urand(2000, 4000);
                markedDeathTimer = urand(5000, 7000);
            }

            void JustSummoned(Creature* summon)
            {
            summons.Summon(summon);
            summon->AI()->AttackStart(me->GetVictim());
            }

            void EnterEvadeMode()
            {
                me->ClearUnitState(UNIT_STATE_STUNNED | UNIT_STATE_ROOT);
                ScriptedAI::EnterEvadeMode();
            }

            void EnterCombat(Unit* /*who*/)
            {
                isAttacked = true;
                Talk(SAY_AGGRO_2);
                DoZoneInCombat(me, 150.0f);
                SetEquipmentSlots(false, EQUIP_SWORD, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);

                if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_MAIN_GATE)))
                    _instance->HandleGameObject(go->GetGUID(), false);
                if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_PORTCULLIS)))
                    _instance->HandleGameObject(go->GetGUID(), false);

                if (_instance)
                    _instance->SetData(BOSS_BLACK_KNIGHT, IN_PROGRESS);

                // TODO: add Intro
                if (Creature* brightstar = me->FindNearestCreature(NPC_ARELAS, 250.0f))
                {
                    me->Kill(brightstar);
                    brightstar->DespawnOrUnsummon(1000);
                    me->SummonCreature(NPC_RISEN_BRIGHTSTAR, *brightstar);
                }
                else
                    me->SummonCreature(NPC_RISEN_BRIGHTSTAR, *me);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage)
            {
                if (isResurrecting)
                {
                    damage = 0;
                    return;
                }

                if (damage >= me->GetHealth() && phase <= PHASE_SKELETON)
                {
                    damage = 0;
                    me->SetHealth(0);
                    me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);

                    summons.DespawnAll();

                    switch (phase)
                    {
                        case PHASE_UNDEAD:
                            me->SetDisplayId(MODEL_SKELETON);
                            break;
                        case PHASE_SKELETON:
                            me->SetDisplayId(MODEL_GHOST);
                            break;
                    }
                    isResurrecting = true;
                }
            }

            void DoAction(int32 const param)
            {
                if (param == DATA_I_VE_HAD_WORSE)
                    iveHadWorse = false;
            }

            uint32 GetData(uint32 type)
            {
                if (type == DATA_I_VE_HAD_WORSE)
                    return iveHadWorse ? 1 : 0;

                return 0;
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(urand(0, 1) ? SAY_KILL : SAY_KILL1);
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCast(me, SPELL_BLACK_KNIGHT_CREDIT, true);
                Talk(SAY_DEATH_3);

                if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_PORTCULLIS) : 0))
                    _instance->HandleGameObject(go->GetGUID(), true);

                if (_instance)
                    _instance->SetData(BOSS_BLACK_KNIGHT, DONE);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (isResurrecting)
                {
                    if (resurrectTimer <= diff)
                    {
                        me->SetFullHealth();
                        switch (phase)
                        {
                            case PHASE_UNDEAD:
                                Talk(SAY_DEATH_1);
                                break;
                            case PHASE_SKELETON:
                                Talk(SAY_DEATH);
                                break;
                        }
                        DoCast(me, SPELL_BLACK_KNIGHT_RES, true);
                        ++phase;
                        resurrectTimer = 4000;
                        isResurrecting = false;
                        me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                    }
                    else
                        resurrectTimer -= diff;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING) || isResurrecting)
                    return;

                switch (phase)
                {
                    case PHASE_UNDEAD:
                    case PHASE_SKELETON:
                    {
                        if (icyTouchTimer <= diff)
                        {
                            DoCastVictim(SPELL_ICY_TOUCH);
                            icyTouchTimer = urand(5000, 7000);
                        }
                        else
                            icyTouchTimer -= diff;

                        if (plagueStrikeTimer <= diff)
                        {
                            DoCastVictim(SPELL_PLAGUE_STRIKE);
                            plagueStrikeTimer = urand(12000, 15000);
                        }
                        else
                            plagueStrikeTimer -= diff;

                        if (obliterateTimer <= diff)
                        {
                            DoCastVictim(SPELL_OBLITERATE);
                            obliterateTimer = urand(12000, 17000);
                        }
                        else
                            obliterateTimer -= diff;

                        switch (phase)
                        {
                            case PHASE_UNDEAD:
                            {
                                if (deathRespiteTimer <= diff)
                                {
                                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                                        DoCast(target, SPELL_DEATH_RESPITE);
                                     deathRespiteTimer = urand(10000, 12000);
                                }
                                else
                                    deathRespiteTimer -= diff;
                                break;
                            }
                            case PHASE_SKELETON:
                            {
                                if (!isSummoningArmy)
                                {
                                    isSummoningArmy = true;
                                    me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                                    DoCast(me, SPELL_ARMY_DEAD);
                                }

                                if (!isDeathArmySummoned)
                                {
                                    if (deathArmyCheckTimer <= diff)
                                    {
                                        me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                                        deathArmyCheckTimer = 0;
                                        ghoulExplodeTimer = urand(3000, 5000);
                                        isDeathArmySummoned = true;
                                    }
                                    else
                                        deathArmyCheckTimer -= diff;
                                }

                                if (desecrationTimer <= diff)
                                {
                                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                        DoCast(target, SPELL_DESECRATION);
                                    desecrationTimer = urand(15000, 16000);
                                }
                                else
                                    desecrationTimer -= diff;

                                if (ghoulExplodeTimer <= diff)
                                {
                                    DoCast(me, SPELL_GHOUL_EXPLODE);
                                    ghoulExplodeTimer = 5000;
                                }
                                else
                                    ghoulExplodeTimer -= diff;
                                break;
                            }
                            break;
                        }
                        break;
                    }
                    case PHASE_GHOST:
                    {
                        if (deathBiteTimer <= diff)
                        {
                            DoCast(SPELL_DEATH_BITE);
                            deathBiteTimer = 3000;
                        }
                        else
                            deathBiteTimer -= diff;

                        if (markedDeathTimer <= diff)
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                                DoCast(target, SPELL_MARKED_DEATH);
                            markedDeathTimer = 10000;
                        }
                        else
                            markedDeathTimer -= diff;
                        break;
                    }
                }

                if (!me->HasUnitState(UNIT_STATE_STUNNED))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_black_knightAI(creature);
        }
};

class npc_risen_ghoul : public CreatureScript
{
    public:
        npc_risen_ghoul() : CreatureScript("npc_risen_ghoul") { }

        struct npc_risen_ghoulAI : public ScriptedAI
        {
            npc_risen_ghoulAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = me->GetInstanceScript();
            }

            void Reset()
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _events.ScheduleEvent(EVENT_CLAW, urand(5000, 10000));

                if (me->GetEntry() != NPC_RISEN_CHAMPION)
                    _events.ScheduleEvent(EVENT_LEAP, urand(10000, 15000));
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_GHOUL_EXPLODE)
                {
                    _events.Reset();
                    DoCast(SPELL_EXPLODE);
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (spell->Id == DUNGEON_MODE<uint32>(SPELL_EXPLODE, SPELL_EXPLODE_H) && target->GetTypeId() == TYPEID_PLAYER)
                    if (Creature* knight = ObjectAccessor::GetCreature(*me, _instance->GetGuidData(DATA_BLACK_KNIGHT) : 0))
                        knight->AI()->DoAction(DATA_I_VE_HAD_WORSE);
            }

            void DoAction(int32 const action)
            {
                if (action == 1)
                {
                    _events.Reset();
                    me->AddAura(SPELL_GHOUL_EXPLODE, me);
                    DoCast(SPELL_EXPLODE);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLAW:
                            if (me->IsWithinCombatRange(me->GetVictim(), 5.0f))
                            {
                                DoCastVictim(SPELL_CLAW);
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                                {
                                    DoResetThreat();
                                    me->AddThreat(target, 1000.0f);
                                }
                                _events.ScheduleEvent(EVENT_CLAW, urand(4000, 8000));
                            }
                            else
                                _events.ScheduleEvent(EVENT_CLAW, 1000);
                            break;
                        case EVENT_LEAP:
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 1, 30.0f, true))
                            {
                                DoResetThreat();
                                me->AddThreat(target, 1000.0f);
                                DoCast(target, SPELL_LEAP);
                            }
                            _events.ScheduleEvent(EVENT_LEAP, urand(10000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_risen_ghoulAI(creature);
        }
};

class npc_black_knight_skeletal_gryphon : public CreatureScript
{
public:
    npc_black_knight_skeletal_gryphon() : CreatureScript("npc_black_knight_skeletal_gryphon") { }

    struct npc_black_knight_skeletal_gryphonAI : public npc_escortAI
    {
        npc_black_knight_skeletal_gryphonAI(Creature* creature) : npc_escortAI(creature)
        {
            Start(false, true);
            instance = creature->GetInstanceScript();
        }
		
		void Initialize()
        {
            pHighlord = NULL;
        }

        Creature* pHighlord;
        InstanceScript* instance;


        void Reset() override
        {
            Initialize();
        }
		
            void WaypointReached(uint32 /*i*/) { }
            void AttackStart(Unit* /*who*/) { }

            void UpdateAI(uint32 const diff)
            {
                npc_escortAI::UpdateAI(diff);

                UpdateVictim();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_black_knight_skeletal_gryphonAI(creature);
        }
};

class achievement_i_ve_had_worse : public AchievementCriteriaScript
{
    public:
        achievement_i_ve_had_worse() : AchievementCriteriaScript("achievement_i_ve_had_worse")
        {
        }

        bool OnCheck(Player* player, Unit* /*target*/)
        {
            if (!player)
                return false;

            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* knight = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_BLACK_KNIGHT)))
                    if (knight->AI()->GetData(DATA_I_VE_HAD_WORSE))
                        return true;

            return false;
        }
};

void AddSC_boss_black_knight()
{
    new boss_black_knight();
    new npc_risen_ghoul();
    new npc_black_knight_skeletal_gryphon();
    new achievement_i_ve_had_worse();
}