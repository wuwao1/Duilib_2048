﻿#include "StdAfx.h"

#ifdef _DEBUG
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

namespace DuiLib
{

/////////////////////////////////////////////////////////////////////////////////////
//
//

void UILIB_API DUI__Trace(LPCTSTR pstrFormat, ...)
{
#ifdef _DEBUG
    TCHAR szBuffer[2048] = { 0 };
    va_list args;
    va_start(args, pstrFormat);
    _vsntprintf(szBuffer, 2048, pstrFormat, args);
    va_end(args);

    lstrcat(szBuffer, _T("\n"));
    OutputDebugString(szBuffer);
#endif
}

LPCTSTR DUI__TraceMsg(UINT uMsg)
{
#define MSGDEF(x) if(uMsg==x) return _T(#x)
    MSGDEF(WM_SETCURSOR);
    MSGDEF(WM_NCHITTEST);
    MSGDEF(WM_NCPAINT);
    MSGDEF(WM_PAINT);
    MSGDEF(WM_ERASEBKGND);
    MSGDEF(WM_NCMOUSEMOVE);
    MSGDEF(WM_MOUSEMOVE);
    MSGDEF(WM_MOUSELEAVE);
    MSGDEF(WM_MOUSEHOVER);
    MSGDEF(WM_NOTIFY);
    MSGDEF(WM_COMMAND);
    MSGDEF(WM_MEASUREITEM);
    MSGDEF(WM_DRAWITEM);
    MSGDEF(WM_LBUTTONDOWN);
    MSGDEF(WM_LBUTTONUP);
    MSGDEF(WM_LBUTTONDBLCLK);
    MSGDEF(WM_RBUTTONDOWN);
    MSGDEF(WM_RBUTTONUP);
    MSGDEF(WM_RBUTTONDBLCLK);
    MSGDEF(WM_SETFOCUS);
    MSGDEF(WM_KILLFOCUS);
    MSGDEF(WM_MOVE);
    MSGDEF(WM_SIZE);
    MSGDEF(WM_SIZING);
    MSGDEF(WM_MOVING);
    MSGDEF(WM_GETMINMAXINFO);
    MSGDEF(WM_CAPTURECHANGED);
    MSGDEF(WM_WINDOWPOSCHANGED);
    MSGDEF(WM_WINDOWPOSCHANGING);
    MSGDEF(WM_NCCALCSIZE);
    MSGDEF(WM_NCCREATE);
    MSGDEF(WM_NCDESTROY);
    MSGDEF(WM_TIMER);
    MSGDEF(WM_KEYDOWN);
    MSGDEF(WM_KEYUP);
    MSGDEF(WM_CHAR);
    MSGDEF(WM_SYSKEYDOWN);
    MSGDEF(WM_SYSKEYUP);
    MSGDEF(WM_SYSCOMMAND);
    MSGDEF(WM_SYSCHAR);
    MSGDEF(WM_VSCROLL);
    MSGDEF(WM_HSCROLL);
    MSGDEF(WM_CHAR);
    MSGDEF(WM_SHOWWINDOW);
    MSGDEF(WM_PARENTNOTIFY);
    MSGDEF(WM_CREATE);
    MSGDEF(WM_NCACTIVATE);
    MSGDEF(WM_ACTIVATE);
    MSGDEF(WM_ACTIVATEAPP);
    MSGDEF(WM_CLOSE);
    MSGDEF(WM_DESTROY);
    MSGDEF(WM_GETICON);
    MSGDEF(WM_GETTEXT);
    MSGDEF(WM_GETTEXTLENGTH);
    static TCHAR szMsg[10];
    ::wsprintf(szMsg, _T("0x%04X"), uMsg);
    return szMsg;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

//////////////////////////////////////////////////////////////////////////
//
DUI_BASE_BEGIN_MESSAGE_MAP(CNotifyPump)
DUI_END_MESSAGE_MAP()

static const DUI_MSGMAP_ENTRY* DuiFindMessageEntry(const DUI_MSGMAP_ENTRY* lpEntry, TNotifyUI& msg)
{
    faw::string_t sMsgType = msg.sType;
    faw::string_t sCtrlName = msg.pSender->GetName();
    const DUI_MSGMAP_ENTRY* pMsgTypeEntry = nullptr;
    while(lpEntry->nSig != DuiSig_end)
    {
        if(lpEntry->sMsgType == sMsgType)
        {
            if(!lpEntry->sCtrlName.empty())
            {
                if(lpEntry->sCtrlName == sCtrlName)
                {
                    return lpEntry;
                }
            }
            else
            {
                pMsgTypeEntry = lpEntry;
            }
        }
        lpEntry++;
    }
    return pMsgTypeEntry;
}

bool CNotifyPump::AddVirtualWnd(faw::string_t strName, CNotifyPump* pObject)
{
    if(!m_VirtualWndMap.Find(strName))
    {
        m_VirtualWndMap.Insert(strName, (LPVOID) pObject);
        return true;
    }
    return false;
}

bool CNotifyPump::RemoveVirtualWnd(faw::string_t strName)
{
    if(m_VirtualWndMap.Find(strName))
    {
        m_VirtualWndMap.Remove(strName);
        return true;
    }
    return false;
}

bool CNotifyPump::LoopDispatch(TNotifyUI& msg)
{
    const DUI_MSGMAP_ENTRY* lpEntry = nullptr;
    const DUI_MSGMAP* pMessageMap = nullptr;

#ifndef UILIB_STATIC
    for(pMessageMap = GetMessageMap(); pMessageMap; pMessageMap = (*pMessageMap->pfnGetBaseMap)())
#else
    for(pMessageMap = GetMessageMap(); pMessageMap; pMessageMap = pMessageMap->pBaseMap)
#endif
    {
#ifndef UILIB_STATIC
        ASSERT(pMessageMap != (*pMessageMap->pfnGetBaseMap)());
#else
        ASSERT(pMessageMap != pMessageMap->pBaseMap);
#endif
        if((lpEntry = DuiFindMessageEntry(pMessageMap->lpEntries, msg)))
        {
            goto LDispatch;
        }
    }
    return false;

LDispatch:
    union DuiMessageMapFunctions mmf;
    mmf.pfn = lpEntry->pfn;

    bool bRet = false;
    int nSig;
    nSig = lpEntry->nSig;
    switch(nSig)
    {
    default:
        ASSERT(FALSE);
        break;
    case DuiSig_lwl:
        (this->*mmf.pfn_Notify_lwl)(msg.wParam, msg.lParam);
        bRet = true;
        break;
    case DuiSig_vn:
        (this->*mmf.pfn_Notify_vn)(msg);
        bRet = true;
        break;
    }
    return bRet;
}

void CNotifyPump::NotifyPump(TNotifyUI& msg)
{
    ///遍历虚拟窗口
    if(!msg.sVirtualWnd.empty())
    {
        for(int i = 0; i < m_VirtualWndMap.GetSize(); i++)
        {
            faw::string_t key = m_VirtualWndMap.GetAt(i)->Key;
            if(!key.empty() && key == msg.sVirtualWnd)
            {
                CNotifyPump* pObject = static_cast<CNotifyPump*>(m_VirtualWndMap.Find(key, false));
                if(pObject && pObject->LoopDispatch(msg))
                    return;
            }
        }
    }

    ///
    //遍历主窗口
    LoopDispatch(msg);
}

//////////////////////////////////////////////////////////////////////////
///
CWindowWnd::CWindowWnd() :
    m_hWnd(nullptr), m_OldWndProc(::DefWindowProc), m_bSubclassed(false), m_hIcon(NULL) {}

HWND CWindowWnd::CreateDuiWindow(HWND hwndParent, faw::string_t pstrWindowName, DWORD dwStyle /*=0*/, DWORD dwExStyle /*=0*/)
{
    return Create(hwndParent, pstrWindowName, dwStyle, dwExStyle, 0, 0, 0, 0, nullptr);
}

HWND CWindowWnd::Create(HWND hwndParent, faw::string_t pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu)
{
    return Create(hwndParent, pstrName, dwStyle, dwExStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
}

HWND CWindowWnd::Create(HWND hwndParent, faw::string_t pstrName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HMENU hMenu)
{
    if((!!GetSuperClassName()) && !RegisterSuperclass()) return nullptr;
    if((!GetSuperClassName()) && !RegisterWindowClass()) return nullptr;
    //产生 WM_CREATE 消息
    m_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName.data(), dwStyle, x, y, cx, cy, hwndParent, hMenu, CPaintManagerUI::GetInstance(), this);
    ASSERT(m_hWnd);
    return m_hWnd;
}

HWND CWindowWnd::Subclass(HWND hWnd)
{
    ASSERT(::IsWindow(hWnd));
    ASSERT(!m_hWnd);
    m_OldWndProc = SubclassWindow(hWnd, __WndProc);
    if(!m_OldWndProc) return nullptr;
    m_bSubclassed = true;
    m_hWnd = hWnd;
    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
    return m_hWnd;
}

void CWindowWnd::Unsubclass()
{
    ASSERT(::IsWindow(m_hWnd));
    if(!::IsWindow(m_hWnd)) return;
    if(!m_bSubclassed) return;
    SubclassWindow(m_hWnd, m_OldWndProc);
    m_OldWndProc = ::DefWindowProc;
    m_bSubclassed = false;
}

void CWindowWnd::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
    ASSERT(::IsWindow(m_hWnd));
    if(!::IsWindow(m_hWnd)) return;
    ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

UINT CWindowWnd::ShowModal()
{
    ASSERT(::IsWindow(m_hWnd));
    UINT nRet = 0;
    HWND hWndParent = GetWindowOwner(m_hWnd);
    ::ShowWindow(m_hWnd, SW_SHOWNORMAL);
    ::EnableWindow(hWndParent, FALSE);
    MSG msg = { 0 };
    while(::IsWindow(m_hWnd) && ::GetMessage(&msg, nullptr, 0, 0))
    {
        if(msg.message == WM_CLOSE && msg.hwnd == m_hWnd)
        {
            nRet = (UINT) msg.wParam;
            ::EnableWindow(hWndParent, TRUE);
            ::SetFocus(hWndParent);
        }
        if(!CPaintManagerUI::TranslateMessage(&msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        if(msg.message == WM_QUIT) break;
    }
    ::EnableWindow(hWndParent, TRUE);
    ::SetFocus(hWndParent);
    if(msg.message == WM_QUIT) ::PostQuitMessage((int) msg.wParam);
    return nRet;
}

void CWindowWnd::Close(UINT nRet)
{
    ASSERT(::IsWindow(m_hWnd));
    if(!::IsWindow(m_hWnd)) return;
    PostMessage(WM_CLOSE, (WPARAM) nRet, 0L);
}

void CWindowWnd::CenterWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((GetWindowStyle(m_hWnd)&WS_CHILD) == 0);
    RECT rcDlg = { 0 };
    ::GetWindowRect(m_hWnd, &rcDlg);
    RECT rcArea = { 0 };
    RECT rcCenter = { 0 };
    HWND hWnd = GetHWND();
    HWND hWndParent = ::GetParent(m_hWnd);
    HWND hWndCenter = ::GetWindowOwner(m_hWnd);
    if(hWndCenter) hWnd = hWndCenter;

    // 处理多显示器模式下屏幕居中
    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
    rcArea = oMonitor.rcWork;

    if(!hWndCenter)rcCenter = rcArea;
    else
        ::GetWindowRect(hWndCenter, &rcCenter);

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // The dialog is outside the screen, move it inside
    if(xLeft < rcArea.left) xLeft = rcArea.left;
    else if(xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
    if(yTop < rcArea.top) yTop = rcArea.top;
    else if(yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
    ::SetWindowPos(m_hWnd, nullptr, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CWindowWnd::SetIcon(UINT nRes)
{
    HICON hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                                     (::GetSystemMetrics(SM_CXICON) + 15) & ~15, (::GetSystemMetrics(SM_CYICON) + 15) & ~15,	// 防止高DPI下图标模糊
                                     LR_DEFAULTCOLOR);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);

    hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                               (::GetSystemMetrics(SM_CXICON) + 15) & ~15, (::GetSystemMetrics(SM_CYICON) + 15) & ~15,	// 防止高DPI下图标模糊
                               LR_DEFAULTCOLOR);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);
}

void CWindowWnd::SetIcon(faw::string_t strIconPath)
{
    if(m_hIcon) ::DestroyIcon(m_hIcon);
    TimageArgs imageArgs;
    imageArgs.bitmap = strIconPath;
    m_hIcon = CRenderEngine::GdiplusLoadIcon(imageArgs);
    if(m_hIcon)
    {
        ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);
        ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
    }
}

void CWindowWnd::BringToTop()
{
    const long lThreadID1 = ::GetWindowThreadProcessId(::GetForegroundWindow(), 0);
    const long lThreadID2 = ::GetWindowThreadProcessId(m_hWnd, 0);
    if(lThreadID1 != lThreadID2)
    {
        ::AttachThreadInput(lThreadID1, lThreadID2, TRUE);
        ::AllowSetForegroundWindow(ASFW_ANY);
        ::SetForegroundWindow(m_hWnd);
        ::AttachThreadInput(lThreadID1, lThreadID2, false);
    }
    else
    {
        ::SetForegroundWindow(m_hWnd);
    }

    const DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
    if(dwStyle & WS_MINIMIZE)
        ::ShowWindow(m_hWnd, SW_RESTORE);
    else
        ::ShowWindow(m_hWnd, SW_SHOW);
}

bool CWindowWnd::RegisterWindowClass()
{
    WNDCLASS wc = { 0 };
    wc.style = GetClassStyle();
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = nullptr;
    wc.lpfnWndProc = CWindowWnd::__WndProc;
    wc.hInstance = CPaintManagerUI::GetInstance();
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = GetWindowClassName();
    const ATOM ret = ::RegisterClass(&wc);
    ASSERT(ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CWindowWnd::RegisterSuperclass()
{
    // Get the class information from an existing
    // window so we can subclass it later on...
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    if(!::GetClassInfoEx(nullptr, GetSuperClassName(), &wc))
    {
        if(!::GetClassInfoEx(CPaintManagerUI::GetInstance(), GetSuperClassName(), &wc))
        {
            ASSERT(!"Unable to locate window class");
            return false;
        }
    }
    m_OldWndProc = wc.lpfnWndProc;
    wc.lpfnWndProc = CWindowWnd::__ControlProc;
    wc.hInstance = CPaintManagerUI::GetInstance();
    wc.lpszClassName = GetWindowClassName();
    const ATOM ret = ::RegisterClassEx(&wc);
    ASSERT(ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK CWindowWnd::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindowWnd* pThis = nullptr;
    if(uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
        pThis->m_hWnd = hWnd;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<CWindowWnd*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if(uMsg == WM_NCDESTROY && pThis)
        {
            const LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
            if(pThis->m_bSubclassed) pThis->Unsubclass();
            pThis->m_hWnd = nullptr;
            pThis->OnFinalMessage(hWnd);
            return lRes;
        }
    }
    if(pThis)
    {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK CWindowWnd::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindowWnd* pThis = nullptr;
    if(uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
        ::SetProp(hWnd, _T("WndX"), (HANDLE) pThis);
        pThis->m_hWnd = hWnd;
    }
    else
    {
        pThis = reinterpret_cast<CWindowWnd*>(::GetProp(hWnd, _T("WndX")));
        if(uMsg == WM_NCDESTROY && pThis)
        {
            LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
            if(pThis->m_bSubclassed) pThis->Unsubclass();
            ::SetProp(hWnd, _T("WndX"), nullptr);
            pThis->m_hWnd = nullptr;
            pThis->OnFinalMessage(hWnd);
            return lRes;
        }
    }
    if(pThis)
    {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT CWindowWnd::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT CWindowWnd::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void CWindowWnd::ResizeClient(int cx /*= -1*/, int cy /*= -1*/)
{
    ASSERT(::IsWindow(m_hWnd));
    RECT rc = { 0 };
    if(!::GetClientRect(m_hWnd, &rc)) return;
    if(cx != -1) rc.right = cx;
    if(cy != -1) rc.bottom = cy;
    if(!::AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), (!(GetWindowStyle(m_hWnd) & WS_CHILD) && (::GetMenu(m_hWnd))), GetWindowExStyle(m_hWnd))) return;
    ::SetWindowPos(m_hWnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

LRESULT CWindowWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}

void CWindowWnd::OnFinalMessage(HWND /*hWnd*/) {}

} // namespace DuiLib
