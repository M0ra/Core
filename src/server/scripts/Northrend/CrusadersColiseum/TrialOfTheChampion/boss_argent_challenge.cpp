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
SDName: Argent Challenge Encounter.
SD%Complete: 90 %
SDComment: AI from bosses need more improvements. Need AI for lightwell
SDCategory: Trial of the Champion
EndScriptData */

#include "ScriptPCH.h"
#include "trial_of_the_champion.h"
#include "ScriptedEscortAI.h"
/*
enum Yells
{
    // Eadric the Pure
    SAY_INTRO                   = 0,
    SAY_AGGRO                   = 1,
    EMOTE_RADIANCE              = 2,
    EMOTE_HAMMER_RIGHTEOUS      = 3,
    SAY_HAMMER_RIGHTEOUS        = 4,
    SAY_KILL_PLAYER             = 5,
    SAY_DEFEATED                = 6,

    // Argent Confessor Paletress
    SAY_INTRO_1                 = 0,
    SAY_INTRO_2                 = 1,
    SAY_AGGRO                   = 2,
    SAY_MEMORY_SUMMON           = 3,
    SAY_MEMORY_DEATH            = 4,
    SAY_KILL_PLAYER             = 5,
    SAY_DEFEATED                = 6,

    // Memory of X
    EMOTE_WAKING_NIGHTMARE      = 0
};
*/
enum Spells
{
    // Eadric the Pure
    SPELL_EADRIC_ACHIEVEMENT    = 68197,
    SPELL_HAMMER_JUSTICE        = 66863,
    SPELL_HAMMER_JUSTICE_STUN   = 66940,
    SPELL_HAMMER_RIGHTEOUS      = 66867,
    SPELL_HAMMER_OVERRIDE_BAR   = 66904, // overrides players cast bar
    SPELL_HAMMER_THROWBACK_DMG  = 66905, // the hammer that is thrown back by the player
    SPELL_RADIANCE              = 66935,
    SPELL_VENGEANCE             = 66865,

    // Paletress
    SPELL_CONFESSOR_ACHIEVEMENT = 68206,
    SPELL_SMITE                 = 66536,
    SPELL_SMITE_H               = 67674,
    SPELL_HOLY_FIRE             = 66538,
    SPELL_HOLY_FIRE_H           = 67676,
    SPELL_RENEW                 = 66537,
    SPELL_RENEW_H               = 67675,
    SPELL_HOLY_NOVA             = 66546,
    SPELL_SHIELD                = 66515,
    SPELL_CONFESS               = 66680,
    
    //Npc_argent_soldier
    SPELL_STRIKE                = 67237,
    SPELL_CLEAVE                = 15284,
    SPELL_PUMMEL                = 67235,
    SPELL_PAIN                  = 34942,
    SPELL_MIND                  = 67229,
    SPELL_SSMITE                = 67289,
    SPELL_LIGHT_H               = 67290,
    SPELL_LIGHT                 = 67247,
    SPELL_FLURRY                = 67233,
    SPELL_FINAL                 = 67255,
    SPELL_DIVINE                = 67251,

    // Memory  of X (Summon)
    SPELL_OLD_WOUNDS            = 66620,
    SPELL_OLD_WOUNDS_H          = 67679,
    SPELL_SHADOWS_PAST          = 66619,
    SPELL_SHADOWS_PAST_H        = 67678,
    SPELL_WAKING_NIGHTMARE      = 66552,
    SPELL_WAKING_NIGHTMARE_H    = 67677
};
enum Misc
{
    ACHIEV_FACEROLLER           = 3803,
    ACHIEV_CONF                 = 3802
};

enum Text
{
    SAY_MEM_DIE                             = 1,
    SAY_DEATH_P                             = 2,
    SAY_INTRO_P2                            = 3,
    SAY_INTRO_P1                            = 4,
    SAY_INTRO_E                             = 5,
    SAY_HAMMER_E                            = 6,
    SAY_DEATH_E                             = 7,
    SAY_START_E                             = 8,
    SAY_KILL1_P                             = 9,
    SAY_KILL2_P                             = 10,
    SAY_KILL1_E                             = 11,
    SAY_KILL2_E                             = 12,
    SAY_START_10                            = 13,
    SAY_START_8                             = 14,
    SAY_START_P                             = 15,
    SAY_START_7                             = 16,
    SAY_START_6                             = 17
};

