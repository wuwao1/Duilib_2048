﻿#include "StdAfx.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CControlUI)

CControlUI::CControlUI()
{
    m_cXY.cx = m_cXY.cy = 0;
    m_cxyFixed.cx = m_cxyFixed.cy = 0;
    m_cxyMin.cx = m_cxyMin.cy = 0;
    m_cxyMax.cx = m_cxyMax.cy = 9999;
    m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

    ::ZeroMemory(&m_rcPadding, sizeof(RECT));
    ::ZeroMemory(&m_rcItem, sizeof(RECT));
    ::ZeroMemory(&m_rcPaint, sizeof(RECT));
    ::ZeroMemory(&m_rcBorderSize, sizeof(RECT));
    m_piFloatPercent.left = m_piFloatPercent.top = m_piFloatPercent.right = m_piFloatPercent.bottom = 0.0f;
}

CControlUI::~CControlUI()
{
    if(OnDestroy) OnDestroy(this);
    RemoveAllCustomAttribute();
    if(m_pManager) m_pManager->ReapObjects(shared_from_this());
}

faw::string_t CControlUI::GetName() const
{
    return m_sName;
}

void CControlUI::SetName(faw::string_t pstrName)
{
    m_sName = pstrName;
}

std::shared_ptr<CControlUI> CControlUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_CONTROL) return shared_from_this();
    return nullptr;
}

faw::string_t CControlUI::GetClass() const
{
    return _T("ControlUI");
}

UINT CControlUI::GetControlFlags() const
{
    return 0;
}

bool CControlUI::Activate()
{
    if(!IsVisible()) return false;
    if(!IsEnabled()) return false;
    return true;
}

CPaintManagerUI* CControlUI::GetManager() const
{
    return m_pManager;
}

void CControlUI::SetManager(CPaintManagerUI* pManager, std::shared_ptr<CControlUI> pParent, bool bInit)
{
    m_pManager = pManager;
    m_pParent = pParent;
    if(bInit && m_pParent) Init();
}

std::shared_ptr<CControlUI> CControlUI::GetParent() const
{
    return m_pParent;
}

bool CControlUI::SetTimer(UINT nTimerID, UINT nElapse)
{
    if(!m_pManager) return false;
    return m_pManager->SetTimer(shared_from_this(), nTimerID, nElapse);
}

void CControlUI::KillTimer(UINT nTimerID)
{
    if(!m_pManager) return;
    m_pManager->KillTimer(shared_from_this(), nTimerID);
}

faw::string_t CControlUI::GetText() const
{
    if(!IsResourceText()) return m_sText;
    return CResourceManager::GetInstance()->GetText(m_sText);
}

void CControlUI::SetText(faw::string_t pstrText)
{
    if(m_sText == pstrText) return;

    m_sText = pstrText;
    // 解析字体图标
    // 1.找到标识符&#ptx
    faw::string_t UnicodeMark(_T("&#@"));
    size_t fonticonpos = m_sText.find(UnicodeMark);
    while(fonticonpos != faw::string_t::npos)
    {
        // 2.跳过标识符
        faw::string_t strUnicode = faw::string_t(m_sText).substr(fonticonpos + 3);
        // 3.字串大小大于4，取前4位
        if(strUnicode.size() >= 4)
        {
            strUnicode = strUnicode.substr(0, 4);
            for(int i = 0; i < 4; i++)
                // 4.不是数值,跳过,查找后面字串
                if(!isdigit(strUnicode[i]) || !isalpha(strUnicode[i]))
                {
                    fonticonpos = m_sText.find(UnicodeMark, fonticonpos + 7);
                    continue;
                }
        }
        else break;

        wchar_t wch[2] = { 0 };
        wch[0] = static_cast<wchar_t>(FawTools::parse_hex(strUnicode));
        // 5.替换unicode标识
        FawTools::replace_self(m_sText, UnicodeMark+ strUnicode, wch);
        // 6.替换完,从头找unicode标识
        fonticonpos = m_sText.find(UnicodeMark);
    }
    // 解析xml换行符
    FawTools::replace_self(m_sText, _T("{\\n}"), _T("\r\n"));
    Invalidate();
}

bool CControlUI::IsResourceText() const
{
    return m_bResourceText;
}

void CControlUI::SetResourceText(bool bResource)
{
    if(m_bResourceText == bResource) return;
    m_bResourceText = bResource;
    Invalidate();
}

bool CControlUI::IsDragEnabled() const
{
    return m_bDragEnabled;
}

void CControlUI::SetDragEnable(bool bDrag)
{
    m_bDragEnabled = bDrag;
}

bool CControlUI::IsDropEnabled() const
{
    return m_bDropEnabled;
}

void CControlUI::SetDropEnable(bool bDrop)
{
    m_bDropEnabled = bDrop;
}

faw::string_t CControlUI::GetGradient()
{
    return m_sGradient;
}

void CControlUI::SetGradient(faw::string_t pStrImage)
{
    if(m_sGradient == pStrImage) return;

    m_sGradient = pStrImage;
    Invalidate();
}

DWORD CControlUI::GetBkColor() const
{
    return m_dwBackColor;
}

