#include "Movement.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"
#include "../Ticks/Ticks.h"


void CMovement::RunPre(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming()
		|| pLocal->IsTaunting() || pLocal->InCond(TF_COND_HALLOWEEN_KART) || pLocal->InCond(TF_COND_SHIELD_CHARGE))
		return;

	AutoJump(pLocal, pCmd);
	EdgeJump(pLocal, pCmd);
	AutoJumpbug(pLocal, pCmd);
	AutoStrafe(pLocal, pCmd);
	MovementLock(pLocal, pCmd);
	BreakJump(pLocal, pCmd);
}

void CMovement::RunPost(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket)
{
	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming()
		|| pLocal->InCond(TF_COND_SHIELD_CHARGE))
		return;

	if (pLocal->IsTaunting() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		TauntKartControl(pLocal, pCmd);
	else
	{
		EdgeJump(pLocal, pCmd, true);
		LongJump(pLocal, pCmd);
		FastMovement(pLocal, pCmd);
	}
}

void CMovement::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::Velocity) || !pLocal->IsAlive())
		return;
	int x = Vars::Menu::VelocityDisplay.Value.x;
	int y = Vars::Menu::VelocityDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(1);

	EAlign align = ALIGN_TOP;
	if (x <= 100 + H::Draw.Scale(50, Scale_Round))
	{
		x -= H::Draw.Scale(42, Scale_Round);
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - 100 - H::Draw.Scale(50, Scale_Round))
	{
		x += H::Draw.Scale(42, Scale_Round);
		align = ALIGN_TOPRIGHT;
	}
	static float fOldVelocity = 0;
	float fCurrentVelocity = pLocal->GetAbsVelocity().Length();
	if (!(pLocal->m_fFlags() & FL_ONGROUND))
	{
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("{:.0f} ({:.0f})", currentVelocity, bOldVelocity).c_str());
	}
	else
	{
		fOldVelocity = fCurrentVelocity;
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("{:.0f}", currentVelocity).c_str());
	}
}

void CMovement::AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::Bunnyhop.Value)
		return;

	if (auto pWeapon = H::Entities.GetWeapon(); pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_GRAPPLINGHOOK && pWeapon->As<CTFGrapplingHook>()->m_hProjectile())
		return;

	static bool bStaticJump = false, bStaticGrounded = false, bLastAttempted = false;
	const bool bLastJump = bStaticJump, bLastGrounded = bStaticGrounded;
	const bool bCurJump = bStaticJump = pCmd->buttons & IN_JUMP, bCurGrounded = bStaticGrounded = pLocal->m_hGroundEntity();

	if (bCurJump && bLastJump && (bCurGrounded ? !pLocal->IsDucking() : true))
	{
		if (!(bCurGrounded && !bLastGrounded))
			pCmd->buttons &= ~IN_JUMP;

		if (!(pCmd->buttons & IN_JUMP) && bCurGrounded && !bLastAttempted)
			pCmd->buttons |= IN_JUMP;
	}

	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
	{	// prevent more than 9 bhops occurring. if a server has this under that threshold they're retarded anyways
		static int iJumps = 0;
		if (bCurGrounded)
		{
			if (!bLastGrounded && pCmd->buttons & IN_JUMP)
				iJumps++;
			else
				iJumps = 0;

			if (iJumps > 9)
				pCmd->buttons &= ~IN_JUMP;
		}
	}
	bLastAttempted = pCmd->buttons & IN_JUMP;
}

void CMovement::AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoJumpbug.Value || !(pCmd->buttons & IN_DUCK) || pLocal->m_hGroundEntity() || pLocal->m_vecVelocity().z > -650.f)
		return;

	float flUnduckHeight = 20 * pLocal->m_flModelScale();
	float flTraceDistance = flUnduckHeight + 2;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

	Vec3 vOrigin = pLocal->m_vecOrigin();
	SDK::TraceHull(vOrigin, vOrigin - Vec3(0, 0, flTraceDistance), pLocal->m_vecMins(), pLocal->m_vecMaxs(), pLocal->SolidMask(), &filter, &trace);
	if (!trace.DidHit() || trace.fraction * flTraceDistance < flUnduckHeight) // don't try if we aren't in range to unduck or are too low
		return;

	pCmd->buttons &= ~IN_DUCK;
	pCmd->buttons |= IN_JUMP;
}

