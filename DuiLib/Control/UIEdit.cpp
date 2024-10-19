﻿#include "StdAfx.h"
#include "UIEdit.h"

namespace DuiLib
{
class CEditWnd: public CWindowWnd
{
public:
    CEditWnd();

    void Init(CEditUI::ptr pOwner);
    RECT CalPos();

    LPCTSTR GetWindowClassName() const;
    LPCTSTR GetSuperClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    std::optional<LRESULT> OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
    std::optional<LRESULT> OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:

    CEditUI::ptr m_pOwner;
    HBRUSH m_hBkBrush;
    bool m_bInit;
    bool m_bDrawCaret;
};


CEditWnd::CEditWnd() :
    m_pOwner(nullptr), m_hBkBrush(NULL), m_bInit(false), m_bDrawCaret(false) {}

void CEditWnd::Init(CEditUI::ptr pOwner)
{
    m_pOwner = pOwner;
    RECT rcPos = CalPos();
    UINT uStyle = 0;
    if(m_pOwner->GetManager()->IsLayered())
    {
        uStyle = WS_POPUP | ES_AUTOHSCROLL | WS_VISIBLE;
        RECT rcWnd = { 0 };
        ::GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWnd);
        rcPos.left += rcWnd.left;
        rcPos.right += rcWnd.left;
        rcPos.top += rcWnd.top - 1;
        rcPos.bottom += rcWnd.top - 1;
    }
    else
    {
        uStyle = WS_CHILD | ES_AUTOHSCROLL;
    }
    UINT uTextStyle = m_pOwner->GetTextStyle();
    if(uTextStyle & DT_LEFT) uStyle |= ES_LEFT;
    else if(uTextStyle & DT_CENTER) uStyle |= ES_CENTER;
    else if(uTextStyle & DT_RIGHT) uStyle |= ES_RIGHT;
    if(m_pOwner->IsPasswordMode()) uStyle |= ES_PASSWORD;
    Create(m_pOwner->GetManager()->GetPaintWindow(), _T(""), uStyle, 0, rcPos);
    HFONT hFont = NULL;
    int iFontIndex = m_pOwner->GetFont();
    if(iFontIndex != -1)
        hFont = m_pOwner->GetManager()->GetFont(iFontIndex);
    if(hFont == NULL)
        hFont = m_pOwner->GetManager()->GetDefaultFontInfo()->hFont;

    SetWindowFont(m_hWnd, hFont, TRUE);
    Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
    if(m_pOwner->IsPasswordMode()) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());
    Edit_SetText(m_hWnd, m_pOwner->GetText().data());
    Edit_SetModify(m_hWnd, FALSE);
    SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
    Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
    Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);

    //Styls
    LONG styleValue = ::GetWindowLong(m_hWnd, GWL_STYLE);
    styleValue |= pOwner->GetWindowStyls();
    ::SetWindowLong(GetHWND(), GWL_STYLE, styleValue);
    //Styls
    ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
    ::SetFocus(m_hWnd);
    if(m_pOwner->IsAutoSelAll())
    {
        int nSize = GetWindowTextLength(m_hWnd);
        if(nSize == 0) nSize = 1;
        Edit_SetSel(m_hWnd, 0, nSize);
    }
    else
    {
        int nSize = GetWindowTextLength(m_hWnd);
        Edit_SetSel(m_hWnd, nSize, nSize);
    }

    m_bInit = true;
}

RECT CEditWnd::CalPos()
{
    RECT rcPos = m_pOwner->GetPos();
    RECT rcInset = m_pOwner->GetTextPadding();
    rcPos.left += rcInset.left;
    rcPos.top += rcInset.top;
    rcPos.right -= rcInset.right;
    rcPos.bottom -= rcInset.bottom;
    LONG lEditHeight = m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->tm.tmHeight;
    if(lEditHeight < rcPos.bottom - rcPos.top)
    {
        rcPos.top += (rcPos.bottom - rcPos.top - lEditHeight) / 2;
        rcPos.bottom = rcPos.top + lEditHeight;
    }

    std::shared_ptr<CControlUI> pParent = m_pOwner;
    RECT rcParent = { 0 };
    while(!!(pParent = pParent->GetParent()))
    {
        if(!pParent->IsVisible())
        {
            rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
            break;
        }
        rcParent = pParent->GetClientPos();
        if(!::IntersectRect(&rcPos, &rcPos, &rcParent))
        {
            rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
            break;
        }
    }

    return rcPos;
}

