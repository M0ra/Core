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
 

#include "../../../game/Anticheat/AnticheatMgr.h"
#include "Log.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Corpse.h"
#include "Config.h"
#include "Player.h"
#include "Vehicle.h"
#include "Transport.h"
#include "ObjectMgr.h"
#include "MapManager.h"

#define CLIMB_ANGLE 1.9f

enum WardenCheckType
{
    MEM_CHECK               = 0xF3,                         
    PAGE_CHECK_A            = 0xB2,                        
    PAGE_CHECK_B            = 0xBF,                         
    MPQ_CHECK               = 0x98,                         
    LUA_STR_CHECK           = 0x8B,                         
    DRIVER_CHECK            = 0x71,                         
    TIMING_CHECK            = 0x57,                         
    PROC_CHECK              = 0x7E,                         
    MODULE_CHECK            = 0xD9                    
};

AntiCheat::AntiCheat(Player* new_plMover)
{
    plMover = new_plMover;

    number_cheat_find = 0;

    activateACCheck = false;
    disableACCheckTimer = 5000;
    alarmACCheckTimer = 0; 
    ac_goactivate = 0;

    for (int i = 0; i < MAX_CHEAT; i++)
    m_CheatList[i] = 0;
    m_CheatList_reset_diff = sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA);
	
    m_anti_LastServerTime  = 0;          // last movement server time
    m_anti_DeltaServerTime = 0;          // server side session time

    m_anti_TeleToPlane_Count = 0;        // Teleport To Plane alarm counter

    m_logfile_time = 0;                  // Time for logs file
    m_logdb_time = 0;                    // Time for logs DB

	  cheat_find = false;
    warden_cheat_find = false;
    map_count = true;
    map_block = true;
    map_puni = true;
}

void AntiCheat::UpdateDiffAntiCheat(uint32 diff)
{
    if (ac_goactivate > 0)
    {
        activateACCheck = true;
    }

    if (alarmACCheckTimer <= diff)
    {
        alarmACCheckTimer = 0;
        if (!activateACCheck)
        {
            if (disableACCheckTimer <= diff)
            {
                activateACCheck = true;
                ac_goactivate = sWorld->getIntConfig(CONFIG_AC_ALIVE_COUNT);
            } 
            else 
                disableACCheckTimer -= diff;
        }
    } 
    else
    {
        alarmACCheckTimer -= diff;
        activateACCheck = true;
    }
}

void AntiCheat::UpdateAntiCheat()
{
    if (ac_goactivate > 0)
    {
        ac_goactivate--;
        return;
    }

    if (!alarmACCheckTimer)
    {
        disableACCheckTimer = sWorld->getIntConfig(CONFIG_AC_SLEEP_DELTA);
        activateACCheck = false;
    }
}

void AntiCheat::SetAlarm(uint32 delta)
{
    alarmACCheckTimer = delta;
}

void AntiCheat::SetSleep(uint32 delta)
{
    disableACCheckTimer = delta; // Set sleep
    alarmACCheckTimer = 0; // Disable alert
    activateACCheck = false;
    ac_goactivate = 0;
}

void AntiCheat::ResetCheatList(uint32 diff)
{
    if (sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA) == 0)
        return;

    if (m_CheatList_reset_diff >= diff)
		m_CheatList_reset_diff -= diff;
	  else 
		m_CheatList_reset_diff = 0;

	  if (m_CheatList_reset_diff == 0)
    {
		for (int i = 0; i < MAX_CHEAT; i++)
            m_CheatList[i] = 0;
        number_cheat_find = 0;
        warden_cheat_find = false;

        m_CheatList_reset_diff = sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA);
    }
}

