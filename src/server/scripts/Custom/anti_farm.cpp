#include "ScriptPCH.h"
 
class NoFarming : public PlayerScript
{
public: NoFarming() : PlayerScript("NoFarming") {}
 
void OnPVPKill(Player * killer, Player * killed)
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
                str = "|cFFFFFC00[Anti-Farm System]|cFF00FFFF[|cFF60FF00" + std::string(killer->GetName()) + "|cFF00FFFF] Possivel Farmer |CFFEE0000 [ATENÇÃO GMS!]";
                WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
                data << str;
                sWorld->SendGlobalGMMessage(&data);
        }
        else
        {
                return;
        }
 
}
 
};
void AddSC_NoFarming()
{
   new NoFarming;
}