void CMovement::AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoStrafe.Value || pLocal->m_hGroundEntity() || !(pLocal->m_afButtonLast() & IN_JUMP) && (pCmd->buttons & IN_JUMP))
		return;

	switch (Vars::Misc::Movement::AutoStrafe.Value)
	{
	case Vars::Misc::Movement::AutoStrafeEnum::Legit:
	{
		static auto cl_sidespeed = U::ConVars.FindVar("cl_sidespeed");
		const float flSideSpeed = cl_sidespeed->GetFloat();

		if (pCmd->mousedx)
		{
			pCmd->forwardmove = 0.f;
			pCmd->sidemove = pCmd->mousedx > 0 ? flSideSpeed : -flSideSpeed;
		}
		break;
	}
	case Vars::Misc::Movement::AutoStrafeEnum::Directional:
	{
		// credits: KGB
		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			break;

		float flForward = pCmd->forwardmove, flSide = pCmd->sidemove;

		Vec3 vForward, vRight; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
		vForward.Normalize2D(), vRight.Normalize2D();

		Vec3 vWishDir = Math::VectorAngles({ vForward.x * flForward + vRight.x * flSide, vForward.y * flForward + vRight.y * flSide, 0.f });
		Vec3 vCurDir = Math::VectorAngles(pLocal->m_vecVelocity());
		float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
		if (fabsf(flDirDelta) > Vars::Misc::Movement::AutoStrafeMaxDelta.Value)
			break;

		float flTurnScale = Math::RemapVal(Vars::Misc::Movement::AutoStrafeTurnScale.Value, 0.f, 1.f, 0.9f, 1.f);
		float flRotation = DEG2RAD((flDirDelta > 0.f ? -90.f : 90.f) + flDirDelta * flTurnScale);
		float flCosRot = cosf(flRotation), flSinRot = sinf(flRotation);

		pCmd->forwardmove = flCosRot * flForward - flSinRot * flSide;
		pCmd->sidemove = flSinRot * flForward + flCosRot * flSide;
	}
	}
}

void CMovement::MovementLock(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static bool bLock = false;

	if (!Vars::Misc::Movement::MovementLock.Value)
	{
		bLock = false;
		return;
	}

	static Vec3 vMove = {}, vView = {};
	if (!bLock)
	{
		bLock = true;
		vMove = { pCmd->forwardmove, pCmd->sidemove, pCmd->upmove };
		vView = pCmd->viewangles;
	}

	pCmd->forwardmove = vMove.x, pCmd->sidemove = vMove.y, pCmd->upmove = vMove.z;
	SDK::FixMovement(pCmd, vView, pCmd->viewangles);
}

void CMovement::BreakJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::BreakJump.Value || F::AutoRocketJump.IsRunning())
		return;

	static bool bStaticJump = false;
	const bool bLastJump = bStaticJump;
	const bool bCurrJump = bStaticJump = pCmd->buttons & IN_JUMP;

	static int iTickSinceGrounded = -1;
	if (pLocal->m_hGroundEntity().Get())
		iTickSinceGrounded = -1;
	iTickSinceGrounded++;

	switch (iTickSinceGrounded)
	{
	case 0:
		if (bLastJump || !bCurrJump || pLocal->IsDucking())
			return;
		break;
	case 1:
		break;
	default:
		return;
	}

	pCmd->buttons |= IN_DUCK;
}

