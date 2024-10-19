#include "StdAfx.h"

namespace DuiLib
{

/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CContainerUI)

CContainerUI::CContainerUI() {}

CContainerUI::~CContainerUI()
{
    m_bDelayedDestroy = false;
    RemoveAll();
}

faw::string_t CContainerUI::GetClass() const
{
    return _T("ContainerUI");
}

std::shared_ptr<CControlUI>  CContainerUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == _T("IContainer")) return std::dynamic_pointer_cast<CContainerUI>(shared_from_this());
    else if(pstrName == DUI_CTRL_CONTAINER) return std::dynamic_pointer_cast<CContainerUI>(shared_from_this());
    return CControlUI::GetInterface(pstrName);
}

std::shared_ptr<CControlUI> CContainerUI::GetItemAt(int iIndex) const
{
    if(iIndex < 0 || iIndex >= (long)m_items.size()) return nullptr;
    auto new_it = m_items.begin();
    std::advance(new_it, iIndex);
    return *new_it;
}

int CContainerUI::GetItemIndex(std::shared_ptr<CControlUI> pControl) const
{
    auto it = std::find(m_items.begin(), m_items.end(), pControl);
    if(it != m_items.end())
    {
        return std::distance(m_items.begin(), it);
    }
    return -1; // 元素未找到
}

bool CContainerUI::SetItemIndex(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    // 移除pControl
    auto it = std::find(m_items.begin(), m_items.end(), pControl);
    if(it != m_items.end())
        m_items.erase(it);
    // 找到新位置的迭代器
    auto new_it = m_items.begin();
    if(iIndex>= (long)m_items.size())
        return false;
    std::advance(new_it, iIndex);
    // 在新位置插入元素
    m_items.insert(new_it, pControl);
    return true;
}

int CContainerUI::GetCount() const
{
    return m_items.size();
}

bool CContainerUI::Add(std::shared_ptr<CControlUI> pControl)
{
    if(!pControl) return false;

    if(m_pManager) m_pManager->InitControls(pControl, shared_from_this());
    if(IsVisible()) NeedUpdate();
    else pControl->SetInternVisible(false);
    m_items.push_back(pControl);
    return true;
}

bool CContainerUI::AddAt(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    if(!pControl) return false;

    if(m_pManager) m_pManager->InitControls(pControl, shared_from_this());
    if(IsVisible()) NeedUpdate();
    else pControl->SetInternVisible(false);
    auto new_it = m_items.begin();
    if(iIndex >= (long)m_items.size())
        return false;
    std::advance(new_it, iIndex);
    // 在新位置插入元素
    m_items.insert(new_it, pControl);
    return true;
}

bool CContainerUI::Remove(std::shared_ptr<CControlUI> pControl)
{
    if(!pControl) return false;
    auto it = std::find(m_items.begin(), m_items.end(), pControl);
    if(it != m_items.end())
    {
        NeedUpdate();
        if(m_bAutoDestroy)
        {
            if(m_bDelayedDestroy && m_pManager)
                m_pManager->AddDelayedCleanup(pControl);
        }
        m_items.erase(it);
    }
    return true;
}

bool CContainerUI::Remove(CControlUI* pControl)
{
    if (!pControl) return false;
    for (auto it = m_items.begin(); it != m_items.end(); )
    {
        if (it->get() == pControl)
        {
            NeedUpdate();
            if (m_bAutoDestroy)
            {
                if (m_bDelayedDestroy && m_pManager)
                    m_pManager->AddDelayedCleanup(*it);
            }
            it = m_items.erase(it);
        }
        it++;
    }
    return true;
}

bool CContainerUI::RemoveAt(int iIndex)
{
    auto new_it = m_items.begin();
    if(iIndex<0 || iIndex >= (long)m_items.size())
        return false;
    std::advance(new_it, iIndex);
    m_items.erase(new_it);
    return true;
}

void CContainerUI::RemoveAll()
{
    if(m_bAutoDestroy)
    {
        for(auto it = m_items.begin(); it != m_items.end();)
        {
            std::shared_ptr<CControlUI> pItem = *it;
            if(m_bDelayedDestroy && m_pManager)
            {
                m_pManager->AddDelayedCleanup(pItem);
                it = m_items.erase(it); // 删除并获取下一个迭代器
            }
            else
            {
                it = m_items.erase(it); // 删除并获取下一个迭代器
            }
        }
    }
    m_items.clear();
    NeedUpdate();
}

bool CContainerUI::IsAutoDestroy() const
{
    return m_bAutoDestroy;
}

void CContainerUI::SetAutoDestroy(bool bAuto)
{
    m_bAutoDestroy = bAuto;
}

bool CContainerUI::IsDelayedDestroy() const
{
    return m_bDelayedDestroy;
}

void CContainerUI::SetDelayedDestroy(bool bDelayed)
{
    m_bDelayedDestroy = bDelayed;
}

RECT CContainerUI::GetInset() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_rcInset);
    return m_rcInset;
}

void CContainerUI::SetInset(RECT rcInset)
{
    m_rcInset = rcInset;
    NeedUpdate();
}

int CContainerUI::GetChildPadding() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_iChildPadding);
    return m_iChildPadding;
}


void CContainerUI::SetChildPadding(int iPadding)
{
    m_iChildPadding = iPadding;
    NeedUpdate();
}

UINT CContainerUI::GetChildAlign(std::shared_ptr<CControlUI> pControl) const
{
    if(pControl)
    {
        UINT iChildFloatAlign = pControl->GetFloatAlign();
        return (iChildFloatAlign == DT_LEFT ? m_iChildAlign : iChildFloatAlign);
    }
    return m_iChildAlign;
}