void CControlUI::SetBkColor(DWORD dwBackColor)
{
    if(m_dwBackColor == dwBackColor) return;

    m_dwBackColor = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor2() const
{
    return m_dwBackColor2;
}

void CControlUI::SetBkColor2(DWORD dwBackColor)
{
    if(m_dwBackColor2 == dwBackColor) return;

    m_dwBackColor2 = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor3() const
{
    return m_dwBackColor3;
}

void CControlUI::SetBkColor3(DWORD dwBackColor)
{
    if(m_dwBackColor3 == dwBackColor) return;

    m_dwBackColor3 = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetForeColor() const
{
    return m_dwForeColor;
}

void CControlUI::SetForeColor(DWORD dwForeColor)
{
    if(m_dwForeColor == dwForeColor) return;

    m_dwForeColor = dwForeColor;
    Invalidate();
}

faw::string_t CControlUI::GetBkImage()
{
    return m_sBkImage;
}

void CControlUI::SetBkImage(faw::string_t pStrImage)
{
    //if (m_pManager) m_pManager->RemoveImage (pStrImage);
    if(m_sBkImage == pStrImage) return;

    m_sBkImage = pStrImage;
    Invalidate();
}

faw::string_t CControlUI::GetForeImage() const
{
    return m_sForeImage;
}

void CControlUI::SetForeImage(faw::string_t pStrImage)
{
    if(m_sForeImage == pStrImage) return;

    m_sForeImage = pStrImage;
    Invalidate();
}

DWORD CControlUI::GetBorderColor() const
{
    return m_dwBorderColor;
}

void CControlUI::SetBorderColor(DWORD dwBorderColor)
{
    if(m_dwBorderColor == dwBorderColor) return;

    m_dwBorderColor = dwBorderColor;
    Invalidate();
}

DWORD CControlUI::GetFocusBorderColor() const
{
    return m_dwFocusBorderColor;
}

void CControlUI::SetFocusBorderColor(DWORD dwBorderColor)
{
    if(m_dwFocusBorderColor == dwBorderColor) return;

    m_dwFocusBorderColor = dwBorderColor;
    Invalidate();
}

bool CControlUI::IsColorHSL() const
{
    return m_bColorHSL;
}

void CControlUI::SetColorHSL(bool bColorHSL)
{
    if(m_bColorHSL == bColorHSL) return;

    m_bColorHSL = bColorHSL;
    Invalidate();
}

int CControlUI::GetBorderSize() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_nBorderSize);
    return m_nBorderSize;
}

void CControlUI::SetBorderSize(int nSize)
{
    if(m_nBorderSize == nSize) return;

    m_nBorderSize = nSize;
    Invalidate();
}

void CControlUI::SetBorderSize(RECT rc)
{
    m_rcBorderSize = rc;
    Invalidate();
}

SIZE CControlUI::GetBorderRound() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_cxyBorderRound);
    return m_cxyBorderRound;
}

void CControlUI::SetBorderRound(SIZE cxyRound)
{
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

bool CControlUI::DrawImage(HDC hDC, faw::string_t pStrImage, faw::string_t pStrModify)
{
    return CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, pStrImage, pStrModify, m_instance);
}

const RECT& CControlUI::GetPos() const
{
    return m_rcItem;
}

RECT CControlUI::GetRelativePos() const
{
    std::shared_ptr<CControlUI> pParent = GetParent();
    if(pParent)
    {
        RECT rcParentPos = pParent->GetPos();
        RECT rcRelativePos(m_rcItem);
        ::OffsetRect(&rcRelativePos, -rcParentPos.left, -rcParentPos.top);
        return rcRelativePos;
    }
    else
    {
        return { 0, 0, 0, 0 };
    }
}

RECT CControlUI::GetClientPos() const
{
    return m_rcItem;
}

void CControlUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    if(rc.right < rc.left) rc.right = rc.left;
    if(rc.bottom < rc.top) rc.bottom = rc.top;

    RECT invalidateRc = m_rcItem;
    if(::IsRectEmpty(&invalidateRc)) invalidateRc = rc;

    m_rcItem = rc;
    if(!m_pManager) return;

    if(!m_bSetPos)
    {
        m_bSetPos = true;
        if(OnSize) OnSize(this);
        m_bSetPos = false;
    }

    m_bUpdateNeeded = false;

    if(bNeedInvalidate && IsVisible())
    {
        invalidateRc.left = min(invalidateRc.left, m_rcItem.left);
        invalidateRc.top = min(invalidateRc.top, m_rcItem.top);
        invalidateRc.right = max(invalidateRc.right, m_rcItem.right);
        invalidateRc.bottom = max(invalidateRc.bottom, m_rcItem.bottom);
        std::shared_ptr<CControlUI> pParent = shared_from_this();
        RECT rcTemp = { 0 };
        RECT rcParent = { 0 };
        while(!!(pParent = pParent->GetParent()))
        {
            if(!pParent->IsVisible()) return;
            rcTemp = invalidateRc;
            rcParent = pParent->GetPos();
            if(!::IntersectRect(&invalidateRc, &rcTemp, &rcParent)) return;
        }
        m_pManager->Invalidate(invalidateRc);
    }
}

void CControlUI::Move(SIZE szOffset, bool bNeedInvalidate)
{
    m_cXY.cx += szOffset.cx;
    m_cXY.cy += szOffset.cy;
    NeedParentUpdate();
}

int CControlUI::GetWidth() const
{
    return m_rcItem.right - m_rcItem.left;
}

int CControlUI::GetHeight() const
{
    return m_rcItem.bottom - m_rcItem.top;
}

int CControlUI::GetX() const
{
    return m_rcItem.left;
}

int CControlUI::GetY() const
{
    return m_rcItem.top;
}

RECT CControlUI::GetPadding() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_rcPadding);
    return m_rcPadding;
}

void CControlUI::SetPadding(RECT rcPadding)
{
    m_rcPadding = rcPadding;
    NeedParentUpdate();
}

