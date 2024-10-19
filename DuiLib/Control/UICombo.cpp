﻿#include "StdAfx.h"

namespace DuiLib
{

/////////////////////////////////////////////////////////////////////////////////////
//
//

class CComboWnd: public CWindowWnd, public INotifyUI
{
public:
    void Init(CComboUI* pOwner);
    LPCTSTR GetWindowClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void Notify(TNotifyUI& msg) override;

    void EnsureVisible(int iIndex);
    void Scroll(int dx, int dy);

#if(_WIN32_WINNT >= 0x0501)
    virtual UINT GetClassStyle() const;
#endif
    bool IsHitItem(POINT ptMouse);
public:
    CPaintManagerUI m_pm;
    CComboUI* m_pOwner;
    std::shared_ptr<CVerticalLayoutUI> m_pLayout;
    int m_nOldSel;
    bool m_bHitItem;
};

void CComboWnd::Notify(TNotifyUI& msg)
{
    if(msg.sType == _T("windowinit"))
    {
        EnsureVisible(m_nOldSel);
    }
    else if(msg.sType == _T("click"))
    {
        // 测试代码
        faw::string_t sName = msg.pSender->GetName();
        std::shared_ptr<CControlUI> pCtrl = msg.pSender;
        while(pCtrl)
        {
            std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI> (pCtrl->GetInterface(DUI_CTRL_LISTITEM));
            if(pListItem)
            {
                break;
            }
            pCtrl = pCtrl->GetParent();
        }
    }
}

void CComboWnd::Init(CComboUI* pOwner)
{
    m_bHitItem = false;
    m_pOwner = pOwner;
    m_pLayout = nullptr;
    m_nOldSel = m_pOwner->GetCurSel();

    // Position the popup window in absolute space
    SIZE szDrop = m_pOwner->GetDropBoxSize();
    RECT rcInset = m_pOwner->GetDropBoxInset();
    RECT rcOwner = pOwner->GetPos();
    RECT rc = rcOwner;
    auto Scale = pOwner->GetManager()->GetDPIObj()->GetScale();
    if(Scale > 100)
        rc.right = rc.left + pOwner->GetWidth() * 100 / Scale;
    rc.top = rc.bottom;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
    if(szDrop.cx > 0) rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度

    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    int cyFixed = rcInset.top;
    for(int it = 0; it < pOwner->GetCount(); it++)
    {
        std::shared_ptr<CControlUI> pControl = static_cast<std::shared_ptr<CControlUI>>(pOwner->GetItemAt(it));
        if(!pControl->IsVisible()) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
    }
    cyFixed += 4;
    rc.bottom = rc.top + MIN(cyFixed, szDrop.cy);

    ::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);

    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
    RECT rcWork = oMonitor.rcWork;
    if(rc.bottom > rcWork.bottom)
    {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if(szDrop.cx > 0) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - MIN(cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        ::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
    }

    Create(pOwner->GetManager()->GetPaintWindow(), _T(""), WS_POPUP, WS_EX_TOOLWINDOW, rc);
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = m_hWnd;
    while(::GetParent(hWndParent) != NULL) hWndParent = ::GetParent(hWndParent);
    ::ShowWindow(m_hWnd, SW_SHOW);
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

LPCTSTR CComboWnd::GetWindowClassName() const
{
    return _T("ComboWnd");
}

void CComboWnd::OnFinalMessage(HWND hWnd)
{
    m_pOwner->m_pWindow = nullptr;
    m_pOwner->m_uButtonState &= ~UISTATE_PUSHED;
    m_pOwner->Invalidate();
    delete this;
}

bool CComboWnd::IsHitItem(POINT ptMouse)
{
    std::shared_ptr<CControlUI> pControl = m_pm.FindControl(ptMouse);
    if(pControl)
    {
        std::shared_ptr<CControlUI> pInterface = pControl->GetInterface(DUI_CTRL_SCROLLBAR);
        if(pInterface) return false;

        while(pControl)
        {
            std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(DUI_CTRL_LISTITEM));
            if(pListItem)
            {
                return true;
            }
            pControl = pControl->GetParent();
        }
    }

    return false;
}

LRESULT CComboWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg == WM_CREATE)
    {
        m_pm.SetForceUseSharedRes(true);
        m_pm.Init(m_hWnd);
        m_pm.SetLayered(true);
        // The trick is to add the items to the new container. Their owner gets
        // reassigned by this operation - which is why it is important to reassign
        // the items back to the righfull owner/manager when the window closes.
        m_pLayout = std::make_shared<CVerticalLayoutUI>();
        m_pLayout->SetManager(&m_pm, nullptr, true);
        faw::string_t pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(_T("VerticalLayout"));
        if(!pDefaultAttributes.empty())
        {
            m_pLayout->ApplyAttributeList(pDefaultAttributes);
        }
        m_pLayout->SetInset({ 1, 1, 1, 1 });
        m_pLayout->SetBkColor(0xFFFFFFFF);
        m_pLayout->SetBorderColor(0xFFC6C7D2);
        m_pLayout->SetBorderSize(1);
        m_pLayout->SetAutoDestroy(false);
        m_pLayout->EnableScrollBar();
        m_pLayout->ApplyAttributeList(m_pOwner->GetDropBoxAttributeList());
        for(int i = 0; i < m_pOwner->GetCount(); i++)
        {
            m_pLayout->Add(static_cast<std::shared_ptr<CControlUI>>(m_pOwner->GetItemAt(i)));
        }
        CShadowUI *pShadow = m_pOwner->GetManager()->GetShadow();
        pShadow->CopyShadow(m_pm.GetShadow());
        m_pm.GetShadow()->ShowShadow(m_pOwner->IsShowShadow());
        m_pm.AttachDialog(m_pLayout);
        m_pm.AddNotifier(this);
        //默认字体为shared,在主窗体已经DPI调整,此处需要还原
        //m_pm.SetDPI (CDPI::GetMainMonitorDPI ());
        auto scale = m_pm.GetDPIObj()->GetScale();
        if(scale > 100)
        {
            auto fontinfo = m_pm.GetDefaultFontInfo();
            m_pm.SetDefaultFont(fontinfo->sFontName, (int)(fontinfo->iSize * 100.0 / scale), fontinfo->bBold, fontinfo->bUnderline, fontinfo->bItalic, false);
        }
        return 0;
    }
    else if(uMsg == WM_CLOSE)
    {
        m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
        RECT rcnullptr = { 0 };
        for(int i = 0; i < m_pOwner->GetCount(); i++) static_cast<std::shared_ptr<CControlUI>>(m_pOwner->GetItemAt(i))->SetPos(rcnullptr);
        m_pOwner->SetFocus();
    }
    else if(uMsg == WM_LBUTTONDOWN)
    {
        POINT pt = { 0 };
        ::GetCursorPos(&pt);
        ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
        m_bHitItem = IsHitItem(pt);
    }
    else if(uMsg == WM_LBUTTONUP)
    {
        POINT pt = { 0 };
        ::GetCursorPos(&pt);
        ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
        if(m_bHitItem && IsHitItem(pt))
        {
            PostMessage(WM_KILLFOCUS);
        }
        m_bHitItem = false;
    }
    else if(uMsg == WM_KEYDOWN)
    {
        switch(wParam)
        {
        case VK_ESCAPE:
            m_pOwner->SelectItem(m_nOldSel, true);
            EnsureVisible(m_nOldSel);
        case VK_RETURN:
            PostMessage(WM_KILLFOCUS);
            break;
        default:
            TEventUI event;
            event.Type = UIEVENT_KEYDOWN;
            event.chKey = (TCHAR) wParam;
            m_pOwner->DoEvent(event);
            EnsureVisible(m_pOwner->GetCurSel());
            return 0;
        }
    }
    else if(uMsg == WM_MOUSEWHEEL)
    {
        int zDelta = (int)(short) HIWORD(wParam);
        TEventUI event = { 0 };
        event.Type = UIEVENT_SCROLLWHEEL;
        event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
        event.lParam = lParam;
        event.dwTimestamp = ::GetTickCount();
        if(m_pOwner->GetScrollSelect())
        {
            m_pOwner->DoEvent(event);
            EnsureVisible(m_pOwner->GetCurSel());
            return 0;
        }
        else
        {
            m_pLayout->DoEvent(event);
            return 0;
        }
    }
    else if(uMsg == WM_KILLFOCUS)
    {
        if(m_hWnd != (HWND) wParam) PostMessage(WM_CLOSE);
    }

    std::optional<LRESULT> lRes = m_pm.MessageHandler(uMsg, wParam, lParam);
    if(lRes.has_value())
        return lRes.value();
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

