#include "Misc.h"

#include "../Backtrack/Backtrack.h"
#include "../Ticks/Ticks.h"
#include "../Players/PlayerUtils.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"

void CMisc::Run(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	CheatsBypass();
	WeaponSway();
	AntiAFK(pLocal, pCmd);
	InstantRespawnMVM(pLocal);
	NoisemakerSpam(pLocal);
}

void CMisc::AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Timer tTimer = {};

	if (pCmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK) || !pLocal->IsAlive())
		tTimer.Update();
	else if (Vars::Misc::Automation::AntiAFK.Value && tTimer.Run(25.f))
		pCmd->buttons |= IN_FORWARD;
}

void CMisc::InstantRespawnMVM(CTFPlayer* pLocal)
{
	if (!Vars::Misc::MannVsMachine::InstantRespawn.Value || pLocal->IsAlive())
		return;

	KeyValues* kv = new KeyValues("MVM_Revive_Response");
	kv->SetInt("accepted", 1);
	I::EngineClient->ServerCmdKeyValues(kv);
}

void CMisc::NoisemakerSpam(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Exploits::NoisemakerSpam.Value || !pLocal->IsAlive() || pLocal->IsAGhost()
		|| pLocal->m_bUsingActionSlot() || pLocal->m_flNextNoiseMakerTime() > I::GlobalVars->curtime)
		return;

	KeyValues* kv = new KeyValues("use_action_slot_item_server");
	I::EngineClient->ServerCmdKeyValues(kv);
}

void CMisc::CheatsBypass()
{
	static bool bCheatSet = false;
	static auto sv_cheats = U::ConVars.FindVar("sv_cheats");
	if (Vars::Misc::Exploits::CheatsBypass.Value)
	{
		sv_cheats->m_nValue = 1;
		bCheatSet = true;
	}
	else if (bCheatSet)
	{
		sv_cheats->m_nValue = 0;
		bCheatSet = false;
	}
}

void CMisc::WeaponSway()
{
	static auto cl_wpn_sway_interp = U::ConVars.FindVar("cl_wpn_sway_interp");
	static auto cl_wpn_sway_scale = U::ConVars.FindVar("cl_wpn_sway_scale");

	bool bSway = Vars::Visuals::Viewmodel::SwayInterp.Value || Vars::Visuals::Viewmodel::SwayScale.Value;
	cl_wpn_sway_interp->SetValue(bSway ? Vars::Visuals::Viewmodel::SwayInterp.Value : 0.f);
	cl_wpn_sway_scale->SetValue(bSway ? Vars::Visuals::Viewmodel::SwayScale.Value : 0.f);
}



void CMisc::Event(IGameEvent* pEvent, uint32_t uHash)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("player_spawn"):
		m_bPeekPlaced = false;
	}
}

