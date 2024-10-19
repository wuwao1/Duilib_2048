﻿#include "StdAfx.h"
#include "UIOption.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(COptionUI)
COptionUI::COptionUI() {}

COptionUI::~COptionUI()
{
    if(!m_sGroupName.empty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, shared_from_this());
}

faw::string_t COptionUI::GetClass() const
{
    return _T("OptionUI");
}

std::shared_ptr<CControlUI> COptionUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_OPTION) return std::dynamic_pointer_cast<COptionUI>(shared_from_this());
    return CButtonUI::GetInterface(pstrName);
}

void COptionUI::SetManager(CPaintManagerUI* pManager, std::shared_ptr<CControlUI> pParent, bool bInit)
{
    CControlUI::SetManager(pManager, pParent, bInit);
    if(bInit && !m_sGroupName.empty())
    {
        if(m_pManager) m_pManager->AddOptionGroup(m_sGroupName, shared_from_this());
    }
}

faw::string_t COptionUI::GetGroup() const
{
    return m_sGroupName;
}

void COptionUI::SetGroup(faw::string_t pStrGroupName)
{
    if(pStrGroupName.empty())
    {
        if(m_sGroupName.empty()) return;
        m_sGroupName.clear();
    }
    else
    {
        if(m_sGroupName == pStrGroupName) return;
        if(!m_sGroupName.empty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, shared_from_this());
        m_sGroupName = pStrGroupName;
    }

    if(!m_sGroupName.empty())
    {
        if(m_pManager) m_pManager->AddOptionGroup(m_sGroupName, shared_from_this());
    }
    else
    {
        if(m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, shared_from_this());
    }

    Selected(m_bSelected);
}

bool COptionUI::IsSelected() const
{
    return m_bSelected;
}

void COptionUI::Selected(bool bSelected, bool bMsg/* = true*/)
{
    if(m_bSelected == bSelected) return;

    m_bSelected = bSelected;
    if(m_bSelected) m_uButtonState |= UISTATE_SELECTED;
    else m_uButtonState &= ~UISTATE_SELECTED;

    if(m_pManager)
    {
        if(!m_sGroupName.empty())
        {
            if(m_bSelected)
            {
                auto aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
                for(auto& Control : aOptionGroup)
                {
                    auto pControl = std::dynamic_pointer_cast<COptionUI>(Control);
                    if(pControl != shared_from_this())
                    {
                        pControl->Selected(false, bMsg);
                    }
                }
                if(bMsg)
                {
                    m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_SELECTCHANGED);
                }
            }
        }
        else
        {
            if(bMsg)
            {
                m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_SELECTCHANGED);
            }
        }
    }

    Invalidate();
}

bool COptionUI::Activate()
{
    if(!CButtonUI::Activate()) return false;
    if(!m_sGroupName.empty()) Selected(true);
    else Selected(!m_bSelected);

    return true;
}

void COptionUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if(!IsEnabled())
    {
        if(m_bSelected) m_uButtonState = UISTATE_DISABLED;
        else m_uButtonState = UISTATE_DISABLED;
    }
    else
    {
        m_uButtonState = 0;
    }
}

faw::string_t COptionUI::GetSelectedImage()
{
    return m_sSelectedImage;
}

void COptionUI::SetSelectedImage(faw::string_t pStrImage)
{
    m_sSelectedImage = pStrImage;
    Invalidate();
}

faw::string_t COptionUI::GetSelectedHotImage()
{
    return m_sSelectedHotImage;
}

void COptionUI::SetSelectedHotImage(faw::string_t pStrImage)
{
    m_sSelectedHotImage = pStrImage;
    Invalidate();
}

faw::string_t COptionUI::GetSelectedPushedImage()
{
    return m_sSelectedPushedImage;
}

void COptionUI::SetSelectedPushedImage(faw::string_t pStrImage)
{
    m_sSelectedPushedImage = pStrImage;
    Invalidate();
}

void COptionUI::SetSelectedTextColor(DWORD dwTextColor)
{
    m_dwSelectedTextColor = dwTextColor;
}