void CComboWnd::EnsureVisible(int iIndex)
{
    if(m_pOwner->GetCurSel() < 0) return;
    m_pLayout->FindSelectable(m_pOwner->GetCurSel(), false);
    RECT rcItem = m_pLayout->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pLayout->GetPos();
    std::shared_ptr<CScrollBarUI> pHorizontalScrollBar = m_pLayout->GetHorizontalScrollBar();
    if(pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
    int iPos = m_pLayout->GetScrollPos().cy;
    if(rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom) return;
    int dx = 0;
    if(rcItem.top < rcList.top) dx = rcItem.top - rcList.top;
    if(rcItem.bottom > rcList.bottom) dx = rcItem.bottom - rcList.bottom;
    Scroll(0, dx);
}

void CComboWnd::Scroll(int dx, int dy)
{
    if(dx == 0 && dy == 0) return;
    SIZE sz = m_pLayout->GetScrollPos();
    m_pLayout->SetScrollPos({ sz.cx + dx, sz.cy + dy });
}

#if(_WIN32_WINNT >= 0x0501)
UINT CComboWnd::GetClassStyle() const
{
    return __super::GetClassStyle();
    if(m_pOwner->IsShowShadow())
    {
        return __super::GetClassStyle();

    }
    else
    {
        return __super::GetClassStyle() | CS_DROPSHADOW;
    }
}
#endif
////////////////////////////////////////////////////////
IMPLEMENT_DUICONTROL(CComboUI)

CComboUI::CComboUI() :
    m_uTextStyle(DT_VCENTER | DT_SINGLELINE)
{
    m_ListInfo.nColumns = 0;
    m_ListInfo.nFont = -1;
    m_ListInfo.uTextStyle = DT_VCENTER;
    m_ListInfo.dwTextColor = 0xFF000000;
    m_ListInfo.dwBkColor = 0;
    m_ListInfo.bAlternateBk = false;
    m_ListInfo.dwSelectedTextColor = 0xFF000000;
    m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
    m_ListInfo.dwHotTextColor = 0xFF000000;
    m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
    m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
    m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
    m_ListInfo.dwLineColor = 0;
    m_ListInfo.bShowHtml = false;
    m_ListInfo.bMultiExpandable = false;
    ::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
    ::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));
}