void CContainerUI::SetChildAlign(UINT iAlign)
{
    m_iChildAlign = iAlign;
    NeedUpdate();
}

UINT CContainerUI::GetChildVAlign() const
{
    return m_iChildVAlign;
}

void CContainerUI::SetChildVAlign(UINT iVAlign)
{
    m_iChildVAlign = iVAlign;
    NeedUpdate();
}

bool CContainerUI::IsMouseChildEnabled() const
{
    return m_bMouseChildEnabled;
}

void CContainerUI::SetMouseChildEnabled(bool bEnable)
{
    m_bMouseChildEnabled = bEnable;
}

void CContainerUI::SetVisible(bool bVisible)
{
    if(m_bVisible == bVisible) return;
    CControlUI::SetVisible(bVisible);
    for(auto& pItem : m_items)
    {
        pItem->SetInternVisible(IsVisible());
    }
}

// 逻辑上，对于Container控件不公开此方法
// 调用此方法的结果是，内部子控件隐藏，控件本身依然显示，背景等效果存在
void CContainerUI::SetInternVisible(bool bVisible)
{
    if(m_bInternVisible == bVisible) return;
    CControlUI::SetInternVisible(bVisible);
    if(m_items.empty()) return;
    for(auto& pItem : m_items)
    {
        // 控制子控件显示状态
        // InternVisible状态应由子控件自己控制
        pItem->SetInternVisible(IsVisible());
    }
}

void CContainerUI::SetEnabled(bool bEnabled)
{
    if(m_bEnabled == bEnabled) return;
    m_bEnabled = bEnabled;
    for(auto& pItem : m_items)
    {
        pItem->SetEnabled(m_bEnabled);
    }
    Invalidate();
}

void CContainerUI::SetMouseEnabled(bool bEnabled)
{
    if(m_pVerticalScrollBar) m_pVerticalScrollBar->SetMouseEnabled(bEnabled);
    if(m_pHorizontalScrollBar) m_pHorizontalScrollBar->SetMouseEnabled(bEnabled);
    CControlUI::SetMouseEnabled(bEnabled);
}

void CContainerUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_SETFOCUS)
    {
        m_bFocused = true;
        return;
    }
    if(event.Type == UIEVENT_KILLFOCUS)
    {
        m_bFocused = false;
        return;
    }
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar->IsEnabled())
    {
        if(event.Type == UIEVENT_KEYDOWN)
        {
            switch(event.chKey)
            {
            case VK_DOWN:
                LineDown();
                return;
            case VK_UP:
                LineUp();
                return;
            case VK_NEXT:
                PageDown();
                return;
            case VK_PRIOR:
                PageUp();
                return;
            case VK_HOME:
                HomeUp();
                return;
            case VK_END:
                EndDown();
                return;
            }
        }
        else if(event.Type == UIEVENT_SCROLLWHEEL)
        {
            switch(LOWORD(event.wParam))
            {
            case SB_LINEUP:
                LineUp();
                return;
            case SB_LINEDOWN:
                LineDown();
                return;
            }
        }
    }
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled())
    {
        if(event.Type == UIEVENT_KEYDOWN)
        {
            switch(event.chKey)
            {
            case VK_DOWN:
                LineRight();
                return;
            case VK_UP:
                LineLeft();
                return;
            case VK_NEXT:
                PageRight();
                return;
            case VK_PRIOR:
                PageLeft();
                return;
            case VK_HOME:
                HomeLeft();
                return;
            case VK_END:
                EndRight();
                return;
            }
        }
        else if(event.Type == UIEVENT_SCROLLWHEEL)
        {
            switch(LOWORD(event.wParam))
            {
            case SB_LINEUP:
                LineLeft();
                return;
            case SB_LINEDOWN:
                LineRight();
                return;
            }
        }
    }
    if(event.Type == UIEVENT_TIMER)
    {
        if(event.wParam == SHOW_CONTANIER_ID)
            UnderAnimateShow();
        else if(event.wParam == HIDE_CONTANIER_ID)
            UnderAnimateHide();
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
        return;
    }
    CControlUI::DoEvent(event);
}

SIZE CContainerUI::GetScrollPos() const
{
    SIZE sz = { 0, 0 };
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) sz.cy = m_pVerticalScrollBar->GetScrollPos();
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) sz.cx = m_pHorizontalScrollBar->GetScrollPos();
    return sz;
}

SIZE CContainerUI::GetScrollRange() const
{
    SIZE sz = { 0, 0 };
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) sz.cy = m_pVerticalScrollBar->GetScrollRange();
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) sz.cx = m_pHorizontalScrollBar->GetScrollRange();
    return sz;
}

void CContainerUI::SetScrollPos(SIZE szPos, bool bMsg)
{
    int cx = 0;
    int cy = 0;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
        m_pVerticalScrollBar->SetScrollPos(szPos.cy);
        cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
    }

    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
        m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
        cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
    }

    if(cx == 0 && cy == 0) return;

    RECT rcPos = { 0 };
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat()) continue;

        rcPos = pControl->GetPos();
        rcPos.left -= cx;
        rcPos.right -= cx;
        rcPos.top -= cy;
        rcPos.bottom -= cy;
        pControl->SetPos(rcPos);
    }

    Invalidate();

    if(m_pVerticalScrollBar)
    {
        // 发送滚动消息
        if(m_pManager && bMsg)
        {
            int nPage = (m_pVerticalScrollBar->GetScrollPos() + m_pVerticalScrollBar->GetLineSize()) / m_pVerticalScrollBar->GetLineSize();
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_SCROLL, (WPARAM) nPage);
        }
    }
}

void CContainerUI::SetScrollStepSize(int nSize)
{
    if(nSize > 0)
        m_nScrollStepSize = nSize;
}

