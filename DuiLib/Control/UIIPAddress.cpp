﻿#include "StdAfx.h"
#pragma comment( lib, "ws2_32.lib" )

DWORD GetLocalIpAddress()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if(WSAStartup(wVersionRequested, &wsaData) != 0)
        return 0;
    char local[255] = { 0 };
    gethostname(local, sizeof(local));
    hostent* ph = gethostbyname(local);
    if(!ph)
        return 0;
    in_addr addr;
    memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr));
    DWORD dwIP = MAKEIPADDRESS(addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
    return dwIP;
}

namespace DuiLib
{
//CDateTimeUI::m_nDTUpdateFlag
#define IP_NONE   0
#define IP_UPDATE 1
#define IP_DELETE 2
#define IP_KEEP   3

class CIPAddressWnd: public CWindowWnd
{
public:
    CIPAddressWnd();

    void Init(CIPAddressUI* pOwner);
    RECT CalPos();

    LPCTSTR GetWindowClassName() const;
    LPCTSTR GetSuperClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    std::optional<LRESULT> OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    CIPAddressUI	*m_pOwner	= nullptr;
    HBRUSH			m_hBkBrush	= NULL;
    bool			m_bInit		= false;
};

CIPAddressWnd::CIPAddressWnd() {}

void CIPAddressWnd::Init(CIPAddressUI* pOwner)
{
    m_pOwner = pOwner;
    m_pOwner->m_nIPUpdateFlag = IP_NONE;

    if(!m_hWnd)
    {
        INITCOMMONCONTROLSEX   CommCtrl;
        CommCtrl.dwSize = sizeof(CommCtrl);
        CommCtrl.dwICC = ICC_INTERNET_CLASSES;//指定Class
        if(InitCommonControlsEx(&CommCtrl))
        {
            RECT rcPos = CalPos();
            UINT uStyle = WS_CHILD | WS_TABSTOP | WS_GROUP;
            Create(m_pOwner->GetManager()->GetPaintWindow(), _T(""), uStyle, 0, rcPos);
        }
        SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
    }

    if(m_pOwner->GetText().empty())
        m_pOwner->m_dwIP = GetLocalIpAddress();
    ::SendMessage(m_hWnd, IPM_SETADDRESS, 0, m_pOwner->m_dwIP);
    ::ShowWindow(m_hWnd, SW_SHOW);
    ::SetFocus(m_hWnd);

    m_bInit = true;
}

RECT CIPAddressWnd::CalPos()
{
    return m_pOwner->GetPos();
}

LPCTSTR CIPAddressWnd::GetWindowClassName() const
{
    return _T("IPAddressWnd");
}

LPCTSTR CIPAddressWnd::GetSuperClassName() const
{
    return WC_IPADDRESS;
}

void CIPAddressWnd::OnFinalMessage(HWND /*hWnd*/)
{
    // Clear reference and die
    if(m_hBkBrush) ::DeleteObject(m_hBkBrush);
    m_pOwner->m_pWindow = nullptr;
    delete this;
}

LRESULT CIPAddressWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::optional<LRESULT> lRes = 0;
    if(uMsg == WM_KILLFOCUS)
    {
        //lRes = OnKillFocus (uMsg, wParam, lParam);
    }
    else if(uMsg == WM_KEYUP && (wParam == VK_DELETE || wParam == VK_BACK))
    {
        lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        m_pOwner->m_nIPUpdateFlag = IP_DELETE;
        m_pOwner->UpdateText();
        PostMessage(WM_CLOSE);
    }
    else if(uMsg == WM_KEYUP && wParam == VK_ESCAPE)
    {
        lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        m_pOwner->m_nIPUpdateFlag = IP_KEEP;
        PostMessage(WM_CLOSE);
    }
    else if(uMsg == OCM_COMMAND)
    {
        if(GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS)
        {
            lRes = OnKillFocus(uMsg, wParam, lParam);
        }
    }
    else
    {
        lRes = std::nullopt;
    }
    if(lRes == std::nullopt)
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    return lRes.value();
}

