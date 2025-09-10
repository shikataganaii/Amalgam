#pragma once
#include "../../../SDK/SDK.h"

class CSpectatorList
{
private:
	struct Spectator_t
	{
		std::string m_sName;
		const char* m_sMode;
		float m_flRespawnIn;
		bool m_bRespawnTimeIncreased;
		int m_iIndex;
	};

	std::vector<Spectator_t> m_vSpectators = {};
	std::unordered_map<int, float> m_mRespawnCache = {};

public:
	bool GetSpectators(CTFPlayer* pTarget);
	void Draw(CTFPlayer* pLocal);
};

ADD_FEATURE(CSpectatorList, SpectatorList);