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

#include "Chat.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ReputationMgr.h"
#include "MapManager.h"
#include "Group.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"

class custom_commandscript : public CommandScript
{
    public:
        custom_commandscript() : CommandScript("custom_commandscript") { }

        ChatCommand* GetCommands() const override
        {
            static ChatCommand questcompleterCommandTable[] =
            {
                { "add",              rbac::RBAC_PERM_COMMAND_QUESTCOMPLETER_ADD,     true, &HandleQuestCompleterAddCommand,     "", NULL },
                { "del",              rbac::RBAC_PERM_COMMAND_QUESTCOMPLETER_DEL,     true, &HandleQuestCompleterDelCommand,     "", NULL },
                { "",                 rbac::RBAC_PERM_COMMAND_QUESTCOMPLETER_STATUS,  true, &HandleQuestCompleterStatusCommand,  "", NULL },
                { NULL,               0,                                              false, NULL,                               "", NULL }
            };

            static ChatCommand commandTable[] =
            {
                { "qc",              rbac::RBAC_PERM_COMMAND_QUESTCOMPLETER,          true,  NULL,                               "", questcompleterCommandTable },
                { NULL,              0,                                               false, NULL,                               "", NULL }
            };
            return commandTable;
        }

        static bool HandleQuestCompleterCompHelper(Player* player, uint32 entry, ChatHandler* handler)
        {
            // actual code for completing
            Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

            //If player doesnt have the quest
            if (!quest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
            {
                handler->PSendSysMessage("Quest not in your quest log.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            // Add quest items for quests that require items
            for(uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
            {
                uint32 id = quest->RequiredItemId[x];
                uint32 count = quest->RequiredItemCount[x];
                if (!id || !count)
                    continue;

                uint32 curItemCount = player->GetItemCount(id, true);

                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count-curItemCount);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, id, true);
                    player->SendNewItem(item, count-curItemCount, true, false);
                }
            }

            // All creature/GO slain/cast (not required, but otherwise it will display "Creature slain 0/10")
            for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            {
                int32 creature = quest->RequiredNpcOrGo[i];
                uint32 creatureCount = quest->RequiredNpcOrGoCount[i];

                if (creature > 0)
                {
                    if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creature))
                        for (uint16 z = 0; z < creatureCount; ++z)
                            player->KilledMonster(creatureInfo, ObjectGuid::Empty);
                }
                else if (creature < 0)
                    for (uint16 z = 0; z < creatureCount; ++z)
                        player->KillCreditGO(creature, ObjectGuid::Empty);
            }
            // If the quest requires reputation to complete
            if (uint32 repFaction = quest->GetRepObjectiveFaction())
            {
                uint32 repValue = quest->GetRepObjectiveValue();
                uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
                if (curRep < repValue)
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                        player->GetReputationMgr().SetReputation(factionEntry, repValue);
            }

            // If the quest requires a SECOND reputation to complete
            if (uint32 repFaction = quest->GetRepObjectiveFaction2())
            {
                uint32 repValue2 = quest->GetRepObjectiveValue2();
                uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
                if (curRep < repValue2)
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                        player->GetReputationMgr().SetReputation(factionEntry, repValue2);
            }

            // If the quest requires money
            int32 ReqOrRewMoney = quest->GetRewOrReqMoney();
            if (ReqOrRewMoney < 0)
                player->ModifyMoney(-ReqOrRewMoney);

