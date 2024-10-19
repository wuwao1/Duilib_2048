#ifndef __UITEXTSCROLLH__
#define __UITEXTSCROLLH__

#pragma once

namespace DuiLib
{
#define ROLLTEXT_LEFT		0
#define ROLLTEXT_RIGHT		1
#define ROLLTEXT_UP 		2
#define ROLLTEXT_DOWN		3


#define ROLLTEXT_TIMERID_SPAN		50U
#define ROLLTEXT_ROLL_END_SPAN		1000*6U

class UILIB_API CRollTextUI: public CLabelUI
{
    DECLARE_DUICONTROL(CRollTextUI)
public:
    using ptr = std::shared_ptr<CRollTextUI>;
    CRollTextUI(void);
    virtual ~CRollTextUI(void);

public:
    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

public:
    void PaintText(HDC hDC) override;
    void DoEvent(TEventUI& event) override ;
    void SetPos(RECT rc, bool bNeedInvalidate = true)override ;
    void SetText(faw::string_t pstrText) override;

public:
    void BeginRoll(int nDirect = ROLLTEXT_RIGHT, LONG lTimeSpan = ROLLTEXT_TIMERID_SPAN, LONG lMaxTimeLimited = 60);
    void EndRoll();

private:
    int		m_nStep				= 5;
    int		m_nScrollPos		= 0;
    BOOL	m_bUseRoll			= FALSE;
    int		m_nRollDirection	= ROLLTEXT_LEFT;
    int		m_nText_W_H			= 0;
};

}	// namespace DuiLib

#endif // __UITEXTSCROLLH__