LPCTSTR CEditWnd::GetWindowClassName() const
{
    return _T("EditWnd");
}

LPCTSTR CEditWnd::GetSuperClassName() const
{
    return WC_EDIT;
}

void CEditWnd::OnFinalMessage(HWND hWnd)
{
    m_pOwner->Invalidate();
    // Clear reference and die
    if(m_hBkBrush) ::DeleteObject(m_hBkBrush);
    if(m_pOwner->GetManager()->IsLayered())
    {
        m_pOwner->GetManager()->RemoveNativeWindow(hWnd);
    }
    m_pOwner->m_pWindow = nullptr;
    delete this;
}

LRESULT CEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::optional<LRESULT> lRes = std::nullopt;
    if(uMsg == WM_CREATE)
    {
        //m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
        //if( m_pOwner->GetManager()->IsLayered() ) {
        //	::SetTimer(m_hWnd, DEFAULT_TIMERID, ::GetCaretBlinkTime(), nullptr);
        //}
    }
    else if(uMsg == WM_KILLFOCUS)
    {
        lRes = OnKillFocus(uMsg, wParam, lParam);
    }
    else if(uMsg == OCM_COMMAND)
    {
        if(GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
        {
            lRes = OnEditChanged(uMsg, wParam, lParam);
        }
        else if(GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE)
        {
            RECT rcClient = { 0 };
            ::GetClientRect(m_hWnd, &rcClient);
            ::InvalidateRect(m_hWnd, &rcClient, FALSE);
            lRes = 0;
        }
    }
    else if(uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN)
    {
        m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_RETURN);
        lRes = 0;
    }
    else if(uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_TAB)
    {
        if(m_pOwner->GetManager()->IsLayered())
        {
            m_pOwner->GetManager()->SetNextTabControl();
        }
        lRes = 0;
    }
    else if(uMsg == OCM__BASE + WM_CTLCOLOREDIT || uMsg == OCM__BASE + WM_CTLCOLORSTATIC)
    {
        //if (m_pOwner->GetManager()->IsLayered() && !m_pOwner->GetManager()->IsPainting()) {
        //	m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
        //}

        ::SetBkMode((HDC) wParam, TRANSPARENT);
        DWORD dwTextColor = m_pOwner->GetTextColor();
        ::SetTextColor((HDC) wParam, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
        DWORD clrColor = m_pOwner->GetBkColor();
        if(clrColor < 0xFF000000)
        {
            if(m_hBkBrush) ::DeleteObject(m_hBkBrush);
            RECT rcWnd = m_pOwner->GetManager()->GetNativeWindowRect(m_hWnd);
            HBITMAP hBmpEditBk = CRenderEngine::GenerateBitmap(m_pOwner->GetManager(), rcWnd, m_pOwner, clrColor);
            m_hBkBrush = ::CreatePatternBrush(hBmpEditBk);
            ::DeleteObject(hBmpEditBk);
        }
        else
        {
            if(!m_hBkBrush)
            {
                m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
            }
        }
        lRes = (LRESULT) m_hBkBrush;
    }
    else if(uMsg == WM_PAINT)
    {
        //if (m_pOwner->GetManager()->IsLayered()) {
        //	m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
        //}
    }
    else if(uMsg == WM_PRINT)
    {
        //if (m_pOwner->GetManager()->IsLayered()) {
        //	lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        //	if( m_pOwner->IsEnabled() && m_bDrawCaret ) {
        //		RECT rcClient = { 0 };
        //		::GetClientRect(m_hWnd, &rcClient);
        //		POINT ptCaret = { 0 };
        //		::GetCaretPos(&ptCaret);
        //		RECT rcCaret = { ptCaret.x, ptCaret.y, ptCaret.x, ptCaret.y+rcClient.bottom-rcClient.top };
        //		CRenderEngine::DrawLine((HDC)wParam, rcCaret, 1, 0xFF000000);
        //	}
        //	return lRes;
        //}
    }
    else if(uMsg == WM_TIMER)
    {
        if(wParam == CARET_TIMER_ID)
        {
            m_bDrawCaret = !m_bDrawCaret;
            RECT rcClient = { 0 };
            ::GetClientRect(m_hWnd, &rcClient);
            ::InvalidateRect(m_hWnd, &rcClient, FALSE);
            lRes = 0;
        }
    }

    if(lRes == std::nullopt)
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    return lRes.value();
}