faw::string_t CComboUI::GetClass() const
{
    return _T("ComboUI");
}

std::shared_ptr<CControlUI> CComboUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_COMBO) return std::dynamic_pointer_cast<CComboUI>(shared_from_this());
    if(pstrName == _T("IListOwner")) return std::dynamic_pointer_cast<CComboUI>(shared_from_this());
    return CContainerUI::GetInterface(pstrName);
}

UINT CComboUI::GetControlFlags() const
{
    return UIFLAG_TABSTOP | UIFLAG_SETCURSOR;
}

void CComboUI::DoInit() {}

UINT CComboUI::GetListType()
{
    return LT_COMBO;
}

TListInfoUI* CComboUI::GetListInfo()
{
    return &m_ListInfo;
}

int CComboUI::GetCurSel() const
{
    return m_iCurSel;
}

bool CComboUI::SelectItem(int iIndex, bool bTakeFocus)
{
    if(iIndex == m_iCurSel) return true;
    int m_nOldSel = m_iCurSel;
    if(m_iCurSel >= 0)
    {
        auto new_it = m_items.begin();
        std::advance(new_it, m_iCurSel);
        std::shared_ptr<CControlUI> pControl = *new_it;
        if(!pControl) return false;
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
        if(pListItem) pListItem->Select(false);
        m_iCurSel = -1;
    }
    if(iIndex < 0) return false;
    if(m_items.size() == 0) return false;
    if(iIndex >= (long)m_items.size()) iIndex = m_items.size() - 1;
    auto new_it = m_items.begin();
    std::advance(new_it, iIndex);
    std::shared_ptr<CControlUI> pControl = *new_it;
    if(!pControl || !pControl->IsEnabled()) return false;
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(!pListItem) return false;
    m_iCurSel = iIndex;
    if(m_pWindow || bTakeFocus) pControl->SetFocus();
    pListItem->Select(true);
    if(m_pManager) m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMSELECT, m_iCurSel, m_nOldSel);
    Invalidate();

    return true;
}

bool CComboUI::SelectMultiItem(int iIndex, bool bTakeFocus)
{
    return SelectItem(iIndex, bTakeFocus);
}

bool CComboUI::UnSelectItem(int iIndex, bool bOthers)
{
    return false;
}

