﻿#include "StdAfx.h"

namespace DuiLib
{

/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CListUI)

CListUI::CListUI()
{
    m_pList = std::make_shared<CListBodyUI>(this);
    m_pHeader = std::make_shared<CListHeaderUI>() ;

    Add(m_pHeader);
    CVerticalLayoutUI::Add(m_pList);

    m_ListInfo.nColumns = 0;
    m_ListInfo.nFont = -1;
    m_ListInfo.uTextStyle = DT_VCENTER | DT_SINGLELINE;
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
    m_ListInfo.bShowRowLine = false;
    m_ListInfo.bShowColumnLine = false;
    m_ListInfo.bShowHtml = false;
    m_ListInfo.bMultiExpandable = false;
    m_ListInfo.bRSelected = false;
    ::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
    ::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));
}

faw::string_t CListUI::GetClass() const
{
    return _T("ListUI");
}

UINT CListUI::GetControlFlags() const
{
    return UIFLAG_TABSTOP;
}

std::shared_ptr<CControlUI> CListUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LIST) return std::dynamic_pointer_cast<CListUI>(shared_from_this());
    if(pstrName ==  _T("IList")) return std::dynamic_pointer_cast<CListUI>(shared_from_this());
    if(pstrName == _T("IListOwner")) return std::dynamic_pointer_cast<CListUI>(shared_from_this());
    return CVerticalLayoutUI::GetInterface(pstrName);
}

std::shared_ptr<CControlUI> CListUI::GetItemAt(int iIndex) const
{
    return m_pList->GetItemAt(iIndex);
}

int CListUI::GetItemIndex(std::shared_ptr<CControlUI> pControl) const
{
    if(pControl->GetInterface(_T("ListHeader"))) return CVerticalLayoutUI::GetItemIndex(pControl);
    // We also need to recognize header sub-items
    if(pControl->GetClass().find(_T("ListHeaderItemUI")) != faw::string_t::npos) return m_pHeader->GetItemIndex(pControl);

    return m_pList->GetItemIndex(pControl);
}

bool CListUI::SetItemIndex(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    if(pControl->GetInterface(_T("ListHeader"))) return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
    // We also need to recognize header sub-items
    if(pControl->GetClass().find(_T("ListHeaderItemUI")) != faw::string_t::npos) return m_pHeader->SetItemIndex(pControl, iIndex);

    int iOrginIndex = m_pList->GetItemIndex(pControl);
    if(iOrginIndex == -1) return false;
    if(iOrginIndex == iIndex) return true;

    std::shared_ptr<IListItemUI> pSelectedListItem = nullptr;
    if(m_iCurSel >= 0)
        pSelectedListItem = std::dynamic_pointer_cast<IListItemUI>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
    if(!m_pList->SetItemIndex(pControl, iIndex)) return false;
    int iMinIndex = min(iOrginIndex, iIndex);
    int iMaxIndex = max(iOrginIndex, iIndex);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i)
    {
        std::shared_ptr<CControlUI> p = m_pList->GetItemAt(i);
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(i);
        }
    }
    if(m_iCurSel >= 0 && pSelectedListItem) m_iCurSel = pSelectedListItem->GetIndex();
    return true;
}

int CListUI::GetCount() const
{
    return m_pList->GetCount();
}

bool CListUI::Add(std::shared_ptr<CControlUI> pControl)
{
    // Override the Add() method so we can add items specifically to
    // the intended widgets. Headers are assumed to be
    // answer the correct interface so we can add multiple list headers.
    if(pControl->GetInterface(_T("ListHeader")))
    {
        if(m_pHeader != pControl && m_pHeader->GetCount() == 0)
        {
            CVerticalLayoutUI::Remove(m_pHeader);
            m_pHeader = std::dynamic_pointer_cast<CListHeaderUI>(pControl);
        }
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return CVerticalLayoutUI::AddAt(pControl, 0);
    }
    // We also need to recognize header sub-items
    if(pControl->GetClass().find(_T("ListHeaderItemUI")) != faw::string_t::npos)
    {
        bool ret = m_pHeader->Add(pControl);
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return ret;
    }
    // The list items should know about us
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(pListItem)
    {
        pListItem->SetOwner(shared_from_this());
        pListItem->SetIndex(GetCount());
        return m_pList->Add(pControl);
    }
    return CVerticalLayoutUI::Add(pControl);
}

bool CListUI::AddAt(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    // Override the AddAt() method so we can add items specifically to
    // the intended widgets. Headers and are assumed to be
    // answer the correct interface so we can add multiple list headers.
    if(pControl->GetInterface(_T("ListHeader")))
    {
        if(m_pHeader != pControl && m_pHeader->GetCount() == 0)
        {
            CVerticalLayoutUI::Remove(m_pHeader);
            m_pHeader = std::dynamic_pointer_cast<CListHeaderUI>(pControl);
        }
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return CVerticalLayoutUI::AddAt(pControl, 0);
    }
    // We also need to recognize header sub-items
    if(pControl->GetClass().find(_T("ListHeaderItemUI")) != faw::string_t::npos)
    {
        bool ret = m_pHeader->AddAt(pControl, iIndex);
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return ret;
    }
    if(!m_pList->AddAt(pControl, iIndex)) return false;

    // The list items should know about us
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(pListItem)
    {
        pListItem->SetOwner(shared_from_this());
        pListItem->SetIndex(iIndex);
    }

    for(int i = iIndex + 1; i < m_pList->GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> p = m_pList->GetItemAt(i);
        pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(i);
        }
    }
    if(m_iCurSel >= iIndex) m_iCurSel += 1;

    std::vector<int> VecTempSelIndex;
    for(auto i = 0; i < m_aSelItems.GetSize(); i++)
    {
        int iSelIndex = (int)m_aSelItems.GetAt(i);
        if(iSelIndex >= iIndex)iSelIndex++;
        VecTempSelIndex.push_back(iSelIndex);
    }
    m_aSelItems.Clear();
    for(size_t i = 0; i < VecTempSelIndex.size(); i++)
        m_aSelItems.Add((void*)VecTempSelIndex[i]);

    return true;
}

bool CListUI::Remove(std::shared_ptr<CControlUI> pControl)
{
    if(pControl->GetInterface(_T("ListHeader"))) return CVerticalLayoutUI::Remove(pControl);
    // We also need to recognize header sub-items
    if(pControl->GetClass().find(_T("ListHeaderItemUI")) != faw::string_t::npos) return m_pHeader->Remove(pControl);

    int iIndex = m_pList->GetItemIndex(pControl);
    if(iIndex == -1) return false;

    if(!m_pList->RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < m_pList->GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> p = m_pList->GetItemAt(i);
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(i);
        }
    }

    std::vector<int> VecTempSelIndex;
    for(auto i = 0; i < m_aSelItems.GetSize(); i++)
    {
        int iSelIndex = (int)m_aSelItems.GetAt(i);
        if(iSelIndex == iIndex)continue;
        if(iSelIndex > iIndex)iSelIndex--;
        VecTempSelIndex.push_back(iSelIndex);
    }
    m_aSelItems.Clear();
    for(size_t i = 0; i < VecTempSelIndex.size(); i++)
        m_aSelItems.Add((void*)VecTempSelIndex[i]);

    if(iIndex == m_iCurSel && m_iCurSel >= 0)
    {
        int iSel = m_iCurSel;
        m_iCurSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if(iIndex < m_iCurSel) m_iCurSel -= 1;
    return true;
}

bool CListUI::Remove(CControlUI* pControl)
{
    return m_pList->Remove(pControl);
}

bool CListUI::RemoveAt(int iIndex)
{
    if(!m_pList->RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < m_pList->GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> p = m_pList->GetItemAt(i);
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(p->GetInterface(_T("ListItem")));
        if(pListItem) pListItem->SetIndex(i);
    }
    std::vector<int> VecTempSelIndex;
    for(auto i = 0; i < m_aSelItems.GetSize(); i++)
    {
        int iSelIndex = (int)m_aSelItems.GetAt(i);
        if(iSelIndex == iIndex)continue;
        if(iSelIndex > iIndex)iSelIndex--;
        VecTempSelIndex.push_back(iSelIndex);
    }
    m_aSelItems.empty();
    for(size_t i = 0; i < VecTempSelIndex.size(); i++)
        m_aSelItems.Add((void*)VecTempSelIndex[i]);

    if(iIndex == m_iCurSel && m_iCurSel >= 0)
    {
        int iSel = m_iCurSel;
        m_iCurSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if(iIndex < m_iCurSel) m_iCurSel -= 1;
    return true;
}

void CListUI::RemoveAll()
{
    m_iCurSel = -1;
    m_iExpandedItem = -1;
    m_aSelItems.Clear();
    m_pList->RemoveAll();
}

void CListUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CVerticalLayoutUI::SetPos(rc, bNeedInvalidate);

    if(!m_pHeader) return;
    // Determine general list information and the size of header columns
    m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
    // The header/columns may or may not be visible at runtime. In either case
    // we should determine the correct dimensions...

    if(!m_pHeader->IsVisible())
    {
        for(int it = 0; it < m_pHeader->GetCount(); it++)
        {
            static_cast<std::shared_ptr<CControlUI>>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
        }
        m_pHeader->SetPos({ rc.left, 0, rc.right, 0 }, bNeedInvalidate);
    }

    for(int i = 0; i < m_ListInfo.nColumns; i++)
    {
        std::shared_ptr<CControlUI> pControl = static_cast<std::shared_ptr<CControlUI>>(m_pHeader->GetItemAt(i));
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat()) continue;
        RECT rcPos = pControl->GetPos();
        m_ListInfo.rcColumn[i] = pControl->GetPos();
    }
    if(!m_pHeader->IsVisible())
    {
        for(int it = 0; it < m_pHeader->GetCount(); it++)
        {
            static_cast<std::shared_ptr<CControlUI>>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
        }
    }
    m_pList->SetPos(m_pList->GetPos(), bNeedInvalidate);
}