std::optional<LRESULT> CIPAddressWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hWndFocus = GetFocus();
    while(hWndFocus)
    {
        if(GetFocus() == m_hWnd)
        {
            return 0;
        }
        hWndFocus = GetParent(hWndFocus);
    }

    LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    if(m_pOwner->m_nIPUpdateFlag == IP_NONE)
    {
        ::SendMessage(m_hWnd, IPM_GETADDRESS, 0, (LPARAM) &m_pOwner->m_dwIP);
        m_pOwner->m_nIPUpdateFlag = IP_UPDATE;
        m_pOwner->UpdateText();
    }
    ::ShowWindow(m_hWnd, SW_HIDE);
    return lRes;
}

//////////////////////////////////////////////////////////////////////////
//
IMPLEMENT_DUICONTROL(CIPAddressUI)

CIPAddressUI::CIPAddressUI()
{
    m_dwIP = GetLocalIpAddress();
    m_nIPUpdateFlag = IP_UPDATE;
    UpdateText();
    m_nIPUpdateFlag = IP_NONE;
}

faw::string_t CIPAddressUI::GetClass() const
{
    return _T("DateTimeUI");
}

std::shared_ptr<CControlUI> CIPAddressUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_IPADDRESS) return std::dynamic_pointer_cast<CIPAddressUI>(shared_from_this());
    return CLabelUI::GetInterface(pstrName);
}

DWORD CIPAddressUI::GetIP()
{
    return m_dwIP;
}

void CIPAddressUI::SetIP(DWORD dwIP)
{
    m_dwIP = dwIP;
    UpdateText();
}

void CIPAddressUI::SetReadOnly(bool bReadOnly)
{
    m_bReadOnly = bReadOnly;
    Invalidate();
}

bool CIPAddressUI::IsReadOnly() const
{
    return m_bReadOnly;
}

void CIPAddressUI::UpdateText()
{
    if(m_nIPUpdateFlag == IP_DELETE)
        SetText(_T(""));
    else if(m_nIPUpdateFlag == IP_UPDATE)
    {
        in_addr addr;
        addr.S_un.S_addr = m_dwIP;
        faw::string_t szIP = FawTools::format_str(_T("%d.%d.%d.%d"), addr.S_un.S_un_b.s_b4, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b1);
        SetText(szIP);
    }
}

void CIPAddressUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }
    else if(event.Type == UIEVENT_SETCURSOR && IsEnabled())
    {
        ::SetCursor(::LoadCursor(nullptr, IDC_IBEAM));
        return;
    }
    else if(event.Type == UIEVENT_WINDOWSIZE)
    {
        if(m_pWindow) m_pManager->SetFocusNeeded(std::dynamic_pointer_cast<CIPAddressUI>(shared_from_this()));
    }
    else if(event.Type == UIEVENT_SCROLLWHEEL)
    {
        if(m_pWindow) return;
    }
    else if(event.Type == UIEVENT_SETFOCUS && IsEnabled())
    {
        if(m_pWindow)
        {
            return;
        }
        m_pWindow = new CIPAddressWnd();
        ASSERT(m_pWindow);
        m_pWindow->Init(this);
        m_pWindow->ShowWindow();
    }
    else if(event.Type == UIEVENT_KILLFOCUS && IsEnabled())
    {
        Invalidate();
    }
    else if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        if(IsEnabled())
        {
            GetManager()->ReleaseCapture();
            if(IsFocused() && !m_pWindow)
            {
                m_pWindow = new CIPAddressWnd();
                ASSERT(m_pWindow);
            }
            if(m_pWindow)
            {
                m_pWindow->Init(this);
                m_pWindow->ShowWindow();
            }
        }
        return;
    }
    else if(event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_BUTTONUP || event.Type == UIEVENT_CONTEXTMENU || event.Type == UIEVENT_MOUSEENTER || event.Type == UIEVENT_MOUSELEAVE)
    {
        return;
    }

    CLabelUI::DoEvent(event);
}

void CIPAddressUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    CLabelUI::SetAttribute(pstrName, pstrValue);
}
}