int CContainerUI::GetScrollStepSize() const
{
    if(m_pManager)return m_pManager->GetDPIObj()->Scale(m_nScrollStepSize);

    return m_nScrollStepSize;
}

void CContainerUI::LineUp()
{
    int cyLine = GetScrollStepSize();
    if(cyLine == 0)
    {
        cyLine = 8;
        if(m_pManager) cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
    }

    SIZE sz = GetScrollPos();
    sz.cy -= cyLine;
    SetScrollPos(sz);
}

void CContainerUI::LineDown()
{
    int cyLine = GetScrollStepSize();
    if(cyLine == 0)
    {
        cyLine = 8;
        if(m_pManager) cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
    }

    SIZE sz = GetScrollPos();
    sz.cy += cyLine;
    SetScrollPos(sz);
}

void CContainerUI::PageUp()
{
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
    sz.cy -= iOffset;
    SetScrollPos(sz);
}

void CContainerUI::PageDown()
{
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
    sz.cy += iOffset;
    SetScrollPos(sz);
}

void CContainerUI::HomeUp()
{
    SIZE sz = GetScrollPos();
    sz.cy = 0;
    SetScrollPos(sz);
}

void CContainerUI::EndDown()
{
    if(m_pManager)
    {
        ::UpdateWindow(m_pManager->GetPaintWindow());
    }
    SIZE sz = GetScrollPos();
    sz.cy = GetScrollRange().cy;
    SetScrollPos(sz);
}

void CContainerUI::LineLeft()
{
    int nScrollStepSize = GetScrollStepSize();
    int cxLine = nScrollStepSize == 0 ? 8 : nScrollStepSize;

    SIZE sz = GetScrollPos();
    sz.cx -= cxLine;
    SetScrollPos(sz);
}

void CContainerUI::LineRight()
{
    int nScrollStepSize = GetScrollStepSize();
    int cxLine = nScrollStepSize == 0 ? 8 : nScrollStepSize;

    SIZE sz = GetScrollPos();
    sz.cx += cxLine;
    SetScrollPos(sz);
}

void CContainerUI::PageLeft()
{
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.right - m_rcItem.left - m_rcInset.left - m_rcInset.right;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
    sz.cx -= iOffset;
    SetScrollPos(sz);
}

void CContainerUI::PageRight()
{
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.right - m_rcItem.left - m_rcInset.left - m_rcInset.right;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
    sz.cx += iOffset;
    SetScrollPos(sz);
}

void CContainerUI::HomeLeft()
{
    SIZE sz = GetScrollPos();
    sz.cx = 0;
    SetScrollPos(sz);
}

void CContainerUI::EndRight()
{
    if(m_pManager)
    {
        ::UpdateWindow(m_pManager->GetPaintWindow());
    }
    SIZE sz = GetScrollPos();
    sz.cx = GetScrollRange().cx;
    SetScrollPos(sz);
}

void CContainerUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
    if(bEnableVertical && !m_pVerticalScrollBar)
    {
        m_pVerticalScrollBar = MAKE_SHARED(CScrollBarUI);
        m_pVerticalScrollBar->SetOwner(std::dynamic_pointer_cast<CContainerUI>(shared_from_this()));
        m_pVerticalScrollBar->SetManager(m_pManager, nullptr, false);
        if(m_pManager)
        {
            faw::string_t pDefaultAttributes = m_pManager->GetDefaultAttributeList(_T("VScrollBar"));
            if(!pDefaultAttributes.empty())
            {
                m_pVerticalScrollBar->ApplyAttributeList(pDefaultAttributes);
            }
        }
    }
    else if(!bEnableVertical && m_pVerticalScrollBar)
    {
        m_pVerticalScrollBar.reset();
    }

    if(bEnableHorizontal && !m_pHorizontalScrollBar)
    {
        m_pHorizontalScrollBar = MAKE_SHARED(CScrollBarUI) ;
        m_pHorizontalScrollBar->SetHorizontal(true);
        m_pHorizontalScrollBar->SetOwner(std::dynamic_pointer_cast<CContainerUI>(shared_from_this()));
        m_pHorizontalScrollBar->SetManager(m_pManager, nullptr, false);

        if(m_pManager)
        {
            faw::string_t pDefaultAttributes = m_pManager->GetDefaultAttributeList(_T("HScrollBar"));
            if(!pDefaultAttributes.empty())
            {
                m_pHorizontalScrollBar->ApplyAttributeList(pDefaultAttributes);
            }
        }
    }
    else if(!bEnableHorizontal && m_pHorizontalScrollBar)
    {
        m_pHorizontalScrollBar.reset();
    }

    NeedUpdate();
}

CScrollBarUI::ptr CContainerUI::GetVerticalScrollBar() const
{
    return m_pVerticalScrollBar;
}

CScrollBarUI::ptr CContainerUI::GetHorizontalScrollBar() const
{
    return m_pHorizontalScrollBar;
}

void CContainerUI::AnimateShow(bool NeedAnimate)
{
    if(!m_pParent)return;
    if(m_pParent->GetClass() != _T("HorizontalLayoutUI") && m_pParent->GetClass() != _T("VerticalLayoutUI"))return;
    m_bAnimate = NeedAnimate;
    if(NeedAnimate)
    {
        if(m_pParent->GetClass() == _T("HorizontalLayoutUI"))
        {
            m_nOldWidth = GetWidth();
            m_cxyFixed.cx = 5;
        }
        if(m_pParent->GetClass() == _T("VerticalLayoutUI"))
        {
            m_nOldHeight = GetHeight();
            m_cxyFixed.cy = 5;
        }
        if(!IsVisible())SetVisible(true);
        KillTimer(HIDE_CONTANIER_ID);
        SetTimer(SHOW_CONTANIER_ID, 15);
    }
    else
    {
        KillTimer(SHOW_CONTANIER_ID);
    }
}