void CListUI::Move(SIZE szOffset, bool bNeedInvalidate)
{
    CVerticalLayoutUI::Move(szOffset, bNeedInvalidate);
    if(!m_pHeader->IsVisible()) m_pHeader->Move(szOffset, false);
}

int CListUI::GetMinSelItemIndex()
{
    if(m_aSelItems.GetSize() <= 0)
        return -1;
    int min = (int) m_aSelItems.GetAt(0);
    int index;
    for(int i = 0; i < m_aSelItems.GetSize(); ++i)
    {
        index = (int) m_aSelItems.GetAt(i);
        if(min > index)
            min = index;
    }
    return min;
}

int CListUI::GetMaxSelItemIndex()
{
    if(m_aSelItems.GetSize() <= 0)
        return -1;
    int max = (int) m_aSelItems.GetAt(0);
    int index;
    for(int i = 0; i < m_aSelItems.GetSize(); ++i)
    {
        index = (int) m_aSelItems.GetAt(i);
        if(max < index)
            max = index;
    }
    return max;
}

void CListUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CVerticalLayoutUI::DoEvent(event);
        return;
    }

    switch(event.Type)
    {
    case UIEVENT_SETFOCUS:
        m_bFocused = true;
        return;
    case UIEVENT_KILLFOCUS:
        m_bFocused = false;
        return;
    case UIEVENT_KEYDOWN:
        switch(event.chKey)
        {
        case VK_UP:
        {
            if(m_aSelItems.GetSize() > 0)
            {
                int index = GetMinSelItemIndex() - 1;
                UnSelectAllItems();
                index > 0 ? SelectItem(index, true) : SelectItem(0, true);
            }
        }
        return;
        case VK_DOWN:
        {
            if(m_aSelItems.GetSize() > 0)
            {
                int index = GetMaxSelItemIndex() + 1;
                UnSelectAllItems();
                index + 1 > m_pList->GetCount() ? SelectItem(GetCount() - 1, true) : SelectItem(index, true);
            }
        }
        return;
        case VK_PRIOR:
            PageUp();
            return;
        case VK_NEXT:
            PageDown();
            return;
        case VK_HOME:
            SelectItem(FindSelectable(0, false), true);
            return;
        case VK_END:
            SelectItem(FindSelectable(GetCount() - 1, true), true);
            return;
        case VK_RETURN:
            if(m_iCurSel != -1) GetItemAt(m_iCurSel)->Activate();
            return;
        case 0x41:// Ctrl+A
        {
            if(IsMultiSelect() && (GetKeyState(VK_CONTROL) & 0x8000))
            {
                SelectAllItems();
            }
            return;
        }
        }
        break;
    case UIEVENT_SCROLLWHEEL:
        switch(LOWORD(event.wParam))
        {
        case SB_LINEUP:
            if(m_bScrollSelect && !IsMultiSelect()) SelectItem(FindSelectable(m_iCurSel - 1, false), true);
            else LineUp();
            return;
        case SB_LINEDOWN:
            if(m_bScrollSelect && !IsMultiSelect()) SelectItem(FindSelectable(m_iCurSel + 1, true), true);
            else LineDown();
            return;
        }
        break;
    }
    CVerticalLayoutUI::DoEvent(event);
}

bool CListUI::IsFixedScrollbar()
{
    return m_bFixedScrollbar;
}

void CListUI::SetFixedScrollbar(bool bFixed)
{
    m_bFixedScrollbar = bFixed;
    Invalidate();
}

RECT CListUI::GetClientPos() const
{
    RECT rc = m_rcItem;
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    if(GetVerticalScrollBar() && GetVerticalScrollBar()->IsVisible())
    {
        auto m_pVerticalScrollBar = GetVerticalScrollBar();
        rc.top -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom += m_pVerticalScrollBar->GetScrollRange();
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if(GetHorizontalScrollBar() && GetHorizontalScrollBar()->IsVisible())
    {
        auto m_pHorizontalScrollBar = GetVerticalScrollBar();
        rc.left -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right += m_pHorizontalScrollBar->GetScrollRange();
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }
    return rc;
}

std::shared_ptr<CListHeaderUI> CListUI::GetHeader() const
{
    return m_pHeader;
}

std::shared_ptr<CContainerUI> CListUI::GetList() const
{
    return m_pList;
}

bool CListUI::GetScrollSelect()
{
    return m_bScrollSelect;
}

void CListUI::SetScrollSelect(bool bScrollSelect)
{
    m_bScrollSelect = bScrollSelect;
}

int CListUI::GetCurSelActivate() const
{
    return m_iCurSelActivate;
}

bool CListUI::SelectItemActivate(int iIndex)
{
    if(!SelectItem(iIndex, true))
    {
        return false;
    }

    m_iCurSelActivate = iIndex;
    return true;
}

int CListUI::GetCurSel() const
{
    if(m_aSelItems.GetSize() <= 0)
    {
        return -1;
    }
    else
    {
        return (int) m_aSelItems.GetAt(0);
    }

    return -1;
}

bool CListUI::SelectItem(int iIndex, bool bTakeFocus)
{
    // 取消所有选择项
    UnSelectAllItems();
    // 判断是否合法列表项
    if(iIndex < 0) return false;
    std::shared_ptr<CControlUI> pControl = GetItemAt(iIndex);
    if(!pControl) return false;
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(!pListItem) return false;
    if(!pListItem->Select(true))
    {
        return false;
    }
    int iLastSel = m_iCurSel;
    m_iCurSel = iIndex;
    m_aSelItems.Add((void*) iIndex);
    EnsureVisible(iIndex);
    if(bTakeFocus) pControl->SetFocus();
    if(m_pManager && iLastSel != m_iCurSel)
    {
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMSELECT, iIndex);
    }

    return true;
}

bool CListUI::SelectMultiItem(int iIndex, bool bTakeFocus)
{
    if(!IsMultiSelect()) return SelectItem(iIndex, bTakeFocus);

    if(iIndex < 0) return false;
    std::shared_ptr<CControlUI> pControl = GetItemAt(iIndex);
    if(!pControl) return false;
    std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
    if(!pListItem) return false;
    if(m_aSelItems.Find((void*) iIndex) >= 0) return false;
    if(!pListItem->SelectMulti(true)) return false;

    m_iCurSel = iIndex;
    m_aSelItems.Add((void*) iIndex);
    EnsureVisible(iIndex);
    if(bTakeFocus) pControl->SetFocus();
    if(m_pManager)
    {
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMSELECT, iIndex);
    }
    return true;
}

void CListUI::SetMultiSelect(bool bMultiSel)
{
    m_bMultiSel = bMultiSel;
    if(!bMultiSel) UnSelectAllItems();
}

bool CListUI::IsMultiSelect() const
{
    return m_bMultiSel;
}

bool CListUI::UnSelectItem(int iIndex, bool bOthers)
{
    if(!IsMultiSelect()) return false;
    if(bOthers)
    {
        for(int i = m_aSelItems.GetSize() - 1; i >= 0; --i)
        {
            int iSelIndex = (int) m_aSelItems.GetAt(i);
            if(iSelIndex == iIndex) continue;
            std::shared_ptr<CControlUI> pControl = GetItemAt(iSelIndex);
            if(!pControl) continue;
            if(!pControl->IsEnabled()) continue;
            std::shared_ptr<IListItemUI> pSelListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
            if(!pSelListItem) continue;
            if(!pSelListItem->SelectMulti(false)) continue;
            m_aSelItems.Remove(i);
        }
    }
    else
    {
        if(iIndex < 0) return false;
        std::shared_ptr<CControlUI> pControl = GetItemAt(iIndex);
        if(!pControl) return false;
        if(!pControl->IsEnabled()) return false;
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
        if(!pListItem) return false;
        int aIndex = m_aSelItems.Find((void*) iIndex);
        if(aIndex < 0) return false;
        if(!pListItem->SelectMulti(false)) return false;
        if(m_iCurSel == iIndex) m_iCurSel = -1;
        m_aSelItems.Remove(aIndex);
    }
    return true;
}

void CListUI::SelectAllItems()
{
    for(int i = 0; i < GetCount(); ++i)
    {
        std::shared_ptr<CControlUI> pControl = GetItemAt(i);
        if(!pControl) continue;
        if(!pControl->IsVisible()) continue;
        if(!pControl->IsEnabled()) continue;
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
        if(!pListItem) continue;
        if(!pListItem->SelectMulti(true)) continue;

        m_aSelItems.Add((void*) i);
        m_iCurSel = i;
    }
}

void CListUI::UnSelectAllItems()
{
    for(int i = 0; i < m_aSelItems.GetSize(); ++i)
    {
        int iSelIndex = (int) m_aSelItems.GetAt(i);
        std::shared_ptr<CControlUI> pControl = GetItemAt(iSelIndex);
        if(!pControl) continue;
        if(!pControl->IsEnabled()) continue;
        std::shared_ptr<IListItemUI> pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
        if(!pListItem) continue;
        if(!pListItem->SelectMulti(false)) continue;
    }
    m_aSelItems.Clear();
    m_iCurSel = -1;
}

int CListUI::GetSelectItemCount() const
{
    return m_aSelItems.GetSize();
}