bool AntiCheat::DoAntiCheatCheck(uint16 opcode, MovementInfo& pMovementInfo, Unit *mover)
{
	if (plMover->GetSession() && plMover->GetSession()->GetSecurity() >= int32(sWorld->getIntConfig(CONFIG_AC_DISABLE_GM_LEVEL)))
		return true;
    	
    //if (!plMover->IsInWorld())
    //    return true;

    if (!activateACCheck || plMover->isInFlight() || 
        plMover->GetTransport() || plMover->GetVehicle() || 
        !plMover->CanFreeMove() || plMover->IsBeingTeleported())
    {
        SaveLastPacket(pMovementInfo);
        SetLastOpcode(opcode);
        return true;        
    }

	  // Calc Delthas for AntiCheat
	  CalcDeltas(pMovementInfo, GetLastPacket());
        
    cheat_find = false;
    map_count = !sWorld->iIgnoreMapIds_ACCount.count(plMover->GetMapId());
    map_block = !sWorld->iIgnoreMapIds_ACBlock.count(plMover->GetMapId());
    map_puni = !sWorld->iIgnoreMapIds_ACPuni.count(plMover->GetMapId());

    // Clean player cheatlist only if we founded a cheat
    if (number_cheat_find)
        ResetCheatList(cServerTimeDelta);

    // Map ignored
    if (sWorld->iIgnoreMapIds_AC.count(plMover->GetMapId()))
    {
        // Go to sleep
        SetSleep(int32(sWorld->getIntConfig(CONFIG_AC_SLEEP_DELTA)));
        return true;
    }

	  // Set to false if block a Cheat
	  bool check_passed = true;

    CalcVariables(GetLastPacket(), pMovementInfo, mover);

    // MultiJump Cheat
    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTIMULTIJUMP))
        if (!CheckAntiMultiJump(pMovementInfo, opcode))
		        check_passed = false;

    // Speed Cheat
    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTISPEED))
	    if (!CheckAntiSpeed(GetLastPacket(), pMovementInfo, opcode))
		    check_passed = false;

    // Tele Cheat
    // if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTITELE))
	  // if (!CheckAntiTele(pMovementInfo, opcode))
	  // check_passed = false;

    // Fly Cheat
    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTIFLY))
	    if (!CheckAntiFly(GetLastPacket(), pMovementInfo))
		    check_passed = false;

    // Waterwalk Cheat
    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTIWATERWALK))
        if (!CheckAntiWaterwalk(GetLastPacket(), pMovementInfo))
		    check_passed = false;

    // Tele To Plane Cheat
    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTITELETOPLANE))
	    if (!CheckAntiTeleToPlane(GetLastPacket(), pMovementInfo))
		    check_passed = false;

    // Climb Cheat
    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_ANTICLIMB))
	    if (!CheckAntiClimb(GetLastPacket(), pMovementInfo, opcode))
		    check_passed = false;

    if (cheat_find)
    {
        if (map_count)
        {
            // Yes, we found a cheater
            ++(number_cheat_find);

            if (sWorld->getIntConfig(CONFIG_AC_REPORTS_FOR_GM_WARNING) &&
                number_cheat_find > sWorld->getIntConfig(CONFIG_AC_REPORTS_FOR_GM_WARNING)) 
            {
                // display warning at the center of the screen, hacky way.
                std::string str = "";
                str = "|cFFFFFC00[LordCraft - Anti-Cheat]|cFF00FFFF[|cFF60FF00" + std::string(plMover->GetName()) + "|cFF00FFFF] Possible cheater!";
                WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
                data << str;
                sWorld->SendGlobalGMMessage(&data);
            }
        }
        // We are are not going to sleep
        SetAlarm(sWorld->getIntConfig(CONFIG_AC_ALARM_DELTA));
        // Increase reset cheat list time
        if (m_CheatList_reset_diff < sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA_FOUND))
            m_CheatList_reset_diff = sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA_FOUND);
        if (map_puni)
        {
            if (!AntiCheatPunisher()) 
                check_passed = false;                
        }
    }

    SaveLastPacket(pMovementInfo);
    SetLastOpcode(opcode);

    UpdateAntiCheat();

	return check_passed;
}