bool CComboUI::SetItemIndex(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    int iOrginIndex = GetItemIndex(pControl);
    if(iOrginIndex == -1) return false;
    if(iOrginIndex == iIndex) return true;

    std::shared_ptr<IListItemUI> pSelectedListItem = nullptr;
    if(m_iCurSel >= 0) pSelectedListItem =
            std::dynamic_pointer_cast<IListItemUI>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
    if(!CContainerUI::SetItemIndex(pControl, iIndex)) return false;
    int iMinIndex = min(iOrginIndex, iIndex);
    int iMaxIndex = max(iOrginIndex, iIndex);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i)
    {
        std::shared_ptr<CControlUI> p = GetItemAt(i);
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(i);
        }
    }
    if(m_iCurSel >= 0 && pSelectedListItem) m_iCurSel = pSelectedListItem->GetIndex();
    return true;
}

bool CComboUI::Add(std::shared_ptr<CControlUI> pControl)
{
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(pListItem)
    {
        pListItem->SetOwner(shared_from_this());
        pListItem->SetIndex(m_items.size());
    }
    return CContainerUI::Add(pControl);
}

bool CComboUI::AddAt(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    if(!CContainerUI::AddAt(pControl, iIndex)) return false;

    // The list items should know about us
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(pListItem)
    {
        pListItem->SetOwner(shared_from_this());
        pListItem->SetIndex(iIndex);
    }

    for(int i = iIndex + 1; i < GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> p = GetItemAt(i);
        pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(i);
        }
    }
    if(m_iCurSel >= iIndex) m_iCurSel += 1;
    return true;
}

bool CComboUI::Remove(std::shared_ptr<CControlUI> pControl)
{
    int iIndex = GetItemIndex(pControl);
    if(iIndex == -1) return false;

    if(!CContainerUI::RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> p = GetItemAt(i);
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(i);
        }
    }

    if(iIndex == m_iCurSel && m_iCurSel >= 0)
    {
        int iSel = m_iCurSel;
        m_iCurSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if(iIndex < m_iCurSel) m_iCurSel -= 1;
    return true;
}

bool CComboUI::RemoveAt(int iIndex)
{
    if(!CContainerUI::RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> p = GetItemAt(i);
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem) pListItem->SetIndex(i);
    }

    if(iIndex == m_iCurSel && m_iCurSel >= 0)
    {
        int iSel = m_iCurSel;
        m_iCurSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if(iIndex < m_iCurSel) m_iCurSel -= 1;
    return true;
}

void CComboUI::RemoveAll()
{
    m_iCurSel = -1;
    CContainerUI::RemoveAll();
}

void CComboUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CContainerUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_SETFOCUS)
    {
        Invalidate();
    }
    if(event.Type == UIEVENT_KILLFOCUS)
    {
        Invalidate();
    }
    if(event.Type == UIEVENT_BUTTONDOWN)
    {
        if(IsEnabled())
        {
            Activate();
            m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
        }
        return;
    }
    if(event.Type == UIEVENT_BUTTONUP)
    {
        if((m_uButtonState & UISTATE_CAPTURED) != 0)
        {
            m_uButtonState &= ~UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_MOUSEMOVE)
    {
        return;
    }
    if(event.Type == UIEVENT_KEYDOWN)
    {
        switch(event.chKey)
        {
        case VK_F4:
            Activate();
            return;
        case VK_UP:
            SelectItem(FindSelectable(m_iCurSel - 1, false));
            return;
        case VK_DOWN:
            SelectItem(FindSelectable(m_iCurSel + 1, true));
            return;
        case VK_PRIOR:
            SelectItem(FindSelectable(m_iCurSel - 1, false));
            return;
        case VK_NEXT:
            SelectItem(FindSelectable(m_iCurSel + 1, true));
            return;
        case VK_HOME:
            SelectItem(FindSelectable(0, false));
            return;
        case VK_END:
            SelectItem(FindSelectable(GetCount() - 1, true));
            return;
        }
    }
    if(event.Type == UIEVENT_SCROLLWHEEL)
    {
        if(GetScrollSelect())
        {
            bool bDownward = LOWORD(event.wParam) == SB_LINEDOWN;
            SelectItem(FindSelectable(m_iCurSel + (bDownward ? 1 : -1), bDownward));
        }
        return;
    }
    if(event.Type == UIEVENT_CONTEXTMENU)
    {
        return;
    }
    if(event.Type == UIEVENT_MOUSEENTER)
    {
        if(::PtInRect(&m_rcItem, event.ptMouse))
        {
            if((m_uButtonState & UISTATE_HOT) == 0)
                m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_MOUSELEAVE)
    {
        if((m_uButtonState & UISTATE_HOT) != 0)
        {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CControlUI::DoEvent(event);
}

SIZE CComboUI::EstimateSize(SIZE szAvailable)
{
    if(m_cxyFixed.cy == 0) return { m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 12 };
    return CControlUI::EstimateSize(szAvailable);
}

bool CComboUI::Activate()
{
    if(!CControlUI::Activate()) return false;
    if(m_pManager) m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_PREDROPDOWN);
    if(m_pWindow) return true;
    m_pWindow = new CComboWnd();
    ASSERT(m_pWindow);
    m_pWindow->Init(this);
    if(m_pManager) m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_DROPDOWN);
    Invalidate();
    return true;
}

faw::string_t CComboUI::GetText() const
{
    if(m_iCurSel < 0) return __super::GetText();
    auto new_it = m_items.begin();
    std::advance(new_it, m_iCurSel);
    std::shared_ptr<CControlUI> pControl = *new_it;
    return pControl->GetText();
}

void CComboUI::SetText(faw::string_t pstrText)
{
    int m_nOldSel = m_iCurSel;
    m_iCurSel = -2;
    int Index = -1;
    for(auto& pItem : m_items)
    {
        Index++;
        std::shared_ptr<CControlUI> ctrl = pItem;
        if(!ctrl)
            continue;
        if(ctrl->GetText() == pstrText && ctrl->IsEnabled() && m_iCurSel == -2)
            m_iCurSel = Index;
        std::shared_ptr<IListItemUI> item = std::dynamic_pointer_cast<IListItemUI>(ctrl->GetInterface(_T("ListItem")));
        if(item)
            item->Select(m_iCurSel == Index);
    }
    CContainerUI::SetText(pstrText);
    if(m_pManager && m_iCurSel >= 0)
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMSELECT, m_iCurSel, m_nOldSel);
}

