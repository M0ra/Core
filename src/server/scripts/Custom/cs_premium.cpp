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

#include "Language.h"
#include "World.h"
#include "AuctionHouseMgr.h"

class premium_commandscript : public CommandScript
{
public:
    premium_commandscript() : CommandScript("premium_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand premiumCommandTable[] =
        {
            { "bank",  SEC_PLAYER, false, &HandlePremiumBankCommand,    "", NULL },
            { "mail",  SEC_PLAYER, false, &HandlePremiumMailCommand,    "", NULL },
            { "auc",  SEC_PLAYER,  false, &HandlePremiumAuctionCommand, "", NULL },
            { NULL,    0,          false, NULL,                         "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "vip", SEC_PLAYER, false, NULL, "", premiumCommandTable },
            { NULL,  0,          false, NULL, "", NULL }
        };

        return commandTable;
    }

    static bool HandlePremiumBankCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *player = handler->GetSession()->GetPlayer();
        if (player->GetSession()->IsPremium() && sWorld->getBoolConfig(COMMAND_BANK_PREMIUM))
        {
            //Different Checks
            if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
            {
                handler->SendSysMessage(LANG_PREMIUM_CANT_DO);
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->GetSession()->SendShowBank(handler->GetSession()->GetPlayer()->GetGUID());
        }
        return true;
    }

    static bool HandlePremiumMailCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (player->GetSession()->IsPremium() && sWorld->getBoolConfig(COMMAND_MAIL_PREMIUM))
        {
            //Different Checks
            if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
            {
                handler->SendSysMessage(LANG_PREMIUM_CANT_DO);
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->GetSession()->SendShowMailBox(player->GetGUID());
        }
        return true;
    }

    static bool HandlePremiumAuctionCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* player = handler->GetSession()->GetPlayer();
        if(player->GetSession()->IsPremium())
        {
            //Different Checks
            if(player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
            {
                handler->SendSysMessage(LANG_PREMIUM_CANT_DO);
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->GetSession()->SendAuctionHello(player->GetGUID(),player->getFaction());

        }
        return true;
    }

};

void AddSC_premium_commandscript()
{
    new premium_commandscript();
}
