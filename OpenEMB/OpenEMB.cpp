// OpenEMB.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "OpenEMB.h"
#include <fstream>
#include <ctime>
#include <iostream>
#include <ntddrilapitypes.h>

using namespace std;

typedef DWORD(*LPRIL_INIT)(DWORD dwModemID);
typedef DWORD(*LPRIL_VERSION)(DWORD VersionRangeLow, DWORD VersionRangeHigh);
typedef DWORD(*LPRIL_OPEN)(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);

typedef BOOL(*LPRIL_IOCONTROL)
(
	DWORD hOpenContext,
	DWORD dwCode,
	PBYTE pBufIn,
	DWORD dwLenIn,
	PBYTE pBufOut,
	DWORD dwLenOut,
	PDWORD pdwActualOut
	);

typedef BOOL(*LPRIL_CLOSE)(DWORD hOpenContext);

HINSTANCE wmril = NULL;

LPRIL_VERSION RRIL_Version;
LPRIL_INIT RRIL_Init;
LPRIL_OPEN RRIL_Open;
LPRIL_IOCONTROL RRIL_IOControl;
LPRIL_CLOSE RRIL_Close;

void Initialize()
{
	wmril = LoadLibrary(L"WMRil.dll");

	RRIL_Version = (LPRIL_VERSION)GetProcAddress((HMODULE)wmril, "RIL_Version");
	RRIL_Init = (LPRIL_INIT)GetProcAddress((HMODULE)wmril, "RIL_Init");
	RRIL_Open = (LPRIL_OPEN)GetProcAddress((HMODULE)wmril, "RIL_Open");
	RRIL_IOControl = (LPRIL_IOCONTROL)GetProcAddress((HMODULE)wmril, "RIL_IOControl");
	RRIL_Close = (LPRIL_CLOSE)GetProcAddress((HMODULE)wmril, "RIL_Close");
}