bool AntiCheat::ControllPunisher()
{
    if (sWorld->getIntConfig(CONFIG_AC_ANTIMULTIJUMP_PUNI_COUNT) && 
        m_CheatList[CHEAT_MULTIJUMP] >= sWorld->getIntConfig(CONFIG_AC_ANTIMULTIJUMP_PUNI_COUNT))
        return true;
    else if (sWorld->getIntConfig(CONFIG_AC_ANTISPEED_PUNI_COUNT) &&
        m_CheatList[CHEAT_SPEED] >= sWorld->getIntConfig(CONFIG_AC_ANTISPEED_PUNI_COUNT))
        return true;
    else if (sWorld->getIntConfig(CONFIG_AC_ANTITELE_PUNI_COUNT) && 
        m_CheatList[CHEAT_TELEPORT] >= sWorld->getIntConfig(CONFIG_AC_ANTITELE_PUNI_COUNT))
        return true;
    else if (sWorld->getIntConfig(CONFIG_AC_ANTIFLY_PUNI_COUNT) &&
        m_CheatList[CHEAT_FLY] >= sWorld->getIntConfig(CONFIG_AC_ANTIFLY_PUNI_COUNT))
        return true;
    else if (sWorld->getIntConfig(CONFIG_AC_ANTIWATERWALK_PUNI_COUNT) && 
        m_CheatList[CHEAT_WATERWALK] >= sWorld->getIntConfig(CONFIG_AC_ANTIWATERWALK_PUNI_COUNT))
        return true;
    else if (sWorld->getIntConfig(CONFIG_AC_ANTITELETOPLANE_PUNI_COUNT) && 
        m_CheatList[CHEAT_TELETOPLANE] >= sWorld->getIntConfig(CONFIG_AC_ANTITELETOPLANE_PUNI_COUNT))
        return true;
    else if (sWorld->getIntConfig(CONFIG_AC_ANTICLIMB_PUNI_COUNT) && 
        m_CheatList[CHEAT_CLIMB] >= sWorld->getIntConfig(CONFIG_AC_ANTICLIMB_PUNI_COUNT))
        return true;   

    return false;
}

