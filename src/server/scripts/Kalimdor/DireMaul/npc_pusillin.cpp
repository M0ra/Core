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
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"

enum Talk
{
    SAY_1                       = 0,
    SAY_2                       = 1,                     
    SAY_3                       = 2,
    SAY_4                       = 3,
    SAY_5                       = 4
};

enum Misc
{
    NPC_IMP                     = 13276
};

enum Spells
{
    SPELL_BLAST_WAVE            = 17145,
    SPELL_FIRE_BLAST            = 14145,
    SPELL_FIREBALL              = 15228,
    SPELL_RUNN                  = 22735
};

const Position SpawnPosition1 = {23.019f,-703.442f,-12.642f,3.23f};
const Position SpawnPosition2 = {22.646f,-691.948f,-12.642f,4.13f};
const Position SpawnPosition3 = {12.277f,-676.333f,-12.642f,4.97f};
const Position SpawnPosition4 = {24.416f,-680.573f,-12.642f,4.31f};

#define GOSSIP_HELLO "Игра? Вы с ума сошли?"
#define Gossip_2 "Почему ты мал..."
#define Gossip_3 "Запомни мои слова, я поймаю тебя, имп!"
#define Gossip_4 "УМРИ!"
#define Gossip_5 "Приготовьтесь к встрече со своим Создателем."

class npc_pusillin : public CreatureScript
{
public:
    npc_pusillin() : CreatureScript("npc_pusillin") { }

    struct npc_pusillinAI : public npc_escortAI
    {
        npc_pusillinAI(Creature* сreature) : npc_escortAI(сreature)
        {
            Initialize();
        }

        void Initialize()
        {
            uiPhase = 0;
            uiGossipStep = 0;
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            uiBlastWaveTimer = urand(7000, 9000);
            uiFireBlastTimer = urand(3000, 6000);
            uiFireBallTimer  = urand(11000, 13000);
            uiSpiritOfRunnTimer = urand(5000, 7000);
            buffed = false;
        }

        uint32 uiPhase;
        uint32 uiGossipStep;
        uint32 uiBlastWaveTimer;
        uint32 uiFireBlastTimer;
        uint32 uiFireBallTimer;
        uint32 uiSpiritOfRunnTimer;
        bool buffed;

        void Reset()
        {
            Initialize();
        }

        void SetHoldState(bool bOnHold)
        {
            SetEscortPaused(bOnHold);
        }