DWORD COptionUI::GetSelectedTextColor()
{
    if(m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
    return m_dwSelectedTextColor;
}

void COptionUI::SetSelectedBkColor(DWORD dwBkColor)
{
    m_dwSelectedBkColor = dwBkColor;
}

DWORD COptionUI::GetSelectedBkColor()
{
    return m_dwSelectedBkColor;
}

faw::string_t COptionUI::GetSelectedForedImage()
{
    return m_sSelectedForeImage;
}

void COptionUI::SetSelectedForedImage(faw::string_t pStrImage)
{
    m_sSelectedForeImage = pStrImage;
    Invalidate();
}

void COptionUI::SetSelectedStateCount(int nCount)
{
    m_nSelectedStateCount = nCount;
    Invalidate();
}

int COptionUI::GetSelectedStateCount() const
{
    return m_nSelectedStateCount;
}

faw::string_t COptionUI::GetSelectedStateImage()
{
    return m_sSelectedStateImage;
}

void COptionUI::SetSelectedStateImage(faw::string_t pStrImage)
{
    m_sSelectedStateImage = pStrImage;
    Invalidate();
}
void COptionUI::SetSelectedFont(int index)
{
    m_iSelectedFont = index;
    Invalidate();
}

int COptionUI::GetSelectedFont() const
{
    return m_iSelectedFont;
}
void COptionUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("group")) SetGroup(pstrValue);
    else if(pstrName == _T("selected")) Selected(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("selectedimage")) SetSelectedImage(pstrValue);
    else if(pstrName == _T("selectedhotimage")) SetSelectedHotImage(pstrValue);
    else if(pstrName == _T("selectedpushedimage")) SetSelectedPushedImage(pstrValue);
    else if(pstrName == _T("selectedforeimage")) SetSelectedForedImage(pstrValue);
    else if(pstrName == _T("selectedstateimage")) SetSelectedStateImage(pstrValue);
    else if(pstrName == _T("selectedstatecount")) SetSelectedStateCount(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("selectedbkcolor")) SetSelectedBkColor((DWORD) FawTools::parse_hex(pstrValue));
    else if(pstrName == _T("selectedtextcolor")) SetSelectedTextColor((DWORD) FawTools::parse_hex(pstrValue));
    else if(pstrName == _T("selectedfont")) SetSelectedFont(FawTools::parse_dec(pstrValue));
    else CButtonUI::SetAttribute(pstrName, pstrValue);
}

void COptionUI::PaintBkColor(HDC hDC)
{
    if(IsSelected())
    {
        if(m_dwSelectedBkColor != 0)
        {
            CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));

        }
    }
    else
    {
        return CButtonUI::PaintBkColor(hDC);
    }
}

void COptionUI::PaintStatusImage(HDC hDC)
{
    if(IsSelected())
    {
        if(!m_sSelectedStateImage.empty() && m_nSelectedStateCount > 0)
        {
            TDrawInfo info;
            info.Parse(m_sSelectedStateImage, _T(""), m_pManager);
            const std::shared_ptr<TImageInfo> pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL);
            if(m_sSelectedImage.empty() && pImage)
            {
                SIZE szImage = { pImage->nX, pImage->nY };
                SIZE szStatus = { pImage->nX / m_nSelectedStateCount, pImage->nY };
                if(szImage.cx > 0 && szImage.cy > 0)
                {
                    RECT rcSrc = { 0, 0, szImage.cx, szImage.cy };
                    if(m_nSelectedStateCount > 0)
                    {
                        int iLeft = rcSrc.left + 0 * szStatus.cx;
                        int iRight = iLeft + szStatus.cx;
                        int iTop = rcSrc.top;
                        int iBottom = iTop + szStatus.cy;
                        m_sSelectedImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                    }
                    if(m_nSelectedStateCount > 1)
                    {
                        int iLeft = rcSrc.left + 1 * szStatus.cx;
                        int iRight = iLeft + szStatus.cx;
                        int iTop = rcSrc.top;
                        int iBottom = iTop + szStatus.cy;
                        m_sSelectedHotImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                        m_sSelectedPushedImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                    }
                    if(m_nSelectedStateCount > 2)
                    {
                        int iLeft = rcSrc.left + 2 * szStatus.cx;
                        int iRight = iLeft + szStatus.cx;
                        int iTop = rcSrc.top;
                        int iBottom = iTop + szStatus.cy;
                        m_sSelectedPushedImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                    }
                }
            }
        }


        if((m_uButtonState & UISTATE_PUSHED) != 0 && !m_sSelectedPushedImage.empty())
        {
            if(!DrawImage(hDC, m_sSelectedPushedImage))
            {
            }
            else return;
        }
        else if((m_uButtonState & UISTATE_HOT) != 0 && !m_sSelectedHotImage.empty())
        {
            if(!DrawImage(hDC, m_sSelectedHotImage))
            {
            }
            else return;
        }

        if(!m_sSelectedImage.empty())
        {
            if(!DrawImage(hDC, m_sSelectedImage))
            {
            }
        }
    }
    else
    {
        CButtonUI::PaintStatusImage(hDC);
    }
}

