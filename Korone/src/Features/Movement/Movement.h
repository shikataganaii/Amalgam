#pragma once
#include "../../SDK/SDK.h"

class CMovement
{
private:
	void AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd);
	void MovementLock(CTFPlayer* pLocal, CUserCmd* pCmd);
	void BreakJump(CTFPlayer* pLocal, CUserCmd* pCmd);

	void TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd);
	void FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd);
	void LongJump(CTFPlayer* pLocal, CUserCmd* pCmd);

	void EdgeJump(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost = false);

	bool m_bPeekPlaced = false;
	Vec3 m_vPeekReturnPos = {};

public:
	void RunPre(CTFPlayer* pLocal, CUserCmd* pCmd);
	void RunPost(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket);
	void Draw(CTFPlayer* pLocal);
	void Event(IGameEvent* pEvent, uint32_t uNameHash);

	void PingReducer();
	void UnlockAchievements();
	void LockAchievements();

	int m_iWishCmdrate = -1;
	int m_iWishUpdaterate = -1;
};

ADD_FEATURE(CMovement, Movement);