std::optional<LRESULT> CEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    //if ((HWND)wParam != m_pOwner->GetManager()->GetPaintWindow()) {
    //	::SendMessage(m_pOwner->GetManager()->GetPaintWindow(), WM_KILLFOCUS, wParam, lParam);
    //}
    PostMessage(WM_CLOSE);
    return lRes;
}

std::optional<LRESULT> CEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if(!m_bInit) return 0;
    if(!m_pOwner) return 0;
    // Copy text back
    int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
    faw::string_t str;
    str.resize(cchLen, _T('\0'));
    ::GetWindowText(m_hWnd, &str[0], cchLen);
    m_pOwner->m_sText = str;
    //m_pOwner->SetText(str.c_str()) ;
    m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_TEXTCHANGED);
    if(m_pOwner->GetManager()->IsLayered()) m_pOwner->Invalidate();
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CEditUI)

CEditUI::CEditUI()
{
    SetTextPadding({ 4, 3, 4, 3 });
    SetBkColor(0xFFFFFFFF);
}

faw::string_t CEditUI::GetClass() const
{
    return _T("EditUI");
}

std::shared_ptr<CControlUI> CEditUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_EDIT) return std::dynamic_pointer_cast<CDateTimeUI>(shared_from_this());
    return CLabelUI::GetInterface(pstrName);
}

