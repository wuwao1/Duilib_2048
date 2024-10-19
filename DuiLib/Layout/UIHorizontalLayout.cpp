﻿#include "StdAfx.h"
#include "UIHorizontalLayout.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CHorizontalLayoutUI)
CHorizontalLayoutUI::CHorizontalLayoutUI() {}

faw::string_t CHorizontalLayoutUI::GetClass() const
{
    return _T("HorizontalLayoutUI");
}

std::shared_ptr<CControlUI> CHorizontalLayoutUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_HORIZONTALLAYOUT) return std::dynamic_pointer_cast<CHorizontalLayoutUI>(shared_from_this());
    return CContainerUI::GetInterface(pstrName);
}

UINT CHorizontalLayoutUI::GetControlFlags() const
{
    if(IsEnabled() && m_iSepWidth != 0) return UIFLAG_SETCURSOR;
    else return 0;
}

void CHorizontalLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    RECT _rcInset = CHorizontalLayoutUI::m_rcInset;
    GetManager()->GetDPIObj()->Scale(&_rcInset);
    rc.left += _rcInset.left;
    rc.top += _rcInset.top;
    rc.right -= _rcInset.right;
    rc.bottom -= _rcInset.bottom;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

    if(m_items.size() == 0)
    {
        ProcessScrollBar(rc, 0, 0);
        return;
    }

    // Determine the minimum size
    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    //if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
    //	szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
        szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();

    int cyNeeded = 0;
    int nAdjustables = 0;
    int cxFixed = 0;
    int nEstimateNum = 0;
    SIZE szControlAvailable = { 0 };
    int iControlMaxWidth = 0;
    int iControlMaxHeight = 0;
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat()) continue;
        szControlAvailable = szAvailable;
        RECT rcPadding = pControl->GetPadding();
        szControlAvailable.cy -= rcPadding.top + rcPadding.bottom;
        iControlMaxWidth = pControl->GetFixedWidth();
        iControlMaxHeight = pControl->GetFixedHeight();
        if(iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
        if(iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
        if(szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
        if(szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
        SIZE sz = pControl->EstimateSize(szControlAvailable);
        if(sz.cx == 0)
        {
            nAdjustables++;
        }
        else
        {
            if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
        }
        cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;

        sz.cy = MAX(sz.cy, 0);
        if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
        if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
        cyNeeded = MAX(cyNeeded, sz.cy + rcPadding.top + rcPadding.bottom);
        nEstimateNum++;
    }
    cxFixed += (nEstimateNum - 1) * m_iChildPadding;
    // Place elements
    int cxNeeded = 0;
    int cxExpand = 0;
    if(nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosX = rc.left;

    // 滚动条
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        iPosX -= m_pHorizontalScrollBar->GetScrollPos();
    }
    else
    {
        // 子控件横向对其方式
        if(nAdjustables <= 0)
        {
            UINT iChildAlign = GetChildAlign();
            if(iChildAlign == DT_CENTER)
            {
                iPosX += (szAvailable.cx -cxFixed) / 2;
            }
            else if(iChildAlign == DT_RIGHT)
            {
                iPosX += (szAvailable.cx - cxFixed);
            }
        }
    }
    int iEstimate = 0;
    int iAdjustable = 0;
    int cxFixedRemaining = cxFixed;
    int Index = -1;
    for(auto& pItem : m_items)
    {
        Index++;
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat())
        {
            SetFloatPos(Index);
            continue;
        }

        iEstimate += 1;
        RECT rcPadding = pControl->GetPadding();
        szRemaining.cx -= rcPadding.left;

        szControlAvailable = szRemaining;
        szControlAvailable.cy -= rcPadding.top + rcPadding.bottom;
        iControlMaxWidth = pControl->GetFixedWidth();
        iControlMaxHeight = pControl->GetFixedHeight();
        if(iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
        if(iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
        if(szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
        if(szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
        cxFixedRemaining = cxFixedRemaining - (rcPadding.left + rcPadding.right);
        if(iEstimate > 1) cxFixedRemaining = cxFixedRemaining - m_iChildPadding;
        SIZE sz = pControl->EstimateSize(szControlAvailable);
        if(sz.cx == 0)
        {
            iAdjustable++;
            sz.cx = cxExpand;
            // Distribute remaining to last element (usually round-off left-overs)
            if(iAdjustable == nAdjustables)
            {
                sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
            }
            if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
        }
        else
        {
            if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            cxFixedRemaining -= sz.cx;
        }

        sz.cy = pControl->GetMaxHeight();
        if(sz.cy == 0) sz.cy = szAvailable.cy - rcPadding.top - rcPadding.bottom;
        if(sz.cy < 0) sz.cy = 0;
        if(sz.cy > szControlAvailable.cy) sz.cy = szControlAvailable.cy;
        if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();

        UINT iChildAlign = GetChildVAlign();
        if(iChildAlign == DT_VCENTER)
        {
            int iPosY = (rc.bottom + rc.top) / 2;
            if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
            {
                iPosY += m_pVerticalScrollBar->GetScrollRange() / 2;
                iPosY -= m_pVerticalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY - sz.cy / 2, iPosX + sz.cx + rcPadding.left, iPosY + sz.cy - sz.cy / 2 };
            pControl->SetPos(rcCtrl, false);
        }
        else if(iChildAlign == DT_BOTTOM)
        {
            int iPosY = rc.bottom;
            if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
            {
                iPosY += m_pVerticalScrollBar->GetScrollRange();
                iPosY -= m_pVerticalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY - rcPadding.bottom - sz.cy, iPosX + sz.cx + rcPadding.left, iPosY - rcPadding.bottom };
            pControl->SetPos(rcCtrl, false);
        }
        else
        {
            int iPosY = rc.top;
            if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
            {
                iPosY -= m_pVerticalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + sz.cx + rcPadding.left, iPosY + sz.cy + rcPadding.top };
            pControl->SetPos(rcCtrl, false);
        }

        iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
        cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
        szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
    }
    cxNeeded += (nEstimateNum - 1) * m_iChildPadding;

    // Process the scrollbar
    ProcessScrollBar(rc, cxNeeded, cyNeeded);
}

void CHorizontalLayoutUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
    if((m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode)
    {
        RECT rcSeparator = GetThumbRect(true);
        CRenderEngine::DrawColor(hDC, rcSeparator, 0xAA000000);
    }
}

void CHorizontalLayoutUI::SetSepWidth(int iWidth)
{
    m_iSepWidth = iWidth;
}

int CHorizontalLayoutUI::GetSepWidth() const
{
    return m_iSepWidth;
}

void CHorizontalLayoutUI::SetSepImmMode(bool bImmediately)
{
    if(m_bImmMode == bImmediately) return;
    if((m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager)
    {
        m_pManager->RemovePostPaint(shared_from_this());
    }

    m_bImmMode = bImmediately;
}

bool CHorizontalLayoutUI::IsSepImmMode() const
{
    return m_bImmMode;
}

void CHorizontalLayoutUI::SetAutoCalcWidth(bool autoCalcWidth)
{
    m_autoCalcWidth = autoCalcWidth;
}

bool CHorizontalLayoutUI::IsAutoCalcWidth() const
{
    return m_autoCalcWidth;
}

void CHorizontalLayoutUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("sepwidth")) SetSepWidth(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("sepimm")) SetSepImmMode(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("autocalcwidth")) SetAutoCalcWidth(FawTools::parse_bool(pstrValue));
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CHorizontalLayoutUI::DoEvent(TEventUI& event)
{
    if(m_iSepWidth != 0)
    {
        if(event.Type == UIEVENT_BUTTONDOWN && IsEnabled())
        {
            RECT rcSeparator = GetThumbRect(false);
            if(::PtInRect(&rcSeparator, event.ptMouse))
            {
                m_uButtonState |= UISTATE_CAPTURED;
                ptLastMouse = event.ptMouse;
                m_rcNewPos = m_rcItem;
                if(!m_bImmMode && m_pManager) m_pManager->AddPostPaint(shared_from_this());
                return;
            }
        }
        if(event.Type == UIEVENT_BUTTONUP)
        {
            if((m_uButtonState & UISTATE_CAPTURED) != 0)
            {
                m_uButtonState &= ~UISTATE_CAPTURED;
                m_rcItem = m_rcNewPos;
                if(!m_bImmMode && m_pManager) m_pManager->RemovePostPaint(shared_from_this());
                NeedParentUpdate();
                return;
            }
        }
        if(event.Type == UIEVENT_MOUSEMOVE)
        {
            if((m_uButtonState & UISTATE_CAPTURED) != 0)
            {
                LONG cx = event.ptMouse.x - ptLastMouse.x;
                ptLastMouse = event.ptMouse;
                RECT rc = m_rcNewPos;
                if(m_iSepWidth >= 0)
                {
                    if(cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_iSepWidth) return;
                    if(cx < 0 && event.ptMouse.x > m_rcNewPos.right) return;
                    rc.right += cx;
                    if(rc.right - rc.left <= GetMinWidth())
                    {
                        if(m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth()) return;
                        rc.right = rc.left + GetMinWidth();
                    }
                    if(rc.right - rc.left >= GetMaxWidth())
                    {
                        if(m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth()) return;
                        rc.right = rc.left + GetMaxWidth();
                    }
                }
                else
                {
                    if(cx > 0 && event.ptMouse.x < m_rcNewPos.left) return;
                    if(cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_iSepWidth) return;
                    rc.left += cx;
                    if(rc.right - rc.left <= GetMinWidth())
                    {
                        if(m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth()) return;
                        rc.left = rc.right - GetMinWidth();
                    }
                    if(rc.right - rc.left >= GetMaxWidth())
                    {
                        if(m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth()) return;
                        rc.left = rc.right - GetMaxWidth();
                    }
                }

                RECT rcInvalidate = GetThumbRect(true);
                m_rcNewPos = rc;
                m_cxyFixed.cx = m_rcNewPos.right - m_rcNewPos.left;

                if(m_bImmMode)
                {
                    m_rcItem = m_rcNewPos;
                    NeedParentUpdate();
                }
                else
                {
                    RECT rc = GetThumbRect(true);
                    rcInvalidate.left = min(rcInvalidate.left, rc.left);
                    rcInvalidate.top = min(rcInvalidate.top, rc.top);
                    rcInvalidate.right = max(rcInvalidate.right, rc.right);
                    rcInvalidate.bottom = max(rcInvalidate.bottom, rc.bottom);
                    rc = GetThumbRect(false);
                    rcInvalidate.left = min(rcInvalidate.left, rc.left);
                    rcInvalidate.top = min(rcInvalidate.top, rc.top);
                    rcInvalidate.right = max(rcInvalidate.right, rc.right);
                    rcInvalidate.bottom = max(rcInvalidate.bottom, rc.bottom);
                    if(m_pManager) m_pManager->Invalidate(rcInvalidate);
                }
                return;
            }
        }
        if(event.Type == UIEVENT_SETCURSOR)
        {
            RECT rcSeparator = GetThumbRect(false);
            if(IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse))
            {
                ::SetCursor(::LoadCursor(nullptr, IDC_SIZEWE));
                return;
            }
        }
    }
    CContainerUI::DoEvent(event);
}

SIZE CHorizontalLayoutUI::EstimateSize(SIZE szAvailable)
{
    SIZE sz = __super::EstimateSize(szAvailable);
    if(!IsAutoCalcWidth())
        return sz;
    int width = 0;
    for(int i = 0; i < GetCount(); ++i)
    {
        auto ctrl = GetItemAt(i);
        if(!ctrl->IsFloat())
            width += ctrl->GetFixedWidth();
    }
    sz.cx = width;
    return sz;
}

bool CHorizontalLayoutUI::IsDynamic(POINT &pt) const
{
    RECT rcSeparator = GetThumbRect(false);
    return (IsEnabled() && ::PtInRect(&rcSeparator, pt)) || CControlUI::IsDynamic(pt);
}

RECT CHorizontalLayoutUI::GetThumbRect(bool bUseNew) const
{
    if((m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew)
    {
        if(m_iSepWidth >= 0) return { m_rcNewPos.right - m_iSepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom };
        else return { m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_iSepWidth, m_rcNewPos.bottom };
    }
    else
    {
        if(m_iSepWidth >= 0) return { m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom };
        else return { m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom };
    }
}
}
