#ifndef _VERSIONHELPERS_H_INCLUDED_
#define _VERSIONHELPERS_H_INCLUDED_
#include <specstrings.h>

namespace DuiLib
{
#define _WIN32_WINNT_NT4                    0x0400
#define _WIN32_WINNT_WIN2K                  0x0500
#define _WIN32_WINNT_WINXP                  0x0501
#define _WIN32_WINNT_WS03                   0x0502
#define _WIN32_WINNT_WIN6                   0x0600
#define _WIN32_WINNT_VISTA                  0x0600
#define _WIN32_WINNT_WS08                   0x0600
#define _WIN32_WINNT_LONGHORN               0x0600
#define _WIN32_WINNT_WIN7                   0x0601
#define _WIN32_WINNT_WIN8                   0x0602
#define _WIN32_WINNT_WINBLUE                0x0603
#define _WIN32_WINNT_WINTHRESHOLD           0x0A00 /* ABRACADABRA_THRESHOLD*/
#define _WIN32_WINNT_WIN10                  0x0A00 /* ABRACADABRA_THRESHOLD*/

#define WM_DPICHANGED                   0x02E0

static BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL), VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    osvi.dwMajorVersion = wMajorVersion;
    osvi.dwMinorVersion = wMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;
    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

static BOOL IsWindowsXPOrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0);
}

static BOOL IsWindowsXPSP1OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 1);
}

static BOOL IsWindowsXPSP2OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 2);
}

static BOOL IsWindowsXPSP3OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 3);
}

static BOOL IsWindowsVistaOrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
}

static BOOL IsWindowsVistaSP1OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 1);
}

static BOOL IsWindowsVistaSP2OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 2);
}

static BOOL IsWindows7OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0);
}

static BOOL IsWindows7SP1OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 1);
}

static BOOL IsWindows8OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
}

static BOOL IsWindows8Point1OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0);
}

static BOOL IsWindowsThresholdOrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
}

static BOOL IsWindows10OrGreater()
{
    return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
}

static BOOL IsWindowsServer()
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0, 0, VER_NT_WORKSTATION };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
    return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
}

static int GetWinOSVersion()
{
    bool bIs64 = []()->bool
    {
        typedef void (WINAPI* pFnGetNativeSystemInfo)(SYSTEM_INFO* lpSystemInfo);
        pFnGetNativeSystemInfo pFun = (pFnGetNativeSystemInfo)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetNativeSystemInfo");
        SYSTEM_INFO si = { 0 };
        if(pFun != NULL)
        {
            pFun(&si);
        }
        else
        {
            GetSystemInfo(&si);
        }
        if(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
        {
            return true;
        }
        return false;
    }();

    do
    {
        //if (IsWindowsServer()){
        //	return 14;//winserver
        //}
        if(IsWindows10OrGreater())
        {
            return bIs64 ? 13 : 12;//win1064 : win1032
        }
        if(IsWindows8Point1OrGreater())
        {
            return bIs64 ? 11 : 10;//win8164 : win8132
        }
        if(IsWindows8OrGreater())
        {
            return bIs64 ? 9 : 8;//win864 : win832
        }
        if(IsWindows7SP1OrGreater() || IsWindows7OrGreater())
        {
            return bIs64 ? 7 : 6;//win764 : win732
        }
        if(IsWindowsVistaSP2OrGreater() || IsWindowsVistaSP1OrGreater() || IsWindowsVistaOrGreater())
        {
            return bIs64 ? 5 : 4;//vista64 : vista32
        }
        if(IsWindowsXPSP3OrGreater())
        {
            return 2;//xp sp3 32
        }
        if(IsWindowsXPSP2OrGreater() || IsWindowsXPSP1OrGreater() || IsWindowsXPOrGreater())
        {
            return bIs64 ? 3 : 1;//xp64 : xp
        }
    }
    while(false);
    return 0;//unknown
}
}


#endif // _VERSIONHELPERS_H_INCLUDED_