bool AntiCheat::AntiCheatPunisher()
{
    if (!sWorld->getIntConfig(CONFIG_AC_PUNI_TYPE))
        return true;

    if (plMover->getLevel() > sWorld->getIntConfig(CONFIG_AC_PUNI_LEVEL_MAX))
        return true;

    if (!ControllPunisher())
        return true;

    if (!warden_cheat_find && sWorld->getBoolConfig(CONFIG_AC_WARDEN_FIND))
        return true;

    
    std::string announce = "";
    switch (sWorld->getIntConfig(CONFIG_AC_PUNI_TYPE))
    {
        case PUNI_NONE:
            return true;
        case PUNI_BLOCK:
            sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER BLOCK",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            break;
        case PUNI_KILL:
            plMover->DealDamage(plMover, plMover->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER KILL",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            break;
        case PUNI_KICK:
            plMover->GetSession()->KickPlayer();
            sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER KICK",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            announce = "AntiCheatPunisher ha Kickato il player ";
            announce += plMover->GetName();
            announce += "per uso di Hack";
            sWorld->SendServerMessage(SERVER_MSG_STRING,announce.c_str());
            break;
        case PUNI_BAN_CHAR:
            sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER BAN_CHARACTER",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());            
            announce = "AntiCheatPunisher ha bannato il player ";
            announce += plMover->GetName();
            announce += " per uso di Hack";
            sWorld->SendServerMessage(SERVER_MSG_STRING,announce.c_str());
            sWorld->BanCharacter(plMover->GetName(),sConfig->GetStringDefault("Anticheat.Punisher.BanTime", "-1"),"Cheat","AntiCheatPunisher");
            break;
        case PUNI_BAN_ACC:
            {
                sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER BAN_ACCOUNT",
                        plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
                announce = "AntiCheatPunisher ha bannato l'account del player ";
                announce += plMover->GetName();
                announce += " per uso di Hack";
                sWorld->SendServerMessage(SERVER_MSG_STRING,announce.c_str());
                sWorld->BanAccount(BAN_CHARACTER,plMover->GetName(),sConfig->GetStringDefault("Anticheat.Punisher.BanTime", "-1"),"Cheat","AntiCheatPunisher");
            }
            break;
        case PUNI_BAN_IP:
            {
                sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER BAN_IP",
                        plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
                announce = "AntiCheatPunisher ha bannato l'ip del player ";
                announce += plMover->GetName();
                announce += " per uso di Hack";
                sWorld->SendServerMessage(SERVER_MSG_STRING,announce.c_str());
                QueryResult result = LoginDatabase.PQuery("SELECT last_ip FROM account WHERE id=%u", plMover->GetSession()->GetAccountId());
                if (result)
                {
                    Field *fields = result->Fetch();
                    std::string LastIP = fields[0].GetString();
                    if(!LastIP.empty())
                    {
                        sWorld->BanAccount(BAN_IP,LastIP,sConfig->GetStringDefault("Anticheat.Punisher.BanTime", "-1"),"Cheat","AntiCheatPunisher");
                    }
                }                
            } break;
        default:
            sLog->outCheat("AC-Punisher-%s Map %u Area %u, X:%f Y:%f Z:%f, PUNISHER TYPE NOT VALID",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            break;
    }

    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_DBLOG))
         CharacterDatabase.PExecute("INSERT INTO cheat_log(cheat_type, guid, name, level, map, area, pos_x, pos_y, pos_z, date) VALUES ('%s', '%u', '%s', '%u', '%u', '%u', '%f', '%f', '%f', NOW())", 
                "AntiCheatPunisher", plMover->GetGUIDLow(), plMover->GetName(), plMover->getLevel(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
    return false;
}

void AntiCheat::CalcDeltas(MovementInfo& pNewPacket,  MovementInfo& pOldPacket)
{
    
	cServerTime = getMSTime();

    
	cServerTimeDelta = 1500;
	if (m_anti_LastServerTime != 0)
	{
        cServerTimeDelta = cServerTime - m_anti_LastServerTime;
		m_anti_LastServerTime = cServerTime;
	}
	else
		m_anti_LastServerTime = cServerTime;
    
    if (!m_logfile_time)
	    difftime_log_file = cServerTime - m_logfile_time;
    if (!m_logdb_time)
        difftime_log_db = cServerTime - m_logdb_time;
}

void AntiCheat::CalcVariables(MovementInfo& pOldPacket, MovementInfo& pNewPacket, Unit *mover)
{
	
    uint8 uiMoveType = 0;
    if (plMover->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING))
        uiMoveType = MOVE_SWIM;
    else if (plMover->IsFlying())
        uiMoveType = MOVE_FLIGHT;
    else if (plMover->HasUnitMovementFlag(MOVEMENTFLAG_WALKING))
        uiMoveType = MOVE_WALK;
    else
        uiMoveType = MOVE_RUN;
    
    
    uSpeedRate = (uint32)(plMover->GetSpeed(UnitMoveType(uiMoveType)) + pNewPacket.j_xyspeed);

    
    uDistance2D = (uint32)pNewPacket.pos.GetExactDist2d(&pOldPacket.pos);
    
    
    uiDiffTime_packets =  getMSTimeDiff(pOldPacket.time, pNewPacket.time);
    if (uiDiffTime_packets == 0)
        uiDiffTime_packets = 1;

   
    uClientSpeedRate = uDistance2D * 1000 / uiDiffTime_packets;

   
	fly_auras = CanFly(pNewPacket);

	bool swim_flags = pNewPacket.flags & MOVEMENTFLAG_SWIMMING;

    Position playerPos;
    plMover->GetPosition(&playerPos);

    float deltaZ = fabs(playerPos.GetPositionZ() - pNewPacket.pos.GetPositionZ());
    float deltaXY = pNewPacket.pos.GetExactDist2d(&playerPos);

    angle = MapManager::NormalizeOrientation(tan(deltaZ/deltaXY));
}

bool AntiCheat::CanFly(MovementInfo& pMovementInfo)
{
    if (plMover->IsUnderWater())
        return true;

    if (plMover->HasAuraType(SPELL_AURA_FLY) || 
        plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED) || 
        plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || 
        plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED) ||
        plMover->HasAuraType(SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS) || 
        plMover->HasAuraType(SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK))
        return true;

    if (Creature* pCreature = plMover->GetVehicleCreatureBase())
        if (pCreature->GetCreatureInfo()->InhabitType & INHABIT_AIR)
            return true;

    if (plMover->HasAura(44795))
        return true;

    if (plMover->HasUnitMovementFlag(MOVEMENTFLAG_JUMPING) ||  
        pMovementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING) || 
        plMover->GetMap()->GetGameObject(pMovementInfo.t_guid))
        return true;

    return false;
}

