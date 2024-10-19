﻿#include "StdAfx.h"
#include "UIRing.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CRingUI)

CRingUI::CRingUI() {}

CRingUI::~CRingUI()
{
    if(m_pManager) m_pManager->KillTimer(shared_from_this(), RING_TIMER_ID);

    DeleteImage();
}

faw::string_t CRingUI::GetClass() const
{
    return _T("RingUI");
}

std::shared_ptr<CControlUI> CRingUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == _T("Ring")) return std::dynamic_pointer_cast<CRingUI>(shared_from_this());
    return CLabelUI::GetInterface(pstrName);
}

void CRingUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("bkimage")) SetBkImage(pstrValue);
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CRingUI::SetBkImage(faw::string_t pStrImage)
{
    if(m_sBkImage == pStrImage) return;
    m_sBkImage = pStrImage;
    DeleteImage();
    Invalidate();
}

void CRingUI::PaintBkImage(HDC hDC)
{
    if(!m_pBkimage)
        InitImage();
    if(!m_pBkimage)
        return;

    RECT rcItem = m_rcItem;
    int iWidth = rcItem.right - rcItem.left;
    int iHeight = rcItem.bottom - rcItem.top;
    Gdiplus::PointF centerPos((Gdiplus::REAL)(rcItem.left + iWidth / 2), (Gdiplus::REAL)(rcItem.top + iHeight / 2));

    Gdiplus::Graphics graphics(hDC);
    graphics.TranslateTransform(centerPos.X, centerPos.Y);
    graphics.RotateTransform(m_fCurAngle);
    graphics.TranslateTransform(-centerPos.X, -centerPos.Y); //还原源点
    graphics.DrawImage(m_pBkimage, rcItem.left, rcItem.top, iWidth, iHeight);

}

void CRingUI::DoEvent(TEventUI& event)
{
    if(event.Type == UIEVENT_TIMER && event.wParam == RING_TIMER_ID)
    {
        if(m_fCurAngle > 359)
        {
            m_fCurAngle = 0;
        }
        m_fCurAngle += 36.0;
        Invalidate();
    }
    else
    {
        CLabelUI::DoEvent(event);
    }
}

void CRingUI::InitImage()
{
    TimageArgs imageArgs;
    imageArgs.bitmap = GetBkImage();
    m_pBkimage = CRenderEngine::GdiplusLoadImage(imageArgs);
    if(nullptr == m_pBkimage) return;
    if(m_pManager) m_pManager->SetTimer(shared_from_this(), RING_TIMER_ID, 100);
}

void CRingUI::DeleteImage()
{
    if(m_pBkimage)
    {
        delete m_pBkimage;
        m_pBkimage = nullptr;
    }
}
}
