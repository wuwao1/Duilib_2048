#include "StdAfx.h"
#include "TrayIcon.h"
#include "Shlwapi.h"

namespace DuiLib
{
CTrayIcon::CTrayIcon(void) :
    m_uMessage(UIMSG_TRAYICON) {}

CTrayIcon::~CTrayIcon(void)
{
    DeleteTrayIcon();
}

void CTrayIcon::CreateTrayIcon(HWND _RecvHwnd, UINT _IconIDResource, faw::string_t _ToolTipText, UINT _Message)
{
    if(!_RecvHwnd || _IconIDResource <= 0)
    {
        return;
    }
    if(_Message != 0) m_uMessage = _Message;
    m_hIcon = LoadIcon(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(_IconIDResource));
    m_trayData.cbSize = sizeof(NOTIFYICONDATA);
    m_trayData.hWnd = _RecvHwnd;
    m_trayData.uID = _IconIDResource;
    m_trayData.hIcon = m_hIcon;
    m_trayData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_trayData.uCallbackMessage = m_uMessage;
    if(!_ToolTipText.empty()) _tcscpy(m_trayData.szTip, _ToolTipText.data());
    Shell_NotifyIcon(NIM_ADD, &m_trayData);
    m_bEnabled = true;
}

void CTrayIcon::DeleteTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &m_trayData);
    m_bEnabled = false;
    m_bVisible = false;
    m_hWnd = nullptr;
    m_uMessage = UIMSG_TRAYICON;
}

bool CTrayIcon::SetTooltipText(faw::string_t _ToolTipText)
{
    if(!_ToolTipText.empty()) _tcscpy(m_trayData.szTip, _ToolTipText.data());
    if(!m_bEnabled) return FALSE;
    m_trayData.uFlags = NIF_TIP;
    return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;
}

bool CTrayIcon::SetTooltipText(UINT _IDResource)
{
    TCHAR mbuf[256] = { 0 };
    LoadString(CPaintManagerUI::GetInstance(), _IDResource, mbuf, 256);
    return SetTooltipText(mbuf);
}

faw::string_t CTrayIcon::GetTooltipText() const
{
    return m_trayData.szTip;
}

bool CTrayIcon::SetIcon(HICON _Hicon)
{
    if(_Hicon) m_hIcon = _Hicon;
    m_trayData.uFlags = NIF_ICON;
    m_trayData.hIcon = _Hicon;

    if(!m_bEnabled) return FALSE;
    return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;

    return false;
}

bool CTrayIcon::SetIcon(faw::string_t _IconFile)
{
    HICON hIcon = LoadIcon(CPaintManagerUI::GetInstance(), _IconFile.data());
    return SetIcon(hIcon);
}

bool CTrayIcon::SetIcon(UINT _IDResource)
{
    HICON hIcon = LoadIcon(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(_IDResource));
    return SetIcon(hIcon);
}

HICON CTrayIcon::GetIcon() const
{
    HICON hIcon = nullptr;
    hIcon = m_trayData.hIcon;
    return hIcon;
}

void CTrayIcon::SetHideIcon()
{
    if(IsVisible())
    {
        SetIcon((HICON)nullptr);
        m_bVisible = TRUE;
    }
}

void CTrayIcon::SetShowIcon()
{
    if(!IsVisible())
    {
        SetIcon(m_hIcon);
        m_bVisible = FALSE;
    }
}

void CTrayIcon::RemoveIcon()
{
    m_trayData.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &m_trayData);
    m_bEnabled = FALSE;
}

BOOL CTrayIcon::SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style /*= BALLOON_INFO*/, UINT nTimeout /*= 1000*/, HICON hUserIcon /*= NULL*/, BOOL bNoSound /*= FALSE*/)
{
    ASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

#if (_WIN32_IE >= 0x0500)
    m_trayData.uFlags = NIF_INFO;
#endif
#if (_MSC_VER >= 1400)
    _tcscpy_s(m_trayData.szInfo, sizeof(m_trayData.szInfo) / sizeof(TCHAR), pszBalloonText);
    _tcscpy_s(m_trayData.szInfoTitle, sizeof(m_trayData.szInfoTitle) / sizeof(TCHAR), pszBalloonCaption);
#else
    _tcscpy(m_trayData.szInfo, pszBalloonText);
    _tcscpy(m_trayData.szInfoTitle, pszBalloonCaption);
#endif
    m_trayData.uTimeout = nTimeout;
#if (_WIN32_IE >= 0x0500)
    switch(style)
    {
    case BALLOON_WARNING:
        m_trayData.dwInfoFlags = NIIF_WARNING;
        break;
    case BALLOON_ERROR:
        m_trayData.dwInfoFlags = NIIF_ERROR;
        break;
    case BALLOON_INFO:
        m_trayData.dwInfoFlags = NIIF_INFO;
        break;
    case BALLOON_NONE:
        m_trayData.dwInfoFlags = NIIF_NONE;
        break;
#if (_WIN32_IE >= 0x0600)
    case BALLOON_USER:
        ASSERT(hUserIcon != NULL);
        m_trayData.dwInfoFlags = NIIF_USER;
        m_trayData.hIcon = hUserIcon;
        break;
#endif
    default:
        ASSERT(FALSE);
        break;
    }
#endif
#if (_WIN32_IE >= 0x0501)
    if(bNoSound)
        m_trayData.dwInfoFlags |= NIIF_NOSOUND;
#endif
    return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) ? TRUE : false;
}

DWORD CTrayIcon::GetShellVersion(void)
{
    if(m_dwShellVersion)
    {
        return m_dwShellVersion;
    }
    else
    {
        m_dwShellVersion = 4;

        typedef HRESULT(CALLBACK* PFNDLLGETVERSION)(DLLVERSIONINFO*);

        HMODULE hShell32 = GetModuleHandle(_T("shell32.dll"));
        PFNDLLGETVERSION lpfnDllGetVersion = reinterpret_cast<PFNDLLGETVERSION>(GetProcAddress(hShell32, "DllGetVersion"));
        if(lpfnDllGetVersion)
        {
            DLLVERSIONINFO vinfo;
            vinfo.cbSize = sizeof(DLLVERSIONINFO);
            if(SUCCEEDED(lpfnDllGetVersion(&vinfo)))
                m_dwShellVersion = vinfo.dwMajorVersion;
        }
    }

    return m_dwShellVersion;
}

}
