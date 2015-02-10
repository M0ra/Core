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

#include "ScriptPCH.h"
#include "trial_of_the_champion.h"
#include "ScriptedEscortAI.h"

enum Spells
{
    // Eadric
    SPELL_EADRIC_ACHIEVEMENT    = 68197,
    SPELL_HAMMER_JUSTICE        = 66863,
    SPELL_HAMMER_STUN           = 66940,
    SPELL_HAMMER_SCRIPT_EFFECT  = 66941,
    SPELL_HAMMER_RIGHTEOUS      = 66867,
    SPELL_HAMMER_CATCH          = 66904,
    SPELL_RADIANCE              = 66935,
    SPELL_VENGEANCE             = 66865,

    // Paletress
    SPELL_SMITE                 = 66536,
    SPELL_SMITE_H               = 67674,
    SPELL_HOLY_FIRE             = 66538,
    SPELL_HOLY_FIRE_H           = 67676,
    SPELL_RENEW                 = 66537,
    SPELL_RENEW_H               = 67675,
    SPELL_HOLY_NOVA             = 66546,
    SPELL_SHIELD                = 66515,
    SPELL_CONFESS_STUN          = 66547,
    SPELL_CONFESS               = 66680,
    SPELL_SUMMON_MEMORY         = 66545,

    // Memory
    SPELL_OLD_WOUNDS            = 66620,
    SPELL_OLD_WOUNDS_H          = 67679,
    SPELL_SHADOWS_PAST          = 66619,
    SPELL_SHADOWS_PAST_H        = 67678,
    SPELL_WAKING_NIGHTMARE      = 66552,
    SPELL_WAKING_NIGHTMARE_H    = 67677,
    SPELL_SPAWN_EFFECT          = 66675,

    // Monk
    SPELL_DIVINE_SHIELD         = 67251,
    SPELL_FINAL_MEDITATION      = 67255,
    SPELL_FLURRY_OF_BLOWS       = 67233,
    SPELL_PUMMEL                = 67235,

    // Priestess
    SPELL_HOLY_SMITE_H          = 67289,
    SPELL_MIND_CONTROL          = 67229,
    SPELL_DOT_PAIN_H            = 34942,
    SPELL_HOLY_SMITE            = 36176,
    SPELL_DOT_PAIN              = 34941,
    SPELL_FOUNTAIN_OF_LIGHT     = 67194,

    // Lightwielder
  //SPELL_BLAZING_LIGHT_H       = 67290,
    SPELL_BLAZING_LIGHT         = 67247,
    SPELL_UNBALANCING_STRIKE    = 67237,
    SPELL_CLEAVE                = 15284,

    SPELL_CONFESSOR_ACHIEVEMENT = 68206,
    SPELL_PALETRESS_CREDIT      = 68574,
    SPELL_EADRIC_CREDIT         = 68575
};

enum Misc
{
    DATA_FACEROLLER = 1
};

enum Enums
{
    SAY_MEM_DIE                 = 1,
    SAY_DEATH_P                 = 2,
    SAY_INTRO_P2                = 3,
    SAY_INTRO_P1                = 4,
    SAY_INTRO_E                 = 5,
    SAY_HAMMER_E                = 6,
    SAY_DEATH_E                 = 7,
    SAY_START_E                 = 8,
    SAY_KILL1_P                 = 9,
    SAY_KILL2_P                 = 10,
    SAY_KILL1_E                 = 11,
    SAY_KILL2_E                 = 12,
    SAY_START_10                = 13,
    SAY_START_9                 = 14,
    SAY_START_8                 = 15,
    SAY_START_P                 = 16,
    SAY_START_7                 = 17,
    SAY_START_6                 = 18
};

