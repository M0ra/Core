#include "ScriptPCH.h"
#include "Channel.h"
 
class System_Censure : public PlayerScript
{
public:
        System_Censure() : PlayerScript("System_Censure") {}
 
void CheckMessage(Player* player, string message)
{
	size_t stringpos;

	if(message.find("|TInterface") != string::npos)
		return;
		
	if(message.find("\n") != string::npos )
		return;

	if((stringpos = message.find("|Hquest:")) != string::npos)
		return;

	if((stringpos = message.find("|Htrade:")) != string::npos)
		return;

	if((stringpos = message.find("|Htalent:")) != string::npos)
		return;

	if((stringpos = message.find("|Henchant:")) != string::npos)
		return;

	if((stringpos = message.find("|Hachievement:")) != string::npos)
		return;

	if((stringpos = message.find("|Hglyph:")) != string::npos)
		return;

	if((stringpos = message.find("|Hspell:")) != string::npos)
		return;

	if((stringpos = message.find("Hitem:")) != string::npos)
		return;

	if(message.find("|c") != string::npos && message.find("|r") != string::npos)
		return;

	if(message.find("|c") != string::npos && message.find("|h") != string::npos)
		return;
 
    uint8 cheksSize = 27;
    std::string checks[27];
    
    checks[0] ="http://";
    checks[1] =".com";
    checks[2] =".www";
    checks[3] =".net";
    checks[4] =".org";
    checks[5] =".ru";
    checks[6] ="www.";
    checks[7] ="wow-";
    checks[8] ="-wow";
    checks[9] ="rondor";
    checks[10] ="no-ip";
    checks[11] =".zapto";
    checks[12] =".lt";
    checks[13] =".biz";
    checks[14] ="spzone";
    checks[15] ="fakewow";
    checks[16] ="deathside";
    checks[17] ="warsong";
    checks[18] ="RiverRise";
    checks[19] ="air-world";
    checks[20] ="wow-cool";
    checks[21] ="elgracia";
    checks[22] ="backkor";
    checks[23] ="isengard";
    checks[24] ="wowcircle";
    checks[25] ="izbooshka";
    checks[26] ="magic";
    
    for (int i = 0; i < cheksSize; ++i)
    {
		    if (message.find(checks[i]) != string::npos)
		    {
		    std::string say = "";
		    std::string str = "";
		    say = message;
		    ChatHandler(player->GetSession()).PSendSysMessage("|cffFF0000Реклама запрещена!");
		    SessionMap ss = sWorld->GetAllSessions();
		    for (SessionMap::const_iterator itr = ss.begin(); itr != ss.end(); ++itr)
			    if (itr->second->GetSecurity() > 0)
				    str = "[Anti-Spam][" + string(player->GetName()) + "][" + say + "]";
		    WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
		    data << str;
		    sWorld->SendGlobalGMMessage(&data);
		    return;
	     }
    }

} 
};
 
void AddSC_System_Censure()
{
    new System_Censure();
}
