/*
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
 
class NoFarming : public PlayerScript
{
    public: NoFarming() : PlayerScript("NoFarming") { }
 
    void OnPVPKill(Player* killer, Player* killed)
    {
        if (killer->GetGUID() == killed->GetGUID())
        {
            return;
        }
 
        if (killer->GetSession()->GetRemoteAddress() == killed->GetSession()->GetRemoteAddress() || killed->GetMaxHealth() <= 10000) // Normally players try farm with new characters with low health then modify the Health according your server.
        {
            std::string str = "";
            SessionMap ss = sWorld->GetAllSessions();
            for (SessionMap::const_iterator itr = ss.begin(); itr != ss.end(); ++itr)
                if (itr->second->GetSecurity() > 0)
                    str = "|cFFFFFC00[Anti-Farm]|cFF00FFFF[|cFF60FF00" + std::string(killer->GetName()) + "|cFF00FFFF] Возможно фармер.";
                WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
                data << str;
                sWorld->SendGlobalGMMessage(&data);
        } else {
            return;
        }
    }
};

void AddSC_NoFarming()
{
   new NoFarming;
}