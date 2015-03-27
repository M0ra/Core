/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Script updated by Fiveofeight
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
#include "Vehicle.h"
#include "Player.h"

#define GOSSIP_START_EVENT1     "Я готов начать испытание."
#define GOSSIP_START_EVENT2     "Я готов к следующему испытанию."

#define ORIENTATION             4.714f

/*######
## npc_herald_toc5
######*/

const Position SpawnPosition  = {746.843f, 695.68f, 412.339f, 4.70776f};
const Position SpawnPosition1 = {746.71f, 661.02f, 411.69f, 4.66995f};

enum Texts
{
    // Starts when you enter toc
    SAY_INTRO_HERALD_1          = 27, 
    SAY_INTRO_HERALD_2          = 50,
    H_SAY_INTRO_HERALD_3        = 0,
    A_SAY_INTRO_HERALD_3        = 50,
    H_SAY_INTRO_HERALD_4        = 50,
    A_SAY_INTRO_HERALD_4        = 10,
    SAY_INTRO_HERALD_5          = 52,
    H_SAY_INTRO_HERALD_6        = 11,
    A_SAY_INTRO_HERALD_6        = 2,
    SAY_INTRO_HERALD_7          = 51,

    //  Horde - Announcing the fighters
    H_SAY_WARRIOR_ENTERS          = 6,
    H_SAY_MAGE_ENTERS             = 5,
    H_SAY_SHAMAN_ENTERS           = 3,
    H_SAY_ROGUE_ENTERS            = 7,

    //  Alliance - Announcing the fighters
    A_SAY_WARRIOR_ENTERS          = 5,
    A_SAY_MAGE_ENTERS             = 3,
    A_SAY_SHAMAN_ENTERS           = 7,
    A_SAY_ROGUE_ENTERS            = 6,

    // Shared emotes - announcer
    SAY_WARRIOR_CHEER           = 20,
    SAY_MAGE_CHEER              = 21,
    SAY_SHAMAN_CHEER            = 22,
    SAY_HUNTER_CHEER            = 23,
    SAY_ROGUE_CHEER             = 24,
    SAY_HUNTER_ENTERS           = 4,

    // Argent Champion
    SAY_ARGENT_CHAMP_ENTERS     = 53,
    SAY_PALETRESS_INTRO_1       = 1,
    SAY_PALETRESS_INTRO_2       = 25,
    SAY_PALETRESS_INTRO_3       = 0,
    SAY_PALETRESS_INTRO_4       = 1,
    SAY_EADRIC_INTRO_1          = 0,
    SAY_EADRIC_INTRO_2          = 26,
    SAY_EADRIC_INTRO_3          = 0,

    // Black Knight
    SAY_INTRO_BLACK_KNIGHT_TIRION   = 55,
    SAY_HERALD_RAFTERS              = 8
};

enum Gossip
{
    GOSSIP_NOT_MOUNTED_A  = 14757,
    GOSSIP_NOT_MOUNTED_H  = 15043
};

enum Events
{
    EVENT_INTRO_1               = 1,
    EVENT_INTRO_2               = 2,
    EVENT_INTRO_3               = 3,
    EVENT_INTRO_4               = 4,
    EVENT_INTRO_5               = 5,
    EVENT_INTRO_6               = 6,
    EVENT_INTRO_7               = 7,
    EVENT_INTRO_8               = 8,
    EVENT_SUMMON_FACTION_2      = 9,
    EVENT_SUMMON_FACTION_3      = 10,
    EVENT_AGGRO_FACTION         = 11,
    EVENT_PALETRESS_1           = 12,
    EVENT_PALETRESS_2           = 13,
    EVENT_PALETRESS_3           = 14,
    EVENT_EADRIC_1              = 15,
    EVENT_EADRIC_2              = 16
};

enum Phases
{
    PHASE_INTRO                 = 1,
    PHASE_INPROGRESS            = 2,

    PHASE_INTRO_MASK            = 1 << PHASE_INTRO,
    PHASE_INPROGRESS_MASK       = 2 << PHASE_INPROGRESS
};

enum Spells
{
    SPELL_FACE_BLACKKNIGHT                = 67482,
    SPELL_HERALD_ARGENT                   = 64787
};

