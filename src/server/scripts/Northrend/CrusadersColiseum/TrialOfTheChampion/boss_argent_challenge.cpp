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

#include "ScriptPCH.h"
#include "trial_of_the_champion.h"
#include "ScriptedEscortAI.h"

enum Spells
{
    // Eadric
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
    SPELL_HOLY_FIRE             = 66538,
    SPELL_RENEW                 = 66537,
    SPELL_HOLY_NOVA             = 66546,
    SPELL_SHIELD                = 66515,
    SPELL_CONFESS               = 66680,
    SPELL_SUMMON_MEMORY         = 66545,
    
    // Monk soldier
    SPELL_PUMMEL                = 67235,
    SPELL_FLURRY                = 67233,
    SPELL_FINAL                 = 67255,
    SPELL_DIVINE                = 67251,
	
    // Lightwielder soldier
    SPELL_LIGHT                 = 67247,
    SPELL_CLEAVE                = 15284,
    SPELL_STRIKE                = 67237,
	
    // Priest soldier
    SPELL_HOLY_SMITE            = 36176,
    SPELL_HOLY_SMITE_H          = 67289,
    SPELL_SHADOW_WORD_PAIN      = 34941,
    SPELL_SHADOW_WORD_PAIN_H    = 34942,
    SPELL_MIND                  = 67229,
    SPELL_FOUNTAIN_OF_LIGHT     = 67194,

    // Memory
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

enum Talk
{
    SAY_ARGENT_ENTERS           = 19,
    SAY_ARGENT_READY            = 20,
    SAY_MEMORY_NIGHTMARE        = 0,

    // Paletress
    SAY_PALETRESS_AGGRO         = 2,
    SAY_PALETRESS_SUMMON_MEMORY = 3,
    SAY_PALETRESS_MEMORY_DIES   = 4,
    SAY_PALETRESS_PLAYER_DIES   = 5,
    SAY_PALETRESS_DEFEATED      = 6,

    // Eadric
    SAY_EADRIC_AGGRO            = 1,
    SAY_EADRIC_RADIATE_LIGHT    = 2,
    SAY_EADRIC_HAMMER_TARGET    = 3,
    SAY_EADRIC_HAMMER           = 4,
    SAY_EADRIC_PLAYER_DIES      = 5,
    SAY_EADRIC_DEFEATED         = 6
};

enum Events
{
    // Eadric the Pure
    EVENT_VENGEANCE            = 1,
    EVENT_RADIANCE             = 2,
    EVENT_HAMMER_OF_JUSTICE    = 3,

    // Argent Confessor Paletress
    EVENT_HOLY_FIRE            = 4,
    EVENT_SMITE                = 5,
    EVENT_RENEW                = 6,

    // Argent Soldiers
    EVENT_CLEAVE               = 7,
    EVENT_STRIKE               = 8,
    EVENT_BLAZINGLIGHT         = 9,
    EVENT_FLURRY               = 10,
    EVENT_PUMMEL               = 11,
    EVENT_MINDCONTROL          = 12,
    EVENT_PAIN                 = 13,
    EVENT_FOUNTAIN             = 14,
    EVENT_HOLYSMITE            = 15
};

enum Data
{
    DATA_THE_FACEROLLER
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

class spell_eadric_radiance: public SpellScriptLoader
{
    public:
        spell_eadric_radiance(): SpellScriptLoader("spell_eadric_radiance") { }

        class spell_eadric_radiance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_radiance_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(OrientationCheck(GetCaster()));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_eadric_radiance_SpellScript();
        }
};

class spell_eadric_hammer_missile: public SpellScriptLoader
{
    public:
        spell_eadric_hammer_missile(): SpellScriptLoader("spell_eadric_hammer_missile") { }