enum Events
{
    // Eadric
    EVENT_HAMMER = 1,
    EVENT_RADIANCE,
    EVENT_VENGEANCE,
    // Paletress
    EVENT_HOLYFIRE,
    EVENT_HOLYSMITE,
    EVENT_RENEW,
    EVENT_CONFESS,
    EVENT_MEMORY,
    // Memory
    EVENT_OLDWOUNDS,
    EVENT_SHADOWS,
    EVENT_NIGHTMARE,
    // Argent Soldiers
    EVENT_CLEAVE,
    EVENT_STRIKE,
    EVENT_BLAZINGLIGHT,
    EVENT_FLURRY,
    EVENT_PUMMEL,
    EVENT_MINDCONTROL,
    EVENT_PAIN,
    EVENT_FOUNTAIN
};

class boss_eadric : public CreatureScript
{
    public:
        boss_eadric() : CreatureScript("boss_eadric") { }

        struct boss_eadricAI : public ScriptedAI
        {
            boss_eadricAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = creature->GetInstanceScript();
                _faceroller = false;
                _resetTimer = 5000;
                _done = false;
                creature->SetReactState(REACT_PASSIVE);
                creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                _events.Reset();
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    _done = true;
                    damage = 0;
                    EnterEvadeMode();
                    me->setFaction(35);
                    Talk(SAY_DEATH_E);

                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_MAIN_GATE)))
                        _instance->HandleGameObject(go->GetGUID(), true);
                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_PORTCULLIS)))
                        _instance->HandleGameObject(go->GetGUID(), true);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (_done)
                    return;

                DoZoneInCombat(me, 150.0f);
                Talk(SAY_START_E);

                if (_instance)
                    _instance->SetData(BOSS_ARGENT_CHALLENGE_E, IN_PROGRESS);

                _events.ScheduleEvent(EVENT_HAMMER, urand(10000, 13000));
                _events.ScheduleEvent(EVENT_RADIANCE, urand(5000, 7000));
                _events.ScheduleEvent(EVENT_VENGEANCE, urand(12000, 15000));
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(urand(0, 1) ? SAY_KILL1_E : SAY_KILL2_E);
            }

            void SetData(uint32 type, uint32 /*value*/)
            {
                if (type == DATA_FACEROLLER)
                    _faceroller = true;
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_HAMMER_SCRIPT_EFFECT)
                    DoCast(target, SPELL_HAMMER_RIGHTEOUS);
            }

            void UpdateAI(uint32 const diff)
            {
                if (_done)
                    if (_resetTimer <= diff)
                    {
                        if (_instance)
                            _instance->SetData(BOSS_ARGENT_CHALLENGE_E, DONE);

                        me->DespawnOrUnsummon(1000);

                        if (_faceroller)
                            DoCast(me, SPELL_EADRIC_ACHIEVEMENT, true);

                        DoCast(me, SPELL_ARGENT_CREDIT, true);
                        DoCast(me, SPELL_EADRIC_CREDIT, true);
                        _resetTimer = 5000;
                    }
                    else
                        _resetTimer -= diff;

                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HAMMER:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250.0f, true))
                                DoCast(target, SPELL_HAMMER_JUSTICE);
                            _events.ScheduleEvent(EVENT_HAMMER, urand(10000, 15000));
                            break;
                        case EVENT_RADIANCE:
                            DoCastAOE(SPELL_RADIANCE);
                            _events.ScheduleEvent(EVENT_RADIANCE, urand(14000, 18000));
                            break;
                        case EVENT_VENGEANCE:
                            DoCast(SPELL_VENGEANCE);
                            _events.ScheduleEvent(EVENT_VENGEANCE, urand(10000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
            EventMap _events;
            uint32 _resetTimer;
            bool _faceroller;
            bool _done;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_eadricAI(creature);
        }
};

class OrientationCheck : public std::unary_function<Unit*, bool>
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

class spell_eadric_hammer_missile : public SpellScriptLoader
{
    public:
        spell_eadric_hammer_missile() : SpellScriptLoader("spell_eadric_hammer_missile") { }