class npc_herald_toc5 : public CreatureScript
{
    public:
        npc_herald_toc5(): CreatureScript("npc_herald_toc5") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_herald_toc5::npc_herald_toc5AI, creature->AI())->StartEncounter();
        }

        if (action == GOSSIP_ACTION_INFO_DEF + 2)
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        return true;
    }
        
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        InstanceScript* instance = creature->GetInstanceScript();

        if ((!player->GetGroup() || !player->GetGroup()->IsLeader(player->GetGUID())) && !player->IsGameMaster())
        {
            player->ADD_GOSSIP_ITEM(0, "Вы не лидер рейда...", 650, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        if (!player->GetVehicle() && instance->GetData(BOSS_GRAND_CHAMPIONS) == NOT_STARTED)
        {
            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                player->SEND_GOSSIP_MENU(GOSSIP_NOT_MOUNTED_H, creature->GetGUID());
            else
                player->SEND_GOSSIP_MENU(GOSSIP_NOT_MOUNTED_A, creature->GetGUID());

            return true;
        }

        else
        {
            if (instance &&
                instance->GetData(BOSS_GRAND_CHAMPIONS) == DONE &&
                instance->GetData(BOSS_BLACK_KNIGHT) == DONE &&
                (instance->GetData(BOSS_ARGENT_CHALLENGE_E) == DONE ||
                instance->GetData(BOSS_ARGENT_CHALLENGE_P) == DONE))

                return false;

            if (instance &&
                instance->GetData(BOSS_GRAND_CHAMPIONS) == NOT_STARTED &&
                instance->GetData(BOSS_ARGENT_CHALLENGE_E) == NOT_STARTED &&
                instance->GetData(BOSS_ARGENT_CHALLENGE_P) == NOT_STARTED &&
                instance->GetData(BOSS_BLACK_KNIGHT) == NOT_STARTED)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_START_EVENT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            else if (instance)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_START_EVENT2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
    
            return true;
        }
    }

    struct npc_herald_toc5AI : public ScriptedAI
    {
        npc_herald_toc5AI(Creature *creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();

            uiSummonTimes = 0;
            uiPosition = 0;
            uiLesserChampions = 0;
            uiDefeatedGrandChampions = 0;

            uiFirstBoss = 0;
            uiSecondBoss = 0;
            uiThirdBoss = 0;

            ArgentChampion = 0;

            Champion1List.clear();
            Champion2List.clear();
            Champion3List.clear();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetGrandChampionsForEncounter();
            SetArgentChampion();
        }

        EventMap events;
        InstanceScript* instance;
        Creature* vehicle_black_knight;

        uint8 uiSummonTimes;
        uint8 uiPosition;
        uint8 uiLesserChampions;
        uint8 uiIntroPhase;
        uint8 uiDefeatedGrandChampions;

        uint32 ArgentChampion;
        uint32 uiFirstBoss;
        uint32 uiSecondBoss;
        uint32 uiThirdBoss;

        ObjectGuid thrallGUID;
        ObjectGuid garroshGUID;
        ObjectGuid varianGUID;
        ObjectGuid proudmooreGUID;
        ObjectGuid tirionGUID;

        ObjectGuid BlackKnightGUID;
        ObjectGuid uiVehicle1GUID;
        ObjectGuid uiVehicle2GUID;
        ObjectGuid uiVehicle3GUID;
        ObjectGuid GrandChampionBoss;

        GuidList Champion1List;
        GuidList Champion2List;
        GuidList Champion3List;

        bool _introDone;

        void Reset() override
        {
            _introDone = false;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!_introDone && (instance->GetData(BOSS_GRAND_CHAMPIONS) != DONE && instance->GetData(BOSS_GRAND_CHAMPIONS) != FAIL) && me->IsWithinDistInMap(who, 75.0f))
            {
                _introDone = true;
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                
                events.SetPhase(PHASE_INTRO);
                events.ScheduleEvent(EVENT_INTRO_1, 10000, 0, PHASE_INTRO);

                SummonNpcs();
            }
        }

        void SetData(uint32 uiType, uint32 uiData) override
        {
            switch (uiType)
            {
                case DATA_START:
                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                        instance->HandleGameObject(go->GetGUID(), true);
                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                        instance->HandleGameObject(go->GetGUID(), false);
                    DoSummonGrandChampion(uiFirstBoss);
                    events.ScheduleEvent(EVENT_SUMMON_FACTION_2, 10000, 0, PHASE_INPROGRESS);
                    break;
                case DATA_IN_POSITION:
                    me->GetMotionMaster()->MovePoint(1, 735.898f, 651.961f, 411.93f);
                    if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                        instance->HandleGameObject(go->GetGUID(), false);
                    events.ScheduleEvent(EVENT_AGGRO_FACTION, 15000, 0, PHASE_INPROGRESS);
                    break;
                case DATA_LESSER_CHAMPIONS_DEFEATED:
                {
                    ++uiLesserChampions;
                    GuidList TempList;
                    if (uiLesserChampions == 3 || uiLesserChampions == 6)
                    {
                        switch(uiLesserChampions)
                        {
                            case 3:
                                TempList = Champion2List;
                                break;
                            case 6:
                                TempList = Champion3List;
                                break;
                        }

                        for(GuidList::const_iterator itr = TempList.begin(); itr != TempList.end(); ++itr)
                            if (Creature* summon = ObjectAccessor::GetCreature(*me, *itr))
                                AggroAllPlayers(summon);
                    } else if (uiLesserChampions == 9)
                        StartGrandChampionsAttack();
                    break;
                }
                case DATA_GRAND_CHAMPIONS_DEFEATED:
                    uiDefeatedGrandChampions = uiData;
                    if (uiDefeatedGrandChampions == 3)
                    {
                        for (uint8 i = 0; i < 3; ++i)
                            if (Creature* GrandChampion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_GRAND_CHAMPION_1 + i)))
                            {
                                switch (i)
                                {
                                    case 0:
                                        GrandChampion->SetHomePosition(739.678f, 662.541f, 412.393f, 4.6f);
                                        break;
                                    case 1:
                                        GrandChampion->SetHomePosition(746.71f, 661.02f, 411.69f, 4.6f);
                                        break;
                                    case 2:
                                        GrandChampion->SetHomePosition(754.34f, 660.70f, 412.39f, 4.6f);
                                        break;
                                }
                                GrandChampion->AI()->SetData(10, 0);
                            }

                        instance->SetData(BOSS_GRAND_CHAMPIONS, IN_PROGRESS);
                    }
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_GRAND_CHAMPIONS_DEFEATED)
                return uiDefeatedGrandChampions;

            return 0;
        }

        void StartGrandChampionsAttack()
        {
            Creature* pGrandChampion1 = ObjectAccessor::GetCreature(*me, uiVehicle1GUID);
            Creature* pGrandChampion2 = ObjectAccessor::GetCreature(*me, uiVehicle2GUID);
            Creature* pGrandChampion3 = ObjectAccessor::GetCreature(*me, uiVehicle3GUID);

            if (pGrandChampion1 && pGrandChampion2 && pGrandChampion3)
            {
                AggroAllPlayers(pGrandChampion1);
                AggroAllPlayers(pGrandChampion2);
                AggroAllPlayers(pGrandChampion3);
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId) override 
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (uiPointId == 1)
            {
                me->SetOrientation(ORIENTATION);
                me->SendMovementFlagUpdate();
            }
        }

        void JustSummoned(Creature* summon) override
        {
            if (instance && instance->GetData(BOSS_GRAND_CHAMPIONS) == NOT_STARTED)
            {
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            switch(summon->GetEntry())
            {
                case VEHICLE_DARNASSIA_NIGHTSABER:
                case VEHICLE_EXODAR_ELEKK:
                case VEHICLE_STORMWIND_STEED:
                case VEHICLE_GNOMEREGAN_MECHANOSTRIDER:
                case VEHICLE_IRONFORGE_RAM:
                case VEHICLE_FORSAKE_WARHORSE:
                case VEHICLE_THUNDER_BLUFF_KODO:
                case VEHICLE_ORGRIMMAR_WOLF:
                case VEHICLE_SILVERMOON_HAWKSTRIDER:
                case VEHICLE_DARKSPEAR_RAPTOR:
                    me->AI()->SetData(DATA_LESSER_CHAMPIONS_DEFEATED, 0);
                    break;
            }
        }
        
        void DoSummonGrandChampion(uint32 uiBoss)
        {
            ++uiSummonTimes;
            uint32 VEHICLE_TO_SUMMON1 = 0;
            uint32 VEHICLE_TO_SUMMON2 = 0;
            switch (uiBoss)
            {
                case 0:
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        Talk(H_SAY_WARRIOR_ENTERS);
                    else
                        Talk(A_SAY_WARRIOR_ENTERS);
                    Talk(SAY_WARRIOR_CHEER);
                    VEHICLE_TO_SUMMON1 = VEHICLE_MOKRA_SKILLCRUSHER_MOUNT;
                    VEHICLE_TO_SUMMON2 = VEHICLE_ORGRIMMAR_WOLF;
                    break;
                case 1:
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        Talk(H_SAY_MAGE_ENTERS);
                    else
                        Talk(A_SAY_MAGE_ENTERS);
                    Talk(SAY_MAGE_CHEER);
                    VEHICLE_TO_SUMMON1 = VEHICLE_ERESSEA_DAWNSINGER_MOUNT;
                    VEHICLE_TO_SUMMON2 = VEHICLE_SILVERMOON_HAWKSTRIDER;
                    break;
                case 2:
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        Talk(H_SAY_SHAMAN_ENTERS);
                    else
                        Talk(A_SAY_SHAMAN_ENTERS);
                    Talk(SAY_SHAMAN_CHEER);
                    VEHICLE_TO_SUMMON1 = VEHICLE_RUNOK_WILDMANE_MOUNT;
                    VEHICLE_TO_SUMMON2 = VEHICLE_THUNDER_BLUFF_KODO;
                    break;
                case 3:
                    Talk(SAY_HUNTER_ENTERS);
                    Talk(SAY_HUNTER_CHEER);
                    VEHICLE_TO_SUMMON1 = VEHICLE_ZUL_TORE_MOUNT;
                    VEHICLE_TO_SUMMON2 = VEHICLE_DARKSPEAR_RAPTOR;
                    break;
                case 4:
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        Talk(H_SAY_ROGUE_ENTERS);
                    else
                        Talk(A_SAY_ROGUE_ENTERS);
                    Talk(SAY_ROGUE_CHEER);
                    VEHICLE_TO_SUMMON1 = VEHICLE_DEATHSTALKER_VESCERI_MOUNT;
                    VEHICLE_TO_SUMMON2 = VEHICLE_FORSAKE_WARHORSE;
                    break;
                default:
                    return;
            }

            if (Creature* pBoss = me->SummonCreature(VEHICLE_TO_SUMMON1, SpawnPosition))
            {
                switch(uiSummonTimes)
                {
                    case 1:
                    {
                        uiVehicle1GUID = pBoss->GetGUID();
                        ObjectGuid uiGrandChampionBoss1;
                        if (Creature* pBoss = ObjectAccessor::GetCreature(*me, uiVehicle1GUID))
                            if (Vehicle* vehicle = pBoss->GetVehicleKit())
                                if (Unit* unit = vehicle->GetPassenger(0))
                                    uiGrandChampionBoss1 = unit->GetGUID();
                        if (instance)
                        {
                            instance->SetGuidData(DATA_GRAND_CHAMPION_VEHICLE_1, uiVehicle1GUID);
                            instance->SetGuidData(DATA_GRAND_CHAMPION_1, uiGrandChampionBoss1);
                        }
                        pBoss->AI()->SetData(1,0);
                        break;
                    }
                    case 2:
                    {
                        uiVehicle2GUID = pBoss->GetGUID();
                        ObjectGuid uiGrandChampionBoss2;
                        if (Creature* pBoss = ObjectAccessor::GetCreature(*me, uiVehicle2GUID))
                            if (Vehicle* vehicle = pBoss->GetVehicleKit())
                                if (Unit* unit = vehicle->GetPassenger(0))
                                    uiGrandChampionBoss2 = unit->GetGUID();
                        if (instance)
                        {
                            instance->SetGuidData(DATA_GRAND_CHAMPION_VEHICLE_2, uiVehicle2GUID);
                            instance->SetGuidData(DATA_GRAND_CHAMPION_2, uiGrandChampionBoss2);
                        }
                        pBoss->AI()->SetData(2, 0);
                        break;
                    }
                    case 3:
                    {
                        uiVehicle3GUID = pBoss->GetGUID();
                        ObjectGuid uiGrandChampionBoss3;
                        if (Creature* pBoss = ObjectAccessor::GetCreature(*me, uiVehicle3GUID))
                            if (Vehicle* vehicle = pBoss->GetVehicleKit())
                                if (Unit* unit = vehicle->GetPassenger(0))
                                    uiGrandChampionBoss3 = unit->GetGUID();
                        if (instance)
                        {
                            instance->SetGuidData(DATA_GRAND_CHAMPION_VEHICLE_3, uiVehicle3GUID);
                            instance->SetGuidData(DATA_GRAND_CHAMPION_3, uiGrandChampionBoss3);
                        }
                        pBoss->AI()->SetData(3, 0);
                        break;
                    }
                    default:
                        return;
                }

                for (uint8 i = 0; i < 3; ++i)
                {
                    if (Creature* pAdd = me->SummonCreature(VEHICLE_TO_SUMMON2, SpawnPosition, TEMPSUMMON_CORPSE_DESPAWN))
                    {
                        switch (uiSummonTimes)
                        {
                            case 1:
                                Champion1List.push_back(pAdd->GetGUID());
                                break;
                            case 2:
                                Champion2List.push_back(pAdd->GetGUID());
                                break;
                            case 3:
                                Champion3List.push_back(pAdd->GetGUID());
                                break;
                        }

                        switch (i)
                        {
                            case 0:
                                pAdd->GetMotionMaster()->MoveFollow(pBoss, 2.5f, M_PI);
                                break;
                            case 1:
                                pAdd->GetMotionMaster()->MoveFollow(pBoss, 2.5f, M_PI / 2);
                                break;
                            case 2:
                                pAdd->GetMotionMaster()->MoveFollow(pBoss, 2.5f, M_PI / 2 + M_PI);
                                break;
                        }
                    }
                }
            }
        }

        void DoStartArgentChampionEncounter()
        {
            if (Creature* tirion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_HIGHLORD)))
                tirion->AI()->Talk(SAY_ARGENT_CHAMP_ENTERS);

            if (ArgentChampion == NPC_PALETRESS)
                events.ScheduleEvent(EVENT_PALETRESS_1, 5000);
            else
                events.ScheduleEvent(EVENT_EADRIC_1, 5000);

            if (Creature* pBoss = me->SummonCreature(ArgentChampion, SpawnPosition))
            {
                pBoss->GetMotionMaster()->MovePoint(1, 746.71f, 661.02f, 411.69f);
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (Creature* pTrash = me->SummonCreature(NPC_ARGENT_LIGHWIELDER, SpawnPosition))
                        pTrash->AI()->SetData(i, 0);
                    if (Creature* pTrash = me->SummonCreature(NPC_ARGENT_MONK, SpawnPosition))
                        pTrash->AI()->SetData(i, 0);
                    if (Creature* pTrash = me->SummonCreature(NPC_PRIESTESS, SpawnPosition))
                        pTrash->AI()->SetData(i, 0);
                }
            }
        }

        void DoAction(int32 actionID) override
        {
            if (actionID == ACTION_RESET_BLACK_KNIGHT)
                StartEncounter();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            me->SetReactState(REACT_PASSIVE);
            if (Creature* ghoul = me->SummonCreature(NPC_RISEN_JAEREN, 742.835f, 639.134f, 411.571f, 1.05731f))
            {
                ghoul->setFaction(14);
            }
            if (instance)
                instance->SetData(DATA_AGRO_DONE,DONE);
        }

        void SetGrandChampionsForEncounter()
        {
            uiFirstBoss = urand(0, 4);

            while(uiSecondBoss == uiFirstBoss || uiThirdBoss == uiFirstBoss || uiThirdBoss == uiSecondBoss)
            {
                uiSecondBoss = urand(0, 4);
                uiThirdBoss = urand(0, 4);
            }
        }

        void SetArgentChampion()
        {
           uint8 uiTempBoss = urand(0, 1);

            switch(uiTempBoss)
            {
                case 0:
                    ArgentChampion = NPC_EADRIC;
                    break;
                case 1:
                    ArgentChampion = NPC_PALETRESS;
                    break;
            }
        }

        void StartEncounter()
        {
            if (!instance)
                return;

            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(go->GetGUID(), false);

            if (instance->GetData(BOSS_BLACK_KNIGHT) == NOT_STARTED)
            {
                if (instance->GetData(BOSS_ARGENT_CHALLENGE_E) == NOT_STARTED && instance->GetData(BOSS_ARGENT_CHALLENGE_P) == NOT_STARTED)
                {
                    if (instance->GetData(BOSS_GRAND_CHAMPIONS) == NOT_STARTED)
                        me->AI()->SetData(DATA_START,NOT_STARTED);

                    if (instance->GetData(BOSS_GRAND_CHAMPIONS) == DONE)
                        DoStartArgentChampionEncounter();
                }

                if (instance->GetData(BOSS_GRAND_CHAMPIONS) == DONE && (instance->GetData(BOSS_ARGENT_CHALLENGE_E) == DONE || instance->GetData(BOSS_ARGENT_CHALLENGE_P) == DONE))
                {
                    SummonNpcs();
                    me->SummonCreature(VEHICLE_BLACK_KNIGHT, 801.369507f, 640.574280f, 469.314362f, 3.97124f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300000);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (Creature* tirion = ObjectAccessor::GetCreature(*me, tirionGUID))
                        tirion->AI()->Talk(SAY_INTRO_BLACK_KNIGHT_TIRION);
                    Talk(SAY_HERALD_RAFTERS);
                }
            }
        }

        void AggroAllPlayers(Creature* temp)
        {
            Map::PlayerList const &PlList = me->GetMap()->GetPlayers();

            if (PlList.isEmpty())
                return;

            for(Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
            {
                if (Player* player = i->GetSource())
                {
                    if (player->IsGameMaster())
                        continue;

                    if (player->IsAlive())
                    {
                        temp->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                        temp->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        temp->SetReactState(REACT_AGGRESSIVE);
                        temp->SetInCombatWith(player);
                        player->SetInCombatWith(temp);
                        temp->AddThreat(player, 0.0f);
                    }
                }
            }
        }

        // Why can't these be pre-spawned already? -- Something to look forward...
        void SummonNpcs()
        {
            if (!me->FindNearestCreature(NPC_THRALL, 200.0f))
                if (Creature* Thrall = me->SummonCreature(NPC_THRALL, 685.569f, 615.103f, 435.396f, 6.23544f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                {
                    thrallGUID = Thrall->GetGUID();
                    Thrall->SetReactState(REACT_PASSIVE);
                    Thrall->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }

            if (!me->FindNearestCreature(NPC_GARROSH, 200.0f))
                if (Creature* Garrosh = me->SummonCreature(NPC_GARROSH, 685.7f, 621.134f, 435.396f, 6.259f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                {
                    garroshGUID = Garrosh->GetGUID();
                    Garrosh->SetReactState(REACT_PASSIVE);
                    Garrosh->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }

            if (!me->FindNearestCreature(NPC_VARIAN, 200.0f))
                if (Creature* Varian = me->SummonCreature(NPC_VARIAN, 807.724f, 617.9f, 435.396f, 3.18416f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                {
                    varianGUID = Varian->GetGUID();
                    Varian->SetReactState(REACT_PASSIVE);
                    Varian->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }

            if (!me->FindNearestCreature(NPC_JAINA_PROUDMOORE, 200.0f))
                if (Creature* Proudmoore = me->SummonCreature(NPC_JAINA_PROUDMOORE, 807.401f, 613.667f, 435.397f, 3.0585f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                {
                    proudmooreGUID = Proudmoore->GetGUID();
                    Proudmoore->SetReactState(REACT_PASSIVE);
                    Proudmoore->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }

            if (!me->FindNearestCreature(NPC_HIGHLORD, 200.0f))
                if (Creature* Tirion = me->SummonCreature(NPC_HIGHLORD, 746.482f, 556.857f, 435.396f, 1.5898f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                {
                    tirionGUID = Tirion->GetGUID();
                    Tirion->SetReactState(REACT_PASSIVE);
                    Tirion->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!(thrallGUID || garroshGUID || varianGUID || proudmooreGUID || tirionGUID))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO_1:
                        instance->DoCastSpellOnPlayers(SPELL_HERALD_ARGENT);
                        Talk(SAY_INTRO_HERALD_1);
                        events.ScheduleEvent(EVENT_INTRO_2, 5000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_2:
                        if (Creature* tirion = ObjectAccessor::GetCreature(*me, tirionGUID))
                            tirion->AI()->Talk(SAY_INTRO_HERALD_2);
                        events.ScheduleEvent(EVENT_INTRO_3, 13000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_3:
                        if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        {
                            if (Creature* thrall = ObjectAccessor::GetCreature(*me, thrallGUID))
                                thrall->AI()->Talk(H_SAY_INTRO_HERALD_3);
                        }
                        else
                            if (Creature* varian = ObjectAccessor::GetCreature(*me, varianGUID))
                                varian->AI()->Talk(A_SAY_INTRO_HERALD_3);
                        events.ScheduleEvent(EVENT_INTRO_4, 4000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_4:
                        if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        {
                            if (Creature* garrosh = ObjectAccessor::GetCreature(*me, garroshGUID))
                                garrosh->AI()->Talk(H_SAY_INTRO_HERALD_4);
                        }
                        else
                            if (Creature* proudmoore = ObjectAccessor::GetCreature(*me, proudmooreGUID))
                                proudmoore->AI()->Talk(A_SAY_INTRO_HERALD_4);
                        events.ScheduleEvent(EVENT_INTRO_5, 4000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_5:
                        if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        {
                            if (Creature* varian = ObjectAccessor::GetCreature(*me, varianGUID))
                                varian->AI()->Talk(SAY_INTRO_HERALD_5);
                        }
                        else
                            if (Creature* garrosh = ObjectAccessor::GetCreature(*me, garroshGUID))
                                garrosh->AI()->Talk(SAY_INTRO_HERALD_5);
                        events.ScheduleEvent(EVENT_INTRO_6, 6000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_6:
                        if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        {
                            if (Creature* proudmoore = ObjectAccessor::GetCreature(*me, proudmooreGUID))
                                proudmoore->AI()->Talk(H_SAY_INTRO_HERALD_6);
                        }
                        else
                            if (Creature* thrall = ObjectAccessor::GetCreature(*me, thrallGUID))
                                thrall->AI()->Talk(A_SAY_INTRO_HERALD_6);
                        events.ScheduleEvent(EVENT_INTRO_7, 6000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_7:
                        if (Creature* tirion = ObjectAccessor::GetCreature(*me, tirionGUID))
                            tirion->AI()->Talk(SAY_INTRO_HERALD_7);
                        events.ScheduleEvent(EVENT_INTRO_8, 1000, 0, PHASE_INTRO);
                        break;
                    case EVENT_INTRO_8:
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        events.SetPhase(PHASE_INPROGRESS);
                        break;
                    case EVENT_SUMMON_FACTION_2:
                        DoSummonGrandChampion(uiSecondBoss);
                        events.ScheduleEvent(EVENT_SUMMON_FACTION_3, 10000, 0, PHASE_INPROGRESS);
                        break;
                    case EVENT_SUMMON_FACTION_3:
                        DoSummonGrandChampion(uiThirdBoss);
                        break;
                    case EVENT_AGGRO_FACTION:
                        if (!Champion1List.empty())
                        {
                            for(GuidList::const_iterator itr = Champion1List.begin(); itr != Champion1List.end(); ++itr)
                                if (Creature* summon = ObjectAccessor::GetCreature(*me, *itr))
                                    AggroAllPlayers(summon);
                        }
                        break;
                    case EVENT_PALETRESS_1:
                        Talk(SAY_PALETRESS_INTRO_1);
                        Talk(SAY_PALETRESS_INTRO_2);
                        events.ScheduleEvent(EVENT_PALETRESS_2, 5000, 0, PHASE_INPROGRESS);
                        break;
                    case EVENT_PALETRESS_2:
                        if (Creature* argentchamp = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARGENT_CHAMPION)))
                            argentchamp->AI()->Talk(SAY_PALETRESS_INTRO_3);
                        events.ScheduleEvent(EVENT_PALETRESS_3, 5000);
                    case EVENT_PALETRESS_3:
                        if (Creature* argentchamp = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARGENT_CHAMPION)))
                            argentchamp->AI()->Talk(SAY_PALETRESS_INTRO_4);
                        events.CancelEvent(EVENT_PALETRESS_3);
                        break;
                    case EVENT_EADRIC_1:
                        Talk(SAY_EADRIC_INTRO_1);
                        Talk(SAY_EADRIC_INTRO_2);
                        events.ScheduleEvent(EVENT_EADRIC_2, 5000);
                        break;
                    case EVENT_EADRIC_2:
                        if (Creature* argentchamp = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARGENT_CHAMPION)))
                            argentchamp->AI()->Talk(SAY_EADRIC_INTRO_3);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_herald_toc5AI (creature);
    };
};

void AddSC_trial_of_the_champion()
{
    new npc_herald_toc5();
}
