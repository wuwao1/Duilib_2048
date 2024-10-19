#ifndef __UITRAICON_H__
#define __UITRAICON_H__

#pragma once
#include <ShellAPI.h>

namespace DuiLib
{

enum BalloonStyle
{
    BALLOON_WARNING,
    BALLOON_ERROR,
    BALLOON_INFO,
    BALLOON_NONE,
    BALLOON_USER,
};

class UILIB_API CTrayIcon
{
public:
    using ptr = std::shared_ptr<CTrayIcon>;
    CTrayIcon(void);
    virtual ~CTrayIcon(void);

public:
    void CreateTrayIcon(HWND _RecvHwnd, UINT _IconIDResource, faw::string_t _ToolTipText = _T(""), UINT _Message = 0);
    void DeleteTrayIcon();
    bool SetTooltipText(faw::string_t _ToolTipText);
    bool SetTooltipText(UINT _IDResource);
    faw::string_t GetTooltipText() const;

    bool SetIcon(HICON _Hicon);
    bool SetIcon(faw::string_t _IconFile);
    bool SetIcon(UINT _IDResource);
    HICON GetIcon() const;
    void SetHideIcon();
    void SetShowIcon();
    void RemoveIcon();
    BOOL SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style, UINT nTimeout, HICON hUserIcon, BOOL bNoSound);
    DWORD GetShellVersion(void);
    bool Enabled()
    {
        return m_bEnabled;
    };
    bool IsVisible()
    {
        return !m_bVisible;
    };

private:
    bool			m_bEnabled	= false;
    bool			m_bVisible	= false;
    HWND			m_hWnd		= NULL;
    UINT			m_uMessage;
    HICON			m_hIcon		= NULL;
    DWORD			m_dwShellVersion = 0;
    NOTIFYICONDATA	m_trayData	= { 0 };
};
}
#endif // 

