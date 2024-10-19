#include "stdafx.h"
#include "UIRollText.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CRollTextUI)

CRollTextUI::CRollTextUI(void) {}

CRollTextUI::~CRollTextUI(void)
{
    m_pManager->KillTimer(shared_from_this(), ROLLTEXT_ROLL_END_ID);
    m_pManager->KillTimer(shared_from_this(), ROLLTEXT_TIMER_ID);
}

faw::string_t CRollTextUI::GetClass() const
{
    return _T("RollTextUI");
}

std::shared_ptr<CControlUI> CRollTextUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == _T("RollText")) return std::dynamic_pointer_cast<CContainerUI>(shared_from_this());
    static_cast<CRollTextUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

void CRollTextUI::BeginRoll(int nDirect, LONG lTimeSpan, LONG lMaxTimeLimited)
{
    m_nRollDirection = nDirect;
    if(m_bUseRoll)
    {
        EndRoll();
    }
    m_nText_W_H = 0;

    m_pManager->KillTimer(shared_from_this(), ROLLTEXT_TIMER_ID);
    m_pManager->SetTimer(shared_from_this(), ROLLTEXT_TIMER_ID, lTimeSpan);

    m_pManager->KillTimer(shared_from_this(), ROLLTEXT_ROLL_END_ID);
    m_pManager->SetTimer(shared_from_this(), ROLLTEXT_ROLL_END_ID, lMaxTimeLimited * 1000);

    m_bUseRoll = TRUE;
}

void CRollTextUI::EndRoll()
{
    if(!m_bUseRoll) return;

    m_pManager->KillTimer(shared_from_this(), ROLLTEXT_ROLL_END_ID);
    m_pManager->KillTimer(shared_from_this(), ROLLTEXT_TIMER_ID);

    m_bUseRoll = FALSE;
}

void CRollTextUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CLabelUI::SetPos(rc, bNeedInvalidate);
    m_nText_W_H = 0;			//布局变化重新计算
}

void CRollTextUI::SetText(faw::string_t pstrText)
{
    CLabelUI::SetText(pstrText);
    m_nText_W_H = 0;			//文本变化重新计算
}

void CRollTextUI::DoEvent(TEventUI& event)
{
    if(event.Type == UIEVENT_TIMER && event.wParam == ROLLTEXT_ROLL_END_ID)
    {
        m_pManager->KillTimer(shared_from_this(), ROLLTEXT_ROLL_END_ID);
        m_pManager->SendNotify(shared_from_this(), DUI_MSGTYPE_TEXTROLLEND);
    }
    else if(event.Type == UIEVENT_TIMER && event.wParam == ROLLTEXT_TIMER_ID)
    {
        Invalidate();
        return;
    }
    CLabelUI::DoEvent(event);
}

void CRollTextUI::PaintText(HDC hDC)
{
    if(m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if(m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();
    DWORD dwTextColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;
    faw::string_t sText = GetText();
    if(sText.empty()) return;
    RECT rcTextPadding = GetTextPadding();
    RECT  rcClient;
    rcClient = m_rcItem;
    rcClient.left += rcTextPadding.left;
    rcClient.right -= rcTextPadding.right;
    rcClient.top += rcTextPadding.top;
    rcClient.bottom -= rcTextPadding.bottom;

    if(m_nText_W_H > 0)
    {
        int nScrollRange = 0;

        if(m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT)  	//左面移动
        {
            nScrollRange = m_nText_W_H + rcClient.right - rcClient.left;
            LONG off_cx = (m_nRollDirection == ROLLTEXT_LEFT ? rcClient.right - rcClient.left : rcClient.left - rcClient.right);
            ::OffsetRect(&rcClient, off_cx, 0);
            off_cx = (m_nRollDirection == ROLLTEXT_LEFT ? -m_nScrollPos : m_nScrollPos);
            ::OffsetRect(&rcClient, off_cx, 0);
            rcClient.right += (m_nText_W_H - (rcClient.right - rcClient.left));
        }
        else  																		//上下移动
        {
            nScrollRange = m_nText_W_H + rcClient.bottom - rcClient.top;
            LONG off_cy = (m_nRollDirection == ROLLTEXT_UP ? rcClient.bottom - rcClient.top : rcClient.top - rcClient.bottom);
            ::OffsetRect(&rcClient, 0, off_cy);
            off_cy = (m_nRollDirection == ROLLTEXT_UP ? -m_nScrollPos : m_nScrollPos);
            ::OffsetRect(&rcClient, 0, off_cy);
            rcClient.bottom += (m_nText_W_H - (rcClient.bottom - rcClient.top));
        }

        m_nScrollPos += m_nStep;
        if(m_nScrollPos > nScrollRange)
        {
            m_nScrollPos = 0;
        }
    }

    RECT rc = rcClient;

    UINT uTextStyle = DT_WORDBREAK | DT_EDITCONTROL;

    if(m_nText_W_H == 0)
    {
        uTextStyle |= DT_CALCRECT;				//第一次计算文本宽度或高度
        if(m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT)  	//左面移动
        {
            rc.right += 10000;
        }
        else  																		//上下移动
        {
            rc.bottom += 10000;
        }
    }

    if(m_bShowHtml)
    {
        int nLinks = 0;
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, dwTextColor, nullptr, nullptr, nLinks, m_iFont, uTextStyle);
    }
    else
    {
        CRenderEngine::DrawText(hDC, m_pManager, rc, sText, dwTextColor, m_iFont, uTextStyle);
    }

    if(m_nText_W_H == 0)
    {
        m_nText_W_H = (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT) ? (rc.right - rc.left) : (rc.bottom - rc.top);		//计算文本宽度或高度
    }
}
}