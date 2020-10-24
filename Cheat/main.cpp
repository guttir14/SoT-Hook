#include "cheat.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) 
{
	switch (fdwReason) 
	{
	case DLL_PROCESS_ATTACH: 
	{
		DisableThreadLibraryCalls(hinstDLL);

		if (!Cheat::Init(hinstDLL))
		{
			return FALSE;
		};

		break;
	}
	case DLL_PROCESS_DETACH: 
	{
		return Cheat::Remove();
	}
	}
	
	return TRUE;
}