void CComboUI::SetEnabled(bool bEnable)
{
    CContainerUI::SetEnabled(bEnable);
    if(!IsEnabled()) m_uButtonState = 0;
}

faw::string_t CComboUI::GetDropBoxAttributeList()
{
    return m_sDropBoxAttributes;
}

void CComboUI::SetDropBoxAttributeList(faw::string_t pstrList)
{
    m_sDropBoxAttributes = pstrList;
}

SIZE CComboUI::GetDropBoxSize() const
{
    return m_szDropBox;
}

void CComboUI::SetDropBoxSize(SIZE szDropBox)
{
    m_szDropBox = szDropBox;
}

RECT CComboUI::GetDropBoxInset() const
{
    return m_rcDropBox;
}

void CComboUI::SetDropBoxInset(RECT rcDropBox)
{
    m_rcDropBox = rcDropBox;
}

void CComboUI::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    Invalidate();
}

UINT CComboUI::GetTextStyle() const
{
    return m_uTextStyle;
}

void CComboUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
    Invalidate();
}

DWORD CComboUI::GetTextColor() const
{
    return m_dwTextColor;
}

void CComboUI::SetDisabledTextColor(DWORD dwTextColor)
{
    m_dwDisabledTextColor = dwTextColor;
    Invalidate();
}

DWORD CComboUI::GetDisabledTextColor() const
{
    return m_dwDisabledTextColor;
}

void CComboUI::SetFont(int index)
{
    m_iFont = index;
    Invalidate();
}

int CComboUI::GetFont() const
{
    return m_iFont;
}

RECT CComboUI::GetTextPadding() const
{
    return m_rcTextPadding;
}

void CComboUI::SetTextPadding(RECT rc)
{
    m_rcTextPadding = rc;
    Invalidate();
}

bool CComboUI::IsShowHtml()
{
    return m_bShowHtml;
}

void CComboUI::SetShowHtml(bool bShowHtml)
{
    if(m_bShowHtml == bShowHtml) return;

    m_bShowHtml = bShowHtml;
    Invalidate();
}

bool CComboUI::IsShowShadow()
{
    return m_bShowShadow;
}

void CComboUI::SetShowShadow(bool bShow)
{
    if(m_bShowShadow == bShow) return;

    m_bShowShadow = bShow;
    Invalidate();
}

faw::string_t CComboUI::GetNormalImage() const
{
    return m_sNormalImage;
}

