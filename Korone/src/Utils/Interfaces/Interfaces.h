#pragma once
#include "../Macros/Macros.h"
#include <vector>

struct InterfaceInit_t
{
	void** m_pPtr;
	const char* m_sDLLName;
	const char* m_sVersion;
	int m_nOffset; //if not -1 we're going to sig scan (m_sVersion is the sig)
	int m_nDereferenceCount;

	InterfaceInit_t(void** pPtr, const char* sDLLName, const char* sVersion, int nOffset, int nDereferenceCount);
};

#define MAKE_INTERFACE_VERSION(type, name, dll, version) namespace I { inline type *name = nullptr; } \
namespace MAKE_INTERFACE_SCOPE \
{\
	inline InterfaceInit_t name##InterfaceInit_t(reinterpret_cast<void **>(&I::name), dll, version, -1, 0); \
}

#define MAKE_INTERFACE_SIGNATURE(type, name, dll, signature, offset, deref) namespace I { inline type *name = nullptr; } \
namespace MAKE_INTERFACE_SCOPE \
{\
	inline InterfaceInit_t name##InterfaceInit_t(reinterpret_cast<void **>(&I::name), dll, signature, offset, deref); \
}

#define MAKE_INTERFACE_NULL(type, name) namespace I { inline type *name = nullptr; }

class CInterfaces
{
private:
	std::vector<InterfaceInit_t*> m_vInterfaces = {};
	bool m_bFailed = false;

public:
	bool Initialize();

	inline void AddInterface(InterfaceInit_t* pInterface)
	{
		m_vInterfaces.push_back(pInterface);
	}
};

ADD_FEATURE_CUSTOM(CInterfaces, Interfaces, U);