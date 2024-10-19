﻿#include "StdAfx.h"
#include "UIButton.h"
#include <math.h>

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CButtonUI)

CButtonUI::CButtonUI()
{
    m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
    m_bCalcByParent = true;
}

faw::string_t CButtonUI::GetClass() const
{
    return _T("ButtonUI");
}

std::shared_ptr<CControlUI> CButtonUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_BUTTON) return std::dynamic_pointer_cast<CButtonUI>(shared_from_this());
    return CLabelUI::GetInterface(pstrName);
}

UINT CButtonUI::GetControlFlags() const
{
    return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
}

void CButtonUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_SETFOCUS)
    {
        Invalidate();
    }
    else if(event.Type == UIEVENT_KILLFOCUS)
    {
        Invalidate();
    }
    else if(event.Type == UIEVENT_KEYDOWN)
    {
        if(IsKeyboardEnabled())
        {
            if(event.chKey == VK_SPACE || event.chKey == VK_RETURN)
            {
                Activate();
                return;
            }
        }
    }
    else if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
    {
        if(::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled())
        {
            m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    else if(event.Type == UIEVENT_MOUSEMOVE)
    {
        if((m_uButtonState & UISTATE_CAPTURED) != 0)
        {
            if(::PtInRect(&m_rcItem, event.ptMouse)) m_uButtonState |= UISTATE_PUSHED;
            else m_uButtonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    else if(event.Type == UIEVENT_BUTTONUP)
    {
        if((m_uButtonState & UISTATE_CAPTURED) != 0)
        {
            m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
            Invalidate();
            if(::PtInRect(&m_rcItem, event.ptMouse)) Activate();
        }
        return;
    }
    else if(event.Type == UIEVENT_CONTEXTMENU)
    {
        if(IsContextMenuUsed())
        {
            m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_MENU, event.wParam, event.lParam);
        }
        return;
    }
    else if(event.Type == UIEVENT_MOUSEENTER)
    {
        if(IsEnabled())
        {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
    }
    else if(event.Type == UIEVENT_MOUSELEAVE)
    {
        if(IsEnabled())
        {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
    }
    CLabelUI::DoEvent(event);
}

bool CButtonUI::Activate()
{
    if(!CControlUI::Activate()) return false;
    if(m_pManager)
    {
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_CLICK);
        BindTriggerTabSel();
    }
    return true;
}

void CButtonUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if(!IsEnabled())
    {
        m_uButtonState = UISTATE_DISABLED;
    }
    else
    {
        m_uButtonState = 0;
    }
}


void CButtonUI::SetHotFont(int index)
{
    m_iHotFont = index;
    Invalidate();
}

int CButtonUI::GetHotFont() const
{
    return m_iHotFont;
}

void CButtonUI::SetPushedFont(int index)
{
    m_iPushedFont = index;
    Invalidate();
}

int CButtonUI::GetPushedFont() const
{
    return m_iPushedFont;
}

void CButtonUI::SetFocusedFont(int index)
{
    m_iFocusedFont = index;
    Invalidate();
}

int CButtonUI::GetFocusedFont() const
{
    return m_iFocusedFont;
}

//alpha=1.5 / 0.8 etc...
void CButtonUI::adjustBrightness(DWORD& dwPixel, float alpha)
{
    // 获取像素的 RGB 分量
    BYTE r = GetRValue(dwPixel);
    BYTE g = GetGValue(dwPixel);
    BYTE b = GetBValue(dwPixel);

    // 计算亮度值
    int max_val = max(max(r, g), b);
    int min_val = min(min(r, g), b);
    int l = (max_val + min_val) / 2;

    // 计算亮度的变化量
    int delta_l = alpha * (255 - l);

    // 对每个通道进行调整
    r = min(255, max(0, r + delta_l));
    g = min(255, max(0, g + delta_l));
    b = min(255, max(0, b + delta_l));

    // 更新像素的数值
    dwPixel = RGB(r, g, b);
}

void CButtonUI::SetBtnBkColor(DWORD dwBackColor)
{
    if(m_dwBackColor == dwBackColor) return;

    m_dwBackColor = dwBackColor;
    Invalidate();
}

void CButtonUI::SetHotBkColor(DWORD dwColor)
{
    m_dwHotBkColor = dwColor;
    Invalidate();
}

DWORD CButtonUI::GetHotBkColor() const
{
    return m_dwHotBkColor;
}

void CButtonUI::SetPushedBkColor(DWORD dwColor)
{
    m_dwPushedBkColor = dwColor;
    Invalidate();
}

DWORD CButtonUI::GetPushedBkColor() const
{
    return m_dwPushedBkColor;
}

void CButtonUI::SetDisabledBkColor(DWORD dwColor)
{
    m_dwDisabledBkColor = dwColor;
    Invalidate();
}

DWORD CButtonUI::GetDisabledBkColor() const
{
    return m_dwDisabledBkColor;
}

void CButtonUI::SetHotTextColor(DWORD dwColor)
{
    m_dwHotTextColor = dwColor;
}

DWORD CButtonUI::GetHotTextColor() const
{
    return m_dwHotTextColor;
}

void CButtonUI::SetPushedTextColor(DWORD dwColor)
{
    m_dwPushedTextColor = dwColor;
}

DWORD CButtonUI::GetPushedTextColor() const
{
    return m_dwPushedTextColor;
}

void CButtonUI::SetFocusedTextColor(DWORD dwColor)
{
    m_dwFocusedTextColor = dwColor;
}

DWORD CButtonUI::GetFocusedTextColor() const
{
    return m_dwFocusedTextColor;
}

faw::string_t CButtonUI::GetNormalImage()
{
    return m_sNormalImage;
}

void CButtonUI::SetNormalImage(faw::string_t pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

faw::string_t CButtonUI::GetHotImage()
{
    return m_sHotImage;
}

void CButtonUI::SetHotImage(faw::string_t pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

faw::string_t CButtonUI::GetPushedImage()
{
    return m_sPushedImage;
}

void CButtonUI::SetPushedImage(faw::string_t pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

faw::string_t CButtonUI::GetFocusedImage()
{
    return m_sFocusedImage;
}

void CButtonUI::SetFocusedImage(faw::string_t pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

faw::string_t CButtonUI::GetDisabledImage()
{
    return m_sDisabledImage;
}

void CButtonUI::SetDisabledImage(faw::string_t pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

faw::string_t CButtonUI::GetHotForeImage()
{
    return m_sHotForeImage;
}

void CButtonUI::SetHotForeImage(faw::string_t pStrImage)
{
    m_sHotForeImage = pStrImage;
    Invalidate();
}

void CButtonUI::SetStateCount(int nCount)
{
    m_nStateCount = nCount;
    Invalidate();
}

int CButtonUI::GetStateCount() const
{
    return m_nStateCount;
}

faw::string_t CButtonUI::GetStateImage()
{
    return m_sStateImage;
}

void CButtonUI::SetStateImage(faw::string_t pStrImage)
{
    m_sNormalImage.clear();
    m_sStateImage = pStrImage;
    Invalidate();
}

void CButtonUI::BindTabIndex(int _BindTabIndex)
{
    if(_BindTabIndex >= 0)
        m_iBindTabIndex = _BindTabIndex;
}

void CButtonUI::BindTabLayoutName(faw::string_t _TabLayoutName)
{
    if(!_TabLayoutName.empty())
        m_sBindTabLayoutName = _TabLayoutName;
}

void CButtonUI::BindTriggerTabSel(int _SetSelectIndex /*= -1*/)
{
    faw::string_t pstrName = GetBindTabLayoutName();
    if(pstrName.empty() || (GetBindTabLayoutIndex() < 0 && _SetSelectIndex < 0))
        return;

    CTabLayoutUI::ptr pTabLayout = std::dynamic_pointer_cast<CTabLayoutUI>(GetManager()->FindControl(pstrName));
    if(!pTabLayout) return;
    pTabLayout->SelectItem(_SetSelectIndex >= 0 ? _SetSelectIndex : GetBindTabLayoutIndex());
}

void CButtonUI::RemoveBindTabIndex()
{
    m_iBindTabIndex = -1;
    m_sBindTabLayoutName.clear();
}

int CButtonUI::GetBindTabLayoutIndex()
{
    return m_iBindTabIndex;
}

faw::string_t CButtonUI::GetBindTabLayoutName()
{
    return m_sBindTabLayoutName;
}

void CButtonUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("normalimage")) SetNormalImage(pstrValue);
    else if(pstrName == _T("hotimage")) SetHotImage(pstrValue);
    else if(pstrName == _T("pushedimage")) SetPushedImage(pstrValue);
    else if(pstrName == _T("focusedimage")) SetFocusedImage(pstrValue);
    else if(pstrName == _T("disabledimage")) SetDisabledImage(pstrValue);
    else if(pstrName == _T("hotforeimage")) SetHotForeImage(pstrValue);
    else if(pstrName == _T("stateimage")) SetStateImage(pstrValue);
    else if(pstrName == _T("statecount")) SetStateCount(_ttoi(pstrValue.data()));
    else if(pstrName == _T("bindtabindex")) BindTabIndex(_ttoi(pstrValue.data()));
    else if(pstrName == _T("bindtablayoutname")) BindTabLayoutName(pstrValue);
    else if(pstrName == _T("bkcolor") || pstrName == _T("bkcolor1"))
    {
        SetBtnBkColor((DWORD)FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("hotbkcolor"))
    {
        SetHotBkColor((DWORD)FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("pushedbkcolor"))
    {
        SetPushedBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("disabledbkcolor"))
    {
        SetDisabledBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("hottextcolor"))
    {
        SetHotTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("pushedtextcolor"))
    {
        SetPushedTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("focusedtextcolor"))
    {
        SetFocusedTextColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("hotfont")) SetHotFont(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("pushedfont")) SetPushedFont(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("focuedfont")) SetFocusedFont(FawTools::parse_dec(pstrValue));

    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CButtonUI::PaintText(HDC hDC)
{
    if(IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~UISTATE_FOCUSED;
    if(!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~UISTATE_DISABLED;

    if(m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if(m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    faw::string_t sText = GetText();
    if(sText.empty()) return;

    RECT _rcTextPadding = CButtonUI::m_rcTextPadding;
    GetManager()->GetDPIObj()->Scale(&_rcTextPadding);
    int nLinks = 0;
    RECT rc = m_rcItem;
    rc.left += _rcTextPadding.left;
    rc.right -= _rcTextPadding.right;
    rc.top += _rcTextPadding.top;
    rc.bottom -= _rcTextPadding.bottom;

    DWORD clrColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;

    if(((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0))
        clrColor = GetPushedTextColor();
    else if(((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0))
        clrColor = GetHotTextColor();
    else if(((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0))
        clrColor = GetFocusedTextColor();

    int iFont = GetFont();
    if(((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedFont() != -1))
        iFont = GetPushedFont();
    else if(((m_uButtonState & UISTATE_HOT) != 0) && (GetHotFont() != -1))
        iFont = GetHotFont();
    else if(((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1))
        iFont = GetFocusedFont();

    if(m_bShowHtml)
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, clrColor, nullptr, nullptr, nLinks, iFont, m_uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rc, sText, clrColor, iFont, m_uTextStyle);
}

void CButtonUI::PaintBkSingleColor(HDC hDC, DWORD bkcolor, SIZE cBorderRound)
{
    if(bkcolor >= 0xFF000000) CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(bkcolor));
    else CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(bkcolor));
}

void CButtonUI::PaintBkColor(HDC hDC)
{
    SIZE cxyBorderRound = GetBorderRound();
    if((m_uButtonState & UISTATE_DISABLED) != 0)
    {
        if(m_dwDisabledBkColor != 0)
        {
            PaintBkSingleColor(hDC, m_dwDisabledBkColor, cxyBorderRound);
            return;
        }
    }
    else if((m_uButtonState & UISTATE_PUSHED) != 0)
    {
        if(m_dwPushedBkColor != 0)
        {
            PaintBkSingleColor(hDC, m_dwPushedBkColor, cxyBorderRound);
            return;
        }
    }
    else if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(m_dwHotBkColor != 0)
        {
            PaintBkSingleColor(hDC, m_dwHotBkColor, cxyBorderRound);
            return;
        }
    }
    CControlUI::PaintBkColor(hDC);
}

void CButtonUI::PaintStatusImage(HDC hDC)
{
    if(!m_sStateImage.empty() && m_nStateCount > 0)
    {
        TDrawInfo info;
        info.Parse(m_sStateImage, _T(""), m_pManager);
        const std::shared_ptr<TImageInfo> pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL);
        if(m_sNormalImage.empty() && pImage)
        {
            SIZE szImage = { pImage->nX, pImage->nY };
            SIZE szStatus = { pImage->nX / m_nStateCount, pImage->nY };
            if(szImage.cx > 0 && szImage.cy > 0)
            {
                RECT rcSrc = { 0, 0, szImage.cx, szImage.cy };
                if(m_nStateCount > 0)
                {
                    int iLeft = rcSrc.left + 0 * szStatus.cx;
                    int iRight = iLeft + szStatus.cx;
                    int iTop = rcSrc.top;
                    int iBottom = iTop + szStatus.cy;
                    m_sNormalImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                }
                if(m_nStateCount > 1)
                {
                    int iLeft = rcSrc.left + 1 * szStatus.cx;
                    int iRight = iLeft + szStatus.cx;
                    int iTop = rcSrc.top;
                    int iBottom = iTop + szStatus.cy;
                    m_sHotImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                    m_sPushedImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                }
                if(m_nStateCount > 2)
                {
                    int iLeft = rcSrc.left + 2 * szStatus.cx;
                    int iRight = iLeft + szStatus.cx;
                    int iTop = rcSrc.top;
                    int iBottom = iTop + szStatus.cy;
                    m_sPushedImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                }
                if(m_nStateCount > 3)
                {
                    int iLeft = rcSrc.left + 3 * szStatus.cx;
                    int iRight = iLeft + szStatus.cx;
                    int iTop = rcSrc.top;
                    int iBottom = iTop + szStatus.cy;
                    m_sDisabledImage = std::format(_T("res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'"), info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
                }
            }
        }
    }

    if(IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~UISTATE_FOCUSED;
    if(!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~UISTATE_DISABLED;
    if(!::IsWindowEnabled(m_pManager->GetPaintWindow()))
    {
        m_uButtonState &= UISTATE_DISABLED;
    }
    if((m_uButtonState & UISTATE_DISABLED) != 0)
    {
        if(!m_sDisabledImage.empty())
        {
            if(!DrawImage(hDC, m_sDisabledImage))
            {
            }
            else return;
        }
    }
    else if((m_uButtonState & UISTATE_PUSHED) != 0)
    {
        if(!m_sPushedImage.empty())
        {
            if(!DrawImage(hDC, m_sPushedImage))
            {
            }
            else return;
        }
    }
    else if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(!m_sHotImage.empty())
        {
            if(!DrawImage(hDC, m_sHotImage))
            {
            }
            else return;
        }
    }
    else if((m_uButtonState & UISTATE_FOCUSED) != 0)
    {
        if(!m_sFocusedImage.empty())
        {
            if(!DrawImage(hDC, m_sFocusedImage))
            {
            }
            else return;
        }
    }

    if(!m_sNormalImage.empty())
    {
        if(!DrawImage(hDC, m_sNormalImage))
        {
        }
    }
}

void CButtonUI::PaintForeImage(HDC hDC)
{
    if((m_uButtonState & UISTATE_PUSHED) != 0)
    {
        if(!m_sPushedForeImage.empty())
        {
            if(!DrawImage(hDC, m_sPushedForeImage))
            {
            }
            else return;
        }
    }
    else if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(!m_sHotForeImage.empty())
        {
            if(!DrawImage(hDC, m_sHotForeImage))
            {
            }
            else return;
        }
    }
    if(!m_sForeImage.empty())
    {
        if(!DrawImage(hDC, m_sForeImage))
        {
        }
    }
}
}