        class spell_eadric_hammer_missile_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_hammer_missile_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HAMMER_OVERRIDE_BAR))
                    return false;

                return true;
            }

            void HandleTriggerMissile(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (caster && target && !target->HasAura(SPELL_HAMMER_JUSTICE_STUN))
                {
                    PreventHitDefaultEffect(EFFECT_0);
                    caster->CastSpell(target, SPELL_HAMMER_OVERRIDE_BAR, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_eadric_hammer_missile_SpellScript::HandleTriggerMissile, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_eadric_hammer_missile_SpellScript();
        }
};

class spell_eadric_hammer_throw_back: public SpellScriptLoader
{
    public:
        spell_eadric_hammer_throw_back(): SpellScriptLoader("spell_eadric_hammer_throw_back") { }

        class spell_eadric_hammer_throw_back_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_hammer_throw_back_SpellScript);

            void RemoveAura()
            {
                GetCaster()->RemoveAurasDueToSpell(SPELL_HAMMER_OVERRIDE_BAR);
            }

            void CheckDamage()
            {
                Creature* target = GetHitCreature();
                if (!target)
                    return;

                if (GetHitDamage() >= int32(target->GetHealth()))
                    target->AI()->SetData(DATA_THE_FACEROLLER, 1);
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_eadric_hammer_throw_back_SpellScript::RemoveAura);
                OnHit += SpellHitFn(spell_eadric_hammer_throw_back_SpellScript::CheckDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_eadric_hammer_throw_back_SpellScript();
        }
};

class boss_eadric : public CreatureScript
{
    public:
        boss_eadric(): CreatureScript("boss_eadric") { }

    struct boss_eadricAI : public BossAI
    {
        boss_eadricAI(Creature* creature) : BossAI(creature, BOSS_ARGENT_CHALLENGE_E)
        {
            Initialize();
            instance = creature->GetInstanceScript();
        }

        void Initialize()
        {
            uiResetTimer = 5000;
            uiBasePoints = 0;
            me->SetReactState(REACT_PASSIVE);
            _theFaceRoller = false;
            bDone = false;
            hasBeenInCombat = false;
            bCredit = false;
        }

        void Reset() override
        {
            Initialize();
            Map* map = me->GetMap();
            if (hasBeenInCombat && map && map->IsDungeon())
            {
                Map::PlayerList const &players = map->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                     if (itr->GetSource() && itr->GetSource()->IsAlive() && !itr->GetSource()->IsGameMaster())
                         return;
                }
                
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(gate->GetGUID(), true);

                if (Creature* announcer = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ANNOUNCER)))
                    announcer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, 0);
                me->RemoveFromWorld();
            }
        }

        void DamageTaken(Unit* /*who*/, uint32& damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                HandleSpellOnPlayersInInstanceToC5(me, 68575);
                EnterEvadeMode();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                Talk(SAY_EADRIC_DEFEATED);
                me->setFaction(35);
                bDone = true;
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(gate->GetGUID(),true);
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(gate->GetGUID(),true);
                instance->SetData(BOSS_ARGENT_CHALLENGE_E, DONE);
                HandleInstanceBind(me);
            }
        }

        void MovementInform(uint32 MovementType, uint32 /*Data*/) override
        {
            if (MovementType != POINT_MOTION_TYPE)
                return;
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_RADIANCE, 16000);
            events.ScheduleEvent(EVENT_VENGEANCE, 10000);
            events.ScheduleEvent(EVENT_HAMMER_OF_JUSTICE, 25000);

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            _EnterCombat();
            me->SetHomePosition(746.843f, 665.000f, 412.339f, 4.670f);
            Talk(SAY_EADRIC_AGGRO);
            hasBeenInCombat = true;
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (IsHeroic() && !bDone && spell->Id == SPELL_HAMMER_THROWBACK_DMG && caster->GetTypeId() == TYPEID_PLAYER)
            {            
                uiBasePoints = spell->Effects[0].BasePoints;
                if (me->GetHealth() <= uiBasePoints)
                {
                    _theFaceRoller = true;
                    HandleSpellOnPlayersInInstanceToC5(me, SPELL_EADRIC_ACHIEVEMENT);
                }
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_THE_FACEROLLER)
                return _theFaceRoller;

            return 0;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bDone && uiResetTimer <= uiDiff)
            {
                me->GetMotionMaster()->MovePoint(0,746.843f, 695.68f, 412.339f);
                bDone = false;
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(gate->GetGUID(),false);
            } else uiResetTimer -= uiDiff;
            
            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RADIANCE:
                        DoCastAOE(SPELL_RADIANCE);
                        Talk(SAY_EADRIC_RADIATE_LIGHT);
                        events.ScheduleEvent(EVENT_RADIANCE, 16000);
                        break;
                    case EVENT_VENGEANCE:
                        DoCast(me, SPELL_VENGEANCE);
                        events.ScheduleEvent(EVENT_VENGEANCE, 10000);
                        break;
                    case EVENT_HAMMER_OF_JUSTICE:
                        me->InterruptNonMeleeSpells(true);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                        {
                            Talk(SAY_EADRIC_HAMMER);
                            Talk(SAY_EADRIC_HAMMER_TARGET, target);
                            DoCast(target, SPELL_HAMMER_JUSTICE);
                            DoCast(target, SPELL_HAMMER_RIGHTEOUS);
                        }
                        events.ScheduleEvent(EVENT_HAMMER_OF_JUSTICE, 25000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        EventMap events;
        InstanceScript* instance;
        uint32 uiResetTimer;
        uint64 uiBasePoints;
        bool bDone;
        bool hasBeenInCombat;
        bool bCredit;
        bool _theFaceRoller;

    };


    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<boss_eadricAI>(creature);
    };
};

