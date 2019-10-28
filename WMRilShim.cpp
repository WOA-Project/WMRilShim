#include "pch.h"
#include "WMRilShim.h"
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

DWORD RIL_Init(DWORD dwModemID)
{
	ofstream logFile;
	logFile.open("C:\\WMRilShim.log", std::ofstream::out | std::ofstream::app);

	logFile << GetDateStr() << " RIL_Init: called" << "\n";
	logFile << GetDateStr() << " RIL_Init: dwModemID = 0x" << std::hex << dwModemID << "\n";

	if (wmril == NULL)
	{
		logFile << GetDateStr() << " RIL_Init: Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		logFile << GetDateStr() << " RIL_Init: WMRil module is loaded" << "\n";
	}

	logFile << GetDateStr() << " RIL_Init: calling WMRil" << "\n";
	DWORD inith = RRIL_Init(dwModemID);
	logFile << GetDateStr() << " RIL_Init: WMRil called" << "\n";

	logFile << GetDateStr() << " RIL_Init: InitHandle = 0x" << std::hex << inith << "\n";

	logFile << GetDateStr() << " RIL_Init: returning" << "\n";
	logFile.close();
	return inith;
}

DWORD RIL_Version(DWORD VersionRangeLow, DWORD VersionRangeHigh)
{
	ofstream logFile;
	logFile.open("C:\\WMRilShim.log", std::ofstream::out | std::ofstream::app);

	logFile << GetDateStr() << " RIL_Version: called" << "\n";
	logFile << GetDateStr() << " RIL_Version: VersionRangeLow = 0x" << std::hex << VersionRangeLow << " VersionRangeHigh = 0x" << std::hex << VersionRangeHigh << "\n";

	if (wmril == NULL)
	{
		logFile << GetDateStr() << " RIL_Version: Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		logFile << GetDateStr() << " RIL_Version: WMRil module is loaded" << "\n";
	}

	logFile << GetDateStr() << " RIL_Version: calling WMRil" << "\n";
	DWORD dwMaxSupportedVersion = RRIL_Version(VersionRangeLow, VersionRangeHigh);
	logFile << GetDateStr() << " RIL_Version: WMRil called" << "\n";

	logFile << GetDateStr() << " RIL_Version: dwMaxSupportedVersion = 0x" << std::hex << dwMaxSupportedVersion << "\n";

	logFile << GetDateStr() << " RIL_Version: returning" << "\n";
	logFile.close();
	return dwMaxSupportedVersion;
}

DWORD RIL_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	ofstream logFile;
	logFile.open("C:\\WMRilShim.log", std::ofstream::out | std::ofstream::app);

	logFile << GetDateStr() << " RIL_Open: called" << "\n";
	logFile << GetDateStr() << " RIL_Open: hDeviceContext = 0x" << std::hex << hDeviceContext << " AccessCode = 0x" << std::hex << AccessCode << " ShareMode = 0x" << std::hex << ShareMode << "\n";

	if (wmril == NULL)
	{
		logFile << GetDateStr() << " RIL_Open: Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		logFile << GetDateStr() << " RIL_Open: WMRil module is loaded" << "\n";
	}

	logFile << GetDateStr() << " RIL_Open: calling WMRil" << "\n";
	DWORD openh = RRIL_Open(hDeviceContext, AccessCode, ShareMode);
	logFile << GetDateStr() << " RIL_Open: WMRil called" << "\n";

	logFile << GetDateStr() << " RIL_Open: OpenHandle = 0x" << std::hex << openh << "\n";

	logFile << GetDateStr() << " RIL_Open: returning" << "\n";
	logFile.close();
	return openh;
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
	ofstream logFile;
	logFile.open("C:\\WMRilShim.log", std::ofstream::out | std::ofstream::app);

	logFile << "\n" << GetDateStr() << " RIL_IOControl: called" << "\n";
	logFile << GetDateStr() << " RIL_IOControl: hOpenContext = 0x" << std::hex << hOpenContext << " dwCode = 0x" << std::hex << dwCode
		<< " dwLenIn = 0x" << std::hex << dwLenIn << " dwLenOut = 0x" << std::hex << dwLenOut << "\n";

	logFile << GetDateStr() << " RIL_IOControl: Code is " << TranslateCommandCodeToString(dwCode) << "\n";

	if (wmril == NULL)
	{
		logFile << GetDateStr() << " RIL_IOControl: Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		logFile << GetDateStr() << " RIL_IOControl: WMRil module is loaded" << "\n";
	}

	logFile << GetDateStr() << " RIL_IOControl: calling WMRil" << "\n";
	BOOL result = RRIL_IOControl(hOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	logFile << GetDateStr() << " RIL_IOControl: WMRil called" << "\n";

	if (dwCode != RIL_COMMAND_GETNEXTNOTIFICATION)
	{
		logFile << "------------------------------------" << "\n";
		logFile << "Input:" << "\n";
		for (unsigned long i = 0; i < dwLenIn; i++)
		{
			unsigned long index = dwLenIn - i - 1;
			BYTE b = pBufIn[index];
			logFile << "0x" << std::hex << (int)b << " ";
		}
		logFile << "\n";
		logFile << "------------------------------------" << "\n";

		logFile << "------------------------------------" << "\n";
		logFile << "Output:" << "\n";
		for (unsigned long i = 0; i < dwLenOut; i++)
		{
			unsigned long index = dwLenOut - i - 1;
			BYTE b = pBufOut[index];
			logFile << "0x" << std::hex << (int)b << " ";
		}
		logFile << "\n";
		logFile << "------------------------------------" << "\n";
	}

	switch (dwCode)
	{
		case RIL_COMMAND_INITNOTIFICATIONS:
		{
			break;
		}
		case RIL_COMMAND_GETNEXTNOTIFICATION:
		{
			logFile << GetDateStr() << " RIL_IOControl: RIL_COMMAND_GETNEXTNOTIFICATION: dwActualOut = 0x" << std::hex << *pdwActualOut << "\n";
			break;
		}
		default:
		{
			if (dwLenOut == 4)
			{
				HRESULT res = *(HRESULT*)pBufOut;
				logFile << GetDateStr() << " RIL_IOControl: HRESULT: pBufOut = 0x" << std::hex << res << "\n";
			}
		}
	}

	logFile << GetDateStr() << " RIL_IOControl: result = 0x" << std::hex << result << "\n";

	logFile << GetDateStr() << " RIL_IOControl: returning" << "\n" << "\n";

	logFile.close();
	return result;
}

BOOL RIL_Close(DWORD hOpenContext)
{
	ofstream logFile;
	logFile.open("C:\\WMRilShim.log", std::ofstream::out | std::ofstream::app);

	logFile << GetDateStr() << " RIL_Close: called" << "\n";
	logFile << GetDateStr() << " RIL_Close: hOpenContext = 0x" << std::hex << hOpenContext << "\n";

	if (wmril == NULL)
	{
		logFile << GetDateStr() << " RIL_Close: Looks like we didn't get initialized yet, loading WMRil module" << "\n";
		Initialize();
		logFile << GetDateStr() << " RIL_Close: WMRil module is loaded" << "\n";
	}

	logFile << GetDateStr() << " RIL_Close: calling WMRil" << "\n";
	BOOL result = RRIL_Close(hOpenContext);
	logFile << GetDateStr() << " RIL_Close: WMRil called" << "\n";

	logFile << GetDateStr() << " RIL_Close: result = 0x" << std::hex << result << "\n";

	logFile << GetDateStr() << " RIL_Close: returning" << "\n";
	logFile.close();
	return result;
}