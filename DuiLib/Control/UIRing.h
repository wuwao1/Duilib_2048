#ifndef __UIROTATE_H__
#define __UIROTATE_H__

#pragma once

namespace DuiLib
{
class UILIB_API CRingUI: public CLabelUI
{
    DECLARE_DUICONTROL(CRingUI)
public:
    using ptr = std::shared_ptr<CRingUI>;
    CRingUI();
    virtual ~CRingUI();

    faw::string_t GetClass() const override;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName)override;
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)override;
    void SetBkImage(faw::string_t pStrImage) ;
    virtual void DoEvent(TEventUI& event)override;
    virtual void PaintBkImage(HDC hDC)override;

private:
    void InitImage();
    void DeleteImage();

public:
    float			m_fCurAngle		= 0.0f;
    Gdiplus::Image	*m_pBkimage		= nullptr;
};
}

#endif // __UIROTATE_H__