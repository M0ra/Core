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
#include "Config.h"
#include "World.h"

#define SQL_CODE "SELECT `CodeId`, `MailMoney`, `MaxCountForCharacter`, `MaxCountForAccount`, `MaxCountTotal`, `MailSubject`, `MailText` FROM `world_coded` WHERE `CodeStart` <= CURRENT_TIMESTAMP AND `CodeEnd` >= CURRENT_TIMESTAMP AND `CodeData` = '%s' LIMIT 1"
#define SQL_CODE_COUNT "SELECT COUNT(*) FROM `world_coded_history` WHERE `CodeId` = '%u'"
#define SQL_CODE_COUNT_ACCOUNT "SELECT COUNT(*) FROM `world_coded_history` WHERE `CodeId` = '%u' AND `AccountId` = '%u'"
#define SQL_CODE_COUNT_CHARACTER "SELECT COUNT(*) FROM `world_coded_history` WHERE `CodeId` = '%u' AND `CharacterGuid` = '%u'"
#define SQL_CODE_HISTORY "INSERT INTO `world_coded_history` (`CodeId`, `CharacterGuid`, `AccountId`, `SessionIp`) VALUES ('%u', '%u', '%u', '%s')"
#define SQL_CODE_ITEMS "SELECT `ItemId`, `ItemCount` FROM `world_coded_items` WHERE `CodeId` = '%u' AND (`ItemClassMask` & %u OR `ItemClassMask` = 0) AND (`ItemRaceMask` & %u OR `ItemRaceMask` = 0)"

class Mod_SpecialCode_CreatureScript : public CreatureScript
{
    public:
        Mod_SpecialCode_CreatureScript() : CreatureScript("Mod_SpecialCode_CreatureScript") { }

    bool OnGossipSelectCode(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 /*action*/, const char* code) override
    {
        if (!sWorld->getBoolConfig(CONFIG_SPECIAL_CODE))
            return false;

        if (!player || !*code)
            return false;

        std::string std_code(code);
        CharacterDatabase.EscapeString(std_code);
        QueryResult result = CharacterDatabase.PQuery(SQL_CODE, std_code.c_str());
        if (!result)
        {
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        uint32 codeId = (*result)[0].GetUInt32();
        std::string subject = (*result)[5].GetString();
        std::string text = (*result)[6].GetString();
        int32 mailMoney = (*result)[1].GetInt32();
        uint8 maxCharacters = (*result)[2].GetUInt8();
        uint8 maxAccount = (*result)[3].GetUInt8();
        uint8 maxTotal = (*result)[4].GetUInt8();
        uint16 currentTotal = 0;
        uint16 currentAccount = 0;
        uint16 currentCharacter = 0;

        if (maxTotal > 0)
        {
            result = CharacterDatabase.PQuery(SQL_CODE_COUNT, codeId);
            if (result)
                currentTotal = (*result)[0].GetUInt16();

            if (currentTotal >= maxTotal)
            {
                player->CLOSE_GOSSIP_MENU();
                return false;
            }
        }

        uint32 accountId = player->GetSession()->GetAccountId();

        if (maxAccount > 0)
        {
            result = CharacterDatabase.PQuery(SQL_CODE_COUNT_ACCOUNT, codeId, accountId);
            if (result)
                currentAccount = (*result)[0].GetUInt16();

            if (currentAccount >= maxAccount)
            {
                player->CLOSE_GOSSIP_MENU();
                return false;
            }
        }

        uint32 guid = player->GetGUID();

        if (maxCharacters > 0)
        {
            result = CharacterDatabase.PQuery(SQL_CODE_COUNT_CHARACTER, codeId, guid);
            if (result)
                currentCharacter = (*result)[0].GetUInt16();

            if (currentCharacter >= maxCharacters)
            {
                player->CLOSE_GOSSIP_MENU();
                return false;
            }
        }

        CharacterDatabase.PExecute(SQL_CODE_HISTORY, codeId, guid, accountId, player->GetSession()->GetRemoteAddress().c_str());

        MailDraft draft(subject, text);
        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        result = CharacterDatabase.PQuery(SQL_CODE_ITEMS, codeId, player->getClassMask(), player->getRaceMask());

        if (result)
        {
            typedef std::pair<uint32, uint32> ItemPair;
            typedef std::list< ItemPair > ItemPairs;
            ItemPairs items;

            do
            {
                Field* fields = result->Fetch();

                uint32 itemId = fields[0].GetUInt32();
                uint16 itemCount = fields[1].GetUInt16();

                ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(itemId);
                if (!itemProto)
                    continue;

                if (itemCount < 1 || (itemProto->MaxCount > 0 && itemCount > uint32(itemProto->MaxCount)))
                    continue;

                while (itemCount > itemProto->GetMaxStackSize())
                {
                    items.push_back(ItemPair(itemId, itemProto->GetMaxStackSize()));
                    itemCount -= itemProto->GetMaxStackSize();
                }

                items.push_back(ItemPair(itemId, itemCount));

                if (items.size() > MAX_MAIL_ITEMS)
                {
                    player->CLOSE_GOSSIP_MENU();
                    return false;
                }
            }
            while (result->NextRow());

            for (ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
            {
                if (Item* item = Item::CreateItem(itr->first, itr->second, 0))
                {
                    item->SaveToDB(trans);
                    draft.AddItem(item);
                }
            }
        }

        if (mailMoney > 0)
            draft.AddMoney(mailMoney);
        else
            draft.AddCOD(mailMoney*-1);

        draft.SendMailTo(trans, MailReceiver(player, receiverGuid.GetCounter()), MailSender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM));
        CharacterDatabase.CommitTransaction(trans);
        player->CLOSE_GOSSIP_MENU();

        return false;
    }
};

void AddSC_Mod_SpecialCode()
{
    new Mod_SpecialCode_CreatureScript();
}