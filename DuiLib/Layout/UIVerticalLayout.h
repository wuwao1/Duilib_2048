﻿#ifndef __UIVERTICALLAYOUT_H__
#define __UIVERTICALLAYOUT_H__

#pragma once

namespace DuiLib
{
class UILIB_API CVerticalLayoutUI: public CContainerUI
{
    DECLARE_DUICONTROL(CVerticalLayoutUI)
public:
    using ptr = std::shared_ptr<CVerticalLayoutUI>;
    CVerticalLayoutUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    UINT GetControlFlags() const;

    void SetSepHeight(int iHeight);
    int GetSepHeight() const;
    void SetSepImmMode(bool bImmediately);
    bool IsSepImmMode() const;
    void SetAutoCalcHeight(bool autoCalcHeight);
    bool IsAutoCalcHeight() const;
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);
    void DoEvent(TEventUI& event);

    SIZE EstimateSize(SIZE szAvailable) override;
    void SetPos(RECT rc, bool bNeedInvalidate = true);
    void DoPostPaint(HDC hDC, const RECT& rcPaint);
    bool IsDynamic(POINT &pt) const override;

    RECT GetThumbRect(bool bUseNew = false) const;

protected:
    int		m_iSepHeight		= 0;
    UINT	m_uButtonState		= 0;
    POINT	ptLastMouse			= { 0 };
    RECT	m_rcNewPos			= { 0, 0, 0, 0 };
    bool	m_bImmMode			= false;
    bool	m_autoCalcHeight	= false;
};
}
#endif // __UIVERTICALLAYOUT_H__