void CContainerUI::UnderAnimateShow()
{
    if(!m_bAnimate || !m_pParent)return;
    if(m_pParent->GetClass() != _T("HorizontalLayoutUI") && m_pParent->GetClass() != _T("VerticalLayoutUI"))return;
    if(m_pParent->GetClass() == _T("HorizontalLayoutUI"))
    {
        if(m_nOldWidth - m_cxyFixed.cx >= 5)m_cxyFixed.cx += 5;
        else
        {
            m_cxyFixed.cx = m_nOldWidth;
            AnimateShow(false);
        }
    }
    if(m_pParent->GetClass() == _T("VerticalLayoutUI"))
    {
        if(m_nOldHeight - m_cxyFixed.cy >= 5)m_cxyFixed.cy += 5;
        else
        {
            m_cxyFixed.cy = m_nOldHeight;
            AnimateShow(false);
        }
    }
    NeedParentUpdate();
}

void CContainerUI::AnimateHide(bool NeedAnimate)
{
    //1.容器不可见,返回
    if(!IsVisible())return;
    m_bAnimate = NeedAnimate;
    //2.需要动画,设置定时器
    if(NeedAnimate)
    {
        KillTimer(HIDE_CONTANIER_ID);
        SetTimer(SHOW_CONTANIER_ID, 15);
    }//3.不需要动画,停止定时器,设为不可见
    else
    {
        SetVisible(false);
        KillTimer(SHOW_CONTANIER_ID);
    }
}

void CContainerUI::UnderAnimateHide()
{
    //1.没有父容器,或者不需要动画,返回
    if(!m_bAnimate || !m_pParent)return;
    //2.父容器不是横向或纵向容器,不支持
    if(m_pParent->GetClass() != _T("HorizontalLayoutUI") && m_pParent->GetClass() != _T("VerticalLayoutUI"))return;
    //3.横向容器,扩展宽度到原宽度
    if(m_pParent->GetClass() == _T("HorizontalLayoutUI"))
    {
        if(m_cxyFixed.cx >= 5)m_cxyFixed.cx -= 5;
        else
        {
            m_cxyFixed.cx = 0;
            AnimateShow(false);
        }
    }
    //4.纵向容器,扩展高度到原高度
    if(m_pParent->GetClass() == _T("VerticalLayoutUI"))
    {
        if(m_cxyFixed.cy >= 5)m_cxyFixed.cy -= 5;
        else
        {
            m_cxyFixed.cy = 0;
            AnimateShow(false);
        }
    }
}

int CContainerUI::FindSelectable(int iIndex, bool bForward /*= true*/) const
{
    // NOTE: This is actually a helper-function for the list/combo/ect controls
    //       that allow them to find the next enabled/available selectable item
    if(GetCount() == 0) return -1;
    iIndex = CLAMP(iIndex, 0, GetCount() - 1);
    if(bForward)
    {
        for(int i = iIndex; i < GetCount(); i++)
        {
            if(GetItemAt(i)->GetInterface(_T("ListItem"))
                    && GetItemAt(i)->IsVisible()
                    && GetItemAt(i)->IsEnabled()) return i;
        }
        return -1;
    }
    else
    {
        for(int i = iIndex; i >= 0; --i)
        {
            if(GetItemAt(i)->GetInterface(_T("ListItem"))
                    && GetItemAt(i)->IsVisible()
                    && GetItemAt(i)->IsEnabled()) return i;
        }
        return FindSelectable(0, true);
    }
}

RECT CContainerUI::GetClientPos() const
{
    RECT rc = m_rcItem;
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        rc.top -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom += m_pVerticalScrollBar->GetScrollRange();
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        rc.left -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right += m_pHorizontalScrollBar->GetScrollRange();
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }
    return rc;
}

void CContainerUI::Move(SIZE szOffset, bool bNeedInvalidate)
{
    CControlUI::Move(szOffset, bNeedInvalidate);
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) m_pVerticalScrollBar->Move(szOffset, false);
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) m_pHorizontalScrollBar->Move(szOffset, false);
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        if(pControl && pControl->IsVisible()) pControl->Move(szOffset, false);
    }
}

void CContainerUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    if(m_items.empty()) return;

    rc = m_rcItem;
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        rc.top -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom += m_pVerticalScrollBar->GetScrollRange();
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        rc.left -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right += m_pHorizontalScrollBar->GetScrollRange();
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }

    int iIndex = -1;
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        iIndex++;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat())
        {
            SetFloatPos(iIndex);
        }
        else
        {
            SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
            if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
            RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy };
            pControl->SetPos(rcCtrl, false);
        }
    }
}

void CContainerUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("inset"))
    {
        RECT rcInset = FawTools::parse_rect(pstrValue);
        SetInset(rcInset);
    }
    else if(pstrName == _T("mousechild")) SetMouseChildEnabled(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("vscrollbar"))
    {
        EnableScrollBar(FawTools::parse_bool(pstrValue), bool(GetHorizontalScrollBar()));
    }
    else if(pstrName == _T("vscrollbarstyle"))
    {
        m_sVerticalScrollBarStyle = pstrValue;
        EnableScrollBar(TRUE, bool(GetHorizontalScrollBar()));
        if(GetVerticalScrollBar())
        {
            faw::string_t pStyle = m_pManager->GetStyle(m_sVerticalScrollBarStyle);
            if(!pStyle.empty())
            {
                GetVerticalScrollBar()->ApplyAttributeList(pStyle);
            }
            else
            {
                GetVerticalScrollBar()->ApplyAttributeList(pstrValue);
            }
        }
    }
    else if(pstrName == _T("hscrollbar"))
    {
        EnableScrollBar(bool(GetVerticalScrollBar()), FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("hscrollbarstyle"))
    {
        m_sHorizontalScrollBarStyle = pstrValue;
        EnableScrollBar(TRUE, bool(GetHorizontalScrollBar()));
        if(GetHorizontalScrollBar())
        {
            faw::string_t pStyle = m_pManager->GetStyle(m_sHorizontalScrollBarStyle);
            if(!pStyle.empty())
            {
                GetHorizontalScrollBar()->ApplyAttributeList(pStyle);
            }
            else
            {
                GetHorizontalScrollBar()->ApplyAttributeList(pstrValue);
            }
        }
    }
    else if(pstrName == _T("childpadding")) SetChildPadding(_ttoi(pstrValue.data()));
    else if(pstrName == _T("childalign"))
    {
        if(pstrValue == _T("left")) m_iChildAlign = DT_LEFT;
        else if(pstrValue == _T("center")) m_iChildAlign = DT_CENTER;
        else if(pstrValue == _T("right")) m_iChildAlign = DT_RIGHT;
    }
    else if(pstrName == _T("childvalign"))
    {
        if(pstrValue == _T("top")) m_iChildVAlign = DT_TOP;
        else if(pstrValue == _T("vcenter")) m_iChildVAlign = DT_VCENTER;
        else if(pstrValue == _T("bottom")) m_iChildVAlign = DT_BOTTOM;
    }
    else if(pstrName == _T("scrollstepsize")) SetScrollStepSize(_ttoi(pstrValue.data()));
    else if(pstrName == _T("croptranslate")) EnableTranslate(FawTools::parse_bool(pstrValue));
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CContainerUI::SetManager(CPaintManagerUI* pManager, std::shared_ptr<CControlUI> pParent, bool bInit)
{
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        pControl->SetManager(pManager, shared_from_this(), bInit);
    }

    if(m_pVerticalScrollBar) m_pVerticalScrollBar->SetManager(pManager, shared_from_this(), bInit);
    if(m_pHorizontalScrollBar) m_pHorizontalScrollBar->SetManager(pManager, shared_from_this(), bInit);
    CControlUI::SetManager(pManager, pParent, bInit);
}

std::shared_ptr<CControlUI> CContainerUI::FindControl(FIND_CONTROL_PROC Proc, LPVOID pData, UINT uFlags)
{
    if((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) return nullptr;
    if((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) return nullptr;
    if((uFlags & UIFIND_HITTEST) != 0 && !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData)))) return nullptr;
    if((uFlags & UIFIND_UPDATETEST) != 0 && Proc(shared_from_this(), pData)) return nullptr;

    std::shared_ptr<CControlUI> pResult = nullptr;
    if((uFlags & UIFIND_ME_FIRST) != 0)
    {
        if((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) pResult = Proc(shared_from_this(), pData);
    }
    if(!pResult && m_pVerticalScrollBar)
    {
        if((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
    }
    if(!pResult && m_pHorizontalScrollBar)
    {
        if((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
    }
    if(pResult) return pResult;

    if((uFlags & UIFIND_HITTEST) == 0 || IsMouseChildEnabled())
    {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;
        if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
        if((uFlags & UIFIND_TOP_FIRST) != 0)
        {
            for(auto rit = m_items.rbegin(); rit != m_items.rend(); ++rit)
            {
                std::shared_ptr<CControlUI> pItem = *rit;
                pResult = pItem->FindControl(Proc, pData, uFlags);
                if(pResult)
                {
                    if((uFlags & UIFIND_HITTEST) != 0 && !pResult->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))))
                        continue;
                    else
                        return pResult;
                }
            }
        }
        else
        {
            for(auto rit = m_items.rbegin(); rit != m_items.rend(); ++rit)
            {
                std::shared_ptr<CControlUI> pItem = *rit;
                pResult = pItem->FindControl(Proc, pData, uFlags);
                if(pResult)
                {
                    if((uFlags & UIFIND_HITTEST) != 0 && !pResult->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))))
                        continue;
                    else
                        return pResult;
                }
            }
        }
    }

    pResult = nullptr;
    if(!pResult && (uFlags & UIFIND_ME_FIRST) == 0)
    {
        if((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) pResult = Proc(shared_from_this(), pData);
    }
    return pResult;
}