void AntiCheat::LogCheat(eCheat m_cheat, MovementInfo& pMovementInfo)
{
    std::string cheat_type = "";
	switch (m_cheat)
	{
		case CHEAT_MULTIJUMP:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u Area %u, X:%f Y:%f Z:%f, multi jump  exception",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            }
            cheat_type = "MultiJump";
            break;
		case CHEAT_SPEED:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u Area %u, X:%f Y:%f Z:%f, speed exception | cDeltaDistance=%u DeltaPacketTime=%u | sSpeed=%u cSpeed=%u", plMover->GetName(), plMover->GetMapId(), 
				    plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), plMover->GetAreaId(), uDistance2D, uiDiffTime_packets, uSpeedRate, uClientSpeedRate);
            }	
            cheat_type = "Speed";
            break;
		case CHEAT_TELEPORT:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u Area %u, X:%f Y:%f Z:%f, teleport exception | cDeltaDistance=%u DeltaPacketTime=%u | sSpeed=%u cSpeed=%u", plMover->GetName(), plMover->GetMapId(), 
				    plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), plMover->GetAreaId(), uDistance2D, uiDiffTime_packets, uSpeedRate, uClientSpeedRate);
            }
            cheat_type = "Teleport";
            break;
		case CHEAT_FLY:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u Area %u X:%f Y:%f Z:%f flight exception. {SPELL_AURA_FLY=[%X]} {SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED=[%X]} {SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED=[%X]} {SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS=[%X]} {SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK=[%X]} {plMover->GetVehicle()=[%X]}",
				    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(),
				    plMover->HasAuraType(SPELL_AURA_FLY), plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED),
				    plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED), plMover->HasAuraType(SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS),
				    plMover->HasAuraType(SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK), plMover->GetVehicle());                        
            }
            cheat_type = "Fly";
            break;
		case CHEAT_WATERWALK:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u, X: %f, Y: %f, waterwalk exception. {pMovementInfo=[%X]}{SPELL_AURA_WATER_WALK=[%X]}",
                    plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(),
                    pMovementInfo.flags, plMover->HasAuraType(SPELL_AURA_WATER_WALK));
            }
            cheat_type = "Waterwalk";
            break;
		case CHEAT_TELETOPLANE:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u, X: %f, Y: %f, Z: %f teleport to plane exception. Exception count: %d", plMover->GetName(), plMover->GetMapId(), 
				    plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), m_anti_TeleToPlane_Count);
            }
            cheat_type = "TeleToPlane";
            break;
        case CHEAT_CLIMB:
            if (difftime_log_file >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_FILE))
			{
				m_logfile_time = cServerTime;  
			    sLog->outCheat("AC-%s Map %u, X: %f, Y: %f, Z: %f Climb exception. Angle %f", plMover->GetName(), plMover->GetMapId(), 
				    plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), angle);
            }
            cheat_type = "Climb";
            break;
	}

    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_DBLOG))
        if (difftime_log_db >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_DB))
        {		                    
             CharacterDatabase.PExecute("INSERT INTO cheat_log(cheat_type, guid, name, level, map, area, pos_x, pos_y, pos_z, date) VALUES ('%s', '%u', '%s', '%u', '%u', '%u', '%f', '%f', '%f', NOW())", 
                cheat_type.c_str(), plMover->GetGUIDLow(), plMover->GetName(), plMover->getLevel(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            m_logdb_time = cServerTime;
        }
}

bool AntiCheat::CheckAntiMultiJump(MovementInfo& pNewPacket, uint32 uiOpcode)
{
   
    if (uiOpcode != MSG_MOVE_JUMP || GetLastOpcode() != MSG_MOVE_JUMP)
        return true;

    if (map_count)
        ++(m_CheatList[CHEAT_MULTIJUMP]);
    cheat_find = true;
	LogCheat(CHEAT_MULTIJUMP, pNewPacket);
	if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTIMULTIJUMP_BLOCK_COUNT) &&
        m_CheatList[CHEAT_MULTIJUMP] >= sWorld->getIntConfig(CONFIG_AC_ANTIMULTIJUMP_BLOCK_COUNT))
    {
	    
	    {
		    WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 12);
		    data.append(plMover->GetPackGUID());
		    data << uint32(0);
		    plMover->GetSession()->SendPacket(&data);
	    }
	    
	    {
		    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 12);
		    data.append(plMover->GetPackGUID());
		    data << uint32(0);
		    plMover->GetSession()->SendPacket(&data);
	    }
	    plMover->FallGround(2);
	    return false;
    }

    return true;
}