        class spell_eadric_hammer_missile_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_hammer_missile_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HAMMER_CATCH))
                    return false;

                return true;
            }

            void HandleTriggerMissile(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (caster && target && !target->HasAura(SPELL_HAMMER_STUN))
                {
                    PreventHitDefaultEffect(EFFECT_0);
                    caster->CastSpell(target, SPELL_HAMMER_CATCH, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_eadric_hammer_missile_SpellScript::HandleTriggerMissile, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eadric_hammer_missile_SpellScript();
        }
};

class spell_eadric_hammer_throw_back : public SpellScriptLoader
{
    public:
        spell_eadric_hammer_throw_back() : SpellScriptLoader("spell_eadric_hammer_throw_back") { }

        class spell_eadric_hammer_throw_back_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_hammer_throw_back_SpellScript);

            void RemoveAura()
            {
                GetCaster()->RemoveAurasDueToSpell(SPELL_HAMMER_CATCH);
            }

            void CheckDamage()
            {
                Creature* target = GetHitCreature();
                if (!target)
                    return;

                if (GetHitDamage() >= int32(target->GetHealth()))
                    target->AI()->SetData(DATA_FACEROLLER, 1);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_eadric_hammer_throw_back_SpellScript::RemoveAura);
                OnHit += SpellHitFn(spell_eadric_hammer_throw_back_SpellScript::CheckDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eadric_hammer_throw_back_SpellScript();
        }
};

class boss_paletress : public CreatureScript
{
    public:
        boss_paletress() : CreatureScript("boss_paletress") { }

        struct boss_paletressAI : public ScriptedAI
        {
            boss_paletressAI(Creature* creature) : ScriptedAI(creature), _summons(me)
            {
                _instance = creature->GetInstanceScript();
                _resetTimer = 5000;
                _done = false;
                creature->SetReactState(REACT_PASSIVE);
                creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                _events.Reset();
                _summons.DespawnAll();
                _health = false;
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    _done = true;
                    damage = 0;
                    EnterEvadeMode();
                    me->setFaction(35);

                    Talk(SAY_DEATH_P);

                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_MAIN_GATE)))
                        _instance->HandleGameObject(go->GetGUID(), true);
                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, _instance->GetGuidData(DATA_PORTCULLIS)))
                        _instance->HandleGameObject(go->GetGUID(), true);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (_done)
                    return;

                DoZoneInCombat(me, 150.0f);
                Talk(SAY_START_P);

                if (_instance)
                    _instance->SetData(BOSS_ARGENT_CHALLENGE_P, IN_PROGRESS);

                _events.ScheduleEvent(EVENT_HOLYFIRE, urand(4000, 8000));
                _events.ScheduleEvent(EVENT_HOLYSMITE, urand(3000, 6000));
            }

            void JustSummoned(Creature* summon)
            {
                _summons.Summon(summon);
                DoZoneInCombat(summon, 150.0f);

                if (_instance)
                    _instance->SetData(DATA_MEMORY_ENTRY, summon->GetEntry());
            }

            void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/)
            {
                me->RemoveAura(SPELL_SHIELD);
                Talk(SAY_MEM_DIE);

                if (me->GetVictim())
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(urand(0, 1) ? SAY_KILL1_P : SAY_KILL2_P);
            }

            void UpdateAI(uint32 const diff)
            {
                if (_done)
                    if (_resetTimer <= diff)
                    {
                        if (_instance)
                            _instance->SetData(BOSS_ARGENT_CHALLENGE_P, DONE);

                        me->DespawnOrUnsummon(1000);

                        if (IsHeroic())
                            DoCast(me, SPELL_CONFESSOR_ACHIEVEMENT, true);

                        DoCast(me, SPELL_ARGENT_CREDIT, true);
                        DoCast(me, SPELL_PALETRESS_CREDIT, true);
                        _resetTimer = 5000;
                    }
                    else
                        _resetTimer -= diff;

                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (!_health && !HealthAbovePct(25))
                {
                    _health = true;
                    me->InterruptNonMeleeSpells(true);
                    DoCastAOE(SPELL_HOLY_NOVA);
                    DoCastAOE(SPELL_CONFESS_STUN, true);
                    DoCast(me, SPELL_SHIELD, true);
                    _events.DelayEvents(7000);
                    _events.ScheduleEvent(EVENT_CONFESS, 1);
                    _events.ScheduleEvent(EVENT_RENEW, urand(12000, 15000));
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CONFESS:
                            DoCast(SPELL_CONFESS);
                            _events.ScheduleEvent(EVENT_MEMORY, 1);
                            return;
                        case EVENT_MEMORY:
                            DoCast(SPELL_SUMMON_MEMORY);
                            if (me->GetVictim())
                                me->GetMotionMaster()->MoveChase(me->GetVictim(), 10.0f);
                            return;
                        case EVENT_HOLYFIRE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                DoCast(target, SPELL_HOLY_FIRE);
                            _events.ScheduleEvent(EVENT_HOLYFIRE, me->HasAura(SPELL_SHIELD) ? 12000 : urand(9000, 11000));
                            break;
                        case EVENT_HOLYSMITE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                DoCast(target, SPELL_SMITE);
                            _events.ScheduleEvent(EVENT_HOLYSMITE, me->HasAura(SPELL_SHIELD) ? 7000 : urand(4000, 6000));
                            break;
                        case EVENT_RENEW:
                            if (Unit* friendly = DoSelectLowestHpFriendly(200.0f, 20000))
                                DoCast(friendly, SPELL_RENEW);
                            _events.ScheduleEvent(EVENT_RENEW, urand(25000, 27000));
                            break;
                    }
                }

                if (!me->HasAura(SPELL_SHIELD))
                    DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
            SummonList _summons;
            EventMap _events;
            uint32 _resetTimer;
            bool _health;
            bool _done;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_paletressAI(creature);
        }
};

