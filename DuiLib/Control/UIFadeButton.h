#ifndef __UIFADEBUTTON_H__
#define __UIFADEBUTTON_H__

#include "UIAnimation.h"
#pragma once

namespace DuiLib
{

class UILIB_API CFadeButtonUI: public CButtonUI, public CUIAnimation
{
    DECLARE_DUICONTROL(CFadeButtonUI)
public:
    using ptr = std::shared_ptr<CFadeButtonUI>;
    CFadeButtonUI();
    virtual ~CFadeButtonUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    void SetNormalImage(faw::string_t pStrImage);

    void DoEvent(TEventUI& event);
    void OnTimer(int nTimerID);
    void PaintStatusImage(HDC hDC);

    virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
    virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
    virtual void OnAnimationStop(INT nAnimationID) {}

protected:
    faw::string_t m_sOldImage;
    faw::string_t m_sNewImage;
    faw::string_t m_sLastImage;
    unsigned char       m_bFadeAlpha;
    BOOL       m_bMouseHove		= FALSE;
    BOOL       m_bMouseLeave	= FALSE;
    enum
    {
        FADE_ELLAPSE = 10,
        FADE_FRAME_COUNT = 30,
    };
};

} // namespace DuiLib

#endif // __UIFADEBUTTON_H__