int CMisc::AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd, bool bSendPacket)
{
	if (!Vars::Misc::Automation::AntiBackstab.Value || !bSendPacket || G::Attacking == 1 || !pLocal || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return 0;

	std::vector<std::pair<Vec3, CBaseEntity*>> vTargets = {};
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
			continue;

		auto pWeapon = pPlayer->m_hActiveWeapon()->As<CTFWeaponBase>();
		if (!pWeapon
			|| pWeapon->GetWeaponID() != TF_WEAPON_KNIFE
			&& !(G::PrimaryWeaponType == EWeaponType::MELEE && SDK::AttribHookValue(0, "crit_from_behind", pWeapon) > 0)
			&& !(pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER && SDK::AttribHookValue(0, "set_flamethrower_back_crit", pWeapon) == 1)
			|| F::PlayerUtils.IsIgnored(pPlayer->entindex()))
			continue;

		Vec3 vLocalPos = pLocal->GetCenter();
		Vec3 vTargetPos1 = pPlayer->GetCenter();
		Vec3 vTargetPos2 = vTargetPos1 + pPlayer->m_vecVelocity() * F::Backtrack.GetReal();
		float flDistance = std::max(std::max(SDK::MaxSpeed(pPlayer), SDK::MaxSpeed(pLocal)), pPlayer->m_vecVelocity().Length());
		if ((vLocalPos.DistTo(vTargetPos1) > flDistance || !SDK::VisPosWorld(pLocal, pPlayer, vLocalPos, vTargetPos1))
			&& (vLocalPos.DistTo(vTargetPos2) > flDistance || !SDK::VisPosWorld(pLocal, pPlayer, vLocalPos, vTargetPos2)))
			continue;

		vTargets.emplace_back(vTargetPos2, pEntity);
	}
	if (vTargets.empty())
		return 0;

	std::sort(vTargets.begin(), vTargets.end(), [&](const auto& a, const auto& b) -> bool
		{
			return pLocal->GetCenter().DistTo(a.first) < pLocal->GetCenter().DistTo(b.first);
		});

	auto& pTargetPos = vTargets.front();
	switch (Vars::Misc::Automation::AntiBackstab.Value)
	{
	case Vars::Misc::Automation::AntiBackstabEnum::Yaw:
	{
		Vec3 vAngleTo = Math::CalcAngle(pLocal->m_vecOrigin(), pTargetPos.first);
		vAngleTo.x = pCmd->viewangles.x;
		SDK::FixMovement(pCmd, vAngleTo);
		pCmd->viewangles = vAngleTo;
		
		return 1;
	}
	case Vars::Misc::Automation::AntiBackstabEnum::Pitch:
	case Vars::Misc::Automation::AntiBackstabEnum::Fake:
	{
		bool bCheater = F::PlayerUtils.HasTag(pTargetPos.second->entindex(), F::PlayerUtils.TagToIndex(CHEATER_TAG));
		// if the closest spy is a cheater, assume auto stab is being used, otherwise don't do anything if target is in front
		if (!bCheater)
		{
			auto TargetIsBehind = [&]()
				{
					Vec3 vToTarget = (pLocal->m_vecOrigin() - pTargetPos.first).To2D();
					const float flDist = vToTarget.Normalize();
					if (!flDist)
						return true;

					float flTolerance = 0.0625f;
					float flExtra = 2.f * flTolerance / flDist; // account for origin compression
					float flPosVsTargetViewMinDot = 0.f - 0.0031f - flExtra;

					Vec3 vTargetForward; Math::AngleVectors(pCmd->viewangles, &vTargetForward);
					vTargetForward.Normalize2D();

					return vToTarget.Dot(vTargetForward) > flPosVsTargetViewMinDot;
				};

			if (!TargetIsBehind())
				return 0;
		}

		if (!bCheater || Vars::Misc::Automation::AntiBackstab.Value == Vars::Misc::Automation::AntiBackstabEnum::Pitch)
		{
			pCmd->forwardmove *= -1;
			pCmd->viewangles.x = 269.f;
		}
		else
		{
			pCmd->viewangles.x = 271.f;
		}
		// may slip up some auto backstabs depending on mode, though we are still able to be stabbed

		return 2;
	}
	}

	return 0;
}

void CMisc::PingReducer()
{
	static Timer tTimer = {};
	if (!tTimer.Run(0.1f))
		return;

	static auto cl_cmdrate = U::ConVars.FindVar("cl_cmdrate");
	int iTarget = Vars::Misc::Exploits::PingReducer.Value ? Vars::Misc::Exploits::PingTarget.Value : cl_cmdrate->GetInt();
	if (m_iWishCmdrate != iTarget)
	{
		m_iWishCmdrate = iTarget;

		auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
		if (pNetChan && I::EngineClient->IsConnected())
		{
			NET_SetConVar tConvar = { "cl_cmdrate", std::to_string(m_iWishCmdrate).c_str() };
			pNetChan->SendNetMsg(tConvar);
		}
	}

	static auto sv_maxupdaterate = U::ConVars.FindVar("sv_maxupdaterate"); // force highest cl_updaterate command possible
	iTarget = sv_maxupdaterate->GetInt();
	if (m_iWishUpdaterate != iTarget)
	{
		m_iWishUpdaterate = iTarget;

		auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
		if (pNetChan && I::EngineClient->IsConnected())
		{
			NET_SetConVar tConvar = { "cl_updaterate", std::to_string(m_iWishUpdaterate).c_str() };
			pNetChan->SendNetMsg(tConvar);
		}
	}
}

void CMisc::UnlockAchievements()
{
	const auto pAchievementMgr = U::Memory.CallVirtual<114, IAchievementMgr*>(I::EngineClient);
	if (pAchievementMgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
			pAchievementMgr->AwardAchievement(pAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::LockAchievements()
{
	const auto pAchievementMgr = U::Memory.CallVirtual<114, IAchievementMgr*>(I::EngineClient);
	if (pAchievementMgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
			I::SteamUserStats->ClearAchievement(pAchievementMgr->GetAchievementByIndex(i)->GetName());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}