            handler->PSendSysMessage("%s completed!", quest->GetTitle().c_str());
            player->CompleteQuest(entry);
            return true;
        }

        static bool HandleQuestCompleterStatusCommand(ChatHandler* handler, char const* args)
        {
            char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
            if (!cId)
            {
                handler->PSendSysMessage("Syntax: .qc $quest\n\nSearches Quest Completer to see if $quest is bugged.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            uint32 entry = atol(cId);
            Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
            if (!quest)
            {
                handler->PSendSysMessage("Please enter a quest link.");
                handler->SetSentErrorMessage(true);
                return false;
            }
            else
            {
                if (entry != 0)
                {
                    uint32 checked = 0;
                    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_QUESTCOMPLETER);
                    stmt->setUInt32(0, entry);
                    std::string questTitle = quest->GetTitle();
                    PreparedQueryResult resultCheck = LoginDatabase.Query(stmt);

                    if (!resultCheck)
                    {
                        handler->PSendSysMessage("Error: no results.");
                        handler->SetSentErrorMessage(true);
                        return false;
                    }

                    checked = (*resultCheck)[0].GetUInt32();
                    if (checked == 1)
                    {
                        std::string name;
                        const char* playerName = handler->GetSession() ? handler->GetSession()->GetPlayer()->GetName().c_str() : NULL;
                        if (playerName)
                        {
                            name = playerName;
                            normalizePlayerName(name);
                            Player* player = sObjectAccessor->FindPlayerByName(name);
                            Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
                            if (!quest || player->GetQuestStatus(entry) != QUEST_STATUS_INCOMPLETE)
                            {
                                handler->PSendSysMessage("%s is bugged!", questTitle.c_str());
                                return true;
                            }
                            else
                            {
                                HandleQuestCompleterCompHelper(player, entry, handler);
                                return true;
                            }
                        }
                        else
                        {
                            handler->PSendSysMessage("%s is bugged!", questTitle.c_str());
                            return true;
                        }
                    }
                    else
                    {
                        handler->PSendSysMessage("%s is not bugged!", questTitle.c_str());
                        return true;
                    }
                }
                else
                {
                    handler->PSendSysMessage("Please enter a quest link.");
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
        }

        static bool HandleQuestCompleterAddCommand(ChatHandler* handler, char const* args)
        {
            char* cId = handler->extractKeyFromLink((char*)args, "Hquest");

            if (!cId)
            {
                handler->PSendSysMessage("Syntax: .qc add $quest\n\nAdds $quest to the quest completer.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            uint32 entry = atol(cId);
            Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
            if (!quest)
            {
                handler->PSendSysMessage("Please enter a quest link.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            // Change from string to int to prevent crash with query
            uint32 checked = 0;
            std::string questTitle = quest->GetTitle();
            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_QUESTCOMPLETER);
            stmt->setUInt32(0, entry);
            PreparedQueryResult resultCheck = LoginDatabase.Query(stmt);

            if (!resultCheck)
            {
                handler->PSendSysMessage("Error: no results.");
            }

            checked = (*resultCheck)[0].GetUInt32();

            if (checked == 1)
                handler->PSendSysMessage("%s is already in Quest Completer!", questTitle.c_str());
            else
            {
                if (entry != 0)
                {
                    PreparedStatement* stmt = NULL;
                    stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_QUESTCOMPLETER);
                    stmt->setUInt32(0, entry);
                    LoginDatabase.Execute(stmt);
                    handler->PSendSysMessage("%s was added!", questTitle.c_str());
                }
                else
                    handler->PSendSysMessage("There was a error with your request.");
            }
            return true;
        }

        static bool HandleQuestCompleterDelCommand(ChatHandler* handler, char const* args)
        {
            char* cId = handler->extractKeyFromLink((char*)args, "Hquest");

            if (!cId)
            {
                handler->PSendSysMessage("Syntax: .qc del $quest\n\nDeletes $quest from the quest completer.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            uint32 entry = atol(cId);
            Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
            if (!quest)
            {
                handler->PSendSysMessage("Please enter a quest link.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            // Change from string to int to prevent crash with query
            uint32 checked = 0;
            std::string questTitle = quest->GetTitle();
            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_QUESTCOMPLETER);
            stmt->setUInt32(0, entry);
            PreparedQueryResult resultCheck = LoginDatabase.Query(stmt);

            if (!resultCheck)
            {
                handler->PSendSysMessage("Error: no results.");
            }

            checked = (*resultCheck)[0].GetUInt32();

            if (checked == 0)
                handler->PSendSysMessage("%s is not in the Quest Completer.", questTitle.c_str());
            else
            {
                if (entry != 0)
                {
                    PreparedStatement* stmt = NULL;
                    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_QUESTCOMPLETER);
                    stmt->setUInt32(0, entry);
                    LoginDatabase.Execute(stmt);
                    handler->PSendSysMessage("%s was removed!", questTitle.c_str());
                }
                else
                    handler->PSendSysMessage("There was a error with your request.");
            }
            return true;
        }
};

void AddSC_custom_commandscript()
{
    new custom_commandscript();
}