int CListUI::GetNextSelItem(int nItem) const
{
    if(m_aSelItems.GetSize() <= 0)
        return -1;

    if(nItem < 0)
    {
        return (int) m_aSelItems.GetAt(0);
    }
    int aIndex = m_aSelItems.Find((void*) nItem);
    if(aIndex < 0) return -1;
    if(aIndex + 1 > m_aSelItems.GetSize() - 1)
        return -1;
    return (int) m_aSelItems.GetAt(aIndex + 1);
}

UINT CListUI::GetListType()
{
    return LT_LIST;
}

TListInfoUI* CListUI::GetListInfo()
{
    return &m_ListInfo;
}

bool CListUI::IsDelayedDestroy() const
{
    return m_pList->IsDelayedDestroy();
}

void CListUI::SetDelayedDestroy(bool bDelayed)
{
    m_pList->SetDelayedDestroy(bDelayed);
}

int CListUI::GetChildPadding() const
{
    return m_pList->GetChildPadding();
}

void CListUI::SetChildPadding(int iPadding)
{
    m_pList->SetChildPadding(iPadding);
}

void CListUI::SetItemFont(int index)
{
    m_ListInfo.nFont = index;
    NeedUpdate();
}

void CListUI::SetItemTextStyle(UINT uStyle)
{
    m_ListInfo.uTextStyle = uStyle;
    NeedUpdate();
}

void CListUI::SetItemTextPadding(RECT rc)
{
    m_ListInfo.rcTextPadding = rc;
    NeedUpdate();
}

RECT CListUI::GetItemTextPadding() const
{
    RECT rect = m_ListInfo.rcTextPadding;
    GetManager()->GetDPIObj()->Scale(&rect);
    return rect;
}

void CListUI::SetItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetItemBkImage(faw::string_t pStrImage)
{
    m_ListInfo.sBkImage = pStrImage;
    Invalidate();
}

void CListUI::SetAlternateBk(bool bAlternateBk)
{
    m_ListInfo.bAlternateBk = bAlternateBk;
    Invalidate();
}

DWORD CListUI::GetItemTextColor() const
{
    return m_ListInfo.dwTextColor;
}

DWORD CListUI::GetItemBkColor() const
{
    return m_ListInfo.dwBkColor;
}

faw::string_t CListUI::GetItemBkImage() const
{
    return m_ListInfo.sBkImage;
}

bool CListUI::IsAlternateBk() const
{
    return m_ListInfo.bAlternateBk;
}

void CListUI::SetSelectedItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwSelectedTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetSelectedItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwSelectedBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetSelectedItemImage(faw::string_t pStrImage)
{
    m_ListInfo.sSelectedImage = pStrImage;
    Invalidate();
}

DWORD CListUI::GetSelectedItemTextColor() const
{
    return m_ListInfo.dwSelectedTextColor;
}

DWORD CListUI::GetSelectedItemBkColor() const
{
    return m_ListInfo.dwSelectedBkColor;
}

faw::string_t CListUI::GetSelectedItemImage() const
{
    return m_ListInfo.sSelectedImage;
}

void CListUI::SetHotItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwHotTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetHotItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwHotBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetHotItemImage(faw::string_t pStrImage)
{
    m_ListInfo.sHotImage = pStrImage;
    Invalidate();
}

DWORD CListUI::GetHotItemTextColor() const
{
    return m_ListInfo.dwHotTextColor;
}
DWORD CListUI::GetHotItemBkColor() const
{
    return m_ListInfo.dwHotBkColor;
}

faw::string_t CListUI::GetHotItemImage() const
{
    return m_ListInfo.sHotImage;
}

void CListUI::SetDisabledItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwDisabledTextColor = dwTextColor;
    Invalidate();
}

void CListUI::SetDisabledItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwDisabledBkColor = dwBkColor;
    Invalidate();
}

void CListUI::SetDisabledItemImage(faw::string_t pStrImage)
{
    m_ListInfo.sDisabledImage = pStrImage;
    Invalidate();
}

DWORD CListUI::GetDisabledItemTextColor() const
{
    return m_ListInfo.dwDisabledTextColor;
}

DWORD CListUI::GetDisabledItemBkColor() const
{
    return m_ListInfo.dwDisabledBkColor;
}

faw::string_t CListUI::GetDisabledItemImage() const
{
    return m_ListInfo.sDisabledImage;
}

DWORD CListUI::GetItemLineColor() const
{
    return m_ListInfo.dwLineColor;
}

void CListUI::SetItemLineColor(DWORD dwLineColor)
{
    m_ListInfo.dwLineColor = dwLineColor;
    Invalidate();
}
void CListUI::SetItemShowRowLine(bool bShowLine)
{
    m_ListInfo.bShowRowLine = bShowLine;
    Invalidate();
}
void CListUI::SetItemShowColumnLine(bool bShowLine)
{
    m_ListInfo.bShowColumnLine = bShowLine;
    Invalidate();
}
bool CListUI::IsItemShowHtml()
{
    return m_ListInfo.bShowHtml;
}

void CListUI::SetItemShowHtml(bool bShowHtml)
{
    if(m_ListInfo.bShowHtml == bShowHtml) return;

    m_ListInfo.bShowHtml = bShowHtml;
    NeedUpdate();
}

bool CListUI::IsItemRSelected()
{
    return m_ListInfo.bRSelected;
}

void CListUI::SetItemRSelected(bool bSelected)
{
    if(m_ListInfo.bRSelected == bSelected) return;

    m_ListInfo.bRSelected = bSelected;
    NeedUpdate();
}

void CListUI::SetMultiExpanding(bool bMultiExpandable)
{
    m_ListInfo.bMultiExpandable = bMultiExpandable;
}

bool CListUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
{
    if(m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable)
    {
        std::shared_ptr<CControlUI> pControl = GetItemAt(m_iExpandedItem);
        if(pControl)
        {
            std::shared_ptr<IListItemUI> pItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
            if(pItem) pItem->Expand(false);
        }
        m_iExpandedItem = -1;
    }
    if(bExpand)
    {
        std::shared_ptr<CControlUI> pControl = GetItemAt(iIndex);
        if(!pControl) return false;
        if(!pControl->IsVisible()) return false;
        std::shared_ptr<IListItemUI> pItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(_T("ListItem")));
        if(!pItem) return false;
        m_iExpandedItem = iIndex;
        if(!pItem->Expand(true))
        {
            m_iExpandedItem = -1;
            return false;
        }
    }
    NeedUpdate();
    return true;
}

int CListUI::GetExpandedItem() const
{
    return m_iExpandedItem;
}