class OrientationCheck
{
    public:
        explicit OrientationCheck(Unit* _caster) : caster(_caster) { }
        bool operator() (WorldObject* object)
        {
            return !object->isInFront(caster, 40.0f) || !object->IsWithinDist(caster, 40.0f);
        }
    private:
        Unit* caster;
};

class spell_eadric_radiance : public SpellScriptLoader
{
    public:
        spell_eadric_radiance() : SpellScriptLoader("spell_eadric_radiance") { }
        class spell_eadric_radiance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_radiance_SpellScript);
            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(OrientationCheck(GetCaster()));
            }
            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };
		
        SpellScript *GetSpellScript() const override
        {
            return new spell_eadric_radiance_SpellScript();
        }
};

class spell_eadric_hoj : public SpellScriptLoader
{
    public:
        spell_eadric_hoj() : SpellScriptLoader("spell_eadric_hoj") { }
        class spell_eadric_hoj_SpellScript: public SpellScript
        {
            PrepareSpellScript(spell_eadric_hoj_SpellScript);
            void HandleOnHit()
            {
                if (GetHitUnit() && GetHitUnit()->GetTypeId() == TYPEID_PLAYER)
                    if (!GetHitUnit()->HasAura(SPELL_HAMMER_JUSTICE_STUN)) // FIXME: Has Catched Hammer...
                    {
                        SetHitDamage(0);
                        GetHitUnit()->AddAura(SPELL_HAMMER_OVERRIDE_BAR, GetHitUnit());
                    }

            }

            void Register()
            {
                OnHit += SpellHitFn(spell_eadric_hoj_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eadric_hoj_SpellScript();
        }
};


class boss_eadric : public CreatureScript
{
    public:
        boss_eadric(): CreatureScript("boss_eadric") {}

    struct boss_eadricAI : public BossAI
    {
        boss_eadricAI(Creature* creature) : BossAI(creature,BOSS_ARGENT_CHALLENGE_E)
        {
		    Initialize();
            instance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
            Talk(SAY_INTRO_E);
            creature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);

            hasBeenInCombat=false;
            bCredit = false;
        }
		
		void Initialize()
        {
            uiVenganceTimer = 10000;
            uiRadianceTimer = 16000;
            uiHammerJusticeTimer = 25000;
            uiResetTimer = 5000;

            bDone = false;
        }

        InstanceScript* instance;

        uint32 uiVenganceTimer;
        uint32 uiRadianceTimer;
        uint32 uiHammerJusticeTimer;
        uint32 uiResetTimer;

        bool bDone;
        bool hasBeenInCombat;
        bool bCredit;

        void Reset() override
        {
		    Initialize();
            Map* pMap = me->GetMap();
            if (hasBeenInCombat && pMap && pMap->IsDungeon())
            {
                Map::PlayerList const &players = pMap->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                     if(itr->GetSource() && itr->GetSource()->IsAlive() && !itr->GetSource()->IsGameMaster())
                        return; //se almeno un player ? vivo, esce						
                }
                
                if(instance)
                {
                   GameObject* GO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1));
                   if(GO)
                      instance->HandleGameObject(GO->GetGUID(),true);
                   Creature* announcer=pMap->GetCreature(instance->GetGuidData(DATA_ANNOUNCER));
                   instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED,0);
                   announcer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                 }
                 me->RemoveFromWorld();