        void WaypointReached(uint32 uiPointId) override
        {
            switch(uiPointId)
            {
                case 0:
                    break;
                case 2:
                    uiGossipStep = 1;
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    SetHoldState(true);
                    break;
                case 4:
                    uiGossipStep = 2;
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    SetHoldState(true);
                    break;
                case 10:
                    uiGossipStep = 3;
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    SetHoldState(true);
                    break;
                case 16:
                    uiGossipStep = 4;
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {

            npc_escortAI::UpdateAI(diff);

            if (uiPhase)
            {
                switch(uiPhase)
                {
                    case 1:
                        Talk(SAY_1);
                        uiPhase = 0;
                        break;
                    case 2:   
                        Talk(SAY_2);
                        uiPhase = 0;
                        break;
                    case 3:
                        Talk(SAY_3);
                        uiPhase = 0;
                        break;
                    case 4:
                        Talk(SAY_4);
                        uiPhase = 0;
                        break;
                    case 5:
                        Talk(SAY_5);
                        me->setFaction(14);
                        float scale = me->GetFloatValue(OBJECT_FIELD_SCALE_X);
                        me->SetFloatValue(OBJECT_FIELD_SCALE_X, scale * 3);
                        Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0);
                            me->AI()->AttackStart(target);
                        for(uint8 i = 0; i < 5; i++)
                        {
                            switch(i)
                            {
                                case 1:
                                    if (Creature* imp = me->SummonCreature(NPC_IMP, SpawnPosition1, TEMPSUMMON_CORPSE_DESPAWN, 120000))
                                    {
                                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                            imp->AI()->AttackStart(target);
                                    }
                                    break;
                                case 2:
                                    if (Creature* imp = me->SummonCreature(NPC_IMP, SpawnPosition2, TEMPSUMMON_CORPSE_DESPAWN, 120000))
                                    {
                                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                            imp->AI()->AttackStart(target);
                                    }
                                    break;
                                case 3:
                                    if (Creature* imp = me->SummonCreature(NPC_IMP, SpawnPosition3, TEMPSUMMON_CORPSE_DESPAWN, 120000))
                                    {
                                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                            imp->AI()->AttackStart(target);
                                    }
                                    break;
                                case 4:
                                    if (Creature* imp = me->SummonCreature(NPC_IMP, SpawnPosition4, TEMPSUMMON_CORPSE_DESPAWN, 120000))
                                    {
                                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                            imp->AI()->AttackStart(target);
                                    }
                                    break;
                            }
                        }
                        uiPhase = 6;
                        break;
                }
            }

            if (uiPhase == 6)
            {
                if (!UpdateVictim())
                    return;

                if (uiBlastWaveTimer <= diff)
                {
                    DoCast(me->GetVictim(), SPELL_BLAST_WAVE);
                    uiBlastWaveTimer = urand(7000, 9000);
                }
                else uiBlastWaveTimer -= diff;

                if (uiFireBlastTimer <= diff)
                {
                    DoCast(me->GetVictim(), SPELL_FIRE_BLAST);
                    uiFireBlastTimer = urand(3000, 6000);
                }
                else uiFireBlastTimer -= diff;

                if (uiFireBallTimer <= diff)
                {
                    DoCast(me->GetVictim(), SPELL_FIREBALL);
                    uiFireBallTimer  = urand(11000, 13000);
                }
                else uiFireBallTimer -= diff;

                if (uiSpiritOfRunnTimer <= diff && buffed == false)
                {
                    DoCast(me, SPELL_RUNN);
                    buffed = true;
                }
                else uiSpiritOfRunnTimer -= diff;         

                DoMeleeAttackIfReady();
            }
        }
    };

    CreatureAI* GetAI(Creature* сreature) const override
    {
        return new npc_pusillinAI(сreature);
    }

    bool OnGossipHello(Player* player, Creature* сreature) override
    {
        npc_pusillinAI* pAI = CAST_AI(npc_pusillinAI, сreature->AI());
        
        switch (pAI->uiGossipStep)
        {
            case 0:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(6877, сreature->GetGUID());
                break;
            case 1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Gossip_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(6878, сreature->GetGUID());
                break;
            case 2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Gossip_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->SEND_GOSSIP_MENU(6879, сreature->GetGUID());
                break;
            case 3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Gossip_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(6880, сreature->GetGUID());
                break;
            case 4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Gossip_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(6881, сreature->GetGUID());
                break;
        }
            return true;
    }

    bool OnGossipSelect(Player* player, Creature* сreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        npc_pusillinAI* pAI = CAST_AI(npc_pusillinAI,сreature->AI());

        if (!pAI)
            return false;

        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            CAST_AI(npc_pusillinAI, сreature->AI())->Start(false, true, player->GetGUID());
            pAI->SetHoldState(false);
            pAI->uiPhase = 1;
        }

        if (uiAction == GOSSIP_ACTION_INFO_DEF + 2)
        {
            pAI->uiPhase = 2;
            pAI->SetHoldState(false);
        }

        if (uiAction == GOSSIP_ACTION_INFO_DEF + 3)
        {
            pAI->uiPhase = 3;
            pAI->SetHoldState(false);
        }

        if (uiAction == GOSSIP_ACTION_INFO_DEF + 4)
        {
            pAI->uiPhase = 4;
            pAI->SetHoldState(false);
        }

        if (uiAction == GOSSIP_ACTION_INFO_DEF + 5)
        {
            pAI->uiPhase = 5;
        }

        player->CLOSE_GOSSIP_MENU();
        pAI->SetDespawnAtEnd(false);
        pAI->SetDespawnAtFar(false);
        сreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        return true;
    }
};

void AddSC_npc_pusillin()
{
    new npc_pusillin();
}