bool AntiCheat::CheckAntiSpeed(MovementInfo& pOldPacket, MovementInfo& pNewPacket, uint32 uiOpcode)
{  
    if (pOldPacket.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
        return true;

    
    if (pNewPacket.GetMovementFlags() != plMover->GetUnitMovementFlags() || 
        pNewPacket.GetMovementFlags() != pOldPacket.GetMovementFlags())
        return true;

    if (!plMover->isAlive())
        return true;
    
    if (pNewPacket.flags & MOVEMENTFLAG_SWIMMING &&
        (plMover->HasAura(1066)
        plMover->HasAura(30174)
        plMover->HasAura(64731)
        plMover->HasAura(7840)
        plMover->HasAura(88026)
        plMover->HasAura(30430)))
        return true; 

    if (plMover->HasAura(56640))
        return true;
    
    if (plMover->GetMotionMaster()->GetCurrentMovementGeneratorType() == TARGETED_MOTION_TYPE)
        return true;    

    
    if (plMover->IsFalling() && fly_auras)
        return true;

    if (plMover->IsFalling() && plMover->GetMapId() == 607) 
        return true;

    
    if (plMover->HasAuraType(SPELL_AURA_FEATHER_FALL) || plMover->HasAuraType(SPELL_AURA_SAFE_FALL))
        return true;

    
    if (pOldPacket.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && plMover->GetMapId() == 369)
        return true;

	if (uDistance2D > 0 && uClientSpeedRate > uSpeedRate)    
	{          
        cheat_find = true;
        if (map_count)
            ++(m_CheatList[CHEAT_SPEED]);
        LogCheat(CHEAT_SPEED, pNewPacket);
        if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTISPEED_BLOCK_COUNT) &&
            m_CheatList[CHEAT_SPEED] >= sWorld->getIntConfig(CONFIG_AC_ANTISPEED_BLOCK_COUNT))
        {
            plMover->FallGround(2);
            return false;
        }
	}    
	return true;
}

bool AntiCheat::CheckAntiTele(MovementInfo& pNewPacket, uint32 uiOpcode)
{
    /*if (uiOpcode == 183 && 
        GetLastOpcode() == 181 && 
        fClientRate > fServerRate)
    {      
        cheat_find = true;
        if (map_count)
	        ++(m_CheatList[CHEAT_TELEPORT]);
		LogCheat(CHEAT_TELEPORT, pNewPacket);
        if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTITELE_BLOCK_COUNT) &&
            m_CheatList[CHEAT_TELEPORT] >= sWorld->getIntConfig(CONFIG_AC_ANTITELE_BLOCK_COUNT))
        {
            plMover->FallGround(2);
            return false;
        }
	}    */
	return true;
}

bool AntiCheat::CheckAntiFly(MovementInfo& pOldPacket, MovementInfo& pNewPacket)
{
    if (!pOldPacket.HasMovementFlag(MOVEMENTFLAG_FLYING)) 
        return true;

    
    if (pNewPacket.GetMovementFlags() != plMover->GetUnitMovementFlags() || 
        pNewPacket.GetMovementFlags() != pOldPacket.GetMovementFlags())
        return true;

    if (plMover->HasAuraType(SPELL_AURA_FEATHER_FALL) || plMover->HasAuraType(SPELL_AURA_SAFE_FALL))
        return true;

    if (plMover->IsFalling())
        return true;

	if (!fly_auras && plMover->IsFlying())
	{
        if (map_count)
            ++(m_CheatList[CHEAT_FLY]);
        cheat_find = true;
		LogCheat(CHEAT_FLY, pNewPacket);
        if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTIFLY_BLOCK_COUNT) &&
            m_CheatList[CHEAT_FLY] >= sWorld->getIntConfig(CONFIG_AC_ANTIFLY_BLOCK_COUNT))
	    {
		    
		    {
			    WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 12);
			    data.append(plMover->GetPackGUID());
			    data << uint32(0);
			    plMover->GetSession()->SendPacket(&data);
		    }
		    
		    {
			    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 12);
			    data.append(plMover->GetPackGUID());
			    data << uint32(0);
			    plMover->GetSession()->SendPacket(&data);
		    }
		    plMover->FallGround(2);
		    return false;
	    }
	}    
	return true;
}