class boss_paletress : public CreatureScript
{
    public:
        boss_paletress(): CreatureScript("boss_paletress") { }

    struct boss_paletressAI : public BossAI
    {
        boss_paletressAI(Creature* creature) : BossAI(creature, BOSS_ARGENT_CHALLENGE_P)
        {
            Initialize();
            instance = creature->GetInstanceScript();
        }

        void Initialize()
        {
            hasBeenInCombat = false;
            _hasSummonedMemory = false;
            bCredit = false;
            uiResetTimer = 7000;
            _hasSummonedMemory = false;
            bDone = false;
        }


        void Reset() override
        {
            me->RemoveAllAuras();
            Initialize();
            me->SetReactState(REACT_PASSIVE);

            if (Creature* memory = ObjectAccessor::GetCreature(*me, memoryGUID))
                if (memory->IsAlive())
                    memory->RemoveFromWorld();

            Map* map = me->GetMap();
            if (hasBeenInCombat && map && map->IsDungeon())
            {
                Map::PlayerList const &players = map->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    if (itr->GetSource() && itr->GetSource()->IsAlive() && !itr->GetSource()->IsGameMaster())
                        return;
                }

                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(gate->GetGUID(), true);

                if (Creature* announcer = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ANNOUNCER)))
                    announcer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, 0);
                me->RemoveFromWorld();
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_HOLY_FIRE, urand(9000, 12000));
            events.ScheduleEvent(EVENT_SMITE, urand(5000, 7000));
            events.ScheduleEvent(EVENT_RENEW, urand(2000, 5000));

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            _EnterCombat();
            me->SetHomePosition(746.843f, 665.000f, 412.339f, 4.670f);
            hasBeenInCombat = true;
            Talk(SAY_PALETRESS_AGGRO);
        }

        void SetData(uint32 uiId, uint32 /*uiValue*/) override
        {
            if (uiId == 1)
                me->RemoveAura(SPELL_SHIELD);
                Talk(SAY_PALETRESS_MEMORY_DIES);
        }

        void DamageTaken(Unit* /*who*/, uint32& damage) override
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                HandleSpellOnPlayersInInstanceToC5(me, 68574);
                EnterEvadeMode();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                Talk(SAY_PALETRESS_DEFEATED);
                me->setFaction(35);
                bDone = true;
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(gate->GetGUID(),true);
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(gate->GetGUID(),true);
                instance->SetData(BOSS_ARGENT_CHALLENGE_P, DONE);
                HandleInstanceBind(me);

                if (Creature* memory = ObjectAccessor::GetCreature(*me, memoryGUID))
                    HandleSpellOnPlayersInInstanceToC5(memory, SPELL_CONFESSOR_ACHIEVEMENT);
            }
        }

        void MovementInform(uint32 MovementType, uint32 /*Data*/) override
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
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(gate->GetGUID(),false);
            } else uiResetTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            events.Update(uiDiff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HOLY_FIRE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                            DoCast(target, SPELL_HOLY_FIRE);
                        if (me->HasAura(SPELL_SHIELD))
                            events.ScheduleEvent(EVENT_HOLY_FIRE, 13000);
                        else
                            events.ScheduleEvent(EVENT_HOLY_FIRE, urand(9000, 12000));
                        break;
                    case EVENT_SMITE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                            DoCast(target, SPELL_SMITE);
                        if (me->HasAura(SPELL_SHIELD))
                            events.ScheduleEvent(EVENT_SMITE, 9000);
                        else
                            events.ScheduleEvent(EVENT_SMITE, urand(5000, 7000));
                        break;
                    case EVENT_RENEW:
                        if (me->HasAura(SPELL_SHIELD))
                        {
                            me->InterruptNonMeleeSpells(true);
                            uint8 uiTarget = urand(0, 1);
                            switch (uiTarget)
                            {
                                case 0:
                                    DoCast(me, SPELL_RENEW);
                                    break;
                                case 1:
                                    if (Creature* memory = ObjectAccessor::GetCreature(*me, memoryGUID))
                                        if (memory->IsAlive())
                                            DoCast(memory, SPELL_RENEW);
                                    break;
                            }
                        }
                        events.ScheduleEvent(EVENT_RENEW, 15000, 17000);
                        break;
                    default:
                        break;

                }
            }

            DoMeleeAttackIfReady();

            if (!_hasSummonedMemory && me->HealthBelowPct(25))
            {
                Talk(SAY_PALETRESS_SUMMON_MEMORY);
                me->InterruptNonMeleeSpells(true);
                DoCastAOE(SPELL_HOLY_NOVA, false);
                DoCast(me, SPELL_SHIELD);
                DoCastAOE(SPELL_CONFESS, false);
                DoCast(SPELL_SUMMON_MEMORY);
                _hasSummonedMemory = true;
            }
        }

        void JustSummoned(Creature* summon) override
        {
            memoryGUID = summon->GetGUID();
        }

    private:
        InstanceScript* instance;
        ObjectGuid memoryGUID;
        bool _hasSummonedMemory;
        bool bDone;
        bool hasBeenInCombat;
        bool bCredit;
        uint32 uiResetTimer;

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<boss_paletressAI>(creature);
    };
};

