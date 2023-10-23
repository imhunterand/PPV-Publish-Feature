#include <stdio.h>
#include "admin_base.h"
#include <string>
#include <ctime>
#include "KeyValues.h"
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <regex>

SH_DECL_HOOK6_void(IServerGameClients, OnClientConnected, SH_NOATTRIB, 0, CPlayerSlot, const char*, uint64, const char*, const char*, bool);

AdminBase g_AdminBase;
IServerGameClients *gameclients = NULL;
IVEngineServer *engine = NULL;
IFileSystem* filesystem = NULL;
ICvar* icvar = NULL;

std::unordered_map<std::string, int> g_BanList;
std::vector<std::string> g_AdmList;

std::string convertto3(std::string steamid);

CON_COMMAND_EXTERN(mm_ban, BanCommand, "Ban Clients");
void BanCommand(const CCommandContext& context, const CCommand& args)
{
	auto slot = context.GetPlayerSlot();
	auto steamid = engine->GetPlayerNetworkIDString(slot);
	if(std::find(g_AdmList.begin(), g_AdmList.end(), steamid) != g_AdmList.end())
	{
		if (args.ArgC() >= 2)
		{
			int slotid = std::atoi(args[1]);
			if (engine->GetPlayerUserId(slotid).Get() == -1)
			{
				g_SMAPI->ClientConPrintf(slot, "[Admin] No valid player at UserID (%d)\n", slotid);
			}
			else
			{
				auto steamid_target = engine->GetPlayerNetworkIDString(slotid);
				auto name = engine->GetClientConVarValue(slotid, "name");
				// engine->KickClient(slotid, "ban", 41);
				engine->DisconnectClient(CEntityIndex(slotid), 41);
				g_SMAPI->ClientConPrintf(slot, "[Admin] Player %s(%s) success Banned!\n", name, steamid_target);
				int time = std::atoi(args[2]);
				if(time != 0)
				{
					time = (time * 60) + std::time(0);
				}
				g_BanList.emplace(steamid_target, time);
			}
		}
		else g_SMAPI->ClientConPrintf(slot, "[Admin] Usage: mm_ban <userid> <time>\n");
	}
	else g_SMAPI->ClientConPrintf(slot, "[Admin] You are denied access\n");
}

CON_COMMAND_EXTERN(mm_unban, UnBanCommand, "UnBan Clients");
void UnBanCommand(const CCommandContext& context, const CCommand& args)
{
	auto slot = context.GetPlayerSlot();
	auto steamid = engine->GetPlayerNetworkIDString(slot);
	if(std::find(g_AdmList.begin(), g_AdmList.end(), steamid) != g_AdmList.end())
	{
		if (args.ArgC() >= 1)
		{
			std::string steamid3 = convertto3(args[1]);
			if(g_BanList.count(steamid3))
			{
				g_BanList.erase(steamid3);
				g_SMAPI->ClientConPrintf(slot, "[Admin] Steamid %s successfully unbanned!\n", steamid3.c_str());
			}
			else
			{
				g_SMAPI->ClientConPrintf(slot, "[Admin] Invalid steamid (%s)\n", steamid3.c_str());
			}
		}
		else g_SMAPI->ClientConPrintf(slot, "[Admin] Usage: mm_unban <steamid>\n");
	}
	else g_SMAPI->ClientConPrintf(slot, "[Admin] You are denied access\n");
}

PLUGIN_EXPOSE(AdminBase, g_AdminBase);
bool AdminBase::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);

	SH_ADD_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, gameclients, this, &AdminBase::Hook_OnClientConnected, false);

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	KeyValues* kv = new KeyValues("Admins");
	kv->LoadFromFile(filesystem, "addons/configs/admins.ini");
	FOR_EACH_VALUE(kv, subkey)
	{
		std::string steamid3 = convertto3(subkey->GetName());
		if(!steamid3.empty())
			g_AdmList.push_back(steamid3);
	}

	delete kv;

	kv = new KeyValues("BanList");
	kv->LoadFromFile(filesystem, "addons/configs/bans.ini");
	FOR_EACH_VALUE(kv, subkey)
	{
		g_BanList.emplace(subkey->GetName(), subkey->GetInt());
	}
	delete kv;

	return true;
}

bool AdminBase::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, gameclients, this, &AdminBase::Hook_OnClientConnected, false);

	g_AdmList.clear();

	KeyValues* kv = new KeyValues("BanList");

	std::time_t time = std::time(0);
	for(auto i : g_BanList)
	{
		if(i.second > time || i.second == 0)
		{
			kv->SetInt(i.first.c_str(), i.second);
		}
	}
	g_BanList.clear();
	kv->SaveToFile(filesystem, "addons/configs/bans.ini");
	delete kv;

	return true;
}

void AdminBase::Hook_OnClientConnected(CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer)
{
	if(!bFakePlayer && g_BanList.count(pszNetworkID))
	{
		int time = g_BanList.at(pszNetworkID);
		if(time > std::time(0) || time == 0)
		{
			// engine->KickClient(slot, "ban", 41);
			engine->DisconnectClient(CEntityIndex(slot.Get()), 41);
		}
		else
		{
			g_BanList.erase(pszNetworkID);
		}
	}
}

std::string convertto3(std::string steamid)
{
    std::regex rgx = std::regex(R"(\[U:[10]:[0-9]+\])");
    if(std::regex_search(steamid, rgx))
    {
        return steamid;
    }
    rgx = std::regex(R"(STEAM_([10]):([10]):([0-9]+))");
    std::smatch matches;
    if(std::regex_search(steamid, matches, rgx))
    {
        int Y = std::atoi(matches[2].str().c_str());
        int Z = std::atoi(matches[3].str().c_str());
        return "[U:1:" + std::to_string((Z * 2) + Y) + "]";
    }
    rgx = std::regex(R"(\d{17})");
    if(std::regex_search(steamid, matches, rgx))
    {
        long steamid64 = std::atoll(matches[0].str().c_str());
        return "[U:1:" + std::to_string(steamid64 - 76561197960265728) + "]";
    }
    
    return "";
}

bool AdminBase::Pause(char *error, size_t maxlen)
{
	return true;
}

bool AdminBase::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *AdminBase::GetLicense()
{
	return ".|.";
}

const char *AdminBase::GetVersion()
{
#ifndef PL_VERSION
	return "1.1.1";
#else
	return PL_VERSION;
#endif
}

const char *AdminBase::GetDate()
{
	return __DATE__;
}

const char *AdminBase::GetLogTag()
{
	return "ADMIN";
}

const char *AdminBase::GetAuthor()
{
	return "Pisex";
}

const char *AdminBase::GetDescription()
{
	return "Admin function plugin";
}

const char *AdminBase::GetName()
{
	return "Base Admin";
}

const char *AdminBase::GetURL()
{
	return "https://discord.gg/g798xERK5Y";
}
