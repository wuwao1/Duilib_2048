﻿#ifndef __UIANIMATIONTABLAYOUT_H__
#define __UIANIMATIONTABLAYOUT_H__

namespace DuiLib
{
class UILIB_API CAnimationTabLayoutUI: public CTabLayoutUI, public CUIAnimation
{
    DECLARE_DUICONTROL(CAnimationTabLayoutUI)
public:
    using ptr = std::shared_ptr<CAnimationTabLayoutUI>;
    CAnimationTabLayoutUI();
    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    void Init()override;
    bool SelectItem(int iIndex);
    void AnimationSwitch();
    void DoEvent(TEventUI& event);
    void OnTimer(int nTimerID);

    virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
    virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
    virtual void OnAnimationStop(INT nAnimationID);

    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

protected:
    bool		m_bIsVerticalDirection	= false;
    int			m_nPositiveDirection	= 1;
    RECT		m_rcCurPos				= { 0 };
    RECT		m_rcItemOld				= { 0 };
    std::shared_ptr<CControlUI> m_pCurrentControl		= nullptr;
    bool		m_bControlVisibleFlag	= false;
    enum
    {
        TAB_ANIMATION_ELLAPSE = 10,
        TAB_ANIMATION_FRAME_COUNT = 15,
    };
};
}
#endif // __UIANIMATIONTABLAYOUT_H__