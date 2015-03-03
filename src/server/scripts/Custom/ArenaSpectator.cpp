/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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
#include "Chat.h"
#include "ArenaTeamMgr.h"
#include "BattlegroundMgr.h"
#include "WorldSession.h"
#include "Player.h"
#include "ArenaTeam.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "CreatureTextMgr.h"
#include "Config.h"

class arena_spectator_commands : public CommandScript
{
    public:
        arena_spectator_commands() : CommandScript("arena_spectator_commands") { }

        static bool HandleSpectateCommand(ChatHandler* handler, char const* args)
        {
            Player* target;
            ObjectGuid target_guid;
            std::string target_name;
            if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
                return false;

            Player* player = handler->GetSession()->GetPlayer();
            if (target == player || target_guid == player->GetGUID())
            {
                handler->PSendSysMessage("Вы не можете смотреть на себя.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (player->IsInCombat())
            {
                handler->PSendSysMessage("Вы находитесь в Бою.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!target)
            {
                handler->PSendSysMessage("Цель не существуют.");
                handler->SetSentErrorMessage(true);
                return false;
            }
			
            if (player->IsMounted())
            {
                handler->PSendSysMessage("Не можете смотреть сидя верхом.");
                handler->SetSentErrorMessage(true);
                return false;
            }			

            if (target && (target->HasAura(32728) || target->HasAura(32727))) // Check Arena Preparation thx XXsupr
            {
                handler->PSendSysMessage("Не могу этого сделать. Арена не началась.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (player->GetPet())
            {
                handler->PSendSysMessage("Вы должны скрыть своего питомца.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (player->GetMap()->IsBattlegroundOrArena() && !player->isSpectator())
            {
                handler->PSendSysMessage("Вы уже находитесь на поле битвы или арене.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            Map* cMap = target->GetMap();
            if (!cMap->IsBattleArena())
            {
                handler->PSendSysMessage("Игрок не на арене.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (player->GetMap()->IsBattleground())
            {
                handler->PSendSysMessage("Не могу сделать это, в то время как вы находитесь на поле боя.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            // all's well, set bg id
            // when porting out from the bg, it will be reset to 0
            player->SetBattlegroundId(target->GetBattlegroundId(), target->GetBattlegroundTypeId());
            // remember current position as entry point for return at bg end teleportation
            if (!player->GetMap()->IsBattlegroundOrArena())
                player->SetBattlegroundEntryPoint();

            if (target->isSpectator())
            {
                handler->PSendSysMessage("Не могу сделать этого. Ваша цель - зритель.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            // stop flight if need
            if (player->IsInFlight())
            {
                player->GetMotionMaster()->MovementExpired();
                player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                player->SaveRecallPosition();

            // search for two teams
            Battleground *bGround = target->GetBattleground();
            if (bGround->isRated())
            {
                uint32 slot = bGround->GetArenaType() - 2;
                if (bGround->GetArenaType() > 3)
                    slot = 2;
                uint32 firstTeamID = target->GetArenaTeamId(slot);
                uint32 secondTeamID = 0;
                Player *firstTeamMember  = target;
                Player *secondTeamMember = NULL;
                for (Battleground::BattlegroundPlayerMap::const_iterator itr = bGround->GetPlayers().begin(); itr != bGround->GetPlayers().end(); ++itr)
                    if (Player* tmpPlayer = ObjectAccessor::FindPlayer(itr->first))
                    {
                        if (tmpPlayer->isSpectator())
                            continue;

                        uint32 tmpID = tmpPlayer->GetArenaTeamId(slot);
                        if (tmpID != firstTeamID && tmpID > 0)
                        {
                            secondTeamID = tmpID;
                            secondTeamMember = tmpPlayer;
                            break;
                        }
                    }

                if (firstTeamID > 0 && secondTeamID > 0 && secondTeamMember)
                {
                    ArenaTeam *firstTeam  = sArenaTeamMgr->GetArenaTeamById(firstTeamID);
                    ArenaTeam *secondTeam = sArenaTeamMgr->GetArenaTeamById(secondTeamID);
                    if (firstTeam && secondTeam)
                    {
                        handler->PSendSysMessage("Вы вошли на Арену.");
                        handler->PSendSysMessage("Команды:");
                        handler->PSendSysMessage("%s - %s", firstTeam->GetName().c_str(), secondTeam->GetName().c_str());
                        handler->PSendSysMessage("%u(%u) - %u(%u)", firstTeam->GetRating(), firstTeam->GetAverageMMR(firstTeamMember->GetGroup()),
                                                                    secondTeam->GetRating(), secondTeam->GetAverageMMR(secondTeamMember->GetGroup()));
                    }
                }
            }

            // to point to see at target with same orientation
            float x, y, z;
            target->GetContactPoint(player, x, y, z);

            player->TeleportTo(target->GetMapId(), x, y, z, player->GetAngle(target), TELE_TO_GM_MODE);
            player->SetPhaseMask(target->GetPhaseMask(), true);
            player->SetSpectate(true);
            target->GetBattleground()->AddSpectator(player->GetGUID());

            return true;
        }
        
        static bool HandleSpectateCancelCommand(ChatHandler* handler, const char* /*args*/)
        {
            Player* player =  handler->GetSession()->GetPlayer();

            if (!player->isSpectator())
            {
                handler->PSendSysMessage("Вы не зритель.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            player->GetBattleground()->RemoveSpectator(player->GetGUID());
            player->CancelSpectate();
            player->TeleportToBGEntryPoint();

            return true;
        }

        static bool HandleSpectateFromCommand(ChatHandler* handler, const char *args)
        {
            Player* target;
            ObjectGuid target_guid;
            std::string target_name;
            if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
                return false;

            Player* player = handler->GetSession()->GetPlayer();

            if (!target)
            {
                handler->PSendSysMessage("Не могу найти игрока.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!player->isSpectator())
            {
                handler->PSendSysMessage("Вы не зритель, смотрит кто-то первее вас.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (target->isSpectator() && target != player)
            {
                handler->PSendSysMessage("Не могу этого сделать. Ваша цель - зритель.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (player->GetMap() != target->GetMap())
            {
                handler->PSendSysMessage("Не могу этого сделать. Различные арены?");
                handler->SetSentErrorMessage(true);
                return false;
            }

            // check for arena preperation
            // if exists than battle didn`t begin
            if (target->HasAura(32728) || target->HasAura(32727))
            {
                handler->PSendSysMessage("Не могу этого сделать. Арена не началась.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            (target == player && player->getSpectateFrom()) ? player->SetViewpoint(player->getSpectateFrom(), false) :
                                                                player->SetViewpoint(target, true);
            return true;
        }

        static bool HandleSpectateResetCommand(ChatHandler* handler, const char *args)
        {
            Player* player = handler->GetSession()->GetPlayer();

            if (!player)
            {
                handler->PSendSysMessage("Не могу найти игрока.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!player->isSpectator())
            {
                handler->PSendSysMessage("Вы не зритель!");
                handler->SetSentErrorMessage(true);
                return false;
            }

            Battleground *bGround = player->GetBattleground();
            if (!bGround)
                return false;

            if (bGround->GetStatus() != STATUS_IN_PROGRESS)
                return true;

            for (Battleground::BattlegroundPlayerMap::const_iterator itr = bGround->GetPlayers().begin(); itr != bGround->GetPlayers().end(); ++itr)
                if (Player* tmpPlayer = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (tmpPlayer->isSpectator())
                        continue;

                    uint32 tmpID = bGround->GetPlayerTeam(tmpPlayer->GetGUID());

                    // generate addon massage
                    std::string pName = tmpPlayer->GetName();
                    std::string tName = "";

                    if (Player *target = tmpPlayer->GetSelectedPlayer())
                        tName = target->GetName();

                    SpectatorAddonMsg msg;
                    msg.SetPlayer(pName);
                    if (tName != "")
                    msg.SetTarget(tName);
                    msg.SetStatus(tmpPlayer->IsAlive());
                    msg.SetClass(tmpPlayer->getClass());
                    msg.SetCurrentHP(tmpPlayer->GetHealth());
                    msg.SetMaxHP(tmpPlayer->GetMaxHealth());
                    Powers powerType = tmpPlayer->getPowerType();
                    msg.SetMaxPower(tmpPlayer->GetMaxPower(powerType));
                    msg.SetCurrentPower(tmpPlayer->GetPower(powerType));
                    msg.SetPowerType(powerType);
                    msg.SetTeam(tmpID);
                    msg.SendPacket(player->GetGUID());
                }

            return true;
        }

        ChatCommand* GetCommands() const
        {
            static ChatCommand spectateCommandTable[] =
            {
                { "player",         SEC_PLAYER,      true,  &HandleSpectateCommand,        "", NULL },
                { "view",           SEC_PLAYER,      true,  &HandleSpectateFromCommand,    "", NULL },
                { "reset",          SEC_PLAYER,      true,  &HandleSpectateResetCommand,   "", NULL },
                { "leave",          SEC_PLAYER,      true,  &HandleSpectateCancelCommand,  "", NULL },
                { NULL,             0,               false, NULL,                          "", NULL }
            };

            static ChatCommand commandTable[] =
            {
                { "spectate",       SEC_PLAYER, false,  NULL, "", spectateCommandTable },
                { NULL,             0,          false,  NULL, "", NULL }
            };
            return commandTable;
        }
};


enum NpcSpectatorAtions 
{
    // will be used for scrolling
    NPC_SPECTATOR_ACTION_LIST_GAMES         = 1000,
    NPC_SPECTATOR_ACTION_LIST_TOP_GAMES     = 2000,

    // NPC_SPECTATOR_ACTION_SELECTED_PLAYER + player.Guid()
    NPC_SPECTATOR_ACTION_SELECTED_PLAYER    = 3000
};

const uint16 TopGamesRating = 1800;
const uint8  GamesOnPage    = 15;

class npc_arena_spectator : public CreatureScript
{
    public:
        npc_arena_spectator() : CreatureScript("npc_arena_spectator") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Смотреть: 2v2 Игры", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Смотреть: 3v3 Игры", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_TOP_GAMES);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Смотреть конкретного игрока", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SELECTED_PLAYER);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            if (action >= NPC_SPECTATOR_ACTION_LIST_GAMES && action < NPC_SPECTATOR_ACTION_LIST_TOP_GAMES)
            {
                ShowPage(player, action - NPC_SPECTATOR_ACTION_LIST_GAMES, false);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }
            else if (action >= NPC_SPECTATOR_ACTION_LIST_TOP_GAMES && action < NPC_SPECTATOR_ACTION_LIST_TOP_GAMES)
            {
                ShowPage(player, action - NPC_SPECTATOR_ACTION_LIST_TOP_GAMES, true);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            }
            else
            {
                
                ObjectGuid guid = action - NPC_SPECTATOR_ACTION_SELECTED_PLAYER;
                if (Player* target = ObjectAccessor::FindPlayer(guid))
                {
                    ChatHandler handler(player->GetSession());
                    std::string str = target->GetName();
                    char* pTarget;
                    std::strcpy (pTarget, str.c_str());
                    arena_spectator_commands::HandleSpectateCommand(&handler, pTarget);
                }
            }
            return true;
        }

        std::string GetClassNameById(uint8 id)
        {
            std::string sClass = "";
            switch (id)
            {
                case CLASS_WARRIOR:         sClass = "Warrior ";        break;
                case CLASS_PALADIN:         sClass = "Pala ";           break;
                case CLASS_HUNTER:          sClass = "Hunt ";           break;
                case CLASS_ROGUE:           sClass = "Rogue ";          break;
                case CLASS_PRIEST:          sClass = "Priest ";         break;
                case CLASS_DEATH_KNIGHT:    sClass = "D.K. ";           break;
                case CLASS_SHAMAN:          sClass = "Shama ";          break;
                case CLASS_MAGE:            sClass = "Mage ";           break;
                case CLASS_WARLOCK:         sClass = "Warlock ";        break;
                case CLASS_DRUID:           sClass = "Druid ";          break;
            }
            return sClass;
        }

        std::string GetGamesStringData(Battleground* team, uint16 mmr)
        {
            std::string teamsMember[BG_TEAMS_COUNT];
            uint32 firstTeamId = 0;
            for (Battleground::BattlegroundPlayerMap::const_iterator itr = team->GetPlayers().begin(); itr != team->GetPlayers().end(); ++itr)
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    if (player->isSpectator())
                        continue;

                    uint32 team = itr->second.Team;
                    if (!firstTeamId)
                        firstTeamId = team;

                    teamsMember[firstTeamId == team] += GetClassNameById(player->getClass());
                }

            std::string data = teamsMember[0] + "(";
            std::stringstream ss;
            ss << mmr;
            data += ss.str();
            data += ") - " + teamsMember[1];
            return data;
        }

        ObjectGuid GetFirstPlayerGuid(Battleground* team)
        {
            for (Battleground::BattlegroundPlayerMap::const_iterator itr = team->GetPlayers().begin(); itr != team->GetPlayers().end(); ++itr)
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                    return itr->first;
            return ObjectGuid::Empty;
        }

        void ShowPage(Player* player, uint16 page, bool isTop)
        {
            uint16 highGames  = 0;
            uint16 lowGames   = 0;
            bool haveNextPage = false;
            for (uint8 i = BATTLEGROUND_NA; i <= BATTLEGROUND_RL; ++i)
            {
                if (!sBattlegroundMgr->IsArenaType((BattlegroundTypeId)i))
                    continue;

                BattlegroundContainer arenas = sBattlegroundMgr->GetBattlegroundsByType((BattlegroundTypeId)i);
		            for (BattlegroundContainer::const_iterator itr = arenas.begin(); itr != arenas.end(); ++itr)
                {
                    Battleground* arena = itr->second;

                    if (!arena->GetPlayersSize())
                        continue;

                    uint16 mmr = arena->GetArenaMatchmakerRating(0) + arena->GetArenaMatchmakerRating(1);
                    mmr /= 2;

                    if (isTop && mmr >= TopGamesRating)
                    {
                        highGames++;
                        if (highGames > (page + 1) * GamesOnPage)
                        {
                            haveNextPage = true;
                            break;
                        }

                        if (highGames >= page * GamesOnPage)
                            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, GetGamesStringData(arena, mmr), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SELECTED_PLAYER + GetFirstPlayerGuid(arena));
                    }
                    else if (!isTop && mmr < TopGamesRating)
                    {
                        lowGames++;
                        if (lowGames > (page + 1) * GamesOnPage)
                        {
                            haveNextPage = true;
                            break;
                        }

                        if (lowGames >= page * GamesOnPage)
                            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, GetGamesStringData(arena, mmr), GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_SELECTED_PLAYER + GetFirstPlayerGuid(arena));
                    }
                }
            }

            if (page > 0)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Предыдущая..", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES + page - 1);

            if (haveNextPage)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Следующая..", GOSSIP_SENDER_MAIN, NPC_SPECTATOR_ACTION_LIST_GAMES + page + 1);
        }
};


void AddSC_arena_spectator_script()
{
    new arena_spectator_commands();
    new npc_arena_spectator();
}