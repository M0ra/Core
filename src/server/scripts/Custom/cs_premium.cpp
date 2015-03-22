/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * dev Mora
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
#include "AuctionHouseMgr.h"
#include "Common.h"
#include "Chat.h"
#include "Player.h"
#include "World.h"
#include "Config.h"
#include "WorldSession.h"
#include "Language.h"
#include "Log.h"
#include "SpellAuras.h"

enum Misc
{
    NPC_TELE    = 100000
};

class premium_commandscript : public CommandScript
{
public:
    premium_commandscript() : CommandScript("premium_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand premiumCommandTable[] =
        {
            { "debuff",  SEC_PLAYER, false, &HandlePremiumDebuffCommand,    "", NULL },
            { "bank",  SEC_PLAYER, false, &HandlePremiumBankCommand,    "", NULL },
            { "mail",  SEC_PLAYER, false, &HandlePremiumMailCommand,    "", NULL },
            { "repair",  SEC_PLAYER, false, &HandlePremiumRepairCommand,    "", NULL },
            { "resettalents",  SEC_PLAYER, false, &HandlePremiumResetTalentsCommand,    "", NULL },
            { "taxi",  SEC_PLAYER, false, &HandlePremiumTaxiCommand,    "", NULL },
            //{ "auc",  SEC_PLAYER,  false, &HandlePremiumAuctionCommand, "", NULL },
            { NULL,    0,          false, NULL,                         "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "vip", SEC_PLAYER, false, NULL, "", premiumCommandTable },
            { NULL,  0,          false, NULL, "", NULL }
        };

        return commandTable;
    }

    static bool HandlePremiumDebuffCommand(ChatHandler* handler, const char* /*args*/)
    {   
        Player* player = handler->GetSession()->GetPlayer();

        if (!handler->GetSession()->IsPremium())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sWorld->getBoolConfig(COMMAND_DEBUFF_PREMIUM))
        {
            handler->SendSysMessage(LANG_VIP_COMMAND_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
        {
            handler->SendSysMessage(LANG_VIP_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->GetSession()->GetPlayer()->RemoveAurasDueToSpell(15007);
        handler->GetSession()->GetPlayer()->RemoveAurasDueToSpell(26013);

        return true;
    }

    static bool HandlePremiumBankCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (player->GetSession()->IsPremium() && sWorld->getBoolConfig(COMMAND_BANK_PREMIUM))
        {
            if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
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
                handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->GetSession()->SendShowMailBox(player->GetGUID());
        }
        return true;
    }

    static bool HandlePremiumRepairCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!handler->GetSession()->IsPremium())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sWorld->getBoolConfig(COMMAND_REPAIR_PREMIUM))
        {
            handler->SendSysMessage(LANG_VIP_COMMAND_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
        {
            handler->SendSysMessage(LANG_VIP_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->GetSession()->GetPlayer()->DurabilityRepairAll(false, 0, false);

        handler->PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetNameLink(handler->GetSession()->GetPlayer()).c_str());
        return true;
    }

    static bool HandlePremiumResetTalentsCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!handler->GetSession()->IsPremium())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sWorld->getBoolConfig(COMMAND_RESET_TALENTS_PREMIUM))
        {
            handler->SendSysMessage(LANG_VIP_COMMAND_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
        {
            handler->SendSysMessage(LANG_VIP_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->resetTalents(true);
        player->SendTalentsInfoData(false);
		handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->GetNameLink(handler->GetSession()->GetPlayer()).c_str());
        return true;
    }

    static bool HandlePremiumTaxiCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!handler->GetSession()->IsPremium())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sWorld->getBoolConfig(COMMAND_TAXI_PREMIUM))
        {
            handler->SendSysMessage(LANG_VIP_COMMAND_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
        {
            handler->SendSysMessage(LANG_VIP_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->GetPosition();
        player->SummonCreature(NPC_TELE, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ()+1.2, player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60*IN_MILLISECONDS);
 
		return true;
    }

    //static bool HandlePremiumAuctionCommand(ChatHandler* handler, char const* /*args*/)
    /*{
        Player* player = handler->GetSession()->GetPlayer();
        if(player->GetSession()->IsPremium())
        {
            //Different Checks
            if(player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead())
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_VIP);
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->GetSession()->SendAuctionHello(player->GetGUID());
            handler->GetSession()->SendAuctionHello(player->getFaction());

        }
        return true;
    }
*/
};

void AddSC_premium_commandscript()
{
    new premium_commandscript();
}