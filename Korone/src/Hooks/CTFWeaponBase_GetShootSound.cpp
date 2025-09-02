#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFWeaponBase_GetShootSound, "client.dll", "40 55 56 41 56 48 83 EC ? 80 B9", 0x0);

MAKE_HOOK(CTFWeaponBase_GetShootSound, S::CTFWeaponBase_GetShootSound(), const char*,
	void* rcx, int iIndex)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFWeaponBase_GetShootSound[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iIndex);
#endif

	if (Vars::Misc::Sound::GiantWeaponSounds.Value)
	{
		auto pWeapon = H::Entities.GetWeapon();
		if (rcx == pWeapon)
		{	// credits: KGB
			int nOldTeam = pWeapon->m_iTeamNum();
			pWeapon->m_iTeamNum() = TF_TEAM_COUNT;
			auto sReturn = CALL_ORIGINAL(rcx, iIndex);
			pWeapon->m_iTeamNum() = nOldTeam;

			switch (FNV1A::Hash32(sReturn))
			{
			case FNV1A::Hash32Const("Weapon_FlameThrower.Fire"): return "MVM.GiantPyro_FlameStart";
			case FNV1A::Hash32Const("Weapon_FlameThrower.FireLoop"): return "MVM.GiantPyro_FlameLoop";
			case FNV1A::Hash32Const("Weapon_GrenadeLauncher.Single"): return "MVM.GiantDemoman_Grenadeshoot";
			}

			return sReturn;
		}
	}

	return CALL_ORIGINAL(rcx, iIndex);
}