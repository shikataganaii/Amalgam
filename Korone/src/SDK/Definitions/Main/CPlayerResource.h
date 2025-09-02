#pragma once
#include "CBaseEntity.h"
#include "../Misc/IGameResources.h"
#include "../Misc/String.h"

#define PLAYER_UNCONNECTED_NAME	"unconnected"
#define PLAYER_ERROR_NAME "ERRORNAME"

class CPlayerResource : public CBaseEntity, public IGameResources
{
public:
	NETVAR_ARRAY(m_iPing, int, "CPlayerResource", "m_iPing");
	NETVAR_ARRAY(m_iScore, int, "CPlayerResource", "m_iScore");
	NETVAR_ARRAY(m_iDeaths, int, "CPlayerResource", "m_iDeaths");
	NETVAR_ARRAY(m_bConnected, bool, "CPlayerResource", "m_bConnected");
	NETVAR_ARRAY(m_iTeam, int, "CPlayerResource", "m_iTeam");
	NETVAR_ARRAY(m_bAlive, bool, "CPlayerResource", "m_bAlive");
	NETVAR_ARRAY(m_iHealth, int, "CPlayerResource", "m_iHealth");
	NETVAR_ARRAY(m_iAccountID, unsigned, "CPlayerResource", "m_iAccountID");
	NETVAR_ARRAY(m_bValid, bool, "CPlayerResource", "m_bValid");
	NETVAR_ARRAY(m_iUserID, int, "CPlayerResource", "m_iUserID");

	NETVAR_ARRAY_OFF(m_szName, const char*, "CPlayerResource", "m_iPing", -816);

	inline bool IsFakePlayer(int iIndex, bool bPlayerInfo = false)
	{
		if (!bPlayerInfo)
			return !m_iPing(iIndex);

		player_info_t tInfo;
		if (I::EngineClient->GetPlayerInfo(iIndex, &tInfo))
			return tInfo.fakeplayer;
		return false;
	}

	inline const char* GetName(int iIndex)
	{
		const char* sName = m_szName(iIndex);
		return sName ? sName : PLAYER_ERROR_NAME;
	}
};