void CMovement::TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (Vars::Misc::Automation::TauntControl.Value && pLocal->IsTaunting() && pLocal->m_bAllowMoveDuringTaunt())
	{
		if (pLocal->m_bTauntForceMoveForward())
		{
			if (pCmd->buttons & IN_BACK)
				pCmd->viewangles.x = 91.f;
			else if (!(pCmd->buttons & IN_FORWARD))
				pCmd->viewangles.x = 90.f;
		}
		if (pCmd->buttons & IN_MOVELEFT)
			pCmd->sidemove = pCmd->viewangles.x == 90.f ? -450.f : -pLocal->m_flTauntForceMoveForwardSpeed();
		else if (pCmd->buttons & IN_MOVERIGHT)
			pCmd->sidemove = pCmd->viewangles.x == 90.f ? 450.f : pLocal->m_flTauntForceMoveForwardSpeed();
	}
	else if (Vars::Misc::Automation::KartControl.Value && pLocal->InCond(TF_COND_HALLOWEEN_KART))
	{
		const bool bForward = pCmd->buttons & IN_FORWARD;
		const bool bBack = pCmd->buttons & IN_BACK;
		const bool bLeft = pCmd->buttons & IN_MOVELEFT;
		const bool bRight = pCmd->buttons & IN_MOVERIGHT;

		const bool flipVar = I::GlobalVars->tickcount % 2;
		if (bForward && (!bLeft && !bRight || !flipVar))
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 0.f;
		}
		else if (bBack && (!bLeft && !bRight || !flipVar))
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 91.f;
		}
		else if (pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
		{
			if (flipVar || !F::Ticks.CanChoke())
			{	// you could just do this if you didn't care about viewangles
				Vec3 vMove = { pCmd->forwardmove, pCmd->sidemove, 0.f };
				Vec3 vAngMoveReverse = Math::VectorAngles(vMove * -1.f);
				pCmd->forwardmove = -vMove.Length();
				pCmd->sidemove = 0.f;
				pCmd->viewangles.y = fmodf(pCmd->viewangles.y - vAngMoveReverse.y, 360.f);
				pCmd->viewangles.z = 270.f;
				G::PSilentAngles = true;
			}
		}
		else
			pCmd->viewangles.x = 90.f;

		G::SilentAngles = true;
	}
}

void CMovement::FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal->m_hGroundEntity() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return;

	const float flSpeed = pLocal->m_vecVelocity().Length2D();
	const int flMaxSpeed = std::min(pLocal->m_flMaxspeed() * 0.9f, 520.f) - 10.f;
	const int iRun = !pCmd->forwardmove && !pCmd->sidemove ? 0 : flSpeed < flMaxSpeed ? 1 : 2;

	switch (iRun)
	{
	case 0:
	{
		if (!Vars::Misc::Movement::FastStop.Value || !flSpeed)
			return;

		Vec3 vDirection = pLocal->m_vecVelocity().ToAngle();
		vDirection.y = pCmd->viewangles.y - vDirection.y;
		Vec3 vNegatedDirection = vDirection.FromAngle() * -flSpeed;
		pCmd->forwardmove = vNegatedDirection.x;
		pCmd->sidemove = vNegatedDirection.y;

		break;
	}
	case 1:
	{
		if ((pLocal->IsDucking() ? !Vars::Misc::Movement::DuckSpeed.Value : !Vars::Misc::Movement::FastAccelerate.Value)
			|| Vars::Misc::Game::AntiCheatCompatibility.Value
			|| G::Attacking == 1 || F::Ticks.m_bDoubletap || F::Ticks.m_bSpeedhack || F::Ticks.m_bRecharge || G::AntiAim || I::GlobalVars->tickcount % 2)
			return;

		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			return;

		Vec3 vMove = { pCmd->forwardmove, pCmd->sidemove, 0.f };
		Vec3 vAngMoveReverse = Math::VectorAngles(vMove * -1.f);
		pCmd->forwardmove = -vMove.Length();
		pCmd->sidemove = 0.f;
		pCmd->viewangles.y = fmodf(pCmd->viewangles.y - vAngMoveReverse.y, 360.f);
		pCmd->viewangles.z = 270.f;
		G::PSilentAngles = true;

		break;
	}
	}
}


void CMovement::LongJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::LongJump.Value)
		return;

	if (pLocal->movetype() == MOVETYPE_LADDER || pLocal->movetype() == MOVETYPE_NOCLIP)
		return;

	static int bLastTick = 0;

	if (pLocal->m_fFlags() & FL_ONGROUND)
		bLastTick = pCmd->tick_count;
	//* TODO: Make the timing changable */
	else if (pCmd->tick_count - bLastTick <= 40)
		pCmd->buttons |= IN_DUCK;


}

void CMovement::EdgeJump(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost)
{
	if (!Vars::Misc::Movement::EdgeJump.Value)
		return;
	
	static bool bStaticGround = false;
	if (!bPost)
		bStaticGround = pLocal->m_hGroundEntity();
	else if (bStaticGround && !pLocal->m_hGroundEntity())
		pCmd->buttons |= IN_JUMP;
}
