#include "StdAfx.h"
#include "UITabLayout.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CTabLayoutUI)
CTabLayoutUI::CTabLayoutUI() {}

faw::string_t CTabLayoutUI::GetClass() const
{
    return _T("TabLayoutUI");
}

std::shared_ptr<CControlUI> CTabLayoutUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_TABLAYOUT) return std::dynamic_pointer_cast<CTabLayoutUI>(shared_from_this());
    return CContainerUI::GetInterface(pstrName);
}

bool CTabLayoutUI::Add(std::shared_ptr<CControlUI> pControl)
{
    bool ret = CContainerUI::Add(pControl);
    if(!ret) return ret;

    if(m_iCurSel == -1 && pControl->IsVisible())
    {
        m_iCurSel = GetItemIndex(pControl);
    }
    else
    {
        pControl->SetVisible(false);
    }

    return ret;
}

bool CTabLayoutUI::AddAt(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    bool ret = CContainerUI::AddAt(pControl, iIndex);
    if(!ret) return ret;

    if(m_iCurSel == -1 && pControl->IsVisible())
    {
        m_iCurSel = GetItemIndex(pControl);
    }
    else if(m_iCurSel != -1 && iIndex <= m_iCurSel)
    {
        m_iCurSel += 1;
    }
    else
    {
        pControl->SetVisible(false);
    }

    return ret;
}

bool CTabLayoutUI::Remove(std::shared_ptr<CControlUI> pControl)
{
    if(!pControl) return false;

    int index = GetItemIndex(pControl);
    bool ret = CContainerUI::Remove(pControl);
    if(!ret) return false;

    if(m_iCurSel == index)
    {
        if(GetCount() > 0)
        {
            m_iCurSel = 0;
            GetItemAt(m_iCurSel)->SetVisible(true);
        }
        else
            m_iCurSel = -1;
        NeedParentUpdate();
    }
    else if(m_iCurSel > index)
    {
        m_iCurSel -= 1;
    }

    return ret;
}

void CTabLayoutUI::RemoveAll()
{
    m_iCurSel = -1;
    CContainerUI::RemoveAll();
    NeedParentUpdate();
}

int CTabLayoutUI::GetCurSel() const
{
    return m_iCurSel;
}

bool CTabLayoutUI::SelectItem(int iIndex)
{
    if(iIndex < 0 || iIndex >= (long)m_items.size()) return false;
    if(iIndex == m_iCurSel) return true;

    int m_nOldSel = m_iCurSel;
    m_iCurSel = iIndex;
    int index = -1;
    for(auto& pItem : m_items)
    {
        index++;
        std::shared_ptr<CControlUI> pControl = pItem;
        if(index == iIndex)
        {
            pControl->SetVisible(true);
            pControl->SetFocus();
            SetPos(m_rcItem);
        }
        else pControl->SetVisible(false);
    }
    NeedParentUpdate();

    if(m_pManager)
    {
        m_pManager->SetNextTabControl();
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_TABSELECT, m_iCurSel, m_nOldSel);
    }
    return true;
}

bool CTabLayoutUI::SelectItem(std::shared_ptr<CControlUI> pControl)
{
    int iIndex = GetItemIndex(pControl);
    if(iIndex == -1)
        return false;
    else
        return SelectItem(iIndex);
}

void CTabLayoutUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("selectedid")) SelectItem(FawTools::parse_dec(pstrValue));
    return CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CTabLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    auto rcItem = rc;

    int index = -1;
    for(auto& pItem : m_items)
    {
        index++;
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat())
        {
            SetFloatPos(index);
            continue;
        }

        if(index != m_iCurSel) continue;

        RECT rcPadding = pControl->GetPadding();
        rcItem.left += rcPadding.left;
        rcItem.top += rcPadding.top;
        rcItem.right -= rcPadding.right;
        rcItem.bottom -= rcPadding.bottom;

        SIZE szAvailable = { rcItem.right - rcItem.left, rcItem.bottom - rcItem.top };

        SIZE sz = pControl->EstimateSize(szAvailable);
        if(sz.cx == 0)
        {
            sz.cx = MAX(0, szAvailable.cx);
        }
        if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
        if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

        if(sz.cy == 0)
        {
            sz.cy = MAX(0, szAvailable.cy);
        }
        if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
        if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

        RECT rcCtrl = { rcItem.left, rcItem.top, rcItem.left + sz.cx, rcItem.top + sz.cy };
        pControl->SetPos(rcCtrl);
    }
}
}