                 //ResetEncounter();
            }
        }

        void DamageTaken(Unit* /*who*/, uint32& damage) override
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                if (!bCredit)
                {
                    bCredit = true;
                    HandleSpellOnPlayersInInstanceToC5(me, 68575);
                }
                EnterEvadeMode();
                me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                Talk(SAY_DEATH_E);
                me->setFaction(35);
                bDone = true;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                        instance->HandleGameObject(pGO->GetGUID(),true);	
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                        instance->HandleGameObject(pGO->GetGUID(),true);		
            instance->SetData(BOSS_ARGENT_CHALLENGE_E, DONE);
            }
        }

        void MovementInform(uint32 MovementType, uint32 Data) override
        {
            if (MovementType != POINT_MOTION_TYPE)
                return;
        }

        void EnterCombat(Unit* pWho) override
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            _EnterCombat();
            me->SetHomePosition(746.843f, 665.000f, 412.339f, 4.670f);
            Talk(SAY_START_E);
            hasBeenInCombat = true;
        }
    	
        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (IsHeroic() && !bDone)
                if (caster->GetTypeId() == TYPEID_PLAYER)
                    if (spell->Id == SPELL_HAMMER_THROWBACK_DMG && me->GetHealth() <= spell->Effects[0].BasePoints)
                        DoCast(caster, SPELL_EADRIC_ACHIEVEMENT);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bDone && uiResetTimer <= uiDiff)
            {
                me->GetMotionMaster()->MovePoint(0,746.843f, 695.68f, 412.339f);
                bDone = false;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(pGO->GetGUID(),false);
            } else uiResetTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            if (uiHammerJusticeTimer <= uiDiff)
            {
                me->InterruptNonMeleeSpells(true);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                {
                    if (target && target->IsAlive())
                    {
                        Talk(SAY_HAMMER_E);
                        DoCast(target, SPELL_HAMMER_JUSTICE);
                        DoCast(target, SPELL_HAMMER_RIGHTEOUS);
                    }
                }
                uiHammerJusticeTimer = 25000;
            } else uiHammerJusticeTimer -= uiDiff;

            if (uiVenganceTimer <= uiDiff)
            {
                DoCast(me,SPELL_VENGEANCE);

                uiVenganceTimer = 10000;
            } else uiVenganceTimer -= uiDiff;

            if (uiRadianceTimer <= uiDiff)
            {
                DoCastAOE(SPELL_RADIANCE);

                uiRadianceTimer = 16000;
            } else uiRadianceTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetInstanceAI<boss_eadricAI>(creature);
    };
};

class boss_paletress : public CreatureScript
{
    public:
        boss_paletress(): CreatureScript("boss_paletress") {}

    struct boss_paletressAI : public BossAI
    {
        boss_paletressAI(Creature* creature) : BossAI(creature,BOSS_ARGENT_CHALLENGE_P)
        {
		    Initialize();
            pInstance = creature->GetInstanceScript();

            hasBeenInCombat = false;
            bCredit = false;
            Talk(SAY_INTRO_P2);
            creature->SetReactState(REACT_PASSIVE);
            creature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            creature->RestoreFaction();
        }
		
		void Initialize()
        {
            uiHolyFireTimer     = urand(9000,12000);
            uiHolySmiteTimer    = urand(5000,7000);
            uiRenewTimer        = urand(2000,5000);

            uiResetTimer        = 7000;

            bHealth = false;
            bDone = false;
        }

        InstanceScript* pInstance;

        Creature* pMemory;
        ObjectGuid  MemoryGUID;

        bool bHealth;
        bool bDone;
        bool hasBeenInCombat;
        bool bCredit;

        uint32 uiHolyFireTimer;
        uint32 uiHolySmiteTimer;
        uint32 uiRenewTimer;
        uint32 uiResetTimer;

        void Reset() override
        {
		    Initialize();
            me->RemoveAllAuras();
		
            if (Creature* pMemory = ObjectAccessor::GetCreature(*me, MemoryGUID))
                if (pMemory->IsAlive())
                    pMemory->RemoveFromWorld();

            Map* pMap = me->GetMap();
            if (hasBeenInCombat && pMap && pMap->IsDungeon())
            {
                Map::PlayerList const &players = pMap->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    if(itr->GetSource() && itr->GetSource()->IsAlive() && !itr->GetSource()->IsGameMaster())
                       return; //se almeno un player ? vivo, esce						
                }
    			 
                if(pInstance)
                {
                   GameObject* GO = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(DATA_MAIN_GATE1));
                   if(GO)
                      pInstance->HandleGameObject(GO->GetGUID(),true);
                   Creature* announcer = pMap->GetCreature(pInstance->GetGuidData(DATA_ANNOUNCER));
                   pInstance->SetData(DATA_ARGENT_SOLDIER_DEFEATED,0);
                   announcer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }

                me->RemoveFromWorld();
                //ResetEncounter();
            }

        }
	    void EnterCombat(Unit* pWho) override
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            _EnterCombat();
            me->SetHomePosition(746.843f, 665.000f, 412.339f, 4.670f);
            hasBeenInCombat = true;
            Talk(SAY_START_10);		
        }

        void SetData(uint32 uiId, uint32 uiValue) override
        {
            if (uiId == 1)
                me->RemoveAura(SPELL_SHIELD);
                Talk(SAY_MEM_DIE);
        }

        void DamageTaken(Unit* /*who*/, uint32& damage) override
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                if (!bCredit)
                {
                    bCredit = true;
                    HandleSpellOnPlayersInInstanceToC5(me, 68574);
                }
                EnterEvadeMode();
                me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                Talk(SAY_DEATH_P);
                me->setFaction(35);
                bDone = true;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(DATA_MAIN_GATE)))
                        pInstance->HandleGameObject(pGO->GetGUID(),true);	
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(DATA_MAIN_GATE1)))
                        pInstance->HandleGameObject(pGO->GetGUID(),true);		
                pInstance->SetData(BOSS_ARGENT_CHALLENGE_P, DONE);
            }
        }

        void MovementInform(uint32 MovementType, uint32 Data) override
        {
            if (MovementType != POINT_MOTION_TYPE)
                return;	

        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bDone && uiResetTimer <= uiDiff)
            {
                me->GetMotionMaster()->MovePoint(0, 746.843f, 695.68f, 412.339f);
                bDone = false;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(DATA_MAIN_GATE)))
                    pInstance->HandleGameObject(pGO->GetGUID(),false);	
            } else uiResetTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            if (uiHolyFireTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target,DUNGEON_MODE(SPELL_HOLY_FIRE,SPELL_HOLY_FIRE_H));
                }
                 if (me->HasAura(SPELL_SHIELD))
                    uiHolyFireTimer = 13000;
                else
                    uiHolyFireTimer = urand(9000,12000);
            } else uiHolyFireTimer -= uiDiff;

            if (uiHolySmiteTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target,DUNGEON_MODE(SPELL_SMITE,SPELL_SMITE_H));
                }
                if (me->HasAura(SPELL_SHIELD))
                    uiHolySmiteTimer = 9000;
                else
                    uiHolySmiteTimer = urand(5000,7000);
            } else uiHolySmiteTimer -= uiDiff;

            if (me->HasAura(SPELL_SHIELD))
            {
                if (uiRenewTimer <= uiDiff)
                {
                    me->InterruptNonMeleeSpells(true);
                    uint8 uiTarget = urand(0,1);
                    switch(uiTarget)
                    {
                        case 0:
                            DoCast(me,DUNGEON_MODE(SPELL_RENEW,SPELL_RENEW_H));
                            break;
                        case 1:
                            if (Creature* pMemory = ObjectAccessor::GetCreature(*me, MemoryGUID))
                                if (pMemory->IsAlive())
                                    DoCast(pMemory, DUNGEON_MODE(SPELL_RENEW,SPELL_RENEW_H));
                            break;
                    }
                    uiRenewTimer = urand(15000,17000);
                } else uiRenewTimer -= uiDiff;
            }

            if (!bHealth && me->GetHealth()*100 / me->GetMaxHealth() <= 35)
            {
                Talk(SAY_START_6);
                me->InterruptNonMeleeSpells(true);
                DoCastAOE(SPELL_HOLY_NOVA,false);
                DoCast(me, SPELL_SHIELD);
                DoCastAOE(SPELL_CONFESS,false);

                bHealth = true;
                switch(urand(0, 24))
                {
                    case 0: me->SummonCreature(MEMORY_ALGALON, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 1: me->SummonCreature(MEMORY_CHROMAGGUS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 2: me->SummonCreature(MEMORY_CYANIGOSA, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 3: me->SummonCreature(MEMORY_DELRISSA, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 4: me->SummonCreature(MEMORY_ECK, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 5: me->SummonCreature(MEMORY_ENTROPIUS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 6: me->SummonCreature(MEMORY_GRUUL, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 7: me->SummonCreature(MEMORY_HAKKAR, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 8: me->SummonCreature(MEMORY_HEIGAN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 9: me->SummonCreature(MEMORY_HEROD, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 10: me->SummonCreature(MEMORY_HOGGER, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 11: me->SummonCreature(MEMORY_IGNIS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 12: me->SummonCreature(MEMORY_ILLIDAN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 13: me->SummonCreature(MEMORY_INGVAR, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 14: me->SummonCreature(MEMORY_KALITHRESH, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 15: me->SummonCreature(MEMORY_LUCIFRON, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 16: me->SummonCreature(MEMORY_MALCHEZAAR, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 17: me->SummonCreature(MEMORY_MUTANUS, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 18: me->SummonCreature(MEMORY_ONYXIA, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 19: me->SummonCreature(MEMORY_THUNDERAAN, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 20: me->SummonCreature(MEMORY_VANCLEEF, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 21: me->SummonCreature(MEMORY_VASHJ, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 22: me->SummonCreature(MEMORY_VEKNILASH, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 23: me->SummonCreature(MEMORY_VEZAX, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break;
                    case 24: me->SummonCreature(MEMORY_ARCHIMONDE, 0.0f, 0.0f, 0.0f, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                    break; 
                }
            }

            DoMeleeAttackIfReady();
        }

        void JustSummoned(Creature* summon) override
        {
            MemoryGUID = summon->GetGUID();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return GetInstanceAI<boss_paletressAI>(creature);
    };
};

class npc_memory : public CreatureScript
{
    public:
        npc_memory(): CreatureScript("npc_memory") {}

    struct npc_memoryAI : public ScriptedAI
    {
        npc_memoryAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }

        void Initialize()
        {
            uiOldWoundsTimer = 12000;
            uiShadowPastTimer = 5000;
            uiWakingNightmare = 7000;
        }

        uint32 uiOldWoundsTimer;
        uint32 uiShadowPastTimer;
        uint32 uiWakingNightmare;

        void Reset() override
        {
            Initialize();
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiOldWoundsTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_OLD_WOUNDS,SPELL_OLD_WOUNDS_H));
                }
                uiOldWoundsTimer = 23000;
            }else uiOldWoundsTimer -= uiDiff;

            if (uiWakingNightmare <= uiDiff)
            {
                DoCast(me, DUNGEON_MODE(SPELL_WAKING_NIGHTMARE,SPELL_WAKING_NIGHTMARE_H));
                uiWakingNightmare = 15000;
            }else uiWakingNightmare -= uiDiff;

            if (uiShadowPastTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,1))
                {
                    if (target && target->IsAlive())
                        DoCast(target,DUNGEON_MODE(SPELL_SHADOWS_PAST,SPELL_SHADOWS_PAST_H));
                }
                uiShadowPastTimer = 20000;
            }else uiShadowPastTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* killer) override
        {
            if (TempSummon* summ = me->ToTempSummon())
                if (Unit* summoner = summ->GetSummoner())
                    if (summoner->IsAlive())
                        summoner->GetAI()->SetData(1, 0);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
		return GetInstanceAI<npc_memoryAI>(creature);
    };
};

// THIS AI NEEDS MORE IMPROVEMENTS
class npc_argent_soldier : public CreatureScript
{
    public:
        npc_argent_soldier(): CreatureScript("npc_argent_soldier") {}

    struct npc_argent_soldierAI : public npc_escortAI
    {
        npc_argent_soldierAI(Creature* creature) : npc_escortAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NOT_SELECTABLE);
            if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(DATA_MAIN_GATE)))
                pInstance->HandleGameObject(pGO->GetGUID(),true);    					
            SetDespawnAtEnd(false);
            uiWaypoint = 0;
            bStarted = false;
        }
		
		void Initialize()
        {
            uiStrikeTimer = 5000;	
            uiCleaveTimer = 6000;
            uiPummelTimer = 10000;
            uiPainTimer = 60000;
            uiMindTimer = 70000;
            uiSsmiteTimer = 6000;
    	
            uiLightTimer = 3000;
   	        uiFlurryTimer = 6000;
            uiFinalTimer = 30000;
            uiDivineTimer = 70000;
        }

        InstanceScript* pInstance;

        uint8 uiWaypoint;
    	
        uint32 uiStrikeTimer;
        uint32 uiCleaveTimer;
        uint32 uiPummelTimer;
        uint32 uiPainTimer;
        uint32 uiMindTimer;
        uint32 uiSsmiteTimer;
    	
        uint32 uiLightTimer;
        uint32 uiFlurryTimer;
        uint32 uiFinalTimer;
        uint32 uiDivineTimer;

        bool bStarted;

        void Reset() override
        {
            Initialize();
            if (bStarted)
            {
                me->SetReactState(REACT_AGGRESSIVE);					
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
            }
        }
    	
        void WaypointReached(uint32 uiPoint) override
        {
            if (uiPoint == 0)
            {
                switch(uiWaypoint)
                {
                    case 1:
                        me->SetFacingTo(4.60f);
                        me->SetReactState(REACT_AGGRESSIVE);					
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                        bStarted = true;
                        break;
                }
            }	
            if (uiPoint == 1)
            {
                switch(uiWaypoint)
                {
                    case 0:
                        me->SetFacingTo(5.81f);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                        bStarted = true;
                        break;
                    case 2:
                        me->SetFacingTo(3.39f);
                        me->SetReactState(REACT_AGGRESSIVE);					
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                        bStarted = true;
                        if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(DATA_MAIN_GATE)))
                            pInstance->HandleGameObject(pGO->GetGUID(),false);					
                        break;
    			
                }
                me->SendMovementFlagUpdate();
            
            }  
        }

        void SetData(uint32 uiType, uint32 uiData) override
        {
            switch(me->GetEntry())
            {
                case NPC_ARGENT_LIGHWIELDER:
                    switch(uiType)
                    {
                        case 0:
                            AddWaypoint(0, 737.14f,655.42f,412.88f);
                            AddWaypoint(1, 712.14f,628.42f,411.88f);
                            break;
                        case 1:
                            AddWaypoint(0, 742.44f, 650.29f, 411.79f);
                            break;
                        case 2:
                            AddWaypoint(0, 756.14f, 655.42f, 411.88f);
                            AddWaypoint(1, 775.912f, 639.033f, 411.907f);
                            break;
                    }
                    break;
                case NPC_ARGENT_MONK:
                    switch(uiType)
                    {
                        case 0:
                            AddWaypoint(0, 737.14f, 655.42f, 412.88f);
                            AddWaypoint(1, 713.12f, 632.97f, 411.90f);
                            break;
                        case 1:
                            AddWaypoint(0, 746.73f, 650.24f, 411.56f);
                            break;
                        case 2:
                            AddWaypoint(0, 756.14f, 655.42f, 411.88f);
                            AddWaypoint(1, 784.817f, 629.883f, 411.908f);
                            break;
                    }
                    break;
                case NPC_PRIESTESS:
                    switch(uiType)
                    {
                        case 0:
                            AddWaypoint(0, 737.14f, 655.42f, 412.88f);
                            AddWaypoint(1, 715.06f, 637.07f, 411.91f);
                            break;
                        case 1:
                            AddWaypoint(0, 750.72f, 650.20f, 411.77f);
                            break;
                        case 2:
                            AddWaypoint(0, 756.14f, 655.42f, 411.88f);
                            AddWaypoint(1, 779.942f, 634.061f, 411.905f);
                            break;
                    }
                    break;
            }

            Start(false, true);
            uiWaypoint = uiType;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            pInstance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, pInstance->GetData(DATA_ARGENT_SOLDIER_DEFEATED) + 1);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetInstanceAI<npc_argent_soldierAI>(creature);
    }
};

void AddSC_boss_argent_challenge()
{
    new boss_eadric();
    new spell_eadric_radiance();
    new boss_paletress();
    new npc_memory();
    new npc_argent_soldier();
}