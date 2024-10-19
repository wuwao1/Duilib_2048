﻿#ifndef __UILABEL_H__
#define __UILABEL_H__

#pragma once

namespace DuiLib
{
class UILIB_API CLabelUI: public CControlUI
{
    DECLARE_DUICONTROL(CLabelUI)
public:
    using ptr = std::shared_ptr<CLabelUI>;
    CLabelUI();
    virtual ~CLabelUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    UINT GetControlFlags() const;

    void SetTextStyle(UINT uStyle);
    UINT GetTextStyle() const;
    void SetTextColor(DWORD dwTextColor);
    DWORD GetTextColor() const;
    void SetDisabledTextColor(DWORD dwTextColor);
    DWORD GetDisabledTextColor() const;
    void SetFont(int index);
    int GetFont() const;
    RECT GetTextPadding() const;
    void SetTextPadding(RECT rc);
    bool IsShowHtml();
    void SetShowHtml(bool bShowHtml = true);

    SIZE EstimateSize(SIZE szAvailable);
    void DoEvent(TEventUI& event);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    void PaintText(HDC hDC);

    virtual bool GetAutoCalcWidth() const;
    virtual void SetAutoCalcWidth(bool bAutoCalcWidth);
    virtual bool GetAutoCalcHeight() const;
    virtual void SetAutoCalcHeight(bool bAutoCalcHeight);
    virtual void SetText(faw::string_t pstrText);

protected:
    DWORD	m_dwTextColor			= 0;
    DWORD	m_dwDisabledTextColor	= 0;
    int		m_iFont					= -1;
    UINT	m_uTextStyle;
    RECT	m_rcTextPadding			= { 0 };
    bool	m_bShowHtml				= false;
    bool	m_bAutoCalcWidth		= false;
    bool	m_bAutoCalcHeight		= false;

    SIZE    m_szAvailableLast		= { 0, 0 };
    SIZE    m_cxyFixedLast			= { 0, 0 };
    bool    m_bNeedEstimateSize		= false;
    bool    m_bCalcByParent         = false;
};
}

#endif // __UILABEL_H__