void CListUI::EnsureVisible(int iIndex)
{
    if(m_iCurSel < 0) return;
    RECT rcItem = m_pList->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pList->GetPos();
    RECT rcListInset = m_pList->GetInset();

    rcList.left += rcListInset.left;
    rcList.top += rcListInset.top;
    rcList.right -= rcListInset.right;
    rcList.bottom -= rcListInset.bottom;

    std::shared_ptr<CScrollBarUI>  pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
    if(pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

    int iPos = m_pList->GetScrollPos().cy;
    if(rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom) return;
    int dx = 0;
    if(rcItem.top < rcList.top) dx = rcItem.top - rcList.top;
    if(rcItem.bottom > rcList.bottom) dx = rcItem.bottom - rcList.bottom;
    Scroll(0, dx);
}

void CListUI::Scroll(int dx, int dy)
{
    if(dx == 0 && dy == 0) return;
    SIZE sz = m_pList->GetScrollPos();
    m_pList->SetScrollPos({ sz.cx + dx, sz.cy + dy });
}

void CListUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("header")) GetHeader()->SetVisible(pstrValue != _T("hidden"));
    else if(pstrName == _T("headerbkimage")) GetHeader()->SetBkImage(pstrValue);
    else if(pstrName == _T("scrollselect")) SetScrollSelect(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("fixedscrollbar")) SetFixedScrollbar(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("multiexpanding")) SetMultiExpanding(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("itemfont")) m_ListInfo.nFont = FawTools::parse_dec(pstrValue);
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
        SetItemTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itembkcolor"))
    {
        SetItemBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itembkimage")) SetItemBkImage(pstrValue);
    else if(pstrName == _T("itemaltbk")) SetAlternateBk(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("itemselectedtextcolor"))
    {
        SetSelectedItemTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemselectedbkcolor"))
    {
        SetSelectedItemBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemselectedimage")) SetSelectedItemImage(pstrValue);
    else if(pstrName == _T("itemhottextcolor"))
    {
        SetHotItemTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemhotbkcolor"))
    {
        SetHotItemBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemhotimage")) SetHotItemImage(pstrValue);
    else if(pstrName == _T("itemdisabledtextcolor"))
    {
        SetDisabledItemTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemdisabledbkcolor"))
    {
        SetDisabledItemBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemdisabledimage")) SetDisabledItemImage(pstrValue);
    else if(pstrName == _T("itemlinecolor"))
    {
        SetItemLineColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("itemshowrowline")) SetItemShowRowLine(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("itemshowcolumnline")) SetItemShowColumnLine(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("itemshowhtml")) SetItemShowHtml(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("multiselect")) SetMultiSelect(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("itemrselected")) SetItemRSelected(FawTools::parse_bool(pstrValue));
    else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
}

IListCallbackUI* CListUI::GetTextCallback() const
{
    return m_pCallback;
}

void CListUI::SetTextCallback(IListCallbackUI* pCallback)
{
    m_pCallback = pCallback;
}

SIZE CListUI::GetScrollPos() const
{
    return m_pList->GetScrollPos();
}

SIZE CListUI::GetScrollRange() const
{
    return m_pList->GetScrollRange();
}

void CListUI::SetScrollPos(SIZE szPos, bool bMsg)
{
    m_pList->SetScrollPos(szPos, bMsg);
}

void CListUI::LineUp()
{
    m_pList->LineUp();
}

void CListUI::LineDown()
{
    m_pList->LineDown();
}

void CListUI::PageUp()
{
    m_pList->PageUp();
}

void CListUI::PageDown()
{
    m_pList->PageDown();
}

void CListUI::HomeUp()
{
    m_pList->HomeUp();
}

void CListUI::EndDown()
{
    m_pList->EndDown();
}

void CListUI::LineLeft()
{
    m_pList->LineLeft();
}

void CListUI::LineRight()
{
    m_pList->LineRight();
}

void CListUI::PageLeft()
{
    m_pList->PageLeft();
}

void CListUI::PageRight()
{
    m_pList->PageRight();
}

void CListUI::HomeLeft()
{
    m_pList->HomeLeft();
}

void CListUI::EndRight()
{
    m_pList->EndRight();
}

void CListUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
    m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
}

std::shared_ptr<CScrollBarUI> CListUI::GetVerticalScrollBar() const
{
    return m_pList->GetVerticalScrollBar();
}

std::shared_ptr<CScrollBarUI> CListUI::GetHorizontalScrollBar() const
{
    return m_pList->GetHorizontalScrollBar();
}

BOOL CListUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
{
    if(!m_pList)
        return FALSE;
    return m_pList->SortItems(pfnCompare, dwData);
}
/////////////////////////////////////////////////////////////////////////////////////
//
//

CListBodyUI::CListBodyUI(CListUI* pOwner) :
    m_pOwner(pOwner)
{
    ASSERT(m_pOwner);
}

BOOL CListBodyUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
{
    if(!pfnCompare)
        return FALSE;
    m_pCompareFunc = pfnCompare;
    m_compareData = dwData;
    // 将 std::list 转换为数组形式
    std::vector<std::shared_ptr<CControlUI>> itemsVec(m_items.begin(), m_items.end());
    qsort_s(itemsVec.data(), itemsVec.size(), sizeof(std::shared_ptr<CControlUI>), CListBodyUI::ItemComareFunc, this);
    // 将排序后的元素重新放回 std::list
    m_items.assign(itemsVec.begin(), itemsVec.end());
    std::shared_ptr<IListItemUI> pListItem = nullptr;
    int Index = -1;
    for(auto& pItem : m_items)
    {
        Index++;
        std::shared_ptr<CControlUI> pControl = pItem;
        pListItem = std::dynamic_pointer_cast<IListItemUI>(pControl->GetInterface(TEXT("ListItem")));
        if(pListItem)
        {
            pListItem->SetIndex(Index);
            pListItem->Select(false);
        }
    }
    m_pOwner->SelectItem(-1);
    if(m_pManager)
    {
        SetPos(GetPos());
        Invalidate();
    }

    return TRUE;
}

int __cdecl CListBodyUI::ItemComareFunc(void *pvlocale, const void *item1, const void *item2)
{
    CListBodyUI *pThis = (CListBodyUI*) pvlocale;
    if(!pThis || !item1 || !item2)
        return 0;
    return pThis->ItemComareFunc(item1, item2);
}

int __cdecl CListBodyUI::ItemComareFunc(const void *item1, const void *item2)
{
    CControlUI *pControl1 = (CControlUI*) item1;
    CControlUI *pControl2 = (CControlUI*) item2;
    return m_pCompareFunc((UINT_PTR) pControl1, (UINT_PTR) pControl2, m_compareData);
}

int CListBodyUI::GetScrollStepSize() const
{
    if(m_pOwner) return m_pOwner->GetScrollStepSize();

    return CVerticalLayoutUI::GetScrollStepSize();
}

void CListBodyUI::SetScrollPos(SIZE szPos, bool bMsg)
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

    RECT rcPos = { 0 };
    int Index = -1;
    for(auto& pItem : m_items)
    {
        Index++;
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat()) continue;

        rcPos = pControl->GetPos();
        rcPos.left -= cx;
        rcPos.right -= cx;
        rcPos.top -= cy;
        rcPos.bottom -= cy;
        pControl->SetPos(rcPos, true);
    }

    Invalidate();
    if(m_pOwner)
    {
        CListHeaderUI::ptr pHeader = m_pOwner->GetHeader();
        if(!pHeader) return;
        TListInfoUI* pInfo = m_pOwner->GetListInfo();
        pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);

        if(!pHeader->IsVisible())
        {
            for(int it = 0; it < pHeader->GetCount(); it++)
            {
                static_cast<std::shared_ptr<CControlUI>>(pHeader->GetItemAt(it))->SetInternVisible(true);
            }
        }
        for(int i = 0; i < pInfo->nColumns; i++)
        {
            std::shared_ptr<CControlUI> pControl = static_cast<std::shared_ptr<CControlUI>>(pHeader->GetItemAt(i));
            if(!pControl->IsVisible()) continue;
            if(pControl->IsFloat()) continue;

            rcPos = pControl->GetPos();
            rcPos.left -= cx;
            rcPos.right -= cx;
            pControl->SetPos(rcPos);
            pInfo->rcColumn[i] = pControl->GetPos();
        }
        if(!pHeader->IsVisible())
        {
            for(int it = 0; it < pHeader->GetCount(); it++)
            {
                static_cast<std::shared_ptr<CControlUI>>(pHeader->GetItemAt(it))->SetInternVisible(false);
            }
        }
    }
}

void CListBodyUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;
    if(m_pOwner->IsFixedScrollbar() && m_pVerticalScrollBar) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    else if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

    // Determine the minimum size
    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
        szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

    int cxNeeded = 0;
    int nAdjustables = 0;
    int cyFixed = 0;
    int nEstimateNum = 0;
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat()) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        if(sz.cy == 0)
        {
            nAdjustables++;
        }
        else
        {
            if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
        }
        cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

        RECT rcPadding = pControl->GetPadding();
        sz.cx = MAX(sz.cx, 0);
        if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
        if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
        cxNeeded = MAX(cxNeeded, sz.cx);
        nEstimateNum++;
    }
    cyFixed += (nEstimateNum - 1) * m_iChildPadding;

    if(m_pOwner)
    {
        CListHeaderUI::ptr pHeader = m_pOwner->GetHeader();
        if(pHeader && pHeader->GetCount() > 0)
        {
            cxNeeded = MAX(0, pHeader->EstimateSize({ rc.right - rc.left, rc.bottom - rc.top }).cx);
            if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
            {
                int nOffset = m_pHorizontalScrollBar->GetScrollPos();
                RECT rcHeader = pHeader->GetPos();
                rcHeader.left = rc.left - nOffset;
                pHeader->SetPos(rcHeader);
            }
        }
    }

    // Place elements
    int cyNeeded = 0;
    int cyExpand = 0;
    if(nAdjustables > 0) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosY = rc.top;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        iPosY -= m_pVerticalScrollBar->GetScrollPos();
    }
    int iPosX = rc.left;
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        iPosX -= m_pHorizontalScrollBar->GetScrollPos();
    }
    int iAdjustable = 0;
    int cyFixedRemaining = cyFixed;
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

        RECT rcPadding = pControl->GetPadding();
        szRemaining.cy -= rcPadding.top;
        SIZE sz = pControl->EstimateSize(szRemaining);
        if(sz.cy == 0)
        {
            iAdjustable++;
            sz.cy = cyExpand;
            // Distribute remaining to last element (usually round-off left-overs)
            if(iAdjustable == nAdjustables)
            {
                sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
            }
            if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
        }
        else
        {
            if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
            cyFixedRemaining -= sz.cy;
        }

        sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

        if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
        if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

        RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
        pControl->SetPos(rcCtrl);

        iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
        cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
        szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
    }
    cyNeeded += (nEstimateNum - 1) * m_iChildPadding;

    if(m_pHorizontalScrollBar)
    {
        if(cxNeeded > rc.right - rc.left)
        {
            if(m_pHorizontalScrollBar->IsVisible())
            {
                m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
            }
            else
            {
                m_pHorizontalScrollBar->SetVisible(true);
                m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
                m_pHorizontalScrollBar->SetScrollPos(0);
                rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
            }
        }
        else
        {
            if(m_pHorizontalScrollBar->IsVisible())
            {
                m_pHorizontalScrollBar->SetVisible(false);
                m_pHorizontalScrollBar->SetScrollRange(0);
                m_pHorizontalScrollBar->SetScrollPos(0);
                rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
            }
        }
    }
    UINT uListType = m_pOwner->GetListType();
    if(uListType == LT_LIST)
    {
        // 计算横向尺寸
        int nItemCount = m_items.size();
        if(nItemCount > 0)
        {
            std::shared_ptr<CControlUI> pControl = *m_items.begin();
            int nFixedWidth = pControl->GetFixedWidth();
            if(nFixedWidth > 0)
            {
                int nRank = (rc.right - rc.left) / nFixedWidth;
                if(nRank > 0)
                {
                    cyNeeded = ((nItemCount - 1) / nRank + 1) * pControl->GetFixedHeight();
                }
            }
        }
    }
    // Process the scrollbar
    ProcessScrollBar(rc, cxNeeded, cyNeeded);
}

void CListBodyUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pOwner) m_pOwner->DoEvent(event);
        else CVerticalLayoutUI::DoEvent(event);
        return;
    }

    CVerticalLayoutUI::DoEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CListHeaderUI)

CListHeaderUI::CListHeaderUI() :
    m_bIsScaleHeader(false) {}

faw::string_t CListHeaderUI::GetClass() const
{
    return _T("ListHeaderUI");
}

