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
#include "ScriptedEscortAI.h"
#include "trial_of_the_champion.h"

enum SaysBlack
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
    SAY_START11             = 13
};

enum Spells
{
    // Phase 1
    SPELL_PLAGUE_STRIKE     = 67724,
    SPELL_PLAGUE_STRIKE_2   = 67884,
    SPELL_ICY_TOUCH         = 67881,
    SPELL_ICY_TOUCH_H       = 67718,
    SPELL_DEATH_RESPITE     = 67745,
    SPELL_OBLITERATE        = 67883,
    SPELL_OBLITERATE_H      = 67725,

    // phase 2 - During this phase, the Black Knight will use the same abilities as in phase 1, except for Death's Respite
    SPELL_ARMY_DEAD         = 67761,
    SPELL_DESECRATION       = 67778,
    SPELL_DESECRATION_2     = 67778,
    SPELL_GHOUL_EXPLODE     = 67751,

    // phase 3
    SPELL_DEATH_BITE_H      = 67875,
    SPELL_DEATH_BITE        = 67808,
    SPELL_MARKED_DEATH      = 67882,
    SPELL_MARKED_DEATH_2    = 67823,
    SPELL_BLACK_KNIGHT_RES  = 67693,

    SPELL_LEAP              = 67749,
    SPELL_LEAP_H            = 67880,
};

enum Models
{
    MODEL_SKELETON          = 29846,
	MODEL_GHOST             = 21300
};

enum Eqip
{
     EQUIP_SWORD            = 40343
};

enum IntroPhase
{
    IDLE,
    INTRO,
    FINISHED
};

enum Phases
{
    PHASE_UNDEAD            = 1,
    PHASE_SKELETON          = 2,
    PHASE_GHOST             = 3
};

enum Misc
{
    ACHIEV_WORSE            = 3804
};