static const char* RILCOMMANDSSTRS[]{
	"RIL_COMMAND_INITNOTIFICATIONS = 0x00000001",
	"RIL_COMMAND_GETNEXTNOTIFICATION = 0x00000002",
	"RIL_COMMAND_ENABLENOTIFICATIONS = 0x00000003",
	"RIL_COMMAND_DISABLENOTIFICATIONS = 0x00000004",
	"RIL_COMMAND_GETDRIVERVERSION = 0x00000005",
	"RIL_COMMAND_GETDEVCAPS = 0x00000006",
	"RIL_COMMAND_GETDEVICEINFO = 0x00000007",
	"RIL_COMMAND_GETEQUIPMENTSTATE = 0x00000008",
	"RIL_COMMAND_SETEQUIPMENTSTATE = 0x00000009",
	"RIL_COMMAND_SETNOTIFICATIONFILTERSTATE = 0x0000000A",
	"RIL_COMMAND_GETNOTIFICATIONFILTERSTATE = 0x0000000B",
	"RIL_COMMAND_ENUMERATESLOTS = 0x0000000C",
	"RIL_COMMAND_GETCARDINFO = 0x0000000D",
	"RIL_COMMAND_SETSLOTPOWER = 0x0000000E",
	"RIL_COMMAND_GETUICCRECORDSTATUS = 0x0000000F",
	"RIL_COMMAND_SENDRESTRICTEDUICCCMD = 0x00000010",
	"RIL_COMMAND_WATCHUICCFILECHANGE = 0x00000011",
	"RIL_COMMAND_GETUICCPRLID = 0x00000012",
	"RIL_COMMAND_GETIMSI = 0x00000013",
	"RIL_COMMAND_GETSUBSCRIBERNUMBERS = 0x00000014",
	"RIL_COMMAND_GETUICCLOCKSTATE = 0x00000015",
	"RIL_COMMAND_GETUICCSERVICELOCK = 0x00000016",
	"RIL_COMMAND_VERIFYUICCLOCK = 0x00000017",
	"RIL_COMMAND_SETUICCLOCKENABLED = 0x00000018",
	"RIL_COMMAND_UNBLOCKUICCLOCK = 0x00000019",
	"RIL_COMMAND_CHANGEUICCLOCKPASSWORD = 0x0000001A",
	"RIL_COMMAND_GETUICCAPPPERSOCHECKSTATE = 0x0000001B",
	"RIL_COMMAND_GETPERSODEACTIVATIONSTATE = 0x0000001C",
	"RIL_COMMAND_DEACTIVATEPERSO = 0x0000001D",
	"RIL_COMMAND_READPHONEBOOKENTRIES = 0x0000001E",
	"RIL_COMMAND_WRITEPHONEBOOKENTRY = 0x0000001F",
	"RIL_COMMAND_DELETEPHONEBOOKENTRY = 0x00000020",
	"RIL_COMMAND_GETPHONEBOOKOPTIONS = 0x00000021",
	"RIL_COMMAND_GETALLADDITIONALNUMBERSTRINGS = 0x00000022",
	"RIL_COMMAND_GETALLEMERGENCYNUMBERS = 0x00000023",
	"RIL_COMMAND_SETRADIOCONFIGURATION = 0x00000024",
	"RIL_COMMAND_GETRADIOCONFIGURATION = 0x00000025",
	"RIL_COMMAND_SETEXECUTORCONFIG = 0x00000026",
	"RIL_COMMAND_GETEXECUTORCONFIG = 0x00000027",
	"RIL_COMMAND_SETSYSTEMSELECTIONPREFS = 0x00000028",
	"RIL_COMMAND_GETSYSTEMSELECTIONPREFS = 0x00000029",
	"RIL_COMMAND_GETOPERATORLIST = 0x0000002A",
	"RIL_COMMAND_GETPREFERREDOPERATORLIST = 0x0000002B",
	"RIL_COMMAND_GETCURRENTREGSTATUS = 0x0000002C",
	"RIL_COMMAND_GETSIGNALQUALITY = 0x0000002D",
	"RIL_COMMAND_SENDUICCTOOLKITCMDRESPONSE = 0x0000002F",
	"RIL_COMMAND_SENDUICCTOOLKITENVELOPE = 0x00000030",
	"RIL_COMMAND_DIAL = 0x00000031",
	"RIL_COMMAND_MANAGECALLS = 0x00000032",
	"RIL_COMMAND_EMERGENCYMODECONTROL = 0x00000033",
	"RIL_COMMAND_GETCALLFORWARDINGSETTINGS = 0x00000034",
	"RIL_COMMAND_SETCALLFORWARDINGSTATUS = 0x00000035",
	"RIL_COMMAND_ADDCALLFORWARDING = 0x00000036",
	"RIL_COMMAND_REMOVECALLFORWARDING = 0x00000037",
	"RIL_COMMAND_GETCALLBARRINGSTATUS = 0x00000038",
	"RIL_COMMAND_SETCALLBARRINGSTATUS = 0x00000039",
	"RIL_COMMAND_CHANGECALLBARRINGPASSWORD = 0x0000003A",
	"RIL_COMMAND_GETCALLWAITINGSETTINGS = 0x0000003B",
	"RIL_COMMAND_SETCALLWAITINGSTATUS = 0x0000003C",
	"RIL_COMMAND_GETCALLERIDSETTINGS = 0x0000003D",
	"RIL_COMMAND_GETDIALEDIDSETTINGS = 0x0000003E",
	"RIL_COMMAND_GETHIDECONNECTEDIDSETTINGS = 0x0000003F",
	"RIL_COMMAND_GETHIDEIDSETTINGS = 0x00000040",
	"RIL_COMMAND_SENDFLASH = 0x00000041",
	"RIL_COMMAND_SENDSUPSERVICEDATA = 0x00000042",
	"RIL_COMMAND_SENDDTMF = 0x00000043",
	"RIL_COMMAND_STARTDTMF = 0x00000044",
	"RIL_COMMAND_STOPDTMF = 0x00000045",
	"RIL_COMMAND_GETMSGSERVICEOPTIONS = 0x00000046",
	"RIL_COMMAND_READMSG = 0x00000047",
	"RIL_COMMAND_WRITEMSG = 0x00000048",
	"RIL_COMMAND_DELETEMSG = 0x00000049",
	"RIL_COMMAND_GETCELLBROADCASTMSGCONFIG = 0x0000004A",
	"RIL_COMMAND_SETCELLBROADCASTMSGCONFIG = 0x0000004B",
	"RIL_COMMAND_GETMSGINUICCSTATUS = 0x0000004C",
	"RIL_COMMAND_SETMSGINUICCSTATUS = 0x0000004D",
	"RIL_COMMAND_SETMSGMEMORYSTATUS = 0x0000004E",
	"RIL_COMMAND_SENDMSG = 0x0000004F",
	"RIL_COMMAND_GETSMSC = 0x00000050",
	"RIL_COMMAND_SETSMSC = 0x00000051",
	"RIL_COMMAND_GETIMSSTATUS = 0x00000052",
	"RIL_COMMAND_GETPOSITIONINFO = 0x00000053",
	"RIL_COMMAND_GETRADIOSTATEGROUPS = 0x00000054",
	"RIL_COMMAND_GETRADIOSTATEDETAILS = 0x00000055",
	"RIL_COMMAND_SETRADIOSTATEDETAILS = 0x00000056",
	"RIL_COMMAND_RADIOSTATEPASSWORDCOMPARE = 0x00000057",
	"RIL_COMMAND_RADIOSTATEGETPASSWORDRETRYCOUNT = 0x00000058",
	"RIL_COMMAND_DEVSPECIFIC = 0x00000059",
	"RIL_COMMAND_SETRFSTATE = 0x0000005A",
	"RIL_COMMAND_GETRFSTATE = 0x0000005B",
	"RIL_COMMAND_GETDMPROFILECONFIGINFO = 0x0000005C",
	"RIL_COMMAND_SETDMPROFILECONFIGINFO = 0x0000005D",
	"RIL_COMMAND_WRITEADDITIONALNUMBERSTRING = 0x0000005E",
	"RIL_COMMAND_DELETEADDITIONALNUMBERSTRING = 0x0000005F",
	"RIL_COMMAND_GETUICCATR = 0x00000060",
	"RIL_COMMAND_OPENUICCLOGICALCHANNEL = 0x00000061",
	"RIL_COMMAND_CLOSEUICCLOGICALCHANNEL = 0x00000062",
	"RIL_COMMAND_EXCHANGEUICCAPDU = 0x00000063",
	"RIL_COMMAND_SENDSUPSERVICEDATARESPONSE = 0x00000064",
	"RIL_COMMAND_CANCELSUPSERVICEDATASESSION = 0x00000065",
	"RIL_COMMAND_SETUICCTOOLKITPROFILE = 0x00000066",
	"RIL_COMMAND_GETUICCTOOLKITPROFILE = 0x00000067",
	"RIL_COMMAND_REGISTERUICCTOOLKITSERVICE = 0x00000068",
	"RIL_COMMAND_SENDMSGACK = 0x00000069",
	"RIL_COMMAND_CLOSEUICCLOGICALCHANNELGROUP = 0x0000006A",
	"RIL_COMMAND_SETPREFERREDOPERATORLIST = 0x0000006B",
	"RIL_COMMAND_GETUICCSERVICESTATE = 0x0000006C",
	"RIL_COMMAND_SETUICCSERVICESTATE = 0x0000006D",
	"RIL_COMMAND_GETCALLLIST = 0x0000006E",
	"RIL_COMMAND_GETEXECUTORFOCUS = 0x0000006F",
	"RIL_COMMAND_SETEXECUTORFOCUS = 0x00000070",
	"RIL_COMMAND_GETEMERGENCYMODE = 0x00000071",
	"RIL_COMMAND_GETEXECUTORRFSTATE = 0x00000072",
	"RIL_COMMAND_SETEXECUTORRFSTATE = 0x00000073",
	"RIL_COMMAND_RESETMODEM = 0x00000074",
	"RIL_COMMAND_CANCELGETOPERATORLIST = 0x00000075",
	"RIL_COMMAND_AVOIDCDMASYSTEM = 0x00000076",
	"RIL_COMMAND_SETPSMEDIACONFIGURATION = 0x00000077",
	"RIL_COMMAND_GETPSMEDIACONFIGURATION = 0x00000078",
	"RIL_COMMAND_ENABLEMODEMFILTERS = 0x00000400",
	"RIL_COMMAND_DISABLEMODEMFILTERS = 0x00000401",
	"RIL_COMMAND_STARTMODEMLOGS = 0x00000402",
	"RIL_COMMAND_STOPMODEMLOGS = 0x00000403",
	"RIL_COMMAND_DRAINMODEMLOGS = 0x00000404",
	"RIL_COMMAND_COUNT = 0x00000404"
};