std::shared_ptr<CControlUI> CListHeaderUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LISTHEADER) return std::dynamic_pointer_cast<CListHeaderUI>(shared_from_this()); ;
    return CHorizontalLayoutUI::GetInterface(pstrName);
}

SIZE CListHeaderUI::EstimateSize(SIZE szAvailable)
{
    SIZE cXY = { 0, m_cxyFixed.cy };
    if(cXY.cy == 0 && m_pManager)
    {
        for(auto& pItem : m_items)
        {
            std::shared_ptr<CControlUI> pControl = pItem;
            cXY.cy = MAX(cXY.cy, pControl->EstimateSize(szAvailable).cy);
        }
        int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6;
        cXY.cy = MAX(cXY.cy, nMin);
    }

    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        cXY.cx += pItem->EstimateSize(szAvailable).cx;
    }

    if(cXY.cx < szAvailable.cx) cXY.cx = szAvailable.cx;
    return cXY;
}

void CListHeaderUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    if(m_items.size() == 0)
    {
        return;
    }


    // Determine the width of elements that are sizeable
    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

    int nAdjustables = 0;
    int cxFixed = 0;
    int nEstimateNum = 0;
    for(auto& pItem : m_items)
    {
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat()) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
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
        nEstimateNum++;
    }
    cxFixed += (nEstimateNum - 1) * m_iChildPadding;

    int cxExpand = 0;
    int cxNeeded = 0;
    if(nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosX = rc.left;

    int iAdjustable = 0;
    int cxFixedRemaining = cxFixed;

    int nHeaderWidth = GetWidth();
    CListUI::ptr pList = std::dynamic_pointer_cast<CListUI>(GetParent());
    if(pList)
    {
        std::shared_ptr<CScrollBarUI>  pVScroll = pList->GetVerticalScrollBar();
        if(pVScroll)
            nHeaderWidth -= pVScroll->GetWidth();
    }
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
        RECT rcPadding = pControl->GetPadding();
        szRemaining.cx -= rcPadding.left;

        SIZE sz = { 0, 0 };
        if(m_bIsScaleHeader)
        {
            CListHeaderItemUI::ptr pHeaderItem = std::dynamic_pointer_cast<CListHeaderItemUI>(pControl);
            sz.cx = int (nHeaderWidth * (float) pHeaderItem->GetScale() / 100);
        }
        else
        {
            sz = pControl->EstimateSize(szRemaining);
        }

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

        sz.cy = pControl->GetFixedHeight();
        if(sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
        if(sz.cy < 0) sz.cy = 0;
        if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
        if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

        RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy };
        pControl->SetPos(rcCtrl);
        iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
        cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
        szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
    }
    cxNeeded += (nEstimateNum - 1) * m_iChildPadding;
}

void CListHeaderUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("scaleheader")) SetScaleHeader(FawTools::parse_bool(pstrValue));
    else CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
}

void CListHeaderUI::SetScaleHeader(bool bIsScale)
{
    m_bIsScaleHeader = bIsScale;
}

bool CListHeaderUI::IsScaleHeader() const
{
    return m_bIsScaleHeader;
}
/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CListHeaderItemUI)

CListHeaderItemUI::CListHeaderItemUI() :
    m_bDragable(true), m_uButtonState(0), m_iSepWidth(4),
    m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), m_dwTextColor(0), m_iFont(-1), m_bShowHtml(false), m_nScale(0)
{
    SetTextPadding({ 2, 0, 2, 0 });
    ptLastMouse.x = ptLastMouse.y = 0;
    SetMinWidth(16);
}

faw::string_t CListHeaderItemUI::GetClass() const
{
    return _T("ListHeaderItemUI");
}

std::shared_ptr<CControlUI> CListHeaderItemUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LISTHEADERITEM) return std::dynamic_pointer_cast<CListHeaderItemUI>(shared_from_this()); ;
    return CContainerUI::GetInterface(pstrName);
}

UINT CListHeaderItemUI::GetControlFlags() const
{
    if(IsEnabled() && m_iSepWidth != 0) return UIFLAG_SETCURSOR;
    else return 0;
}

void CListHeaderItemUI::SetEnabled(bool bEnable)
{
    CContainerUI::SetEnabled(bEnable);
    if(!IsEnabled())
    {
        m_uButtonState = 0;
    }
}

bool CListHeaderItemUI::IsDragable() const
{
    return m_bDragable;
}

void CListHeaderItemUI::SetDragable(bool bDragable)
{
    m_bDragable = bDragable;
    if(!m_bDragable) m_uButtonState &= ~UISTATE_CAPTURED;
}

DWORD CListHeaderItemUI::GetSepWidth() const
{
    return m_iSepWidth;
}

void CListHeaderItemUI::SetSepWidth(int iWidth)
{
    m_iSepWidth = iWidth;
}

DWORD CListHeaderItemUI::GetTextStyle() const
{
    return m_uTextStyle;
}

void CListHeaderItemUI::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    Invalidate();
}

DWORD CListHeaderItemUI::GetTextColor() const
{
    return m_dwTextColor;
}


void CListHeaderItemUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
}

RECT CListHeaderItemUI::GetTextPadding() const
{
    return m_rcTextPadding;
}

void CListHeaderItemUI::SetTextPadding(RECT rc)
{
    m_rcTextPadding = rc;
    Invalidate();
}

void CListHeaderItemUI::SetFont(int index)
{
    m_iFont = index;
}

bool CListHeaderItemUI::IsShowHtml()
{
    return m_bShowHtml;
}

void CListHeaderItemUI::SetShowHtml(bool bShowHtml)
{
    if(m_bShowHtml == bShowHtml) return;

    m_bShowHtml = bShowHtml;
    Invalidate();
}

faw::string_t CListHeaderItemUI::GetNormalImage() const
{
    return m_sNormalImage;
}

