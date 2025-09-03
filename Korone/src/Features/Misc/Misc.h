#pragma once
#include "../../SDK/SDK.h"

class CMisc
{
private:
	void AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd);
	void InstantRespawnMVM(CTFPlayer* pLocal);
	void NoisemakerSpam(CTFPlayer* pLocal);

	void CheatsBypass();
	void WeaponSway();

	void TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd);

	bool m_bPeekPlaced = false;
	Vec3 m_vPeekReturnPos = {};

	//bool bSteamCleared = false;

public:
	void Run(CTFPlayer* pLocal, CUserCmd* pCmd);

	void Event(IGameEvent* pEvent, uint32_t uNameHash);
	int AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd, bool bSendPacket);

	void PingReducer();
	void UnlockAchievements();
	void LockAchievements();

	int m_iWishCmdrate = -1;
	int m_iWishUpdaterate = -1;
};

ADD_FEATURE(CMisc, Misc);