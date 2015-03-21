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

class Reset_OnDuelEnd : public PlayerScript
{
public:
    Reset_OnDuelEnd() : PlayerScript("Reset_OnDuelEnd") { }

    void OnDuelEnd(Player *winner, Player *looser, DuelCompleteType /*uiType*/)
    {
        // Resets Players 1
        winner->RemoveArenaSpellCooldowns();
        winner->SetHealth(winner->GetMaxHealth());
        winner->SetPower(winner->getPowerType(), winner->GetMaxPower(winner->getPowerType()));

        // Resets Player 2
        looser->RemoveArenaSpellCooldowns();
        looser->SetHealth(looser->GetMaxHealth());
        looser->SetPower(looser->getPowerType(), looser->GetMaxPower(looser->getPowerType()));
    }
};

void AddSC_Reset()
{
    new Reset_OnDuelEnd;
}