void COptionUI::PaintForeImage(HDC hDC)
{
    if(IsSelected())
    {
        if(!m_sSelectedForeImage.empty())
        {
            if(!DrawImage(hDC, m_sSelectedForeImage))
            {
            }
            else return;
        }
    }

    return CButtonUI::PaintForeImage(hDC);
}

void COptionUI::PaintText(HDC hDC)
{
    if((m_uButtonState & UISTATE_SELECTED) != 0)
    {
        DWORD oldTextColor = m_dwTextColor;
        if(m_dwSelectedTextColor != 0) m_dwTextColor = m_dwSelectedTextColor;

        if(m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
        if(m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

        int iFont = GetFont();
        if(GetSelectedFont() != -1)
        {
            iFont = GetSelectedFont();
        }
        faw::string_t sText = GetText();
        if(sText.empty()) return;
        int nLinks = 0;
        RECT rc = m_rcItem;
        RECT _rcTextPadding = CButtonUI::m_rcTextPadding;
        GetManager()->GetDPIObj()->Scale(&_rcTextPadding);
        rc.left += _rcTextPadding.left;
        rc.right -= _rcTextPadding.right;
        rc.top += _rcTextPadding.top;
        rc.bottom -= _rcTextPadding.bottom;

        if(m_bShowHtml)
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor, \
                                        nullptr, nullptr, nLinks, iFont, m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, sText, IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor, \
                                    iFont, m_uTextStyle);

        m_dwTextColor = oldTextColor;
    }
    else
        CButtonUI::PaintText(hDC);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
IMPLEMENT_DUICONTROL(CCheckBoxUI)

CCheckBoxUI::CCheckBoxUI() :
    m_bAutoCheck(false)
{

}

faw::string_t CCheckBoxUI::GetClass() const
{
    return _T("CheckBoxUI");
}

std::shared_ptr<CControlUI> CCheckBoxUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_CHECKBOX) return std::dynamic_pointer_cast<CCheckBoxUI>(shared_from_this());
    return COptionUI::GetInterface(pstrName);
}

void CCheckBoxUI::SetCheck(bool bCheck)
{
    Selected(bCheck);
}

bool  CCheckBoxUI::GetCheck() const
{
    return IsSelected();
}

void CCheckBoxUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("EnableAutoCheck")) SetAutoCheck(FawTools::parse_bool(pstrValue));

    COptionUI::SetAttribute(pstrName, pstrValue);
}
void CCheckBoxUI::SetAutoCheck(bool bEnable)
{
    m_bAutoCheck = bEnable;
}
void CCheckBoxUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else COptionUI::DoEvent(event);
        return;
    }
    if(m_bAutoCheck && (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK))
    {
        if(::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled())
        {
            SetCheck(!GetCheck());
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_CHECKCLICK, 0, 0);
            Invalidate();
        }
        return;
    }
    COptionUI::DoEvent(event);
}
void CCheckBoxUI::Selected(bool bSelected, bool bMsg/* = true*/)
{
    if(m_bSelected == bSelected) return;
    m_bSelected = bSelected;
    if(m_bSelected) m_uButtonState |= UISTATE_SELECTED;
    else m_uButtonState &= ~UISTATE_SELECTED;

    if(m_pManager)
    {
        if(!m_sGroupName.empty())
        {
            if(m_bSelected)
            {
                auto aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
                for(auto& Control: aOptionGroup)
                {
                    auto pControl = std::dynamic_pointer_cast<COptionUI>(Control);
                    if(pControl != shared_from_this())
                    {
                        pControl->Selected(false, bMsg);
                    }
                }
                if(bMsg)
                {
                    m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_SELECTCHANGED, m_bSelected, 0);
                }
            }
        }
        else
        {
            if(bMsg)
            {
                m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_SELECTCHANGED, m_bSelected, 0);
            }
        }
    }

    Invalidate();
}
}