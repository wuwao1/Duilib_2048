﻿#ifndef __UIPROGRESS_H__
#define __UIPROGRESS_H__

#pragma once

namespace DuiLib
{
class UILIB_API CProgressUI: public CLabelUI
{
    DECLARE_DUICONTROL(CProgressUI)
public:
    using ptr = std::shared_ptr<CProgressUI>;
    CProgressUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    bool IsShowText();
    void SetShowText(bool bShowText = true);
    bool IsHorizontal();
    void SetHorizontal(bool bHorizontal = true);
    bool IsStretchForeImage();
    void SetStretchForeImage(bool bStretchForeImage = true);
    int GetMinValue() const;
    void SetMinValue(int nMin);
    int GetMaxValue() const;
    void SetMaxValue(int nMax);
    int GetValue() const;
    void SetValue(int nValue);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);
    void PaintForeColor(HDC hDC);
    void PaintForeImage(HDC hDC);
    virtual void UpdateText();

protected:
    bool		m_bShowText			= false;
    bool		m_bHorizontal		= true;
    bool		m_bStretchForeImage	= true;
    int			m_nMax				= 100;
    int			m_nMin				= 0;
    int			m_nValue			= 0;

    faw::string_t	m_sForeImageModify;
};

} // namespace DuiLib

#endif // __UIPROGRESS_H__