bool CContainerUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    RECT rcTemp = { 0 };
    //1.rcPaint与m_rcItem自身无交集,返回
    if(!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) return true;
    //2.裁剪交集区域rcTemp,更新该区域视图数据
    CRenderClip clip;
    CRenderClip::GenerateClip(hDC, rcTemp, clip);

    //3.是否为圆角矩形
    int nBorderSize = GetBorderSize();
    SIZE cxyBorderRound = GetBorderRound();
    //4.是圆角矩形
    if(cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0)
    {
        //4k
        if(m_rcItem.right - m_rcItem.left >= 1920 || m_rcItem.bottom - m_rcItem.top > 1080)
        {
            CRenderClip roundClip;
            CRenderClip::GenerateRoundClip(hDC, m_rcPaint, m_rcItem, cxyBorderRound.cx, cxyBorderRound.cy, roundClip);
            PaintItem(hDC, rcPaint, pStopControl, rcTemp);
            PaintBorder(hDC);
        }
        else  //2K
        {
            auto parent = GetParent();
            while(parent && m_dwBackColor == 0)
            {
                m_dwBackColor = parent->GetBkColor();
                m_dwBackColor2 = parent->GetBkColor2();
                m_dwBackColor3 = parent->GetBkColor3();
                parent = parent->GetParent();
            }

            m_rcPreItem = { 0 };
            RECT m_rcPrePaint = { 0 };
            if(m_bTranslate)
            {
                m_rcPreItem = m_rcItem;
                m_rcPrePaint = m_rcPaint;
                m_rcItem = { 0,0,m_rcItem.right - m_rcItem.left,m_rcItem.bottom - m_rcItem.top };
                m_rcPaint = m_rcItem;
            }
            //std::cout << std::format("m_rcPreItem: {} {} {} {}		", m_rcPreItem.left, m_rcPreItem.top, m_rcPreItem.right - m_rcPreItem.left, m_rcPreItem.bottom - m_rcPreItem.top);
            //std::cout << std::format("m_rcItem: {} {} {} {}\r\n", m_rcItem.left, m_rcItem.top, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
            //////2.draw round Rect Path picture to hdc
            HDC MemoryHDC = CreateCompatibleDC(hDC);
            void* BitmapBits = nullptr;
            LPBITMAPINFO lpbiSrc = nullptr;
            lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
            lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            lpbiSrc->bmiHeader.biWidth = m_rcItem.right + 1;
            lpbiSrc->bmiHeader.biHeight = m_rcItem.bottom + 1;
            lpbiSrc->bmiHeader.biPlanes = 1;
            lpbiSrc->bmiHeader.biBitCount = 32;
            lpbiSrc->bmiHeader.biCompression = BI_RGB;
            lpbiSrc->bmiHeader.biSizeImage = (m_rcItem.right + 1) * (m_rcItem.bottom + 1);
            lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
            lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
            lpbiSrc->bmiHeader.biClrUsed = 0;
            lpbiSrc->bmiHeader.biClrImportant = 0;
            HBITMAP MemoryHBitmap = CreateDIBSection(hDC, lpbiSrc, DIB_RGB_COLORS, &BitmapBits, nullptr, 0);
            ON_SCOPE_EXIT([&] {	delete[] lpbiSrc; });
            auto oldBitmap = SelectObject(MemoryHDC, MemoryHBitmap);

            ////是否确实有顺序画到需要裁剪的元素
            PaintItem(MemoryHDC, m_rcPaint, pStopControl, rcTemp, m_bTranslate);

            float x = m_rcItem.left;
            float y = m_rcItem.top;
            float width = m_rcItem.right - m_rcItem.left - 1;
            float height = m_rcItem.bottom - m_rcItem.top - 1;
            float arcSize = cxyBorderRound.cx;
            float arcDiameter = arcSize * 2;
            Gdiplus::Graphics  graphics(hDC);
            //设置绘图时的滤波模式为消除锯齿现象，即使用 SmoothingModeHighQuality 来获得更平滑的绘图效果。
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            //创建一个 Gdiplus::GraphicsPath 对象 roundRectPath，用于定义矩形的路径。在路径中添加直线和弧线，以实现四个圆角。
            Gdiplus::GraphicsPath roundRectPath;
            //在路径中依次添加顶部横线、右上圆角、右侧竖线、右下圆角、底部横线、左下圆角、左侧竖线和左上圆角，完成整个矩形的路径。
            roundRectPath.AddLine(x + arcSize, y, x + width - arcSize, y);  // 顶部横线
            roundRectPath.AddArc(x + width - arcDiameter, y, arcDiameter, arcDiameter, 270, 90); // 右上圆角
            roundRectPath.AddLine(x + width, y + arcSize, x + width, y + height - arcSize);  // 右侧竖线
            roundRectPath.AddArc(x + width - arcDiameter, y + height - arcDiameter, arcDiameter, arcDiameter, 0, 90); // 右下圆角
            roundRectPath.AddLine(x + width - arcSize, y + height, x + arcSize, y + height);  // 底部横线
            roundRectPath.AddArc(x, y + height - arcDiameter, arcDiameter, arcDiameter, 90, 90); // 左下圆角
            roundRectPath.AddLine(x, y + height - arcSize, x, y + arcSize);  // 左侧竖线
            roundRectPath.AddArc(x, y, arcDiameter, arcDiameter, 180, 90); // 左上圆角
            roundRectPath.CloseFigure();

            //设置裁剪圆,SetClip正如你所注意到的，它不会给你一个抗锯齿的边缘
            //graphics.SetClip(&roundRectPath, Gdiplus::CombineModeIntersect);
            Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromBITMAPINFO(lpbiSrc, BitmapBits);
            ON_SCOPE_EXIT([&] {	delete image; });
            Gdiplus::TextureBrush brush(image);
            auto restorePoint = graphics.Save();
            if(m_bTranslate)graphics.TranslateTransform(m_rcPreItem.left, m_rcPreItem.top);
            graphics.FillPath(&brush, &roundRectPath);
            if(m_bTranslate)
            {
                graphics.Restore(restorePoint);
                m_rcItem = m_rcPreItem;
                m_rcPaint = m_rcPrePaint;
            }

            SelectObject(MemoryHDC, oldBitmap);
            DeleteObject(MemoryHBitmap);
            DeleteDC(MemoryHDC);

            PaintBorder(hDC);
        }
    }
    else
    {
        PaintItem(hDC, rcPaint, pStopControl, rcTemp);
        PaintBorder(hDC);
    }


    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        if(m_pVerticalScrollBar == pStopControl) return false;
        if(::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()))
        {
            if(!m_pVerticalScrollBar->Paint(hDC, rcPaint, pStopControl)) return false;
        }
    }

    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        if(m_pHorizontalScrollBar == pStopControl) return false;
        if(::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()))
        {
            if(!m_pHorizontalScrollBar->Paint(hDC, rcPaint, pStopControl)) return false;
        }
    }
    return true;
}