bool AntiCheat::CheckAntiWaterwalk(MovementInfo& pOldPacket, MovementInfo& pNewPacket)
{
    if (!pOldPacket.HasMovementFlag(MOVEMENTFLAG_WATERWALKING))
        return true;

    if (!plMover->IsAlive())
        return true;
    
    if (plMover->HasAura(1066))
        return true;

    if (plMover->IsUnderWater())
        return true;

    float water_level = plMover->GetBaseMap()->GetWaterLevel(plMover->GetPositionX(),plMover->GetPositionY());
    float water_level_diff = fabs(water_level - plMover->GetPositionZ());

    if (water_level_diff > 0.20f)
        return true;

	if (plMover->HasAuraType(SPELL_AURA_WATER_WALK) ||
        plMover->HasAuraType(SPELL_AURA_FEATHER_FALL) ||
        plMover->HasAuraType(SPELL_AURA_SAFE_FALL))
        return true;

	if (!fly_auras && !plMover->IsFlying())
	{
        if (map_count)
            ++(m_CheatList[CHEAT_WATERWALK]);
        cheat_find = true;
		LogCheat(CHEAT_WATERWALK, pNewPacket);
		if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTIWATERWALK_BLOCK_COUNT) && 
            m_CheatList[CHEAT_WATERWALK] >= sWorld->getIntConfig(CONFIG_AC_ANTIWATERWALK_BLOCK_COUNT))
	    {
		    
		    {
			    WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 12);
			    data.append(plMover->GetPackGUID());
			    data << uint32(0);
			    plMover->GetSession()->SendPacket(&data);
		    }
		   
		    {
			    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 12);
			    data.append(plMover->GetPackGUID());
			    data << uint32(0);
			    plMover->GetSession()->SendPacket(&data);
		    }
		    plMover->FallGround(2);
		    return false;
	    }
	}
	return true;
}

bool AntiCheat::CheckAntiTeleToPlane(MovementInfo& pOldPacket, MovementInfo& pNewPacket)
{
    if (pOldPacket.pos.GetPositionZ() != 0 ||
        pNewPacket.pos.GetPositionZ() != 0)
        return true;

    if (pNewPacket.HasMovementFlag(MOVEMENTFLAG_FALLING))
        return true;

    if (plMover->getDeathState() == DEAD_FALLING)
        return true;

    float x, y, z;
    plMover->GetPosition(x, y, z);
    float ground_Z = plMover->GetMap()->GetHeight(x, y, z);
    float z_diff = fabs(ground_Z - z);

	
    if (z_diff > 1.0f)
    {
        if (map_count)
	        ++(m_anti_TeleToPlane_Count);
	    if (m_anti_TeleToPlane_Count > sWorld->getIntConfig(CONFIG_AC_ANTITELETOPLANE_ALARMS))
	    {
            ++(m_CheatList[CHEAT_TELETOPLANE]);
            cheat_find = true;
		    LogCheat(CHEAT_TELETOPLANE, pNewPacket);
		    if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTITELETOPLANE_BLOCK_COUNT) &&
                m_CheatList[CHEAT_TELETOPLANE] >= sWorld->getIntConfig(CONFIG_AC_ANTITELETOPLANE_BLOCK_COUNT))
	        {
		        return false;
	        }
	    }
    }
	else
		m_anti_TeleToPlane_Count = 0;
	return true;
}

bool AntiCheat::CheckAntiClimb(MovementInfo& pOldPacket, MovementInfo& pNewPacket, uint32 uiOpcode)
{  
    if (uiOpcode != MSG_MOVE_HEARTBEAT ||
        plMover->GetAntiCheat()->GetLastOpcode() != MSG_MOVE_HEARTBEAT)
        return true;

    if (plMover->IsInWater() || 
        plMover->IsFlying() || 
        plMover->IsFalling())
        return true;
   
	if (angle > CLIMB_ANGLE)    
	{          
        cheat_find = true;
        if (map_count)
            ++(m_CheatList[CHEAT_CLIMB]);
        LogCheat(CHEAT_CLIMB, pNewPacket);
        if (map_block && sWorld->getIntConfig(CONFIG_AC_ANTICLIMB_BLOCK_COUNT) &&
            m_CheatList[CHEAT_CLIMB] >= sWorld->getIntConfig(CONFIG_AC_ANTICLIMB_BLOCK_COUNT))
        {
            
		    {
			    WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 12);
			    data.append(plMover->GetPackGUID());
			    data << uint32(0);
			    plMover->GetSession()->SendPacket(&data);
		    }
		    
		    {
			    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 12);
			    data.append(plMover->GetPackGUID());
			    data << uint32(0);
			    plMover->GetSession()->SendPacket(&data);
		    }
		    plMover->FallGround(2);
            return false;
        }
	}    
	return true;
}