class npc_memory : public CreatureScript
{
    public:
        npc_memory(): CreatureScript("npc_memory") { }

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
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_OLD_WOUNDS, SPELL_OLD_WOUNDS_H));
                }
                uiOldWoundsTimer = 23000;
            } else uiOldWoundsTimer -= uiDiff;

            if (uiWakingNightmare <= uiDiff)
            {
                Talk(SAY_MEMORY_NIGHTMARE);
                DoCast(me, DUNGEON_MODE(SPELL_WAKING_NIGHTMARE, SPELL_WAKING_NIGHTMARE_H));
                uiWakingNightmare = 15000;
            } else uiWakingNightmare -= uiDiff;

            if (uiShadowPastTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_SHADOWS_PAST, SPELL_SHADOWS_PAST_H));
                }
                uiShadowPastTimer = 20000;
            } else uiShadowPastTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (me->IsSummon())
            {
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                {
                    if (summoner && summoner->IsAlive() && summoner->GetTypeId() == TYPEID_UNIT)
                        summoner->ToCreature()->AI()->SetData(1, 0);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_memoryAI>(creature);
    };
};

class npc_argent_soldier : public CreatureScript
{
    public:
        npc_argent_soldier(): CreatureScript("npc_argent_soldier") { }

        struct npc_argent_soldierAI : public npc_escortAI
        {
            npc_argent_soldierAI(Creature* creature) : npc_escortAI(creature)
            {
                Initialize();
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_DEFENSIVE);
                if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(gate->GetGUID(),true);
                SetDespawnAtEnd(false);
            }