void CComboUI::SetNormalImage(faw::string_t pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

faw::string_t CComboUI::GetHotImage() const
{
    return m_sHotImage;
}

void CComboUI::SetHotImage(faw::string_t pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

faw::string_t CComboUI::GetPushedImage() const
{
    return m_sPushedImage;
}

void CComboUI::SetPushedImage(faw::string_t pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

faw::string_t CComboUI::GetFocusedImage() const
{
    return m_sFocusedImage;
}

void CComboUI::SetFocusedImage(faw::string_t pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

faw::string_t CComboUI::GetDisabledImage() const
{
    return m_sDisabledImage;
}

void CComboUI::SetDisabledImage(faw::string_t pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

bool CComboUI::GetScrollSelect()
{
    return m_bScrollSelect;
}

void CComboUI::SetScrollSelect(bool bScrollSelect)
{
    m_bScrollSelect = bScrollSelect;
}

void CComboUI::SetItemFont(int index)
{
    m_ListInfo.nFont = index;
    Invalidate();
}

void CComboUI::SetItemTextStyle(UINT uStyle)
{
    m_ListInfo.uTextStyle = uStyle;
    Invalidate();
}

RECT CComboUI::GetItemTextPadding() const
{
    return m_ListInfo.rcTextPadding;
}

void CComboUI::SetItemTextPadding(RECT rc)
{
    m_ListInfo.rcTextPadding = rc;
    Invalidate();
}

void CComboUI::SetItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwTextColor = dwTextColor;
    Invalidate();
}

void CComboUI::SetItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwBkColor = dwBkColor;
}

void CComboUI::SetItemBkImage(faw::string_t pStrImage)
{
    m_ListInfo.sBkImage = pStrImage;
}

DWORD CComboUI::GetItemTextColor() const
{
    return m_ListInfo.dwTextColor;
}

DWORD CComboUI::GetItemBkColor() const
{
    return m_ListInfo.dwBkColor;
}

faw::string_t CComboUI::GetItemBkImage() const
{
    return m_ListInfo.sBkImage;
}

bool CComboUI::IsAlternateBk() const
{
    return m_ListInfo.bAlternateBk;
}

void CComboUI::SetAlternateBk(bool bAlternateBk)
{
    m_ListInfo.bAlternateBk = bAlternateBk;
}

void CComboUI::SetSelectedItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwSelectedTextColor = dwTextColor;
}

void CComboUI::SetSelectedItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwSelectedBkColor = dwBkColor;
}

void CComboUI::SetSelectedItemImage(faw::string_t pStrImage)
{
    m_ListInfo.sSelectedImage = pStrImage;
}

DWORD CComboUI::GetSelectedItemTextColor() const
{
    return m_ListInfo.dwSelectedTextColor;
}

DWORD CComboUI::GetSelectedItemBkColor() const
{
    return m_ListInfo.dwSelectedBkColor;
}

faw::string_t CComboUI::GetSelectedItemImage() const
{
    return m_ListInfo.sSelectedImage;
}

void CComboUI::SetHotItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwHotTextColor = dwTextColor;
}

void CComboUI::SetHotItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwHotBkColor = dwBkColor;
}

void CComboUI::SetHotItemImage(faw::string_t pStrImage)
{
    m_ListInfo.sHotImage = pStrImage;
}

DWORD CComboUI::GetHotItemTextColor() const
{
    return m_ListInfo.dwHotTextColor;
}
DWORD CComboUI::GetHotItemBkColor() const
{
    return m_ListInfo.dwHotBkColor;
}

faw::string_t CComboUI::GetHotItemImage() const
{
    return m_ListInfo.sHotImage;
}

void CComboUI::SetDisabledItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwDisabledTextColor = dwTextColor;
}

void CComboUI::SetDisabledItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwDisabledBkColor = dwBkColor;
}

void CComboUI::SetDisabledItemImage(faw::string_t pStrImage)
{
    m_ListInfo.sDisabledImage = pStrImage;
}

DWORD CComboUI::GetDisabledItemTextColor() const
{
    return m_ListInfo.dwDisabledTextColor;
}

DWORD CComboUI::GetDisabledItemBkColor() const
{
    return m_ListInfo.dwDisabledBkColor;
}

faw::string_t CComboUI::GetDisabledItemImage() const
{
    return m_ListInfo.sDisabledImage;
}

DWORD CComboUI::GetItemLineColor() const
{
    return m_ListInfo.dwLineColor;
}

void CComboUI::SetItemLineColor(DWORD dwLineColor)
{
    m_ListInfo.dwLineColor = dwLineColor;
}