void CListHeaderItemUI::SetNormalImage(faw::string_t pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

faw::string_t CListHeaderItemUI::GetHotImage() const
{
    return m_sHotImage;
}

void CListHeaderItemUI::SetHotImage(faw::string_t pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

faw::string_t CListHeaderItemUI::GetPushedImage() const
{
    return m_sPushedImage;
}

void CListHeaderItemUI::SetPushedImage(faw::string_t pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

faw::string_t CListHeaderItemUI::GetFocusedImage() const
{
    return m_sFocusedImage;
}

void CListHeaderItemUI::SetFocusedImage(faw::string_t pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

faw::string_t CListHeaderItemUI::GetSepImage() const
{
    return m_sSepImage;
}

void CListHeaderItemUI::SetSepImage(faw::string_t pStrImage)
{
    m_sSepImage = pStrImage;
    Invalidate();
}

void CListHeaderItemUI::SetScale(int nScale)
{
    m_nScale = nScale;
}

int CListHeaderItemUI::GetScale() const
{
    return m_nScale;
}

void CListHeaderItemUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("dragable")) SetDragable(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("sepwidth")) SetSepWidth(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("align"))
    {
        if(pstrValue.find(_T("left")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_uTextStyle |= DT_LEFT;
        }
        if(pstrValue.find(_T("center")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_uTextStyle |= DT_CENTER;
        }
        if(pstrValue.find(_T("right")) != faw::string_t::npos)
        {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_uTextStyle |= DT_RIGHT;
        }
    }
    else if(pstrName == _T("endellipsis"))
    {
        if(FawTools::parse_bool(pstrValue)) m_uTextStyle |= DT_END_ELLIPSIS;
        else m_uTextStyle &= ~DT_END_ELLIPSIS;
    }
    else if(pstrName == _T("font")) SetFont((int) FawTools::parse_hex(pstrValue));
    else if(pstrName == _T("textcolor"))
    {
        SetTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("textpadding"))
    {
        RECT rcTextPadding = FawTools::parse_rect(pstrValue);
        SetTextPadding(rcTextPadding);
    }
    else if(pstrName == _T("showhtml")) SetShowHtml(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("normalimage")) SetNormalImage(pstrValue);
    else if(pstrName == _T("hotimage")) SetHotImage(pstrValue);
    else if(pstrName == _T("pushedimage")) SetPushedImage(pstrValue);
    else if(pstrName == _T("focusedimage")) SetFocusedImage(pstrValue);
    else if(pstrName == _T("sepimage")) SetSepImage(pstrValue);
    else if(pstrName == _T("scale"))
    {
        SetScale(FawTools::parse_dec(pstrValue));

    }
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CListHeaderItemUI::DoEvent(TEventUI& event)
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
    if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
    {
        if(!IsEnabled()) return;
        RECT rcSeparator = GetThumbRect();
        if(m_iSepWidth >= 0)
            rcSeparator.left -= 4;
        else
            rcSeparator.right += 4;
        if(::PtInRect(&rcSeparator, event.ptMouse))
        {
            if(m_bDragable)
            {
                m_uButtonState |= UISTATE_CAPTURED;
                ptLastMouse = event.ptMouse;
            }
        }
        else
        {
            m_uButtonState |= UISTATE_PUSHED;
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_HEADERCLICK);
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_BUTTONUP)
    {
        if((m_uButtonState & UISTATE_CAPTURED) != 0)
        {
            m_uButtonState &= ~UISTATE_CAPTURED;
            if(GetParent())
                GetParent()->NeedParentUpdate();
        }
        else if((m_uButtonState & UISTATE_PUSHED) != 0)
        {
            m_uButtonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_MOUSEMOVE)
    {
        if((m_uButtonState & UISTATE_CAPTURED) != 0)
        {
            RECT rc = m_rcItem;
            if(m_iSepWidth >= 0)
            {
                rc.right -= ptLastMouse.x - event.ptMouse.x;
            }
            else
            {
                rc.left -= ptLastMouse.x - event.ptMouse.x;
            }

            if(rc.right - rc.left > GetMinWidth())
            {
                m_cxyFixed.cx = rc.right - rc.left;
                ptLastMouse = event.ptMouse;
                if(GetParent())
                    GetParent()->NeedParentUpdate();
            }
        }
        return;
    }
    if(event.Type == UIEVENT_SETCURSOR)
    {
        RECT rcSeparator = GetThumbRect();
        if(m_iSepWidth >= 0)
            rcSeparator.left -= 4;
        else
            rcSeparator.right += 4;
        if(IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse))
        {
            ::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
            return;
        }
    }
    if(event.Type == UIEVENT_MOUSEENTER)
    {
        if(IsEnabled())
        {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_MOUSELEAVE)
    {
        if(IsEnabled())
        {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CContainerUI::DoEvent(event);
}

SIZE CListHeaderItemUI::EstimateSize(SIZE szAvailable)
{
    if(m_cxyFixed.cy == 0) return { m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14 };
    return CContainerUI::EstimateSize(szAvailable);
}

RECT CListHeaderItemUI::GetThumbRect() const
{
    if(m_iSepWidth >= 0) return { m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom };
    else return { m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom };
}

void CListHeaderItemUI::PaintStatusImage(HDC hDC)
{
    if(IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~UISTATE_FOCUSED;

    if((m_uButtonState & UISTATE_PUSHED) != 0)
    {
        if(m_sPushedImage.empty() && !m_sNormalImage.empty()) DrawImage(hDC, m_sNormalImage);
        if(!DrawImage(hDC, m_sPushedImage))
        {
        }
    }
    else if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(m_sHotImage.empty() && !m_sNormalImage.empty()) DrawImage(hDC, m_sNormalImage);
        if(!DrawImage(hDC, m_sHotImage))
        {
        }
    }
    else if((m_uButtonState & UISTATE_FOCUSED) != 0)
    {
        if(m_sFocusedImage.empty() && !m_sNormalImage.empty()) DrawImage(hDC, m_sNormalImage);
        if(!DrawImage(hDC, m_sFocusedImage))
        {
        }
    }
    else
    {
        if(!m_sNormalImage.empty())
        {
            if(!DrawImage(hDC, m_sNormalImage))
            {
            }
        }
    }

    if(!m_sSepImage.empty())
    {
        RECT rcThumb = GetThumbRect();
        rcThumb.left -= m_rcItem.left;
        rcThumb.top -= m_rcItem.top;
        rcThumb.right -= m_rcItem.left;
        rcThumb.bottom -= m_rcItem.top;

        m_sSepImageModify.clear();
        m_sSepImageModify = std::format(_T("dest='{},{},{},{}'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if(!DrawImage(hDC, m_sSepImage, m_sSepImageModify))
        {
        }
    }
}

void CListHeaderItemUI::PaintText(HDC hDC)
{
    if(m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();

    RECT rcText = m_rcItem;
    rcText.left += m_rcTextPadding.left;
    rcText.top += m_rcTextPadding.top;
    rcText.right -= m_rcTextPadding.right;
    rcText.bottom -= m_rcTextPadding.bottom;

    faw::string_t sText = GetText();
    if(sText.empty()) return;
    int nLinks = 0;
    if(m_bShowHtml)
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, m_dwTextColor, nullptr, nullptr, nLinks, m_iFont, m_uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, m_dwTextColor, m_iFont, m_uTextStyle);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
CListElementUI::CListElementUI() :
    m_iIndex(-1),
    m_pOwner(nullptr),
    m_bSelected(false),
    m_uButtonState(0) {}

faw::string_t CListElementUI::GetClass() const
{
    return _T("ListElementUI");
}

UINT CListElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN;
}

std::shared_ptr<CControlUI> CListElementUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LISTITEM) return std::dynamic_pointer_cast<CListElementUI>(shared_from_this());
    if(pstrName == DUI_CTRL_LISTELEMENT) return std::dynamic_pointer_cast<CListElementUI>(shared_from_this());
    return CControlUI::GetInterface(pstrName);
}

std::shared_ptr<IListOwnerUI> CListElementUI::GetOwner()
{
    return m_pOwner;
}

void CListElementUI::SetOwner(std::shared_ptr<CControlUI> pOwner)
{
    m_pOwner = std::dynamic_pointer_cast<IListOwnerUI>(pOwner->GetInterface(_T("IListOwner")));
}

void CListElementUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if(!IsVisible() && m_bSelected)
    {
        m_bSelected = false;
        if(m_pOwner) m_pOwner->SelectItem(-1);
    }
}

void CListElementUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if(!IsEnabled())
    {
        m_uButtonState = 0;
    }
}

int CListElementUI::GetIndex() const
{
    return m_iIndex;
}

void CListElementUI::SetIndex(int iIndex)
{
    m_iIndex = iIndex;
}

void CListElementUI::Invalidate()
{
    if(!IsVisible()) return;

    if(GetParent())
    {
        CContainerUI::ptr pParentContainer = std::dynamic_pointer_cast<CContainerUI>(GetParent()->GetInterface(_T("Container")));
        if(pParentContainer)
        {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            std::shared_ptr<CScrollBarUI>  pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if(pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
            std::shared_ptr<CScrollBarUI>  pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if(pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if(!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
            {
                return;
            }

            std::shared_ptr<CControlUI> pParent = GetParent();
            RECT rcTemp = { 0 };
            RECT rcParent = { 0 };
            while(!!(pParent = pParent->GetParent()))
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if(!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
                {
                    return;
                }
            }

            if(m_pManager) m_pManager->Invalidate(invalidateRc);
        }
        else
        {
            CControlUI::Invalidate();
        }
    }
    else
    {
        CControlUI::Invalidate();
    }
}

bool CListElementUI::Activate()
{
    if(!CControlUI::Activate()) return false;
    if(m_pManager) m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMACTIVATE);
    return true;
}

bool CListElementUI::IsSelected() const
{
    return m_bSelected;
}

bool CListElementUI::Select(bool bSelect)
{
    if(!IsEnabled()) return false;
    if(m_pOwner && m_bSelected) m_pOwner->UnSelectItem(m_iIndex, true);
    if(bSelect == m_bSelected) return true;
    m_bSelected = bSelect;
    if(bSelect && m_pOwner) m_pOwner->SelectItem(m_iIndex);
    Invalidate();

    return true;
}

bool CListElementUI::SelectMulti(bool bSelect)
{
    if(!IsEnabled()) return false;
    if(bSelect == m_bSelected) return true;

    m_bSelected = bSelect;
    if(bSelect && m_pOwner) m_pOwner->SelectMultiItem(m_iIndex);
    Invalidate();
    return true;
}

bool CListElementUI::IsExpanded() const
{
    return false;
}

bool CListElementUI::Expand(bool /*bExpand = true*/)
{
    return false;
}

void CListElementUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pOwner) m_pOwner->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_DBLCLICK)
    {
        if(IsEnabled())
        {
            Activate();
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_KEYDOWN && IsEnabled())
    {
        if(event.chKey == VK_RETURN)
        {
            Activate();
            Invalidate();
            return;
        }
    }
    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if(m_pOwner) m_pOwner->DoEvent(event);
    else CControlUI::DoEvent(event);
}

void CListElementUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("selected")) Select();
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CListElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
    ASSERT(m_pOwner);
    if(!m_pOwner) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iBackColor = 0;
    if(!pInfo->bAlternateBk || m_iIndex % 2 == 0) iBackColor = pInfo->dwBkColor;
    if((m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHotBkColor > 0)
    {
        iBackColor = pInfo->dwHotBkColor;
    }
    if(IsSelected() && pInfo->dwSelectedBkColor > 0)
    {
        iBackColor = pInfo->dwSelectedBkColor;
    }
    if(!IsEnabled() && pInfo->dwDisabledBkColor > 0)
    {
        iBackColor = pInfo->dwDisabledBkColor;
    }

    if(iBackColor != 0)
    {
        CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(iBackColor));
    }

    if(!IsEnabled())
    {
        if(!pInfo->sDisabledImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sDisabledImage))
            {
            }
            else return;
        }
    }
    if(IsSelected())
    {
        if(!pInfo->sSelectedImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sSelectedImage))
            {
            }
            else return;
        }
    }
    if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(!pInfo->sHotImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sHotImage))
            {
            }
            else return;
        }
    }

    if(!m_sBkImage.empty())
    {
        if(!pInfo->bAlternateBk || m_iIndex % 2 == 0)
        {
            if(!DrawImage(hDC, m_sBkImage))
            {
            }
        }
    }

    if(m_sBkImage.empty())
    {
        if(!pInfo->sBkImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sBkImage))
            {
            }
            else return;
        }
    }

    if(pInfo->dwLineColor != 0)
    {
        if(pInfo->bShowRowLine)
        {
            RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
            CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
        }
        if(pInfo->bShowColumnLine)
        {
            for(int i = 0; i < pInfo->nColumns; i++)
            {
                RECT rcLine = { pInfo->rcColumn[i].right - 1, m_rcItem.top, pInfo->rcColumn[i].right - 1, m_rcItem.bottom };
                CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CListLabelElementUI)

CListLabelElementUI::CListLabelElementUI()
{
    SetMaxHeight(20);
}
CListLabelElementUI::CListLabelElementUI(faw::string_t text, int height)
{
    SetText(text);
    SetMaxHeight(height);
}

faw::string_t CListLabelElementUI::GetClass() const
{
    return _T("ListLabelElementUI");
}

std::shared_ptr<CControlUI> CListLabelElementUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LISTLABELELEMENT) return std::dynamic_pointer_cast<CListLabelElementUI>(shared_from_this());
    return CListElementUI::GetInterface(pstrName);
}

void CListLabelElementUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pOwner) m_pOwner->DoEvent(event);
        else CListElementUI::DoEvent(event);
        return;
    }

    // 右键选择
    if(m_pOwner)
    {
        if(m_pOwner->GetListInfo()->bRSelected && event.Type == UIEVENT_RBUTTONDOWN)
        {
            if(IsEnabled())
            {
                if((GetKeyState(VK_CONTROL) & 0x8000))
                {
                    SelectMulti(!IsSelected());
                }
                else Select();
            }
            return;
        }
    }

    if(event.Type == UIEVENT_BUTTONDOWN)
    {
        if(IsEnabled())
        {
            if((GetKeyState(VK_CONTROL) & 0x8000))
            {
                SelectMulti(!IsSelected());
            }
            else
            {
                Select();
            }
        }
        return;
    }
    if(event.Type == UIEVENT_BUTTONUP)
    {
        if(IsEnabled())
        {
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMCLICK);
        }
        return;
    }

    if(event.Type == UIEVENT_MOUSEMOVE)
    {
        return;
    }

    if(event.Type == UIEVENT_MOUSEENTER)
    {
        if(IsEnabled())
        {
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
    CListElementUI::DoEvent(event);
}

SIZE CListLabelElementUI::EstimateSize(SIZE szAvailable)
{
    if(!m_pOwner) return { 0, 0 };
    faw::string_t sText = GetText();

    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    SIZE cXY = m_cxyFixed;
    if(cXY.cy == 0 && m_pManager)
    {
        cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + 8;
        cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
    }

    if(cXY.cx == 0 && m_pManager)
    {
        //RECT rcText = { 0, 0, 9999, cXY.cy };
        //if (pInfo->bShowHtml) {
        //	int nLinks = 0;
        //	CRenderEngine::DrawHtmlText (m_pManager->GetPaintDC (), m_pManager, rcText, sText, 0, nullptr, nullptr, nLinks, pInfo->nFont, DT_SINGLELINE | DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
        //} else {
        //	CRenderEngine::DrawText (m_pManager->GetPaintDC (), m_pManager, rcText, sText, 0, pInfo->nFont, DT_SINGLELINE | DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
        //}
        //cXY.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right;
        cXY.cx = szAvailable.cx;
    }

    return cXY;
}

bool CListLabelElementUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    DrawItemBk(hDC, m_rcItem);
    DrawItemText(hDC, m_rcItem);
    return true;
}

void CListLabelElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    faw::string_t sText = GetText();
    if(sText.empty()) return;

    if(!m_pOwner) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iTextColor = pInfo->dwTextColor;
    if((m_uButtonState & UISTATE_HOT) != 0)
    {
        iTextColor = pInfo->dwHotTextColor;
    }
    if(IsSelected())
    {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if(!IsEnabled())
    {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    int nLinks = 0;
    RECT rcText = rcItem;
    rcText.left += pInfo->rcTextPadding.left;
    rcText.right -= pInfo->rcTextPadding.right;
    rcText.top += pInfo->rcTextPadding.top;
    rcText.bottom -= pInfo->rcTextPadding.bottom;

    if(pInfo->bShowHtml)
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, iTextColor, nullptr, nullptr, nLinks, pInfo->nFont, pInfo->uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, iTextColor, pInfo->nFont, pInfo->uTextStyle);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CListTextElementUI)

CListTextElementUI::CListTextElementUI() :
    m_nLinks(0), m_nHoverLink(-1), m_pOwner(nullptr)
{
    ::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
}

CListTextElementUI::~CListTextElementUI()
{
    faw::string_t* pText;
    for(int it = 0; it < m_aTexts.GetSize(); it++)
    {
        pText = static_cast<faw::string_t*>(m_aTexts[it]);
        if(pText) delete pText;
    }
    m_aTexts.Clear();
}

faw::string_t CListTextElementUI::GetClass() const
{
    return _T("ListTextElementUI");
}

std::shared_ptr<CControlUI> CListTextElementUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LISTTEXTELEMENT) return std::dynamic_pointer_cast<CListTextElementUI>(shared_from_this());
    return CListLabelElementUI::GetInterface(pstrName);
}

UINT CListTextElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN | ((IsEnabled() && m_nLinks > 0) ? UIFLAG_SETCURSOR : 0);
}

faw::string_t CListTextElementUI::GetText(int iIndex) const
{
    faw::string_t* pText = static_cast<faw::string_t*>(m_aTexts.GetAt(iIndex));
    if(pText)
    {
        if(!IsResourceText())
            return pText->c_str();
        return CResourceManager::GetInstance()->GetText(*pText);
    }
    return _T("");
}

void CListTextElementUI::SetText(int iIndex, faw::string_t pstrText)
{
    if(!m_pOwner) return;

    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    if(iIndex < 0 || iIndex >= pInfo->nColumns) return;
    while(m_aTexts.GetSize() < pInfo->nColumns)
    {
        m_aTexts.Add(nullptr);
    }

    faw::string_t* pText = static_cast<faw::string_t*>(m_aTexts[iIndex]);
    if((!pText && pstrText.empty()) || (pText && *pText == pstrText)) return;

    if(pText)
    {
        delete pText;
        pText = nullptr;
    }
    m_aTexts.SetAt(iIndex, new faw::string_t (pstrText));

    Invalidate();
}

void CListTextElementUI::SetOwner(std::shared_ptr<CControlUI> pOwner)
{
    CListElementUI::SetOwner(pOwner);
    m_pOwner = std::dynamic_pointer_cast<IListUI>(pOwner->GetInterface(_T("IList")));
}

faw::string_t* CListTextElementUI::GetLinkContent(int iIndex)
{
    if(iIndex >= 0 && iIndex < m_nLinks) return &m_sLinks[iIndex];
    return nullptr;
}

void CListTextElementUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pOwner) m_pOwner->DoEvent(event);
        else CListLabelElementUI::DoEvent(event);
        return;
    }

    // When you hover over a link
    if(event.Type == UIEVENT_SETCURSOR)
    {
        for(int i = 0; i < m_nLinks; i++)
        {
            if(::PtInRect(&m_rcLinks[i], event.ptMouse))
            {
                ::SetCursor(::LoadCursor(NULL, IDC_HAND));
                return;
            }
        }
    }
    if(event.Type == UIEVENT_BUTTONUP && IsEnabled())
    {
        for(int i = 0; i < m_nLinks; i++)
        {
            if(::PtInRect(&m_rcLinks[i], event.ptMouse))
            {
                m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_LINK, i);
                return;
            }
        }
    }
    if(m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE)
    {
        int nHoverLink = -1;
        for(int i = 0; i < m_nLinks; i++)
        {
            if(::PtInRect(&m_rcLinks[i], event.ptMouse))
            {
                nHoverLink = i;
                break;
            }
        }

        if(m_nHoverLink != nHoverLink)
        {
            Invalidate();
            m_nHoverLink = nHoverLink;
        }
    }
    if(m_nLinks > 0 && event.Type == UIEVENT_MOUSELEAVE)
    {
        if(m_nHoverLink != -1)
        {
            Invalidate();
            m_nHoverLink = -1;
        }
    }
    CListLabelElementUI::DoEvent(event);
}

SIZE CListTextElementUI::EstimateSize(SIZE szAvailable)
{
    TListInfoUI* pInfo = nullptr;
    if(m_pOwner) pInfo = m_pOwner->GetListInfo();

    SIZE cXY = m_cxyFixed;
    if(cXY.cy == 0 && m_pManager)
    {
        cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + 8;
        if(pInfo) cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
    }

    return cXY;
}

void CListTextElementUI::DrawItemText(HDC hDC, const RECT& /*rcItem*/)
{
    if(!m_pOwner) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iTextColor = pInfo->dwTextColor;

    if((m_uButtonState & UISTATE_HOT) != 0)
    {
        iTextColor = pInfo->dwHotTextColor;
    }
    if(IsSelected())
    {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if(!IsEnabled())
    {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    IListCallbackUI* pCallback = m_pOwner->GetTextCallback();

    m_nLinks = 0;
    int nLinks = lengthof(m_rcLinks);
    for(int i = 0; i < pInfo->nColumns; i++)
    {
        RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
        rcItem.left += pInfo->rcTextPadding.left;
        rcItem.right -= pInfo->rcTextPadding.right;
        rcItem.top += pInfo->rcTextPadding.top;
        rcItem.bottom -= pInfo->rcTextPadding.bottom;

        faw::string_t strText;
        if(pCallback)
            strText = pCallback->GetItemText(shared_from_this(), m_iIndex, i);
        else
            strText = GetText(i);

        if(pInfo->bShowHtml)
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rcItem, strText, iTextColor, &m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, pInfo->nFont, pInfo->uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rcItem, strText, iTextColor, pInfo->nFont, pInfo->uTextStyle);

        m_nLinks += nLinks;
        nLinks = lengthof(m_rcLinks) - m_nLinks;
    }
    for(int i = m_nLinks; i < lengthof(m_rcLinks); i++)
    {
        ::ZeroMemory(m_rcLinks + i, sizeof(RECT));
        ((faw::string_t*)(m_sLinks + i))->clear();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CListContainerElementUI)

CListContainerElementUI::CListContainerElementUI() :
    m_iIndex(-1),
    m_pOwner(nullptr),
    m_bSelected(false),
    m_uButtonState(0) {}

faw::string_t CListContainerElementUI::GetClass() const
{
    return _T("ListContainerElementUI");
}

UINT CListContainerElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN;
}

std::shared_ptr<CControlUI> CListContainerElementUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_LISTITEM) return std::dynamic_pointer_cast<CListContainerElementUI>(shared_from_this());
    if(pstrName == DUI_CTRL_LISTCONTAINERELEMENT) return std::dynamic_pointer_cast<CListContainerElementUI>(shared_from_this());
    return CContainerUI::GetInterface(pstrName);
}