            void Initialize()
            {
                _shielded = false;
                _events.Reset();
                uiWaypoint = 0;
            }

            void Reset() override
            {
                Initialize();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                switch (me->GetEntry())
                {
                    case NPC_ARGENT_LIGHWIELDER:
                        _events.ScheduleEvent(EVENT_CLEAVE, urand(4000, 8000));
                        _events.ScheduleEvent(EVENT_BLAZINGLIGHT, urand(7000, 10000));
                        if (IsHeroic())
                            _events.ScheduleEvent(EVENT_STRIKE, urand(9000, 12000));
                        break;
                    case NPC_ARGENT_MONK:
                        _events.ScheduleEvent(EVENT_FLURRY, urand(8000, 12000));
                        _events.ScheduleEvent(EVENT_PUMMEL, urand(5000, 6000));
                        break;
                    case NPC_PRIESTESS:
                        _events.ScheduleEvent(EVENT_PAIN, urand(3000, 6000));
                        _events.ScheduleEvent(EVENT_FOUNTAIN, urand(15000, 20000));
                        _events.ScheduleEvent(EVENT_HOLYSMITE, urand(4000, 8000));
                        if (IsHeroic())
                            _events.ScheduleEvent(EVENT_MINDCONTROL, urand(17000, 25000));
                        break;
                }
            }

            void WaypointReached(uint32 uiPoint) override
            {
                if (uiPoint == 0)
                {
                    switch (uiWaypoint)
                    {
                        case 0:
                            me->SetOrientation(5.81f);
                            break;
                        case 1:
                            me->SetOrientation(4.60f);
                            break;
                        case 2:
                            me->SetOrientation(2.79f);
                            break;
                    }

                    me->SendMovementFlagUpdate();
                }
            }

            void SetData(uint32 uiType, uint32 /*uiData*/) override
            {
                switch (me->GetEntry())
                {
                    case NPC_ARGENT_LIGHWIELDER:
                        switch (uiType)
                        {
                            case 0:
                                AddWaypoint(0, 712.14f, 628.42f, 411.88f);
                                break;
                            case 1:
                                AddWaypoint(0, 742.44f, 650.29f, 411.79f);
                                break;
                            case 2:
                                AddWaypoint(0, 783.33f, 615.29f, 411.84f);
                                break;
                        }
                        break;
                    case NPC_ARGENT_MONK:
                        switch (uiType)
                        {
                            case 0:
                                AddWaypoint(0, 713.12f, 632.97f, 411.90f);
                                break;
                            case 1:
                                AddWaypoint(0, 746.73f, 650.24f, 411.56f);
                                break;
                            case 2:
                                AddWaypoint(0, 781.32f, 610.54f, 411.82f);
                                break;
                        }
                        break;
                    case NPC_PRIESTESS:
                        switch (uiType)
                        {
                            case 0:
                                AddWaypoint(0, 715.06f, 637.07f, 411.91f);
                                break;
                            case 1:
                                AddWaypoint(0, 750.72f, 650.20f, 411.77f);
                                break;
                            case 2:
                                AddWaypoint(0, 779.77f, 607.03f, 411.81f);
                                break;
                        }
                        break;
                }

                Start(false, true);
                uiWaypoint = uiType;
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (!IsHeroic() || _shielded || me->GetEntry() != NPC_ARGENT_MONK)
                    return;

                if (damage >= me->GetHealth())
                {
                    DoCast(me, SPELL_FINAL);
                    DoCast(me, SPELL_DIVINE, true);
                    me->SetHealth(1);
                    damage = 0;
                    _shielded = true;
                }
            }

