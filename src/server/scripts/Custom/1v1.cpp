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
#include "ArenaTeamMgr.h"
#include "Common.h"
#include "DisableMgr.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ArenaTeam.h"
#include "Language.h"

#define ARENA_1V1_MIN_LEVEL 80      // min level to create an arenateam
#define ARENA_1V1_COST 4000 * 10000 // costs for create a team: 4000 gold

class npc_1v1arena : public CreatureScript  
{
public:
    npc_1v1arena() : CreatureScript("npc_1v1arena") 
    {
    }

    bool JoinQueueArena(Player* player, Creature* /*me*/)
    {
        if (ARENA_1V1_MIN_LEVEL > player->getLevel())
            return false;

        uint8 arenaslot = ArenaTeam::GetSlotByType(ARENA_TEAM_1v1);
        uint8 arenatype = ARENA_TYPE_1v1;
        uint32 arenaRating = 0;
        uint32 matchmakerRating = 0;
        bool isRated = true;

        // ignore if we already in BG or BG queue
        if (player->InBattleground())
            return false;

        // check existance
        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
        if (!bg)
        {
            //TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Battleground: template bg (all arenas) not found");
            return false;
        }

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_AA, NULL))
        {
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_ARENA_DISABLED);
            return false;
        }

        BattlegroundTypeId bgTypeId = bg->GetTypeID();
        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, arenatype);
        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
        if (!bracketEntry)
            return false;

        GroupJoinBattlegroundResult err = ERR_GROUP_JOIN_BATTLEGROUND_FAIL;

        // check if already in queue
        if (player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        // player is already in this queue
            return false;
        // check if has free queue slots
        if (!player->HasFreeBattlegroundQueueId())
            return false;

        uint32 ateamId = 0;

        ateamId = player->GetArenaTeamId(arenaslot);
        ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(ateamId);
        if (!at)
        {
            player->GetSession()->SendNotInArenaTeamPacket(arenatype);
            return false;
        }
        // get the team rating for queueing
        arenaRating = at->GetRating();
        matchmakerRating = arenaRating;
        // the arenateam id must match for everyone in the group
        if (arenaRating <= 0)
            arenaRating = 1;

        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        bg->SetRated(isRated);

        GroupQueueInfo* ginfo = bgQueue.AddGroup(player, NULL, bgTypeId, bracketEntry, arenatype, isRated, false, arenaRating, matchmakerRating, ateamId);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        uint32 queueSlot = player->AddBattlegroundQueueId(bgQueueTypeId);

        WorldPacket data;
        // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0, arenatype, 0);
        player->GetSession()->SendPacket(&data);

        sBattlegroundMgr->ScheduleQueueUpdate(matchmakerRating, arenatype, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());

        return true;
    }

    bool CreateArenateam(Player* player, Creature* /*me*/)
    {
        uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_1v1);
        if (slot >= MAX_ARENA_SLOT)
            return false;

        // Check if player is already in an arena team
        if (player->GetArenaTeamId(slot))
        {
            player->GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, player->GetName(), "", ERR_ALREADY_IN_ARENA_TEAM);
            return false;
        }

        // Teamname = playername
        // if teamname exist, we have to choose another name (playername  number)
        int i = 1;
        std::stringstream teamName;
        teamName << player->GetName();
        do
        {
            if (sArenaTeamMgr->GetArenaTeamByName(teamName.str()) != NULL) // teamname exist, so choose another name
            {
                teamName.str(std::string());
                teamName << player->GetName() << i;
            }
            else
                break;
        } while (i < 100); // should never happen

        // Create arena team
        ArenaTeam* arenaTeam = new ArenaTeam();

        if (!arenaTeam->Create(player->GetGUID(), ARENA_TEAM_1v1, teamName.str(), 0, 0, 0, 0, 0))
        {
            delete arenaTeam;
            return false;
        }

        // Register arena team
        sArenaTeamMgr->AddArenaTeam(arenaTeam);
        arenaTeam->AddMember(player->GetGUID());

        ChatHandler(player->GetSession()).SendSysMessage("1х1 команда успешно создана!");

        return true;
    }

    bool OnGossipHello(Player* player, Creature* me) override
    {
        if (player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_1v1)) == NULL)
            player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_CHAT, "|TInterface/ICONS/Achievement_Arena_2v2_7:30|t Создать 1х1 Команду", GOSSIP_SENDER_MAIN, 1, "Создать 1х1 Команду?", ARENA_1V1_COST, false);
        else
        {
            if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_1v1))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/Achievement_Arena_2v2_7:30|t Оставить 1х1 Арена", GOSSIP_SENDER_MAIN, 3);
            else
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/Achievement_Arena_3v3_5:30|t Соединение 1х1 Арена", GOSSIP_SENDER_MAIN, 2);
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_CHAT, "|TInterface/ICONS/Achievement_Arena_2v2_7:30|t Распустить Команду.", GOSSIP_SENDER_MAIN, 5, "Вы уверены?", 0, false);
            }

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/INV_Misc_Coin_01:30|t Показать статистику.", GOSSIP_SENDER_MAIN, 4);
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/INV_Misc_Coin_03:30|t Как использовать NPC?", GOSSIP_SENDER_MAIN, 8);
        player->SEND_GOSSIP_MENU(68, me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* me, uint32 /*uiSender*/, uint32 uiAction) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (uiAction)
        {
            case 1: // Create new Arenateam
            {
                if (ARENA_1V1_MIN_LEVEL <= player->getLevel())
                {
                    if (player->GetMoney() >= ARENA_1V1_COST && CreateArenateam(player, me))
                        player->ModifyMoney(-(int32)ARENA_1V1_COST);
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вам нужен %u+ уровень, чтобы создать 1х1 команду Арены.", ARENA_1V1_MIN_LEVEL);
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }
            }
            break;
            case 2: // Join Queue Arena
            {
                if (JoinQueueArena(player, me) == false)
                    ChatHandler(player->GetSession()).SendSysMessage("Что-то пошло не так при присоединении к очереди.");
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
            break;
            case 3: // Leave Queue
            {
                WorldPacket Data;
                Data << (uint8)0x1 << (uint8)0x0 << (uint32)BATTLEGROUND_AA << (uint16)0x0 << (uint8)0x0;
                player->GetSession()->HandleBattleFieldPortOpcode(Data);
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
            break;
            case 4: // get statistics
            {
                ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_1v1)));
                if (at)
                {
                    std::stringstream s;
                    s << "Рейтинг: " << at->GetStats().Rating;
                    s << "\nРанг: " << at->GetStats().Rank;
                    s << "\nСезон Игры: " << at->GetStats().SeasonGames;
                    s << "\nПобедные сезоны: " << at->GetStats().SeasonWins;
                    s << "\nИгры недели: " << at->GetStats().WeekGames;
                    s << "\nПобедные недели: " << at->GetStats().WeekWins;
                    ChatHandler(player->GetSession()).PSendSysMessage(s.str().c_str());
                }
            }
            break;
            case 5: // Disband arenateam
            {
                WorldPacket Data;
                Data << (uint32)player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_1v1));
                player->GetSession()->HandleArenaTeamLeaveOpcode(Data);
                ChatHandler(player->GetSession()).PSendSysMessage("Команда арены удалена!");
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
            break;
            case 8: // Script Info
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Создать новую команду 1х1 Арена", GOSSIP_SENDER_MAIN, uiAction);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Присоединиться 1х1 Арена и вперёд!", GOSSIP_SENDER_MAIN, uiAction);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Наслаждаться!", GOSSIP_SENDER_MAIN, uiAction);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "<- Назад", GOSSIP_SENDER_MAIN, 7);
                player->SEND_GOSSIP_MENU(68, me->GetGUID());
                return true;
            }
            break;
        }

        OnGossipHello(player, me);
        return true;
    }
};

void AddSC_npc_1v1arena()
{
    new npc_1v1arena();
}
