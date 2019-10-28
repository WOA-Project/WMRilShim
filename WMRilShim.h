// NPETSEC.h - Contains declarations of npetsec functions
#pragma once
#include <Windows.h>

#ifdef WMRILSHIM_EXPORTS
#define WMRILSHIM_API __declspec(dllexport)
#else
#define WMRILSHIM_API __declspec(dllimport)
#endif

extern "C" WMRILSHIM_API DWORD RIL_Init(DWORD dwModemID);
extern "C" WMRILSHIM_API DWORD RIL_Version(DWORD VersionRangeLow, DWORD VersionRangeHigh);
extern "C" WMRILSHIM_API DWORD RIL_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
extern "C" WMRILSHIM_API BOOL RIL_IOControl(
	DWORD hOpenContext,
	DWORD dwCode,
	PBYTE pBufIn,
	DWORD dwLenIn,
	PBYTE pBufOut,
	DWORD dwLenOut,
	PDWORD pdwActualOut
);
extern "C" WMRILSHIM_API BOOL RIL_Close(DWORD hOpenContext);