class boss_black_knight : public CreatureScript
{
public:
    boss_black_knight() : CreatureScript("boss_black_knight") { }
	
	    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetInstanceAI<boss_black_knightAI>(creature);
    }

    struct boss_black_knightAI : public ScriptedAI
    {
        boss_black_knightAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            Initialize();
            instance = (InstanceScript*)creature->GetInstanceScript();
        }

        void Initialize()
        {
            bEventInProgress = false;
            bEvent = false;
            bSummonArmy = false;
            bDeathArmyDone = false;
			bEventInBattle = false;
            bFight = false;
            
            if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(pGO->GetGUID(),true);
            if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(pGO->GetGUID(),false);

            uiPhase = PHASE_UNDEAD;

            uiIcyTouchTimer         = urand(5000,9000);
            uiIcyTouch1Timer        = urand(15000,15000);
            uiPlagueStrikeTimer     = urand(10000,13000);
            uiPlagueStrike1Timer    = urand(14000,14000);
            uiDeathRespiteTimer     = urand(17000,17000);
            uiObliterateTimer       = urand(17000,19000);
            uiObliterate1Timer      = urand(15000,15000);
            uiDesecrationTimer      = urand(15000,16000);
            uiDesecration1Timer     = urand(22000,22000);
            uiDeathArmyCheckTimer   = 7000;
            uiResurrectTimer        = 4000;
            uiGhoulExplodeTimer     = 8000;
            uiDeathBiteTimer        = urand (2000,4000);
            uiMarkedDeathTimer      = urand (5000,7000);
            uiIntroTimer            = 5000;
        }

        void EnterEvadeMode()
        {
            if (!instance) 
                return;
            
            if (bEventInBattle)
            {
                me->GetMotionMaster()->MovePoint(1, 743.396f, 635.411f, 411.575f);
                me->setFaction(14);
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
            }
            ScriptedAI::EnterEvadeMode();
        }

        InstanceScript* instance;
        SummonList summons;

        bool bEventInProgress;
        bool bEvent;
        bool bSummonArmy;
        bool bDeathArmyDone;
		bool bEventInBattle;
        bool bFight;

        uint8 uiPhase;
		uint8 uiIntroPhase;
        
        IntroPhase Phase;
        
        uint32 uiIntroTimer;
        uint32 uiPlagueStrikeTimer;
		uint32 uiPlagueStrike1Timer;
        uint32 uiIcyTouchTimer;
		uint32 uiIcyTouch1Timer;
        uint32 uiDeathRespiteTimer;
        uint32 uiObliterateTimer;
        uint32 uiObliterate1Timer;
        uint32 uiDesecrationTimer;
        uint32 uiDesecration1Timer;
        uint32 uiResurrectTimer; 
        uint32 uiDeathArmyCheckTimer;
        uint32 uiGhoulExplodeTimer;
        uint32 uiDeathBiteTimer;
        uint32 uiMarkedDeathTimer;

        void Reset() override
        {
            summons.DespawnAll();
            me->SetDisplayId(me->GetNativeDisplayId());
            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);

            Initialize();
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->AI()->AttackStart(me->GetVictim());
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            summons.Despawn(summon);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (bEventInProgress)
                if (uiResurrectTimer <= uiDiff)
                {
                    me->SetHealth(me->GetMaxHealth());
                    me->AttackStop();

                    switch (uiPhase)
                    {
                        case PHASE_UNDEAD:
                            Talk(SAY_DEATH_1);
                            break;
                        case PHASE_SKELETON:
                            Talk(SAY_DEATH);
                            break;
                    }
                    DoCast(me, SPELL_BLACK_KNIGHT_RES, true);
					me->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                    uiPhase++;
                    uiResurrectTimer = 3000;
                    bEventInProgress = false;
                    me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                } else uiResurrectTimer -= uiDiff;

            switch (uiPhase)
            {
                case PHASE_UNDEAD:
                {
                    if (uiPlagueStrikeTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        uiPlagueStrikeTimer = urand(12000, 15000);
                    } else uiPlagueStrikeTimer -= uiDiff;
					
                    if (uiObliterateTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_OBLITERATE);
                        uiObliterateTimer = urand(17000, 19000);
                    } else uiObliterateTimer -= uiDiff;

					if (uiIcyTouchTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_ICY_TOUCH);
                        uiIcyTouchTimer = urand(5000,7000);
                    } else uiIcyTouchTimer -= uiDiff;
                    break;
                }
                case PHASE_SKELETON:
                {
                    if (!bSummonArmy)
                    {
                        bSummonArmy = true;
                        me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                        me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                        DoCast(me, SPELL_ARMY_DEAD);
                    }

                    if (!bDeathArmyDone)
                        if (uiDeathArmyCheckTimer <= uiDiff)
                        {
                            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                            me->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                            uiDeathArmyCheckTimer = 0;
                            bDeathArmyDone = true;
                        } else uiDeathArmyCheckTimer -= uiDiff;

                    if (uiDesecration1Timer <= uiDiff)
                    {
                        if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->IsAlive())
                                DoCast(target, SPELL_DESECRATION);
                        }
                        uiDesecration1Timer = urand(15000,16000);
                    } else uiDesecration1Timer -= uiDiff;

                    if (uiGhoulExplodeTimer <= uiDiff)
                    {
                        DoCast(me, SPELL_GHOUL_EXPLODE);
                        uiGhoulExplodeTimer = 8000;
                    } else uiGhoulExplodeTimer -= uiDiff;

                    if (uiPlagueStrike1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        uiPlagueStrike1Timer = urand(12000,15000);
                    } else uiPlagueStrike1Timer -= uiDiff;

                    if (uiObliterate1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_OBLITERATE);
                        uiObliterate1Timer = urand(17000,19000);
                    } else uiObliterate1Timer -= uiDiff;

                    if (uiIcyTouch1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_ICY_TOUCH);
                        uiIcyTouch1Timer = urand(5000,7000);
                    } else uiIcyTouch1Timer -= uiDiff;

                    if (uiDeathRespiteTimer <= uiDiff)
                    {
                        if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->IsAlive())
                                DoCast(target, SPELL_DEATH_RESPITE);
                        }
                        uiDeathRespiteTimer = urand(15000,16000);
                    } else uiDeathRespiteTimer -= uiDiff;
                    break;
                }
				break;
                case PHASE_GHOST:
                {
                    if (uiDeathBiteTimer <= uiDiff)
                    {
                        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
                        DoCast(me,SPELL_DEATH_BITE);
                        uiDeathBiteTimer = urand(2000, 4000);
                    } else uiDeathBiteTimer -= uiDiff;
					
                    if (uiMarkedDeathTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->IsAlive())
                                DoCast(target, SPELL_MARKED_DEATH);
                        }
                        uiMarkedDeathTimer = urand(5000, 7000);
                    } else uiMarkedDeathTimer -= uiDiff;
                    break;
                }
            }
            if (!me->HasUnitState(UNIT_STATE_ROOT) && !me->HealthBelowPct(1))
                DoMeleeAttackIfReady();
        }

        void EnterCombat(Unit* who) override
        {   
            bEventInBattle = true;
            Talk(SAY_AGGRO_2);
            SetEquipmentSlots(false, EQUIP_SWORD, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(pGO->GetGUID(),false);
            if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                instance->HandleGameObject(pGO->GetGUID(),false);
        }
        
        void KilledUnit(Unit* victim) override
        {
            if (instance)
                instance->SetData(BOSS_BLACK_KNIGHT,IN_PROGRESS);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
        {
            if (uiDamage > me->GetHealth() && uiPhase <= PHASE_SKELETON)
            {
                uiDamage = 0;
                me->SetHealth(0);
                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
				me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                summons.DespawnAll();
                switch (uiPhase)
                {
                    case PHASE_UNDEAD:
                        me->SetDisplayId(MODEL_SKELETON);
                        break;
                    case PHASE_SKELETON:
                        me->SetDisplayId(MODEL_GHOST);
						SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
                        break;
                }
                bEventInProgress = true;
            }
        }

        void JustDied(Unit* killer) override
        {
            Talk(SAY_DEATH_3);
            if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(pGO->GetGUID(),true);

            if (instance)
			{
                instance->SetData(BOSS_BLACK_KNIGHT, DONE);

                /*if (IsHeroic())
                    instance->DoCompleteAchievement(ACHIEV_WORSE);*/
            }
        }
    };
};