bool CContainerUI::PaintItem(HDC hDC, RECT rcPaint, std::shared_ptr<CControlUI> pStopControl, RECT rcIntersect, bool bTranslate)
{
    PaintBkColor(hDC);
    PaintBkImage(hDC);
    PaintStatusImage(hDC);
    PaintForeColor(hDC);
    PaintForeImage(hDC);
    PaintText(hDC);
    //5.不是圆角矩形
    if(m_items.size() > 0)
    {
        RECT rcInset = GetInset();
        RECT rc = m_rcItem;
        rc.left += rcInset.left;
        rc.top += rcInset.top;
        rc.right -= rcInset.right;
        rc.bottom -= rcInset.bottom;
        if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

        if(bTranslate)
        {
            OffsetRect(&rcIntersect, -m_rcPreItem.left, -m_rcPreItem.top);
        }
        if(!::IntersectRect(&rcIntersect, &rcPaint, &rc))
        {
            for(auto& pItem : m_items)
            {
                std::shared_ptr<CControlUI> pControl = pItem;
                if(pControl == pStopControl) return false;
                if(!pControl->IsVisible()) continue;
                if(bTranslate)
                {
                    RECT rcPos = pControl->GetPos();
                    OffsetRect(&rcPos,-m_rcPreItem.left,-m_rcPreItem.top);
                    pControl->SetPos(rcPos);
                }
                if(!::IntersectRect(&rcIntersect, &rcPaint, &pControl->GetPos())) continue;
                if(pControl->IsFloat())
                {
                    if(!::IntersectRect(&rcIntersect, &m_rcItem, &pControl->GetPos())) continue;
                    if(!pControl->Paint(hDC, rcPaint, pStopControl)) return false;
                }
                if(bTranslate)
                {
                    RECT rcPos = pControl->GetPos();
                    OffsetRect(&rcPos, m_rcPreItem.left, m_rcPreItem.top);
                    pControl->SetPos(rcPos);
                }
            }
        }
        else
        {
            CRenderClip childClip;
            CRenderClip::GenerateClip(hDC, rcIntersect, childClip);
            for(auto& pItem : m_items)
            {
                std::shared_ptr<CControlUI> pControl = pItem;
                if(pControl == pStopControl) return false;
                if(!pControl->IsVisible()) continue;
                if(bTranslate)
                {
                    RECT rcPos = pControl->GetPos();
                    OffsetRect(&rcPos, -m_rcPreItem.left, -m_rcPreItem.top);
                    pControl->SetPos(rcPos);
                }
                if(!::IntersectRect(&rcIntersect, &rcPaint, &pControl->GetPos())) continue;
                if(pControl->IsFloat())
                {
                    if(!::IntersectRect(&rcIntersect, &m_rcItem, &pControl->GetPos())) continue;
                    CRenderClip::UseOldClipBegin(hDC, childClip);
                    if(!pControl->Paint(hDC, rcPaint, pStopControl)) return false;
                    CRenderClip::UseOldClipEnd(hDC, childClip);
                }
                else
                {
                    if(!::IntersectRect(&rcIntersect, &rc, &pControl->GetPos())) continue;
                    if(!pControl->Paint(hDC, rcPaint, pStopControl)) return false;
                }
                if(bTranslate)
                {
                    RECT rcPos = pControl->GetPos();
                    OffsetRect(&rcPos, m_rcPreItem.left, m_rcPreItem.top);
                    pControl->SetPos(rcPos);
                }
            }
        }
        if(bTranslate)
        {
            OffsetRect(&rcIntersect, m_rcPreItem.left, m_rcPreItem.top);
        }
    }
    return true;
}

void CContainerUI::EnableTranslate(bool bTranslate)
{
    m_bTranslate = bTranslate;
}

void CContainerUI::SetFloatPos(int iIndex)
{
    // 因为CControlUI::SetPos对float的操作影响，这里不能对float组件添加滚动条的影响
    if(iIndex < 0 || iIndex >= (long)m_items.size()) return;

    auto new_it = m_items.begin();
    std::advance(new_it, iIndex);
    std::shared_ptr<CControlUI> pControl = *new_it;

    if(!pControl->IsVisible()) return;
    if(!pControl->IsFloat()) return;

    SIZE szXY = pControl->GetFixedXY();
    SIZE sz = { pControl->GetFixedWidth(), pControl->GetFixedHeight() };

    int nParentWidth = m_rcItem.right - m_rcItem.left;
    int nParentHeight = m_rcItem.bottom - m_rcItem.top;

    UINT uAlign = pControl->GetFloatAlign();
    if(uAlign != 0)
    {
        RECT rcCtrl = { 0, 0, sz.cx, sz.cy };
        if((uAlign & DT_CENTER) != 0)
        {
            ::OffsetRect(&rcCtrl, (nParentWidth - sz.cx) / 2, 0);
        }
        else if((uAlign & DT_RIGHT) != 0)
        {
            ::OffsetRect(&rcCtrl, nParentWidth - sz.cx, 0);
        }
        else
        {
            ::OffsetRect(&rcCtrl, szXY.cx, 0);
        }

        if((uAlign & DT_VCENTER) != 0)
        {
            ::OffsetRect(&rcCtrl, 0, (nParentHeight - sz.cy) / 2);
        }
        else if((uAlign & DT_BOTTOM) != 0)
        {
            ::OffsetRect(&rcCtrl, 0, nParentHeight - sz.cy);
        }
        else
        {
            ::OffsetRect(&rcCtrl, 0, szXY.cy);
        }

        ::OffsetRect(&rcCtrl, m_rcItem.left, m_rcItem.top);
        pControl->SetPos(rcCtrl, false);
    }
    else
    {
        TPercentInfo rcPercent = pControl->GetFloatPercent();
        LONG width = m_rcItem.right - m_rcItem.left;
        LONG height = m_rcItem.bottom - m_rcItem.top;
        RECT rcCtrl = { 0 };
        rcCtrl.left = (LONG)(width*rcPercent.left) + szXY.cx + m_rcItem.left;
        rcCtrl.top = (LONG)(height*rcPercent.top) + szXY.cy + m_rcItem.top;
        rcCtrl.right = (LONG)(width*rcPercent.right) + szXY.cx + sz.cx + m_rcItem.left;
        rcCtrl.bottom = (LONG)(height*rcPercent.bottom) + szXY.cy + sz.cy + m_rcItem.top;
        pControl->SetPos(rcCtrl, false);
    }
}