bool CComboUI::IsItemShowHtml()
{
    return m_ListInfo.bShowHtml;
}

void CComboUI::SetItemShowHtml(bool bShowHtml)
{
    if(m_ListInfo.bShowHtml == bShowHtml) return;

    m_ListInfo.bShowHtml = bShowHtml;
    Invalidate();
}

void CComboUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    if(!::EqualRect(&rc, &m_rcItem))
    {
        // 隐藏下拉窗口
        if(m_pWindow && ::IsWindow(m_pWindow->GetHWND())) m_pWindow->Close();
        // 所有元素大小置为0
        RECT rcnullptr = { 0 };
        for(auto& pItem : m_items)
            pItem->SetPos(rcnullptr);
        // 调整位置
        CControlUI::SetPos(rc, bNeedInvalidate);
    }
}

void CComboUI::Move(SIZE szOffset, bool bNeedInvalidate)
{
    CControlUI::Move(szOffset, bNeedInvalidate);
}
void CComboUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("align"))
    {
        if(pstrValue.find(_T("left")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_SINGLELINE);
            m_uTextStyle |= DT_LEFT;
        }
        if(pstrValue.find(_T("center")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_uTextStyle |= DT_CENTER;
        }
        if(pstrValue.find(_T("right")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_SINGLELINE);
            m_uTextStyle |= DT_RIGHT;
        }
    }
    else if(pstrName == _T("valign"))
    {
        if(pstrValue.find(_T("top")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
            m_uTextStyle |= (DT_TOP | DT_SINGLELINE);
        }
        if(pstrValue.find(_T("vcenter")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_TOP | DT_BOTTOM);
            m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
        }
        if(pstrValue.find(_T("bottom")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
            m_uTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
        }
    }
    else if(pstrName == _T("endellipsis"))
    {
        if(FawTools::parse_bool(pstrValue)) m_uTextStyle |= DT_END_ELLIPSIS;
        else m_uTextStyle &= ~DT_END_ELLIPSIS;
    }
    else if(pstrName == _T("wordbreak"))
    {
        if(FawTools::parse_bool(pstrValue))
        {
            m_uTextStyle &= ~DT_SINGLELINE;
            m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
        }
        else
        {
            m_uTextStyle &= ~DT_WORDBREAK & ~DT_EDITCONTROL;
            m_uTextStyle |= DT_SINGLELINE;
        }
    }
    else if(pstrName == _T("font")) SetFont(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("textcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetTextColor(clrColor);
    }
    else if(pstrName == _T("disabledtextcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetDisabledTextColor(clrColor);
    }
    else if(pstrName == _T("textpadding"))
    {
        RECT rcTextPadding = FawTools::parse_rect(pstrValue);
        SetTextPadding(rcTextPadding);
    }
    else if(pstrName == _T("showhtml")) SetShowHtml(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("showshadow")) SetShowShadow(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("normalimage")) SetNormalImage(pstrValue);
    else if(pstrName == _T("hotimage")) SetHotImage(pstrValue);
    else if(pstrName == _T("pushedimage")) SetPushedImage(pstrValue);
    else if(pstrName == _T("focusedimage")) SetFocusedImage(pstrValue);
    else if(pstrName == _T("disabledimage")) SetDisabledImage(pstrValue);
    else if(pstrName == _T("scrollselect")) SetScrollSelect(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("dropbox")) SetDropBoxAttributeList(pstrValue);
    else if(pstrName == _T("dropboxsize"))
    {
        SIZE szDropBoxSize = FawTools::parse_size(pstrValue);
        SetDropBoxSize(szDropBoxSize);
    }
    else if(pstrName == _T("dropboxinset"))
    {
        RECT rcDropboxInset = FawTools::parse_rect(pstrValue);
        SetDropBoxInset(rcDropboxInset);
    }
    else if(pstrName == _T("itemfont")) SetItemFont(_ttoi(pstrValue.data()));
    else if(pstrName == _T("itemalign"))
    {
        if(pstrValue.find(_T("left")) != faw::string_t::npos)
        {
            m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_ListInfo.uTextStyle |= DT_LEFT;
        }
        if(pstrValue.find(_T("center")) != faw::string_t::npos)
        {
            m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_ListInfo.uTextStyle |= DT_CENTER;
        }
        if(pstrValue.find(_T("right")) != faw::string_t::npos)
        {
            m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_ListInfo.uTextStyle |= DT_RIGHT;
        }
    }
    else if(pstrName == _T("itemvalign"))
    {
        if(pstrValue.find(_T("top")) != faw::string_t::npos)
        {
            m_ListInfo.uTextStyle &= ~(DT_VCENTER | DT_BOTTOM);
            m_ListInfo.uTextStyle |= DT_TOP;
        }
        if(pstrValue.find(_T("vcenter")) != faw::string_t::npos)
        {
            m_ListInfo.uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);
            m_ListInfo.uTextStyle |= DT_VCENTER | DT_SINGLELINE;
        }
        if(pstrValue.find(_T("bottom")) != faw::string_t::npos)
        {
            m_ListInfo.uTextStyle &= ~(DT_TOP | DT_VCENTER);
            m_ListInfo.uTextStyle |= DT_BOTTOM;
        }
    }
    else if(pstrName == _T("itemendellipsis"))
    {
        if(FawTools::parse_bool(pstrValue)) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
        else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
    }
    else if(pstrName == _T("itemtextpadding"))
    {
        RECT rcTextPadding = FawTools::parse_rect(pstrValue);
        SetItemTextPadding(rcTextPadding);
    }
    else if(pstrName == _T("itemtextcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetItemTextColor(clrColor);
    }
    else if(pstrName == _T("itembkcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetItemBkColor(clrColor);
    }
    else if(pstrName == _T("itembkimage")) SetItemBkImage(pstrValue);
    else if(pstrName == _T("itemaltbk")) SetAlternateBk(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("itemselectedtextcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetSelectedItemTextColor(clrColor);
    }
    else if(pstrName == _T("itemselectedbkcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetSelectedItemBkColor(clrColor);
    }
    else if(pstrName == _T("itemselectedimage")) SetSelectedItemImage(pstrValue);
    else if(pstrName == _T("itemhottextcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetHotItemTextColor(clrColor);
    }
    else if(pstrName == _T("itemhotbkcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetHotItemBkColor(clrColor);
    }
    else if(pstrName == _T("itemhotimage")) SetHotItemImage(pstrValue);
    else if(pstrName == _T("itemdisabledtextcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetDisabledItemTextColor(clrColor);
    }
    else if(pstrName == _T("itemdisabledbkcolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetDisabledItemBkColor(clrColor);
    }
    else if(pstrName == _T("itemdisabledimage")) SetDisabledItemImage(pstrValue);
    else if(pstrName == _T("itemlinecolor"))
    {
        DWORD clrColor = (DWORD) FawTools::parse_hex(pstrValue);
        SetItemLineColor(clrColor);
    }
    else if(pstrName == _T("itemshowhtml")) SetItemShowHtml(FawTools::parse_bool(pstrValue));
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

bool CComboUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    return CControlUI::DoPaint(hDC, rcPaint, pStopControl);
}

void CComboUI::PaintStatusImage(HDC hDC)
{
    if(IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~UISTATE_FOCUSED;
    if(!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~UISTATE_DISABLED;

    if((m_uButtonState & UISTATE_DISABLED) != 0)
    {
        if(!m_sDisabledImage.empty())
        {
            if(DrawImage(hDC, m_sDisabledImage))
                return;
        }
    }
    else if((m_uButtonState & UISTATE_PUSHED) != 0)
    {
        if(!m_sPushedImage.empty())
        {
            if(DrawImage(hDC, m_sPushedImage))
                return;
        }
    }
    else if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(!m_sHotImage.empty())
        {
            if(DrawImage(hDC, m_sHotImage))
                return;
        }
    }
    else if((m_uButtonState & UISTATE_FOCUSED) != 0)
    {
        if(!m_sFocusedImage.empty())
        {
            if(DrawImage(hDC, m_sFocusedImage))
                return;
        }
    }

    if(!m_sNormalImage.empty())
    {
        if(!DrawImage(hDC, m_sNormalImage))
        {
        }
        else return;
    }
}

void CComboUI::PaintText(HDC hDC)
{
    if(m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if(m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

    faw::string_t sText = GetText();
    if(sText.empty()) return;
    int nLinks = 0;
    if(IsEnabled())
    {
        if(m_bShowHtml)
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, m_dwTextColor, nullptr, nullptr, nLinks, m_iFont, m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwTextColor, m_iFont, m_uTextStyle);
    }
    else
    {
        if(m_bShowHtml)
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, nullptr, nullptr, nLinks, m_iFont, m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, m_iFont, m_uTextStyle);
    }
}

} // namespace DuiLib