std::shared_ptr<IListOwnerUI> CListContainerElementUI::GetOwner()
{
    return m_pOwner;
}

void CListContainerElementUI::SetOwner(std::shared_ptr<CControlUI> pOwner)
{
    m_pOwner = std::dynamic_pointer_cast<IListOwnerUI>(pOwner->GetInterface(_T("IListOwner")));
}

void CListContainerElementUI::SetVisible(bool bVisible)
{
    CContainerUI::SetVisible(bVisible);
    if(!IsVisible() && m_bSelected)
    {
        m_bSelected = false;
        if(m_pOwner) m_pOwner->SelectItem(-1);
    }
}

void CListContainerElementUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if(!IsEnabled())
    {
        m_uButtonState = 0;
    }
}

int CListContainerElementUI::GetIndex() const
{
    return m_iIndex;
}

void CListContainerElementUI::SetIndex(int iIndex)
{
    m_iIndex = iIndex;
}

void CListContainerElementUI::Invalidate()
{
    if(!IsVisible()) return;

    if(GetParent())
    {
        CContainerUI::ptr pParentContainer = std::dynamic_pointer_cast<CContainerUI>(GetParent()->GetInterface(_T("Container")));
        if(pParentContainer)
        {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            std::shared_ptr<CScrollBarUI>  pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if(pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
            std::shared_ptr<CScrollBarUI>  pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if(pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if(!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
            {
                return;
            }

            std::shared_ptr<CControlUI> pParent = GetParent();
            RECT rcTemp = { 0 };
            RECT rcParent = { 0 };
            while(!!(pParent = pParent->GetParent()))
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if(!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
                {
                    return;
                }
            }

            if(m_pManager) m_pManager->Invalidate(invalidateRc);
        }
        else
        {
            CContainerUI::Invalidate();
        }
    }
    else
    {
        CContainerUI::Invalidate();
    }
}

bool CListContainerElementUI::Activate()
{
    if(!CContainerUI::Activate()) return false;
    if(m_pManager) m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMACTIVATE);
    return true;
}

bool CListContainerElementUI::IsSelected() const
{
    return m_bSelected;
}

bool CListContainerElementUI::Select(bool bSelect)
{
    if(!IsEnabled()) return false;
    if(m_pOwner && m_bSelected) m_pOwner->UnSelectItem(m_iIndex, true);
    if(bSelect == m_bSelected) return true;
    m_bSelected = bSelect;
    if(bSelect && m_pOwner) m_pOwner->SelectItem(m_iIndex);
    Invalidate();

    return true;
}

bool CListContainerElementUI::SelectMulti(bool bSelect)
{
    if(!IsEnabled()) return false;
    if(bSelect == m_bSelected) return true;

    m_bSelected = bSelect;
    if(bSelect && m_pOwner) m_pOwner->SelectMultiItem(m_iIndex);
    Invalidate();
    return true;
}

bool CListContainerElementUI::IsExpanded() const
{
    return false;
}

bool CListContainerElementUI::Expand(bool /*bExpand = true*/)
{
    return false;
}

void CListContainerElementUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pOwner) m_pOwner->DoEvent(event);
        else CContainerUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_DBLCLICK)
    {
        if(IsEnabled())
        {
            Activate();
            Invalidate();
        }
        return;
    }
    if(event.Type == UIEVENT_KEYDOWN && IsEnabled())
    {
        if(event.chKey == VK_RETURN)
        {
            Activate();
            Invalidate();
            return;
        }
    }
    if(event.Type == UIEVENT_BUTTONDOWN)
    {
        if(IsEnabled())
        {
            if((GetKeyState(VK_CONTROL) & 0x8000))
            {
                SelectMulti(!IsSelected());
            }
            else Select();
        }
        return;
    }
    // 右键选择
    if(m_pOwner)
    {
        if(m_pOwner->GetListInfo()->bRSelected && event.Type == UIEVENT_RBUTTONDOWN)
        {
            if(IsEnabled())
            {
                if((GetKeyState(VK_CONTROL) & 0x8000))
                {
                    SelectMulti(!IsSelected());
                }
                else Select();
            }
            return;
        }
    }

    if(event.Type == UIEVENT_BUTTONUP)
    {
        if(IsEnabled())
        {
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_ITEMCLICK);
        }
        return;
    }
    if(event.Type == UIEVENT_MOUSEMOVE)
    {
        return;
    }
    if(event.Type == UIEVENT_MOUSEENTER)
    {
        if(IsEnabled())
        {
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
    if(event.Type == UIEVENT_TIMER)
    {
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
        return;
    }

    if(event.Type == UIEVENT_CONTEXTMENU)
    {
        if(IsContextMenuUsed())
        {
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_MENU, event.wParam, event.lParam);
            return;
        }
    }
    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if(m_pOwner) m_pOwner->DoEvent(event);
    else CControlUI::DoEvent(event);
}

void CListContainerElementUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("selected")) Select();
    //else if (pstrName == _T("expandable")) SetExpandable (FawTools::parse_bool (pstrValue));
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

bool CListContainerElementUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    DrawItemBk(hDC, m_rcItem);
    return CContainerUI::DoPaint(hDC, rcPaint, pStopControl);
}

void CListContainerElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    return;
}

void CListContainerElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
    ASSERT(m_pOwner);
    if(!m_pOwner) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iBackColor = 0;
    if(!pInfo->bAlternateBk || m_iIndex % 2 == 0) iBackColor = pInfo->dwBkColor;

    if((m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHotBkColor > 0)
    {
        iBackColor = pInfo->dwHotBkColor;
    }
    if(IsSelected() && pInfo->dwSelectedBkColor > 0)
    {
        iBackColor = pInfo->dwSelectedBkColor;
    }
    if(!IsEnabled() && pInfo->dwDisabledBkColor > 0)
    {
        iBackColor = pInfo->dwDisabledBkColor;
    }
    if(iBackColor != 0)
    {
        CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(iBackColor));
    }

    if(!IsEnabled())
    {
        if(!pInfo->sDisabledImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sDisabledImage))
            {
            }
            else return;
        }
    }
    if(IsSelected())
    {
        if(!pInfo->sSelectedImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sSelectedImage))
            {
            }
            else return;
        }
    }
    if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(!pInfo->sHotImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sHotImage))
            {
            }
            else return;
        }
    }
    if(!m_sBkImage.empty())
    {
        if(!pInfo->bAlternateBk || m_iIndex % 2 == 0)
        {
            if(!DrawImage(hDC, m_sBkImage))
            {
            }
        }
    }

    if(m_sBkImage.empty())
    {
        if(!pInfo->sBkImage.empty())
        {
            if(!DrawImage(hDC, pInfo->sBkImage))
            {
            }
            else return;
        }
    }

    if(pInfo->dwLineColor != 0)
    {
        if(pInfo->bShowRowLine)
        {
            RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
            CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
        }
        if(pInfo->bShowColumnLine)
        {
            for(int i = 0; i < pInfo->nColumns; i++)
            {
                RECT rcLine = { pInfo->rcColumn[i].right - 1, m_rcItem.top, pInfo->rcColumn[i].right - 1, m_rcItem.bottom };
                CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
            }
        }
    }
}

void CListContainerElementUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    if(!m_pOwner) return;

    UINT uListType = m_pOwner->GetListType();
    if(uListType == LT_LIST)
    {
        int nFixedWidth = GetFixedWidth();
        if(nFixedWidth > 0)
        {
            int nRank = (rc.right - rc.left) / nFixedWidth;
            if(nRank > 0)
            {
                int nIndex = GetIndex();
                int nfloor = nIndex / nRank;
                int nHeight = rc.bottom - rc.top;

                rc.top = rc.top - nHeight * (nIndex - nfloor);
                rc.left = rc.left + nFixedWidth * (nIndex % nRank);
                rc.right = rc.left + nFixedWidth;
                rc.bottom = nHeight + rc.top;
            }
        }
    }
    CHorizontalLayoutUI::SetPos(rc, bNeedInvalidate);

    if(uListType != LT_LIST && uListType != LT_TREE) return;
    auto pList = std::dynamic_pointer_cast<CListUI>(m_pOwner);
    if(uListType == LT_TREE)
    {
        pList = std::dynamic_pointer_cast<CListUI>(pList->CControlUI::GetInterface(_T("List")));
        if(!pList) return;
    }

    auto pHeader = pList->GetHeader();
    if(!pHeader || !pHeader->IsVisible()) return;
    int Index = -1;
    for(auto& pItem : m_items)
    {
        Index++;
        std::shared_ptr<CControlUI> pListItem = pItem;
        auto pHeaderItem = pHeader->GetItemAt(Index);
        if(!pHeaderItem) return;
        RECT rcHeaderItem = pHeaderItem->GetPos();
        if(pListItem && !(rcHeaderItem.left == 0 && rcHeaderItem.right == 0))
        {
            RECT rt = pListItem->GetPos();
            rt.left = rcHeaderItem.left;
            rt.right = rcHeaderItem.right;
            pListItem->SetPos(rt);
        }
    }
}
} // namespace DuiLib
