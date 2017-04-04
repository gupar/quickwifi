#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS    // some CString constructors will be explicit
#define IDS_ERROR_CANNOT_DISPLAY_SSID            200


#pragma warning(disable: 4430) 
#pragma warning(disable: 4995)
#pragma warning(disable: 4002)
#pragma warning(disable: 4005)

#include <ws2tcpip.h>
#include <WindowsX.h>
#include <tchar.h>
#include <strsafe.h>
#include <netcon.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <wlanapi.h>
#include <string>
//#include <wininet.h>
#include <iphlpapi.h>
#include "include\UIlib.h"

#include "common.h"
#include "util.h"
#include "WlanMgr.h"
#include "device.h"
#include "notif.h"
#include "icsconn.h"
#include "icsmgr.h"
#include "utils.h"
#include "Wininet.h"
#include "Shlobj.h"
#include <vector>
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "netAPI32.lib")
#pragma comment(lib, "Shell32.lib")
#include "resource.h"
namespace DuiLib 
{
	typedef std::basic_string<TCHAR> tString;
}
#ifndef NO_USING_DUILIB_NAMESPACE
	using namespace DuiLib;
	using namespace std;
#endif

// #ifdef _DEBUG
// #   ifdef _UNICODE
// #       pragma comment(lib, "include\\MiniUI_d.lib")
// #   else
// #       pragma comment(lib, "include\\MiniUI_d.lib")
// #   endif
// #else
// #   ifdef _UNICODE
// #       pragma comment(lib, "include\\MiniUI.lib")
// #   else
// #       pragma comment(lib, "include\\MiniUI.lib")
// #   endif
// #endif
#pragma comment(lib,"OleDlg.lib")
#pragma comment(lib,"Riched20.lib")
#pragma comment(lib,"Imm32.lib")
#pragma comment(lib,"Comctl32.lib")

#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "UILib_d.lib")
#   else
#       pragma comment(lib, "UILib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "UILib.lib")
#   else
#       pragma comment(lib, "UILib.lib")
#   endif
#endif


	BOOL IsCompare(CString str,CString str2);
	
#include "win_base.h"
#include "RegDlg.h"
#include "main_frame.h"

#define USE(FEATURE) (defined USE_##FEATURE  && USE_##FEATURE)
#define ENABLE(FEATURE) (defined ENABLE_##FEATURE  && ENABLE_##FEATURE)

#define USE_ZIP_SKIN 1
#define USE_EMBEDED_RESOURCE 1

#define MESSAGE_RICHEDIT_MAX  1024
#define CM_TRAYMESSAGE WM_USER+100
#define  WM_HASREG WM_USER+101
#define  WM_REGCLOSE WM_USER+102