const char* TranslateCommandCodeToString(DWORD dwCode)
{
	DWORD code = dwCode - 1;

	if (dwCode > 0x78 && dwCode < 0x400)
		return "Unknown";

	if (dwCode >= 0x400)
	{
		code = code - 0x400 + 0x78 + 1;
	}

	if (code >= 125)
		return "Unknown";

	return RILCOMMANDSSTRS[code];
}

std::string GetDateStr()
{
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &timeinfo);
	std::string str(buffer);
	return str;
}


BOOL RIL_IOControl(
	DWORD hOpenContext,
	DWORD dwCode,
	PBYTE pBufIn,
	DWORD dwLenIn,
	PBYTE pBufOut,
	DWORD dwLenOut,
	PDWORD pdwActualOut
)
{
	if (dwCode == RIL_COMMAND_GETNEXTNOTIFICATION)
	{
		return RRIL_IOControl(hOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	}

	std::cout << "\n" << GetDateStr() << " RIL_IOControl: called" << "\n";
	std::cout << GetDateStr() << " RIL_IOControl: hOpenContext = 0x" << std::hex << hOpenContext << " dwCode = 0x" << std::hex << dwCode
		<< " dwLenIn = 0x" << std::hex << dwLenIn << " dwLenOut = 0x" << std::hex << dwLenOut << "\n";

	std::cout << GetDateStr() << " RIL_IOControl: Code is " << TranslateCommandCodeToString(dwCode) << "\n";

	if (wmril == NULL)
	{
		std::cout << GetDateStr() << " RIL_IOControl: Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		std::cout << GetDateStr() << " RIL_IOControl: WMRil module is loaded" << "\n";
	}

	std::cout << GetDateStr() << " RIL_IOControl: calling WMRil" << "\n";
	BOOL result = RRIL_IOControl(hOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	std::cout << GetDateStr() << " RIL_IOControl: WMRil called" << "\n";

	if (dwCode != RIL_COMMAND_GETNEXTNOTIFICATION)
	{
		std::cout << "------------------------------------" << "\n";
		std::cout << "Input:" << "\n";
		for (unsigned long i = 0; i < dwLenIn; i++)
		{
			BYTE b = pBufIn[i];
			std::cout << "0x" << std::hex << (int)b << " ";
		}
		std::cout << "\n";
		std::cout << "------------------------------------" << "\n";

		std::cout << "------------------------------------" << "\n";
		std::cout << "Output:" << "\n";
		for (unsigned long i = 0; i < dwLenOut; i++)
		{
			BYTE b = pBufOut[i];
			std::cout << "0x" << std::hex << (int)b << " ";
		}
		std::cout << "\n";
		std::cout << "------------------------------------" << "\n";
	}

	switch (dwCode)
	{
	case RIL_COMMAND_INITNOTIFICATIONS:
	{
		break;
	}
	case RIL_COMMAND_GETNEXTNOTIFICATION:
	{
		std::cout << GetDateStr() << " RIL_IOControl: RIL_COMMAND_GETNEXTNOTIFICATION: dwActualOut = 0x" << std::hex << *pdwActualOut << "\n";
		break;
	}
	default:
	{
		if (dwLenOut == 4)
		{
			HRESULT res = *(HRESULT*)pBufOut;
			std::cout << GetDateStr() << " RIL_IOControl: HRESULT: pBufOut = 0x" << std::hex << res << "\n";
		}
	}
	}

	std::cout << GetDateStr() << " RIL_IOControl: result = 0x" << std::hex << result << "\n";

	std::cout << GetDateStr() << " RIL_IOControl: returning" << "\n" << "\n";

	return result;
}

#define THREADCOUNT 1

HANDLE ghNotificationEvent;
HANDLE ghCancellationEvent;
HANDLE ghThreads[THREADCOUNT];

DWORD hOpenContext;

DWORD WINAPI ThreadProc(LPVOID);

void CreateEventsAndThreads(DWORD hOpenContext)
{
	int i;
	DWORD dwThreadID;

	// Create a manual-reset event object. The write thread sets this
	// object to the signaled state when it finishes writing to a 
	// shared buffer. 

	ghCancellationEvent = CreateEvent(
		NULL, 
		TRUE, 
		FALSE, 
		L"CancellationEvent");


	std::cout << GetDateStr() << " Initializing RIL notifications with our event" << "\n";

	DWORD realout;
	BOOL result = RRIL_IOControl(hOpenContext, RIL_COMMAND_INITNOTIFICATIONS, (PBYTE)&ghCancellationEvent, sizeof(ghCancellationEvent), (PBYTE)&ghNotificationEvent, sizeof(ghNotificationEvent), &realout);
	if (!result)
	{
		std::cout << GetDateStr() << " We failed to initialize notifications. This is fatal. Leaving." << "\n";
		exit(1);
	}

	std::cout << GetDateStr() << " Initialized notification queue with RIL" << "\n";

	if (ghNotificationEvent == NULL)
	{
		printf("CreateEvent failed (%d)\n", GetLastError());
		return;
	}
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	// lpParam not used in this example.
	UNREFERENCED_PARAMETER(lpParam);

	DWORD dwWaitResult;
	BOOL result;
	DWORD realout;
	RILDRVNOTIFICATION* notifications = (RILDRVNOTIFICATION*)malloc(0x1000);


	while (true)
	{
		printf("Thread %d waiting for write event...\n", GetCurrentThreadId());

		dwWaitResult = WaitForSingleObject(
			ghNotificationEvent, // event handle
			INFINITE);    // indefinite wait

		switch (dwWaitResult)
		{
			// Event object was signaled
		case WAIT_OBJECT_0:
			//
			// TODO: Read from the shared buffer
			//
			printf("Thread %d reading from buffer\n",
				GetCurrentThreadId());

			result = RIL_IOControl(hOpenContext, RIL_COMMAND_GETNEXTNOTIFICATION, NULL, NULL, (PBYTE)notifications, 0x1000, &realout);
			if (notifications->dwDataSize == 0)
			{
				std::cout << GetDateStr() << " No notifications present currently" << "\n";
			}
			else
			{
				std::cout << GetDateStr() << " Notification found" << "\n";
			}

			ResetEvent(ghNotificationEvent);

			break;

			// An error occurred
		default:
			printf("Wait error (%d)\n", GetLastError());
			return 0;
		}
	}
	// Now that we are done reading the buffer, we could use another
	// event to signal that this thread is no longer reading. This
	// example simply uses the thread handle for synchronization (the
	// handle is signaled when the thread terminates.)

	printf("Thread %d exiting\n", GetCurrentThreadId());
	return 1;
}

int main()
{
	std::cout << GetDateStr() << " OpenEMB v0.1" << "\n";
	std::cout << GetDateStr() << " Copyright 2018-2019 (c) LumiaWOA" << "\n" << "\n";

	if (wmril == NULL)
	{
		std::cout << GetDateStr() << " Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		std::cout << GetDateStr() << " WMRil module is loaded" << "\n" << "\n";
	}

	std::cout << GetDateStr() << " Negotiating RIL version 1.1 with WMRil" << "\n" ;
	DWORD maxVer = RRIL_Version(1, 1);
	std::cout << GetDateStr() << " Negotiated max version: 0x" << std::hex << maxVer << " with WMRil" << "\n";

	if (maxVer != 1)
	{
		std::cout << GetDateStr() << " We do not support this protocol version. Leaving." << "\n";
		return 1;
	}

	std::cout << GetDateStr() << " We support this protocol version. Continuing." << "\n";

	std::cout << GetDateStr() << " Initializing RIL for modem (0)" << "\n";
	DWORD inith = RRIL_Init(0);
	std::cout << GetDateStr() << " Got the following device context: 0x" << std::hex << inith << "\n";
	std::cout << GetDateStr() << " Opening this context with AccessCode=0 and ShareMode=0" << "\n";
	hOpenContext = RRIL_Open(inith, 0, 0);
	std::cout << GetDateStr() << " Got the following open context: 0x" << std::hex << hOpenContext << "\n";

	std::cout << GetDateStr() << " Creating cancellation event" << "\n";

	CreateEventsAndThreads(hOpenContext);

	DWORD dwWaitResult = WaitForSingleObject(
		ghNotificationEvent, // event handle
		INFINITE);    // indefinite wait

	std::cout << GetDateStr() << " Retrieving WMRIL version info" << "\n";

	HRESULT rescode = { 0 };
	RILGETDRIVERVERSIONPARAMS versionparams = { 0 };

	versionparams.dwMaxVersion = 0x320000;
	versionparams.dwMinVersion = 0x20000;

	DWORD realout;
	BOOL result = RIL_IOControl(hOpenContext, RIL_COMMAND_GETDRIVERVERSION, (PBYTE)&versionparams, sizeof(versionparams), (PBYTE)&rescode, sizeof(rescode), &realout);

	if (!result)
	{
		std::cout << GetDateStr() << " An error occured while we were trying to negotiate WMRIL version information. This is fatal. Leaving." << "\n";
		return 1;
	}

	dwWaitResult = WaitForSingleObject(
		ghNotificationEvent, // event handle
		INFINITE);    // indefinite wait

	// TODO fix this
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