            void UpdateAI(uint32 uiDiff) override
            {
                npc_escortAI::UpdateAI(uiDiff);

                if (!UpdateVictim())
                    return;

                _events.Update(uiDiff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLEAVE:
                            DoCastVictim(SPELL_CLEAVE);
                            _events.ScheduleEvent(EVENT_CLEAVE, urand(5000, 8000));
                            break;
                        case EVENT_STRIKE:
                            DoCastVictim(SPELL_STRIKE);
                            _events.ScheduleEvent(EVENT_STRIKE, urand(10000, 12000));
                            break;
                        case EVENT_BLAZINGLIGHT:
                            DoCast(me, SPELL_LIGHT);
                            _events.ScheduleEvent(EVENT_BLAZINGLIGHT, urand(9000, 13000));
                            break;
                        case EVENT_FLURRY:
                            DoCast(me, SPELL_FLURRY);
                            _events.ScheduleEvent(EVENT_FLURRY, urand(13000, 15000));
                            break;
                        case EVENT_PUMMEL:
                            DoCastVictim(SPELL_PUMMEL);
                            _events.ScheduleEvent(EVENT_PUMMEL, urand(4000, 7000));
                            break;
                        case EVENT_HOLYSMITE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f))
                                DoCast(target, DUNGEON_MODE(SPELL_HOLY_SMITE, SPELL_HOLY_SMITE_H));
                            _events.ScheduleEvent(EVENT_HOLYSMITE, urand(5000, 7000));
                            break;
                        case EVENT_MINDCONTROL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 30.0f))
                                DoCast(target, SPELL_MIND);
                            _events.ScheduleEvent(EVENT_MINDCONTROL, urand(20000, 25000));
                            break;
                        case EVENT_PAIN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, false, DUNGEON_MODE(SPELL_SHADOW_WORD_PAIN, SPELL_SHADOW_WORD_PAIN_H)))
                                DoCast(target, DUNGEON_MODE(SPELL_SHADOW_WORD_PAIN, SPELL_SHADOW_WORD_PAIN_H));
                            _events.ScheduleEvent(EVENT_PAIN, urand(7000, 90000));
                            break;
                        case EVENT_FOUNTAIN:
                            DoCast(SPELL_FOUNTAIN_OF_LIGHT);
                            _events.ScheduleEvent(EVENT_FOUNTAIN, urand(20000, 30000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*killer*/) override
            {
                instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, instance->GetData(DATA_ARGENT_SOLDIER_DEFEATED) + 1);
            }

        private:
            InstanceScript* instance;
            EventMap _events;
            bool _shielded;
            uint8 uiWaypoint;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetTrialOfTheChampionAI<npc_argent_soldierAI>(creature);
        }
};

enum ReflectiveShield
{
    SPELL_REFLECTIVE_SHIELD_TRIGGERED = 33619
};

class spell_gen_reflective_shield : public SpellScriptLoader
{
public:
    spell_gen_reflective_shield() : SpellScriptLoader("spell_gen_reflective_shield") { }