SIZE CControlUI::GetFixedXY() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_cXY);
    return m_cXY;
}

void CControlUI::SetFixedXY(SIZE szXY)
{
    m_cXY.cx = szXY.cx;
    m_cXY.cy = szXY.cy;
    NeedParentUpdate();
}

int CControlUI::GetFixedWidth() const
{
    if(m_pManager)
    {
        return m_pManager->GetDPIObj()->Scale(m_cxyFixed.cx);
    }

    return m_cxyFixed.cx;
}

void CControlUI::SetFixedWidth(int cx)
{
    if(cx < 0) return;
    m_cxyFixed.cx = cx;
    NeedParentUpdate();
}

int CControlUI::GetFixedHeight() const
{
    if(m_pManager)
    {
        return m_pManager->GetDPIObj()->Scale(m_cxyFixed.cy);
    }

    return m_cxyFixed.cy;
}

void CControlUI::SetFixedHeight(int cy)
{
    if(cy < 0) return;
    m_cxyFixed.cy = cy;
    NeedParentUpdate();
}

int CControlUI::GetMinWidth() const
{
    if(m_pManager)
    {
        return m_pManager->GetDPIObj()->Scale(m_cxyMin.cx);
    }
    return m_cxyMin.cx;
}

void CControlUI::SetMinWidth(int cx)
{
    if(m_cxyMin.cx == cx) return;

    if(cx < 0) return;
    m_cxyMin.cx = cx;
    NeedParentUpdate();
}

int CControlUI::GetMaxWidth() const
{
    if(m_pManager)
    {
        return m_pManager->GetDPIObj()->Scale(m_cxyMax.cx);
    }
    return m_cxyMax.cx;
}

void CControlUI::SetMaxWidth(int cx)
{
    if(m_cxyMax.cx == cx) return;

    if(cx < 0) return;
    m_cxyMax.cx = cx;
    NeedParentUpdate();
}

int CControlUI::GetMinHeight() const
{
    if(m_pManager)
    {
        return m_pManager->GetDPIObj()->Scale(m_cxyMin.cy);
    }

    return m_cxyMin.cy;
}

void CControlUI::SetMinHeight(int cy)
{
    if(m_cxyMin.cy == cy) return;

    if(cy < 0) return;
    m_cxyMin.cy = cy;
    NeedParentUpdate();
}

int CControlUI::GetMaxHeight() const
{
    if(m_pManager)
    {
        return m_pManager->GetDPIObj()->Scale(m_cxyMax.cy);
    }

    return m_cxyMax.cy;
}

void CControlUI::SetMaxHeight(int cy)
{
    if(m_cxyMax.cy == cy) return;

    if(cy < 0) return;
    m_cxyMax.cy = cy;
    NeedParentUpdate();
}

TPercentInfo CControlUI::GetFloatPercent() const
{
    return m_piFloatPercent;
}

void CControlUI::SetFloatPercent(TPercentInfo piFloatPercent)
{
    m_piFloatPercent = piFloatPercent;
    NeedParentUpdate();
}

void CControlUI::SetFloatAlign(UINT uAlign)
{
    m_uFloatAlign = uAlign;
    NeedParentUpdate();
}

UINT CControlUI::GetFloatAlign() const
{
    return m_uFloatAlign;
}

faw::string_t CControlUI::GetToolTip() const
{
    if(!IsResourceText()) return m_sToolTip;
    return CResourceManager::GetInstance()->GetText(m_sToolTip);
}

void CControlUI::SetToolTip(faw::string_t pstrText)
{
    faw::string_t strTemp(pstrText);
    FawTools::replace_self(strTemp, _T("<n>"), _T("\r\n"));
    m_sToolTip = strTemp;
}

void CControlUI::SetToolTipWidth(int nWidth)
{
    m_nTooltipWidth = nWidth;
}

int CControlUI::GetToolTipWidth(void)
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_nTooltipWidth);
    return m_nTooltipWidth;
}

WORD CControlUI::GetCursor()
{
    return m_wCursor;
}

void CControlUI::SetCursor(WORD wCursor)
{
    m_wCursor = wCursor;
    Invalidate();
}

TCHAR CControlUI::GetShortcut() const
{
    return m_chShortcut;
}

void CControlUI::SetShortcut(TCHAR ch)
{
    m_chShortcut = ch;
}

bool CControlUI::IsContextMenuUsed() const
{
    return m_bMenuUsed;
}

void CControlUI::SetContextMenuUsed(bool bMenuUsed)
{
    m_bMenuUsed = bMenuUsed;
}

const faw::string_t& CControlUI::GetUserData()
{
    return m_sUserData;
}

void CControlUI::SetUserData(faw::string_t pstrText)
{
    m_sUserData = pstrText;
}

UINT_PTR CControlUI::GetTag() const
{
    return m_pTag;
}

void CControlUI::SetTag(UINT_PTR pTag)
{
    m_pTag = pTag;
}

bool CControlUI::IsVisible() const
{

    return m_bVisible && m_bInternVisible;
}

void CControlUI::SetVisible(bool bVisible)
{
    if(m_bVisible == bVisible) return;

    bool v = IsVisible();
    m_bVisible = bVisible;
    if(m_bFocused) m_bFocused = false;
    if(!bVisible && m_pManager && m_pManager->GetFocus() == shared_from_this())
    {
        m_pManager->SetFocus(nullptr);
    }
    if(IsVisible() != v)
    {
        NeedParentUpdate();
    }
}