class npc_risen_ghoul : public CreatureScript
{
public:
    npc_risen_ghoul() : CreatureScript("npc_risen_ghoul") { }
	
	    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_risen_ghoulAI(creature);
    }

    struct npc_risen_ghoulAI : public ScriptedAI
    {
        npc_risen_ghoulAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            uiAttackTimer = 3500;
        }

        uint32 uiAttackTimer;

        void Reset() override
        {
            Initialize();
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiAttackTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target, SPELL_LEAP);
                }
                uiAttackTimer = 3500;
            } else uiAttackTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };
};

class npc_black_knight_skeletal_gryphon : public CreatureScript
{
public:
    npc_black_knight_skeletal_gryphon() : CreatureScript("npc_black_knight_skeletal_gryphon") { }
	
	CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_black_knight_skeletal_gryphonAI(creature);
    }

    struct npc_black_knight_skeletal_gryphonAI : public npc_escortAI
    {
        npc_black_knight_skeletal_gryphonAI(Creature* creature) : npc_escortAI(creature)
        {
            Start(false, true);
			instance = (InstanceScript*)creature->GetInstanceScript();
        }
		
		InstanceScript* instance;
        
        void WaypointReached(uint32 uiPointId) override
		{
                switch(uiPointId)
                {
                    case 1:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 2:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        if (instance)
                        {
                            instance->SetData(DATA_BLACK_KNIGHT, NOT_STARTED);
                        }
                        break;
                    case 3:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 4:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 5:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 6:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 7:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 8:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 9:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 10:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 11:
                        me->SetSpeed(MOVE_FLIGHT , 2.0f);
                        break;
                    case 12:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        me->SetSpeed(MOVE_RUN, 2.0f);
                        break;
                    case 13:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        me->SummonCreature(VEHICLE_GR, 744.841f, 634.505f, 411.575f, 2.79f);
                        break;
                }		

        void UpdateAI(uint32 uiDiff) override
        {
            npc_escortAI::UpdateAI(uiDiff);

            UpdateVictim();
        }
			
        };
    };
};

class npc_gr : public CreatureScript
{
public:
    npc_gr() : CreatureScript("npc_gr") { }
 
     CreatureAI* GetAI(Creature* creature) const override
     {
        return new npc_grAI(creature);
     }
	 struct npc_grAI : public npc_escortAI
    {
	
        npc_grAI(Creature* creature) : npc_escortAI(creature)
        {
            Start(false, true);
            instance = (InstanceScript*)creature->GetInstanceScript();	
        }

    	InstanceScript* instance;

        void WaypointReached(uint32 uiPointId) override
        {
            switch(uiPointId)
            {
                case 1:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    if (instance)
                    {
                        instance->SetData(DATA_BLACK_KNIGHT, IN_PROGRESS);
                    }
                    break;
                case 2:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    if (instance)
                    {
                        instance->SetData(DATA_BLACK_KNIGHT, DONE);
                    }   
                    break;
                case 3:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    break;
                case 4:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    if (instance)
                    {
                        instance->SetData(DATA_KNIGHT, NOT_STARTED);
                    }
                    break;
                case 5:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    if (instance)
                    {
                        instance->SetData(DATA_KNIGHT, IN_PROGRESS);
                    }
                    break;
                case 6:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    break;
                case 7:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                    break;
                case 8:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                case 9:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                    if (instance)
                    {
                        instance->SetData(DATA_KNIGHT, DONE);
                    }
                    break;
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (!UpdateVictim())
                return;
        }

    };
};	

void AddSC_boss_black_knight()
{
    new boss_black_knight();
    new npc_risen_ghoul();
    new npc_black_knight_skeletal_gryphon();
	new npc_gr();
}