void AntiCheat::CheckWarden(eWardenCheat wardCheat, uint8 wardenType)
{
    std::string cheat_type = "";
    switch (wardCheat)
    {
        case CHECK_WARDEN_CHECKSUM:
        	sLog->outCheat("AC-%s Map %u Area %u, X:%f Y:%f Z:%f, Warden detect not valid checksum in answer. Possible cheating (WPE hack)",
                plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            break;
        case CHECK_WARDEN_KEY:
            sLog->outCheat("AC-%s Map %u Area %u, X:%f Y:%f Z:%f, Warden detect not valid key code in answer. Possible cheating (WPE hack)",
                plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            break;
        case CHECK_WARDEN_MEMORY:
            switch (wardenType)
            {
                case MEM_CHECK:
                    cheat_type = "WARDEN: Memory Check";
                    break;
                case PAGE_CHECK_A:
                case PAGE_CHECK_B:
                    cheat_type = "WARDEN: Page Check";
                    break;
                case MPQ_CHECK:
                    cheat_type = "WARDEN: MPQ Check";
                    break;
                case LUA_STR_CHECK:
                    cheat_type = "WARDEN: LUA Check";
                    break;
                case DRIVER_CHECK:
                    cheat_type = "WARDEN: Driver Check";
                    break;
                case TIMING_CHECK:
                    cheat_type = "WARDEN: Timing Check";
                    break;
                case PROC_CHECK:
                    cheat_type = "WARDEN: Proc Check";
                    break;
                case MODULE_CHECK:
                    cheat_type = "WARDEN: Module Check";
                    break;
                default:
                    cheat_type = "WARDEN: Unknown Check";
                    break;                
            }
            sLog->outCheat("AC-%s Map %u Area %u, X:%f Y:%f Z:%f, %s",
                plMover->GetName(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), cheat_type.c_str());
            break;        
        default:
            break;
    }

    if (sWorld->getBoolConfig(CONFIG_AC_ENABLE_DBLOG))
        if (difftime_log_db >= sWorld->getIntConfig(CONFIG_AC_DELTA_LOG_DB))
        {		                    
            CharacterDatabase.PExecute("INSERT INTO cheat_log(cheat_type, guid, name, level, map, area, pos_x, pos_y, pos_z, date) VALUES ('%s', '%u', '%s', '%u', '%u', '%u', '%f', '%f', '%f', NOW())", 
                cheat_type.c_str(), plMover->GetGUIDLow(), plMover->GetName(), plMover->getLevel(), plMover->GetMapId(), plMover->GetAreaId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            m_logdb_time = cServerTime;
        }    

    warden_cheat_find = true;

    if (sWorld->getIntConfig(CONFIG_AC_REPORTS_FOR_GM_WARNING)) 
    {
       
        std::string str = "";
        str = "|cFFFFFC00[LordCraft - Antic-Cheat]|cFF00FFFF[|cFF60FF00" + std::string(plMover->GetName()) + "|cFF00FFFF] Eternal Eloawyth!";
        WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
        data << str;
        sWorld->SendGlobalGMMessage(&data);
    }

   
    SetAlarm(sWorld->getIntConfig(CONFIG_AC_ALARM_DELTA));
   
    if (m_CheatList_reset_diff < sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA_FOUND))
        m_CheatList_reset_diff = sWorld->getIntConfig(CONFIG_AC_RESET_CHEATLIST_DELTA_FOUND);
    if (!sWorld->iIgnoreMapIds_ACPuni.count(plMover->GetMapId()))
        AntiCheatPunisher();
}