    class spell_gen_reflective_shield_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gen_reflective_shield_AuraScript);

        bool Validate(SpellInfo const* /*spell*/) override
        {
            if (!sSpellStore.LookupEntry(SPELL_REFLECTIVE_SHIELD_TRIGGERED))
                return false;

            return true;
        }

        void Trigger(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            Unit * target = dmgInfo.GetAttacker();
            if (!target)
                return;
            Unit * caster = GetCaster();
            if (!caster)
                return;
            int32 bp = CalculatePct(absorbAmount, 25);
            target->CastCustomSpell(target, SPELL_REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
        }

        void Register() override
        {
             AfterEffectAbsorb += AuraEffectAbsorbFn(spell_gen_reflective_shield_AuraScript::Trigger, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const override
    {
         return new spell_gen_reflective_shield_AuraScript();
    }
};

class achievement_toc5_argent_challenge : public AchievementCriteriaScript
{
public:
    uint32 creature_entry;

    achievement_toc5_argent_challenge(const char* name, uint32 original_entry) : AchievementCriteriaScript(name)
    {
        creature_entry = original_entry;
    }

    bool OnCheck(Player* /*source*/, Unit* target) override
    {
        if (!target)
            return false;

        if (Creature* creature = target->ToCreature())
            if (creature->GetOriginalEntry() == creature_entry)
                return true;

        return false;
    }
};

class achievement_toc5_argent_confessor : public AchievementCriteriaScript
{
public:
    uint32 creature_entry;

    achievement_toc5_argent_confessor(const char* name, uint32 original_entry) : AchievementCriteriaScript(name) 
    {
        creature_entry = original_entry;
    }

    bool OnCheck(Player* /*source*/, Unit* target) override
    {
        if (!target)
            return false;

        if (Creature* creature = target->ToCreature())
            if (creature->GetEntry() == creature_entry && creature->GetMap()->ToInstanceMap()->IsHeroic())
                return true;

        return false;
    }
};

class achievement_toc5_the_faceroller : public AchievementCriteriaScript
{
public:
    achievement_toc5_the_faceroller(const char* name) : AchievementCriteriaScript(name) { }

    bool OnCheck(Player* /*source*/, Unit* target) override
    {
        if (target && target->GetMap()->ToInstanceMap()->IsHeroic())
            return target->GetAI()->GetData(DATA_THE_FACEROLLER);

        return false;
    }
};

class spell_light_rain: public SpellScriptLoader
{
public:
    spell_light_rain(): SpellScriptLoader("spell_light_rain") { }

    class spell_light_rain_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_light_rain_SpellScript);

        void SelectTarget(std::list<WorldObject*>& unitList)
        {
            if (unitList.empty())
                return;

            unitList.sort(Trinity::HealthPctOrderPred());
            unitList.resize(1);
        }

        void Register() override
        {
             OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_light_rain_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_light_rain_SpellScript();
    }
};

void AddSC_boss_argent_challenge()
{
    new boss_eadric();
    new boss_paletress();
    new npc_memory();
    new npc_argent_soldier();
    new spell_gen_reflective_shield();
    new spell_light_rain();
    new spell_eadric_radiance();
    new spell_eadric_hammer_missile();
    new spell_eadric_hammer_throw_back();
    new achievement_toc5_argent_challenge("achievement_toc5_paletress", NPC_PALETRESS);
    new achievement_toc5_argent_challenge("achievement_toc5_eadric", NPC_EADRIC);
    
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_hogger", MEMORY_HOGGER);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_vancleef", MEMORY_VANCLEEF);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_mutanus", MEMORY_MUTANUS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_herod", MEMORY_HEROD);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_lucifron", MEMORY_LUCIFRON);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_thunderaan", MEMORY_THUNDERAAN);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_chromaggus", MEMORY_CHROMAGGUS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_hakkar", MEMORY_HAKKAR);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_veknilash", MEMORY_VEKNILASH);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_kalithresh", MEMORY_KALITHRESH);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_malchezar", MEMORY_MALCHEZAAR);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_gruul", MEMORY_GRUUL);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_vashj", MEMORY_VASHJ);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_archimonde", MEMORY_ARCHIMONDE);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_illidan", MEMORY_ILLIDAN);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_delrissa", MEMORY_DELRISSA);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_muru", MEMORY_ENTROPIUS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_ingvar", MEMORY_INGVAR);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_cyanigosa", MEMORY_CYANIGOSA);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_eck", MEMORY_ECK);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_onyxia", MEMORY_ONYXIA);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_heigan", MEMORY_HEIGAN);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_ignis", MEMORY_IGNIS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_vezax", MEMORY_VEZAX);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_algalon", MEMORY_ALGALON);

    new achievement_toc5_the_faceroller("achievement_toc5_the_faceroller");
}