void CControlUI::SetInternVisible(bool bVisible)
{
    m_bInternVisible = bVisible;
    if(!bVisible && m_pManager && m_pManager->GetFocus() == shared_from_this())
    {
        m_pManager->SetFocus(nullptr);
    }
}

bool CControlUI::IsEnabled() const
{
    return m_bEnabled;
}

void CControlUI::SetEnabled(bool bEnabled)
{
    if(m_bEnabled == bEnabled) return;

    m_bEnabled = bEnabled;
    Invalidate();
}

bool CControlUI::IsMouseEnabled() const
{
    return m_bMouseEnabled;
}

void CControlUI::SetMouseEnabled(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

bool CControlUI::IsKeyboardEnabled() const
{
    return m_bKeyboardEnabled;
}
void CControlUI::SetKeyboardEnabled(bool bEnabled)
{
    m_bKeyboardEnabled = bEnabled;
}

bool CControlUI::IsFocused() const
{
    return m_bFocused;
}

void CControlUI::SetFocus()
{
    if(m_pManager) m_pManager->SetFocus(shared_from_this());
}

bool CControlUI::IsFloat() const
{
    return m_bFloat;
}

void CControlUI::SetFloat(bool bFloat)
{
    if(m_bFloat == bFloat) return;

    m_bFloat = bFloat;
    NeedParentUpdate();
}

bool CControlUI::IsDynamic(POINT &pt) const
{
    return m_isDynamic;
}

void CControlUI::SetIsDynamic(bool bIsDynamic)
{
    m_isDynamic = bIsDynamic;
}

std::shared_ptr<CControlUI> CControlUI::FindControl(FIND_CONTROL_PROC Proc, LPVOID pData, UINT uFlags)
{
    if((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) return nullptr;
    if((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) return nullptr;
    if((uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, *static_cast<LPPOINT>(pData)))) return nullptr;
    return Proc(shared_from_this(), pData);
}

void CControlUI::Invalidate()
{
    if(!IsVisible()) return;

    RECT invalidateRc = m_rcItem;

    std::shared_ptr<CControlUI> pParent= this->GetParent();
    RECT rcTemp = { 0 };
    RECT rcParent = { 0 };
    while(!!pParent)
    {
        rcTemp = invalidateRc;
        rcParent = pParent->GetPos();
        if(!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
        {
            return;
        }
        pParent = pParent->GetParent();
    }

    if(m_pManager) m_pManager->Invalidate(invalidateRc);
}

bool CControlUI::IsUpdateNeeded() const
{
    return m_bUpdateNeeded;
}

void CControlUI::NeedUpdate()
{
    if(!IsVisible()) return;
    m_bUpdateNeeded = true;
    Invalidate();

    if(m_pManager) m_pManager->NeedUpdate();
}

void CControlUI::NeedParentUpdate()
{
    if(GetParent())
    {
        GetParent()->NeedUpdate();
        GetParent()->Invalidate();
    }
    else
    {
        NeedUpdate();
    }

    if(m_pManager) m_pManager->NeedUpdate();
}

DWORD CControlUI::GetAdjustColor(DWORD dwColor)
{
    if(!m_bColorHSL) return dwColor;
    short H, S, L;
    CPaintManagerUI::GetHSL(&H, &S, &L);
    return CRenderEngine::AdjustColor(dwColor, H, S, L);
}

void CControlUI::Init()
{
    DoInit();
    if(OnInit) OnInit(this);
}

void CControlUI::DoInit()
{

}

void CControlUI::Event(TEventUI& event)
{
    if(OnEvent(&event)) DoEvent(event);
}

void CControlUI::DoEvent(TEventUI& event)
{
    if(event.Type == UIEVENT_SETCURSOR)
    {
        if(GetCursor())
        {
            ::SetCursor(::LoadCursor(nullptr, MAKEINTRESOURCE(GetCursor())));
        }
        else
        {
            ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
        }
        return;
    }

    if(event.Type == UIEVENT_SETFOCUS)
    {
        m_bFocused = true;
        Invalidate();
        return;
    }
    if(event.Type == UIEVENT_KILLFOCUS)
    {
        m_bFocused = false;
        Invalidate();
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

    if(m_pParent) m_pParent->DoEvent(event);
}


void CControlUI::SetVirtualWnd(faw::string_t pstrValue)
{
    m_sVirtualWnd = pstrValue;
    m_pManager->UsedVirtualWnd(true);
}

faw::string_t CControlUI::GetVirtualWnd() const
{
    if(!m_sVirtualWnd.empty())
        return m_sVirtualWnd;
    std::shared_ptr<CControlUI> pParent = GetParent();
    if(pParent)
    {
        return pParent->GetVirtualWnd();
    }
    else
    {
        return _T("");
    }
}

void CControlUI::AddCustomAttribute(faw::string_t pstrName, faw::string_t pstrAttr)
{
    m_mCustomAttrs[pstrName] = pstrAttr;
}

faw::string_t CControlUI::GetCustomAttribute(faw::string_t pstrName)
{
    return m_mCustomAttrs[pstrName];
}

bool CControlUI::RemoveCustomAttribute(faw::string_t pstrName)
{
    m_mCustomAttrs.erase(pstrName);
    return true;
}

void CControlUI::RemoveAllCustomAttribute()
{
    m_mCustomAttrs.clear();
}

void CControlUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    // 样式表
    if(m_pManager)
    {
        faw::string_t pStyle = m_pManager->GetStyle(pstrValue);
        if(!pStyle.empty())
        {
            ApplyAttributeList(pStyle);
            return;
        }
    }
    // 属性
    if(pstrName == _T("innerstyle"))
    {
        ApplyAttributeList(pstrValue);
    }
    else if(pstrName == _T("pos"))
    {
        RECT rcPos = FawTools::parse_rect(pstrValue);
        SIZE szXY = { rcPos.left >= 0 ? rcPos.left : rcPos.right, rcPos.top >= 0 ? rcPos.top : rcPos.bottom };
        SetFixedXY(szXY);
        SetFixedWidth(rcPos.right - rcPos.left);
        SetFixedHeight(rcPos.bottom - rcPos.top);
    }
    else if(pstrName == _T("float"))
    {
        faw::string_t nValue = pstrValue;
        // 动态计算相对比例
        if(nValue.find(',') == faw::string_t::npos)
        {
            SetFloat(FawTools::parse_bool(pstrValue));
        }
        else
        {
            TPercentInfo piFloatPercent = FawTools::parse_TPercentInfo(pstrValue);
            SetFloatPercent(piFloatPercent);
            SetFloat(true);
        }
    }
    else if(pstrName == _T("floatalign"))
    {
        UINT uAlign = GetFloatAlign();
        // 解析文字属性
        while(!pstrValue.empty())
        {
            faw::string_t sValue;
            while(pstrValue[0] == _T(',') || pstrValue[0] == _T(' ')) pstrValue = pstrValue.substr(1);

            while(!pstrValue.empty() && pstrValue[0] != _T(',') && pstrValue[0] != _T(' '))
            {
                faw::string_t pstrTemp = pstrValue.substr(1);
                while(pstrValue.length() > pstrTemp.length())
                {
                    sValue += pstrValue[0];
                    pstrValue = pstrValue.substr(1);
                }
            }
            if(sValue == _T("nullptr"))
            {
                uAlign = 0;
            }
            else if(sValue == _T("left"))
            {
                uAlign &= ~(DT_CENTER | DT_RIGHT);
                uAlign |= DT_LEFT;
            }
            else if(sValue == _T("center"))
            {
                uAlign &= ~(DT_LEFT | DT_RIGHT);
                uAlign |= DT_CENTER;
            }
            else if(sValue == _T("right"))
            {
                uAlign &= ~(DT_LEFT | DT_CENTER);
                uAlign |= DT_RIGHT;
            }
            else if(sValue == _T("top"))
            {
                uAlign &= ~(DT_BOTTOM | DT_VCENTER);
                uAlign |= DT_TOP;
            }
            else if(sValue == _T("vcenter"))
            {
                uAlign &= ~(DT_TOP | DT_BOTTOM);
                uAlign |= DT_VCENTER;
            }
            else if(sValue == _T("bottom"))
            {
                uAlign &= ~(DT_TOP | DT_VCENTER);
                uAlign |= DT_BOTTOM;
            }
        }
        SetFloatAlign(uAlign);
    }
    else if(pstrName == _T("padding"))
    {
        RECT rcPadding = FawTools::parse_rect(pstrValue);
        SetPadding(rcPadding);
    }
    else if(pstrName == _T("gradient"))
    {
        SetGradient(pstrValue);
    }
    else if(pstrName == _T("bkcolor") || pstrName == _T("bkcolor1"))
    {
        SetBkColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("bkcolor2"))
    {
        SetBkColor2((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("bkcolor3"))
    {
        SetBkColor3((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("forecolor"))
    {
        SetForeColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("bordercolor"))
    {
        SetBorderColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("focusbordercolor"))
    {
        SetFocusBorderColor((DWORD) FawTools::parse_hex(pstrValue));
    }
    else if(pstrName == _T("colorhsl"))
    {
        SetColorHSL(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("bordersize"))
    {
        if(pstrValue.find(',') == faw::string_t::npos)
        {
            SetBorderSize(FawTools::parse_dec(pstrValue));
        }
        else
        {
            RECT rcPadding = FawTools::parse_rect(pstrValue);
            SetBorderSize(rcPadding);
        }
    }
    else if(pstrName == _T("leftbordersize"))
    {
        SetLeftBorderSize(_ttoi(pstrValue.data()));
    }
    else if(pstrName == _T("topbordersize"))
    {
        SetTopBorderSize(_ttoi(pstrValue.data()));
    }
    else if(pstrName == _T("rightbordersize"))
    {
        SetRightBorderSize(_ttoi(pstrValue.data()));
    }
    else if(pstrName == _T("bottombordersize"))
    {
        SetBottomBorderSize(_ttoi(pstrValue.data()));
    }
    else if(pstrName == _T("borderstyle"))
    {
        SetBorderStyle(_ttoi(pstrValue.data()));
    }
    else if(pstrName == _T("borderround"))
    {
        SIZE cxyRound = FawTools::parse_size(pstrValue);
        SetBorderRound(cxyRound);
    }
    else if(pstrName == _T("bkimage"))
        SetBkImage(pstrValue);
    else if(pstrName == _T("foreimage")) SetForeImage(pstrValue);
    else if(pstrName == _T("width")) SetFixedWidth(_ttoi(pstrValue.data()));
    else if(pstrName == _T("height")) SetFixedHeight(_ttoi(pstrValue.data()));
    else if(pstrName == _T("minwidth")) SetMinWidth(_ttoi(pstrValue.data()));
    else if(pstrName == _T("minheight")) SetMinHeight(_ttoi(pstrValue.data()));
    else if(pstrName == _T("maxwidth")) SetMaxWidth(_ttoi(pstrValue.data()));
    else if(pstrName == _T("maxheight")) SetMaxHeight(_ttoi(pstrValue.data()));
    else if(pstrName == _T("name")) SetName(pstrValue);
    else if(pstrName == _T("drag")) SetDragEnable(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("drop")) SetDropEnable(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("resourcetext")) SetResourceText(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("text")) SetText(pstrValue);
    else if(pstrName == _T("tooltip")) SetToolTip(pstrValue);
    else if(pstrName == _T("userdata")) SetUserData(pstrValue);
    else if(pstrName == _T("enabled")) SetEnabled(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("mouse")) SetMouseEnabled(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("keyboard")) SetKeyboardEnabled(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("visible")) SetVisible(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("float")) SetFloat(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("shortcut")) SetShortcut(pstrValue[0]);
    else if(pstrName == _T("menu")) SetContextMenuUsed(FawTools::parse_bool(pstrValue));
    else if(pstrName == _T("cursor") && !pstrValue.empty())
    {
        if(pstrValue == _T("arrow"))			SetCursor(DUI_ARROW);
        else if(pstrValue == _T("ibeam"))	SetCursor(DUI_IBEAM);
        else if(pstrValue == _T("wait"))		SetCursor(DUI_WAIT);
        else if(pstrValue == _T("cross"))	SetCursor(DUI_CROSS);
        else if(pstrValue == _T("uparrow"))	SetCursor(DUI_UPARROW);
        else if(pstrValue == _T("size"))		SetCursor(DUI_SIZE);
        else if(pstrValue == _T("icon"))		SetCursor(DUI_ICON);
        else if(pstrValue == _T("sizenwse"))	SetCursor(DUI_SIZENWSE);
        else if(pstrValue == _T("sizenesw"))	SetCursor(DUI_SIZENESW);
        else if(pstrValue == _T("sizewe"))	SetCursor(DUI_SIZEWE);
        else if(pstrValue == _T("sizens"))	SetCursor(DUI_SIZENS);
        else if(pstrValue == _T("sizeall"))	SetCursor(DUI_SIZEALL);
        else if(pstrValue == _T("no"))		SetCursor(DUI_NO);
        else if(pstrValue == _T("hand"))		SetCursor(DUI_HAND);
    }
    else if(pstrName == _T("virtualwnd")) SetVirtualWnd(pstrValue);
    else if(pstrName == _T("isdynamic"))
    {
        SetIsDynamic(FawTools::parse_bool(pstrValue));
    }
    else
    {
        AddCustomAttribute(pstrName, pstrValue);
    }
}

std::shared_ptr<CControlUI> CControlUI::ApplyAttributeList(faw::string_t pstrValue)
{
    // 解析样式表
    if(m_pManager)
    {
        faw::string_t pStyle = m_pManager->GetStyle(pstrValue);
        if(!pStyle.empty())
        {
            return ApplyAttributeList(pStyle);
        }
    }
    faw::string_t sXmlData = pstrValue;
    FawTools::replace_self(sXmlData, _T("&quot;"), _T("\""));
    auto pairs = FawTools::parse_keyvalue_pairs(sXmlData);
    for(const auto& [str_key, str_value] : pairs)
        SetAttribute(str_key, str_value);
    return shared_from_this();
}

SIZE CControlUI::EstimateSize(SIZE szAvailable)
{
    if(m_pManager)
        return m_pManager->GetDPIObj()->Scale(m_cxyFixed);
    return m_cxyFixed;
}

bool CControlUI::Paint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    if(pStopControl == shared_from_this()) return false;
    if(!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return true;
    if(!DoPaint(hDC, m_rcPaint, pStopControl)) return false;
    return true;
}

bool CControlUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    // 绘制循序：背景颜色->背景图->状态图->文本->边框
    int nBorderSize = { 0 };
    SIZE cxyBorderRound = { 0 };
    nBorderSize = GetBorderSize();
    cxyBorderRound = GetBorderRound();

    if(cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0)
    {
        if(m_rcItem.right - m_rcItem.left >= 1920 || m_rcItem.bottom - m_rcItem.top > 1080)
        {
            CRenderClip roundClip;
            CRenderClip::GenerateRoundClip(hDC, m_rcPaint, m_rcItem, cxyBorderRound.cx, cxyBorderRound.cy, roundClip);
            PaintBkColor(hDC);
            PaintBkImage(hDC);
            PaintStatusImage(hDC);
            PaintForeColor(hDC);
            PaintForeImage(hDC);
            PaintText(hDC);
            PaintBorder(hDC);
        }
        else
        {
            auto parent = GetParent();
            while(parent && m_dwBackColor == 0)
            {
                m_dwBackColor = parent->GetBkColor();
                m_dwBackColor2 = parent->GetBkColor2();
                m_dwBackColor3 = parent->GetBkColor3();
                parent = parent->GetParent();
            }

            RECT m_rcPreItem = m_rcItem;
            RECT m_rcPrePaint = m_rcPaint;
            m_rcItem = { 0,0,m_rcItem.right - m_rcItem.left,m_rcItem.bottom - m_rcItem.top };
            m_rcPaint = m_rcItem;
            //////2.draw round Rect Path picture to hdc
            HDC MemoryHDC = CreateCompatibleDC(hDC);
            void* BitmapBits = nullptr;
            LPBITMAPINFO lpbiSrc = nullptr;
            lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
            lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            lpbiSrc->bmiHeader.biWidth = m_rcItem.right;
            lpbiSrc->bmiHeader.biHeight = m_rcItem.bottom;
            lpbiSrc->bmiHeader.biPlanes = 1;
            lpbiSrc->bmiHeader.biBitCount = 32;
            lpbiSrc->bmiHeader.biCompression = BI_RGB;
            lpbiSrc->bmiHeader.biSizeImage = (m_rcItem.right) * (m_rcItem.bottom);
            lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
            lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
            lpbiSrc->bmiHeader.biClrUsed = 0;
            lpbiSrc->bmiHeader.biClrImportant = 0;
            HBITMAP MemoryHBitmap = CreateDIBSection(hDC, lpbiSrc, DIB_RGB_COLORS, &BitmapBits, nullptr, 0);
            ON_SCOPE_EXIT([&] {	delete[] lpbiSrc; });
            auto OldBitmap = SelectObject(MemoryHDC, MemoryHBitmap);

            ////是否确实有顺序画到需要裁剪的元素
            PaintBkColor(MemoryHDC);
            PaintBkImage(MemoryHDC);
            PaintStatusImage(MemoryHDC);
            PaintForeColor(MemoryHDC);
            PaintForeImage(MemoryHDC);

            float ptx = 0;
            float pty = 0;
            float width = m_rcItem.right - 1;
            float height = m_rcItem.bottom - 1;
            float arcSize = cxyBorderRound.cx;
            float arcDiameter = arcSize * 2;
            Gdiplus::Graphics  graphics(hDC);
            //设置绘图时的滤波模式为消除锯齿现象，即使用 SmoothingModeHighQuality 来获得更平滑的绘图效果。
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            //创建一个 Gdiplus::GraphicsPath 对象 roundRectPath，用于定义矩形的路径。在路径中添加直线和弧线，以实现四个圆角。
            Gdiplus::GraphicsPath roundRectPath;
            //在路径中依次添加顶部横线、右上圆角、右侧竖线、右下圆角、底部横线、左下圆角、左侧竖线和左上圆角，完成整个矩形的路径。
            roundRectPath.AddLine(ptx + arcSize, pty, ptx + width - arcSize, pty);  // 顶部横线
            roundRectPath.AddArc(ptx + width - arcDiameter, pty, arcDiameter, arcDiameter, 270, 90); // 右上圆角
            roundRectPath.AddLine(ptx + width, pty + arcSize, ptx + width, pty + height - arcSize);  // 右侧竖线
            roundRectPath.AddArc(ptx + width - arcDiameter, pty + height - arcDiameter, arcDiameter, arcDiameter, 0, 90); // 右下圆角
            roundRectPath.AddLine(ptx + width - arcSize, pty + height, ptx + arcSize, pty + height);  // 底部横线
            roundRectPath.AddArc(ptx, pty + height - arcDiameter, arcDiameter, arcDiameter, 90, 90); // 左下圆角
            roundRectPath.AddLine(ptx, pty + height - arcSize, ptx, pty + arcSize);  // 左侧竖线
            roundRectPath.AddArc(ptx, pty, arcDiameter, arcDiameter, 180, 90); // 左上圆角
            roundRectPath.CloseFigure();

            //设置裁剪圆,SetClip正如你所注意到的，它不会给你一个抗锯齿的边缘
            //graphics.SetClip(&roundRectPath, Gdiplus::CombineModeIntersect);
            //Gdiplus::Image* image = Gdiplus::Bitmap::FromHBITMAP(MemoryHBitmap, NULL);
            Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromBITMAPINFO(lpbiSrc,(void*)BitmapBits);
            ON_SCOPE_EXIT([&] {	delete image; });
            Gdiplus::TextureBrush brush(image);
            auto s = graphics.Save();
            graphics.TranslateTransform(m_rcPreItem.left, m_rcPreItem.top);
            ////绘制图像
            graphics.FillPath(&brush, &roundRectPath);
            graphics.Restore(s);

            m_rcItem = m_rcPreItem;
            m_rcPaint = m_rcPrePaint;

            SelectObject(MemoryHDC, OldBitmap);
            DeleteObject(MemoryHBitmap);
            DeleteDC(MemoryHDC);

            PaintText(hDC);
            PaintBorder(hDC);
        }
    }
    else
    {
        PaintBkColor(hDC);
        PaintBkImage(hDC);
        PaintStatusImage(hDC);
        PaintForeColor(hDC);
        PaintForeImage(hDC);
        PaintText(hDC);
        PaintBorder(hDC);
    }
    return true;
}

void CControlUI::PaintBkColor(HDC hDC)
{
    if(m_dwBackColor != 0)
    {
        int nBorderSize = { 0 };
        SIZE cxyBorderRound = { 0 };
        nBorderSize = GetBorderSize();
        cxyBorderRound = GetBorderRound();

        bool bVer = (!FawTools::is_equal_nocase(m_sGradient, _T("hor")));
        if(m_dwBackColor2 != 0)
        {
            if(m_dwBackColor3 != 0)
            {
                auto RealBottom = m_rcItem.bottom;
                m_rcItem.bottom = (m_rcItem.bottom + m_rcItem.top) / 2;
                CRenderEngine::DrawGradient(hDC, m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 8);
                m_rcItem.top = m_rcItem.bottom;
                m_rcItem.bottom = RealBottom;
                CRenderEngine::DrawGradient(hDC, m_rcItem, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), bVer, 8);
            }
            else
            {
                CRenderEngine::DrawGradient(hDC, m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 16);
            }
        }
        else
        {
            //// 矩形很窄,不必再考虑圆角边框,直接 DrawColor, 或者矩形无圆角,直接 DrawColor
            //if (m_rcItem.right - m_rcItem.left <= 4 || m_rcItem.bottom - m_rcItem.top <= 4 || (cxyBorderRound.cx == 0 && cxyBorderRound.cy == 0))
            //{
            if(m_dwBackColor >= 0xFF000000) CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwBackColor));
            else CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(m_dwBackColor));
            //}
            //else
            //{
            //	const auto dwBackColor = GetAdjustColor(m_dwBackColor);
            //	const auto dwBorderColor = GetAdjustColor((IsFocused() && m_dwFocusBorderColor != 0) ? m_dwFocusBorderColor : m_dwBorderColor);
            //	//此函数用于画矩形,高度为1的线是画不出来的
            //	CRenderEngine::DrawRoundRectangle(hDC, m_rcItem.left, m_rcItem.top, m_rcItem.right - m_rcItem.left - 1, m_rcItem.bottom - m_rcItem.top - 1, cxyBorderRound.cx, 0, dwBorderColor, true, dwBackColor);
            //}
        }
    }
}

void CControlUI::PaintBkImage(HDC hDC)
{
    if(m_sBkImage.empty()) return;
    if(!DrawImage(hDC, m_sBkImage))
    {
    }
}

void CControlUI::PaintStatusImage(HDC hDC)
{
    return;
}

void CControlUI::PaintForeColor(HDC hDC)
{
    CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(m_dwForeColor));
}

void CControlUI::PaintForeImage(HDC hDC)
{
    if(m_sForeImage.empty()) return;
    DrawImage(hDC, m_sForeImage);
}

void CControlUI::PaintText(HDC hDC)
{
    return;
}

void CControlUI::PaintBorder(HDC hDC)
{
    int nBorderSize;
    SIZE cxyBorderRound = { 0 };
    RECT rcBorderSize = { 0 };
    nBorderSize = GetBorderSize();
    cxyBorderRound = GetBorderRound();
    if(m_pManager)
        rcBorderSize = GetManager()->GetDPIObj()->Scale(m_rcBorderSize);
    else
        rcBorderSize = m_rcBorderSize;

    if(m_dwBorderColor != 0 || m_dwFocusBorderColor != 0)
    {
        //画圆角边框
        if(nBorderSize > 0 && (cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0))
        {
            if(IsFocused() && m_dwFocusBorderColor != 0)
                CRenderEngine::DrawRoundRect(hDC, m_rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwFocusBorderColor), m_nBorderStyle);
            else
                CRenderEngine::DrawRoundRect(hDC, m_rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
        }
        else
        {
            if(IsFocused() && m_dwFocusBorderColor != 0 && nBorderSize > 0)
            {
                CRenderEngine::DrawRect(hDC, m_rcItem, nBorderSize, GetAdjustColor(m_dwFocusBorderColor), m_nBorderStyle);
            }
            else if(rcBorderSize.left > 0 || rcBorderSize.top > 0 || rcBorderSize.right > 0 || rcBorderSize.bottom > 0)
            {
                RECT rcBorder = { 0 };

                if(rcBorderSize.left > 0)
                {
                    rcBorder = m_rcItem;
                    rcBorder.right = rcBorder.left;
                    CRenderEngine::DrawLine(hDC, rcBorder, rcBorderSize.left, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
                }
                if(rcBorderSize.top > 0)
                {
                    rcBorder = m_rcItem;
                    rcBorder.bottom = rcBorder.top;
                    CRenderEngine::DrawLine(hDC, rcBorder, rcBorderSize.top, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
                }
                if(rcBorderSize.right > 0)
                {
                    rcBorder = m_rcItem;
                    rcBorder.right -= 1;
                    rcBorder.left = rcBorder.right;
                    CRenderEngine::DrawLine(hDC, rcBorder, rcBorderSize.right, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
                }
                if(rcBorderSize.bottom > 0)
                {
                    rcBorder = m_rcItem;
                    rcBorder.bottom -= 1;
                    rcBorder.top = rcBorder.bottom;
                    CRenderEngine::DrawLine(hDC, rcBorder, rcBorderSize.bottom, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
                }
            }
            else if(nBorderSize > 0)
            {
                CRenderEngine::DrawRect(hDC, m_rcItem, nBorderSize, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
            }
        }
    }
}

void CControlUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
    return;
}

int CControlUI::GetLeftBorderSize() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_rcBorderSize.left);
    return m_rcBorderSize.left;
}

void CControlUI::SetLeftBorderSize(int nSize)
{
    m_rcBorderSize.left = nSize;
    Invalidate();
}

int CControlUI::GetTopBorderSize() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_rcBorderSize.top);
    return m_rcBorderSize.top;
}

void CControlUI::SetTopBorderSize(int nSize)
{
    m_rcBorderSize.top = nSize;
    Invalidate();
}

int CControlUI::GetRightBorderSize() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_rcBorderSize.right);
    return m_rcBorderSize.right;
}

void CControlUI::SetRightBorderSize(int nSize)
{
    m_rcBorderSize.right = nSize;
    Invalidate();
}

int CControlUI::GetBottomBorderSize() const
{
    if(m_pManager) return m_pManager->GetDPIObj()->Scale(m_rcBorderSize.bottom);
    return m_rcBorderSize.bottom;
}

void CControlUI::SetBottomBorderSize(int nSize)
{
    m_rcBorderSize.bottom = nSize;
    Invalidate();
}

int CControlUI::GetBorderStyle() const
{
    return m_nBorderStyle;
}

void CControlUI::SetBorderStyle(int nStyle)
{
    m_nBorderStyle = nStyle;
    Invalidate();
}

} // namespace DuiLib