void CContainerUI::ProcessScrollBar(RECT rc, int cxRequired, int cyRequired)
{
    while(m_pHorizontalScrollBar)
    {
        // Scroll needed
        if(cxRequired > rc.right - rc.left && !m_pHorizontalScrollBar->IsVisible())
        {
            m_pHorizontalScrollBar->SetVisible(true);
            m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
            m_pHorizontalScrollBar->SetScrollPos(0);
            SetPos(m_rcItem);
            break;
        }

        // No scrollbar required
        if(!m_pHorizontalScrollBar->IsVisible()) break;

        // Scroll not needed anymore?
        int cxScroll = cxRequired - (rc.right - rc.left);
        if(cxScroll <= 0)
        {
            m_pHorizontalScrollBar->SetVisible(false);
            m_pHorizontalScrollBar->SetScrollPos(0);
            m_pHorizontalScrollBar->SetScrollRange(0);
            SetPos(m_rcItem);
        }
        else
        {
            RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight() };
            m_pHorizontalScrollBar->SetPos(rcScrollBarPos);

            if(m_pHorizontalScrollBar->GetScrollRange() != cxScroll)
            {
                int iScrollPos = m_pHorizontalScrollBar->GetScrollPos();
                m_pHorizontalScrollBar->SetScrollRange(::abs(cxScroll));   // if scrollpos>range then scrollpos=range
                if(iScrollPos > m_pHorizontalScrollBar->GetScrollPos())
                {
                    SetPos(m_rcItem);
                }
            }
        }
        break;
    }

    while(m_pVerticalScrollBar)
    {
        // Scroll needed
        if(cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible())
        {
            m_pVerticalScrollBar->SetVisible(true);
            m_pVerticalScrollBar->SetScrollRange(cyRequired - (rc.bottom - rc.top));
            m_pVerticalScrollBar->SetScrollPos(0);
            SetPos(m_rcItem);
            break;
        }

        // No scrollbar required
        if(!m_pVerticalScrollBar->IsVisible()) break;

        // Scroll not needed anymore?
        int cyScroll = cyRequired - (rc.bottom - rc.top);
        if(cyScroll <= 0)
        {
            m_pVerticalScrollBar->SetVisible(false);
            m_pVerticalScrollBar->SetScrollPos(0);
            m_pVerticalScrollBar->SetScrollRange(0);
            SetPos(m_rcItem);
            break;
        }

        RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };
        m_pVerticalScrollBar->SetPos(rcScrollBarPos);

        if(m_pVerticalScrollBar->GetScrollRange() != cyScroll)
        {
            int iScrollPos = m_pVerticalScrollBar->GetScrollPos();
            m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));   // if scrollpos>range then scrollpos=range
            if(iScrollPos > m_pVerticalScrollBar->GetScrollPos())
            {
                SetPos(m_rcItem);
            }
        }
        break;
    }
}

bool CContainerUI::SetSubControlText(faw::string_t pstrSubControlName, faw::string_t pstrText)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(pSubControl)
    {
        pSubControl->SetText(pstrText);
        return TRUE;
    }
    else
        return FALSE;
}

bool CContainerUI::SetSubControlFixedHeight(faw::string_t pstrSubControlName, int cy)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(pSubControl)
    {
        pSubControl->SetFixedHeight(cy);
        return TRUE;
    }
    else
        return FALSE;
}

bool CContainerUI::SetSubControlFixedWdith(faw::string_t pstrSubControlName, int cx)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(pSubControl)
    {
        pSubControl->SetFixedWidth(cx);
        return TRUE;
    }
    else
        return FALSE;
}

bool CContainerUI::SetSubControlUserData(faw::string_t pstrSubControlName, faw::string_t pstrText)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(pSubControl)
    {
        pSubControl->SetUserData(pstrText);
        return TRUE;
    }
    else
        return FALSE;
}

faw::string_t CContainerUI::GetSubControlText(faw::string_t pstrSubControlName)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(!pSubControl)
        return _T("");
    else
        return pSubControl->GetText();
}

int CContainerUI::GetSubControlFixedHeight(faw::string_t pstrSubControlName)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(!pSubControl)
        return -1;
    else
        return pSubControl->GetFixedHeight();
}

int CContainerUI::GetSubControlFixedWdith(faw::string_t pstrSubControlName)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(!pSubControl)
        return -1;
    else
        return pSubControl->GetFixedWidth();
}

const faw::string_t CContainerUI::GetSubControlUserData(faw::string_t pstrSubControlName)
{
    std::shared_ptr<CControlUI> pSubControl = nullptr;
    pSubControl = this->FindSubControl(pstrSubControlName);
    if(!pSubControl)
        return _T("");
    else
        return pSubControl->GetUserData();
}

std::shared_ptr<CControlUI> CContainerUI::FindSubControl(faw::string_t pstrSubControlName)
{
    return static_cast<std::shared_ptr<CControlUI>>(GetManager()->FindSubControlByName(shared_from_this(), pstrSubControlName));
}

} // namespace DuiLib