UINT CEditUI::GetControlFlags() const
{
    if(!IsEnabled()) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CEditUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_SETCURSOR && IsEnabled())
    {
        ::SetCursor(::LoadCursor(nullptr, IDC_IBEAM));
        return;
    }
    if(event.Type == UIEVENT_WINDOWSIZE)
    {
        if(m_pWindow) m_pManager->SetFocusNeeded(shared_from_this());
    }
    if(event.Type == UIEVENT_SCROLLWHEEL)
    {
        if(m_pWindow) return;
    }
    if(event.Type == UIEVENT_SETFOCUS && IsEnabled())
    {
        if(m_pWindow) return;
        m_pWindow = new CEditWnd();
        ASSERT(m_pWindow);
        m_pWindow->Init(std::dynamic_pointer_cast<CEditUI>(shared_from_this()));
        Invalidate();
    }
    if(event.Type == UIEVENT_KILLFOCUS && IsEnabled())
    {
        Invalidate();
    }
    if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        if(IsEnabled())
        {
            GetManager()->ReleaseCapture();
            if(IsFocused() && !m_pWindow)
            {
                m_pWindow = new CEditWnd();
                ASSERT(m_pWindow);
                m_pWindow->Init(std::dynamic_pointer_cast<CEditUI>(shared_from_this()));

                if(PtInRect(&m_rcItem, event.ptMouse))
                {
                    int nSize = GetWindowTextLength(m_pWindow->GetHWND());
                    if(nSize == 0) nSize = 1;
                    Edit_SetSel(m_pWindow->GetHWND(), 0, nSize);
                }
            }
            else if(m_pWindow)
            {
                if(!m_bAutoSelAll)
                {
                    POINT pt = event.ptMouse;
                    pt.x -= m_rcItem.left + m_rcTextPadding.left;
                    pt.y -= m_rcItem.top + m_rcTextPadding.top;
                    Edit_SetSel(m_pWindow->GetHWND(), 0, 0);
                    ::SendMessage(m_pWindow->GetHWND(), WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
                }
            }
        }
        return;
    }
    if(event.Type == UIEVENT_MOUSEMOVE)
    {
        return;
    }
    if(event.Type == UIEVENT_BUTTONUP)
    {
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
            if(IsEnabled())
            {
                if((m_uButtonState & UISTATE_HOT) == 0)
                {
                    m_uButtonState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
    }
    if(event.Type == UIEVENT_MOUSELEAVE)
    {
        if(IsEnabled())
        {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;

        //if( !::PtInRect(&m_rcItem, event.ptMouse ) ) {
        //             if( IsEnabled() ) {
        //                 if( (m_uButtonState & UISTATE_HOT) != 0  ) {
        //                     m_uButtonState &= ~UISTATE_HOT;
        //                     Invalidate();
        //                 }
        //             }
        //             if (m_pManager) m_pManager->RemoveMouseLeaveNeeded(this);
        //         }
        //         else {
        //             if (m_pManager) m_pManager->AddMouseLeaveNeeded(this);
        //             return;
        //         }
    }
    CLabelUI::DoEvent(event);
}

void CEditUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if(!IsEnabled())
    {
        m_uButtonState = 0;
    }
}

void CEditUI::SetText(faw::string_t pstrText)
{
    m_sText = pstrText;
    if(m_pWindow) Edit_SetText(m_pWindow->GetHWND(), m_sText.c_str());
    Invalidate();
}

void CEditUI::SetMaxChar(UINT uMax)
{
    m_uMaxChar = uMax;
    if(m_pWindow) Edit_LimitText(m_pWindow->GetHWND(), m_uMaxChar);
}

UINT CEditUI::GetMaxChar()
{
    return m_uMaxChar;
}

void CEditUI::SetReadOnly(bool bReadOnly)
{
    if(m_bReadOnly == bReadOnly) return;

    m_bReadOnly = bReadOnly;
    if(m_pWindow) Edit_SetReadOnly(m_pWindow->GetHWND(), m_bReadOnly);
    Invalidate();
}

bool CEditUI::IsReadOnly() const
{
    return m_bReadOnly;
}

void CEditUI::SetNumberOnly(bool bNumberOnly)
{
    if(bNumberOnly)
    {
        m_iWindowStyls |= ES_NUMBER;
    }
    else
    {
        m_iWindowStyls &= ~ES_NUMBER;
    }
}

bool CEditUI::IsNumberOnly() const
{
    return (m_iWindowStyls & ES_NUMBER) ? true : false;
}

int CEditUI::GetWindowStyls() const
{
    return m_iWindowStyls;
}

void CEditUI::SetPasswordMode(bool bPasswordMode)
{
    if(m_bPasswordMode == bPasswordMode) return;
    m_bPasswordMode = bPasswordMode;
    Invalidate();
    if(m_pWindow)
    {
        LONG styleValue = ::GetWindowLong(m_pWindow->GetHWND(), GWL_STYLE);
        bPasswordMode ? styleValue |= ES_PASSWORD : styleValue &= ~ES_PASSWORD;
        ::SetWindowLong(m_pWindow->GetHWND(), GWL_STYLE, styleValue);
    }
}

bool CEditUI::IsPasswordMode() const
{
    return m_bPasswordMode;
}

void CEditUI::SetPasswordChar(TCHAR cPasswordChar)
{
    if(m_cPasswordChar == cPasswordChar) return;
    m_cPasswordChar = cPasswordChar;
    if(m_pWindow) Edit_SetPasswordChar(m_pWindow->GetHWND(), m_cPasswordChar);
    Invalidate();
}

TCHAR CEditUI::GetPasswordChar() const
{
    return m_cPasswordChar;
}

faw::string_t CEditUI::GetNormalImage()
{
    return m_sNormalImage;
}

void CEditUI::SetNormalImage(faw::string_t pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

faw::string_t CEditUI::GetHotImage()
{
    return m_sHotImage;
}

void CEditUI::SetHotImage(faw::string_t pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

faw::string_t CEditUI::GetFocusedImage()
{
    return m_sFocusedImage;
}

void CEditUI::SetFocusedImage(faw::string_t pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

faw::string_t CEditUI::GetDisabledImage()
{
    return m_sDisabledImage;
}

void CEditUI::SetDisabledImage(faw::string_t pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

bool CEditUI::IsAutoSelAll()
{
    return m_bAutoSelAll;
}

void CEditUI::SetAutoSelAll(bool bAutoSelAll)
{
    m_bAutoSelAll = bAutoSelAll;
}

void CEditUI::SetSel(long nStartChar, long nEndChar)
{
    if(m_pWindow) Edit_SetSel(m_pWindow->GetHWND(), nStartChar, nEndChar);
}

void CEditUI::SetSelAll()
{
    SetSel(0, -1);
}

void CEditUI::SetReplaceSel(faw::string_t lpszReplace)
{
    if(m_pWindow) Edit_ReplaceSel(m_pWindow->GetHWND(), lpszReplace.data());
}

void CEditUI::SetTipValue(faw::string_t pStrTipValue)
{
    m_sTipValue = pStrTipValue;
}

faw::string_t CEditUI::GetTipValue()
{
    if(!IsResourceText()) return m_sTipValue;
    return CResourceManager::GetInstance()->GetText(m_sTipValue);
}

void CEditUI::SetTipValueColor(faw::string_t pStrColor)
{
    m_dwTipValueColor = (DWORD) FawTools::parse_hex(pStrColor);
}

DWORD CEditUI::GetTipValueColor()
{
    return m_dwTipValueColor;
}


void CEditUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    if(m_pWindow)
    {
        RECT rcPos = m_pWindow->CalPos();
        ::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
                       rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CEditUI::Move(SIZE szOffset, bool bNeedInvalidate)
{
    CControlUI::Move(szOffset, bNeedInvalidate);
    if(m_pWindow)
    {
        RECT rcPos = m_pWindow->CalPos();
        ::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
                       rcPos.bottom - rcPos.top, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CEditUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if(!IsVisible() && m_pWindow) m_pManager->SetFocus(nullptr);
}

void CEditUI::SetInternVisible(bool bVisible)
{
    if(!IsVisible() && m_pWindow) m_pManager->SetFocus(nullptr);
}

SIZE CEditUI::EstimateSize(SIZE szAvailable)
{
    if(m_cxyFixed.cy == 0) return { m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 6 };
    return CControlUI::EstimateSize(szAvailable);
}

void CEditUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("readonly")) SetReadOnly(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("numberonly")) SetNumberOnly(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("autoselall")) SetAutoSelAll(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("password")) SetPasswordMode(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("passwordchar")) SetPasswordChar(pstrValue[0]);
    else if(pstrName == _T("maxchar")) SetMaxChar(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("normalimage")) SetNormalImage(pstrValue);
    else if(pstrName == _T("hotimage")) SetHotImage(pstrValue);
    else if(pstrName == _T("focusedimage")) SetFocusedImage(pstrValue);
    else if(pstrName == _T("disabledimage")) SetDisabledImage(pstrValue);
    else if(pstrName == _T("tipvalue")) SetTipValue(pstrValue);
    else if(pstrName == _T("tipvaluecolor")) SetTipValueColor(pstrValue);
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CEditUI::PaintStatusImage(HDC hDC)
{
    if(IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~UISTATE_FOCUSED;
    if(!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~UISTATE_DISABLED;

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

    if(!m_sNormalImage.empty())
    {
        if(!DrawImage(hDC, m_sNormalImage))
        {
        }
        else return;
    }
}

void CEditUI::PaintText(HDC hDC)
{
    DWORD mCurTextColor = m_dwTextColor;

    if(m_dwTextColor == 0) mCurTextColor = m_dwTextColor = m_pManager->GetDefaultFontColor();
    if(m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    faw::string_t sDrawText = GetText();
    faw::string_t sTipValue = GetTipValue();
    if(sDrawText == sTipValue || sDrawText == _T(""))
    {
        mCurTextColor = m_dwTipValueColor;
        sDrawText = sTipValue;
    }
    else
    {
        faw::string_t sTemp = sDrawText;
        if(m_bPasswordMode)
        {
            sDrawText.clear();
            faw::string_t pStr = sTemp;
            while(!pStr.empty())
            {
                sDrawText += m_cPasswordChar;
                pStr = pStr.substr(1);
            }
        }
    }

    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if(IsEnabled())
    {
        CRenderEngine::DrawText(hDC, m_pManager, rc, sDrawText, mCurTextColor, m_iFont, DT_SINGLELINE | m_uTextStyle);
    }
    else
    {
        CRenderEngine::DrawText(hDC, m_pManager, rc, sDrawText, m_dwDisabledTextColor, m_iFont, DT_SINGLELINE | m_uTextStyle);
    }
}
}
