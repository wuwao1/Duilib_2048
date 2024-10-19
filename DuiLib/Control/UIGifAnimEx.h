#ifndef GifAnimUIEX_h__
#define GifAnimUIEX_h__
#pragma once
/* write by wangji 2016.03.16
** 解决多个gif控件在gdi+环境下占用CPU过高的问题，本类采用ximage替代
** 注意：使用的时候在预编译头文件中包含UIlib.h前先定义宏USE_XIMAGE_EFFECT
** #define USE_XIMAGE_EFFECT
** #include "UIlib.h"
*/
#ifdef USE_XIMAGE_EFFECT
namespace DuiLib
{
class CLabelUI;

class UILIB_API CGifAnimExUI: public CLabelUI
{
    DECLARE_DUICONTROL(CGifAnimExUI)
public:
    using ptr = std::shared_ptr<CGifAnimExUI>;
    CGifAnimExUI(void);
    virtual ~CGifAnimExUI(void);
public:
    virtual faw::string_t	GetClass() const override;
    virtual std::shared_ptr<CControlUI>	GetInterface(faw::string_t pstrName) override;
    virtual void Init() override;
    virtual void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue) override;
    virtual void SetVisible(bool bVisible = true) override;
    virtual void SetInternVisible(bool bVisible = true) override;
    virtual bool DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl) override;
    virtual void DoEvent(TEventUI& event) override;
    void StartAnim();
    void StopAnim();
protected:
    struct Imp;
    Imp* m_pImp;
};
}
#endif //USE_XIMAGE_EFFECT
#endif // GifAnimUIEx_h__