class spell_paletress_reflective_shield : public SpellScriptLoader
{
    public:
        spell_paletress_reflective_shield() : SpellScriptLoader("spell_paletress_reflective_shield") { }

        class spell_paletress_reflective_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_paletress_reflective_shield_AuraScript);

            void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = dmgInfo.GetAttacker();
                Unit* caster = GetCaster();

                if (!target || !caster)
                    return;

                caster->DealDamage(target, uint32(dmgInfo.GetDamage() / 4), NULL, dmgInfo.GetDamageType(), dmgInfo.GetSchoolMask());
                caster->SendSpellNonMeleeDamageLog(target, GetSpellInfo()->Id, uint32(dmgInfo.GetDamage() / 4), dmgInfo.GetSchoolMask(), 0, 0, true, 0);
            }

            void Register()
            {
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_paletress_reflective_shield_AuraScript::HandleAbsorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_paletress_reflective_shield_AuraScript();
        }
};

class npc_memory : public CreatureScript
{
    public:
        npc_memory() : CreatureScript("npc_memory") { }

        struct npc_memoryAI : public ScriptedAI
        {
            npc_memoryAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset()
            {
                _events.Reset();
                _events.ScheduleEvent(EVENT_OLDWOUNDS, urand(7000, 12000));
                _events.ScheduleEvent(EVENT_SHADOWS, urand(3000, 7000));
                _events.ScheduleEvent(EVENT_NIGHTMARE, urand(6000, 9000));
                DoCast(SPELL_SPAWN_EFFECT);
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
                        case EVENT_OLDWOUNDS:
                             DoCastVictim(SPELL_OLD_WOUNDS);
                            _events.ScheduleEvent(EVENT_OLDWOUNDS, urand(12000, 13000));
                            break;
                        case EVENT_SHADOWS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f))
                                DoCast(target, SPELL_SHADOWS_PAST);
                            _events.ScheduleEvent(EVENT_SHADOWS, urand(5000, 7000));
                            return;
                        case EVENT_NIGHTMARE:
                            DoCast(me, SPELL_WAKING_NIGHTMARE);
                            _events.ScheduleEvent(EVENT_NIGHTMARE, urand(13000, 14000));
                            return;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_memoryAI(creature);
        }
};

