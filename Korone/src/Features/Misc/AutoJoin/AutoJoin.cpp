#include "AutoJoin.h"

void CAutoJoin::Run(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::AutoJoin.Value)
		return;

	static Timer tJoinTimer{};

	if (!tJoinTimer.Run(1.0f))
		return;

	const int iClass = Vars::Misc::Automation::AutoJoin.Value - 1;
	//* We already have the class! *//
	if (pLocal->m_iClass() == iClass)
		return;

	if (!pLocal->IsInValidTeam())
	{
		I::EngineClient->ClientCmd_Unrestricted("team_ui_setup");
		I::EngineClient->ClientCmd_Unrestricted("menuopen");
		I::EngineClient->ClientCmd_Unrestricted("autoteam");
		I::EngineClient->ClientCmd_Unrestricted("menuclosed");
		return;
	}

	I::EngineClient->ClientCmd_Unrestricted(std::format("joinclass {}", m_aClassNames[iClass]).c_str());
	I::EngineClient->ClientCmd_Unrestricted("menuclosed");
}