class npc_argent_soldier : public CreatureScript
{
    public:
        npc_argent_soldier() : CreatureScript("npc_argent_soldier") { }

        struct npc_argent_soldierAI : public npc_escortAI
        {
            npc_argent_soldierAI(Creature* creature) : npc_escortAI(creature)
            {
                _instance = creature->GetInstanceScript();
                me->SetReactState(REACT_DEFENSIVE);
                SetDespawnAtEnd(false);
                _waypoint = 0;
            }

            void Reset()
            {
                _shielded = false;
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
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

            void WaypointReached(uint32 point)
            {
                if (point == 0)
                {
                    switch (_waypoint)
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

            void SetData(uint32 type, uint32 /*data*/)
            {
                switch (me->GetEntry())
                {
                    case NPC_ARGENT_LIGHWIELDER:
                        switch (type)
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
                        switch (type)
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
                        switch (type)
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
                _waypoint = type;
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (!IsHeroic() || _shielded || me->GetEntry() != NPC_ARGENT_MONK)
                    return;

                if (damage >= me->GetHealth())
                {
                    DoCast(me, SPELL_FINAL_MEDITATION);
                    DoCast(me, SPELL_DIVINE_SHIELD, true);
                    me->SetHealth(1);
                    damage = 0;
                    _shielded = true;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                npc_escortAI::UpdateAI(diff);

                if (!UpdateVictim())
                    return;

                _events.Update(diff);

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
                            DoCastVictim(SPELL_UNBALANCING_STRIKE);
                            _events.ScheduleEvent(EVENT_STRIKE, urand(10000, 12000));
                            break;
                        case EVENT_BLAZINGLIGHT:
                            DoCast(me, SPELL_BLAZING_LIGHT);
                            _events.ScheduleEvent(EVENT_BLAZINGLIGHT, urand(9000, 13000));
                            break;
                        case EVENT_FLURRY:
                            DoCast(me, SPELL_FLURRY_OF_BLOWS);
                            _events.ScheduleEvent(EVENT_FLURRY, urand(13000, 15000));
                            break;
                        case EVENT_PUMMEL:
                            DoCastVictim(SPELL_PUMMEL);
                            _events.ScheduleEvent(EVENT_PUMMEL, urand(4000, 7000));
                            break;
                        case EVENT_HOLYSMITE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f))
                                DoCast(target, DUNGEON_MODE<uint32>(SPELL_HOLY_SMITE, SPELL_HOLY_SMITE_H));
                            _events.ScheduleEvent(EVENT_HOLYSMITE, urand(5000, 7000));
                            break;
                        case EVENT_MINDCONTROL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 30.0f))
                                DoCast(target, SPELL_MIND_CONTROL);
                            _events.ScheduleEvent(EVENT_MINDCONTROL, urand(20000, 25000));
                            break;
                        case EVENT_PAIN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, false, DUNGEON_MODE<int32>(-SPELL_DOT_PAIN, -SPELL_DOT_PAIN_H)))
                                DoCast(target, DUNGEON_MODE<uint32>(SPELL_DOT_PAIN, SPELL_DOT_PAIN_H));
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

            void JustDied(Unit* /*killer*/)
            {
                if (_instance)
                    _instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, _instance->GetData(DATA_ARGENT_SOLDIER_DEFEATED) + 1);
            }

        private:
            InstanceScript* _instance;
            EventMap _events;
            bool _shielded;
            uint8 _waypoint;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_argent_soldierAI(creature);
        }
};

class spell_light_rain : public SpellScriptLoader
{
    public:
        spell_light_rain() : SpellScriptLoader("spell_light_rain") { }

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

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_light_rain_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_light_rain_SpellScript();
        }
};

void AddSC_boss_argent_challenge()
{
    new boss_eadric();
    new spell_eadric_radiance();
    new spell_eadric_hammer_missile();
    new spell_eadric_hammer_throw_back();
    new boss_paletress();
    new spell_paletress_reflective_shield();
    new npc_memory();
    new npc_argent_soldier();
    new spell_light_rain();
}