#include "StdAfx.h"
#include <functional>

#ifndef WIN_IMPL_BASE_HPP
#define WIN_IMPL_BASE_HPP

namespace DuiLib
{
class UILIB_API WindowImplBase
    : public CWindowWnd
    , public CNotifyPump
    , public INotifyUI
    , public IMessageFilterUI
    , public IDialogBuilderCallback
    , public IQueryControlText
{
public:
    using ptr = std::shared_ptr<WindowImplBase>;
    WindowImplBase() {};
    virtual ~WindowImplBase() {};
    // 只需主窗口重写（初始化资源与多语言接口）
    virtual void InitResource() {};
    // 每个窗口都可以重写
    virtual void InitWindow() {};
    virtual void OnFinalMessage(HWND hWnd);
    virtual void Notify(TNotifyUI& msg);

    DUI_DECLARE_MESSAGE_MAP()
    virtual void OnClick(TNotifyUI& msg);
    virtual void OnHeaderClick(TNotifyUI& msg)  {};
    virtual void OnSelectChanged(TNotifyUI& msg) {};
    virtual void OnTextChanged(TNotifyUI& msg)  {};
    virtual void OnItemSelect(TNotifyUI& msg) {};
    virtual void OnTimer(TNotifyUI& msg)  {};
    virtual BOOL IsInStaticControl(std::shared_ptr<CControlUI> pControl, POINT &pt);

protected:
    virtual faw::string_t GetSkinType()
    {
        return _T("");
    };
    virtual faw::string_t GetSkinFile() = 0;
    virtual LPCTSTR GetWindowClassName(void) const = 0;
    virtual faw::string_t GetManagerName()
    {
        return _T("");
    };
    virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
    CPaintManagerUI m_pm;

public:
    virtual UINT GetClassStyle() const;
    virtual std::shared_ptr<CControlUI> CreateControl(faw::string_t pstrClass);
    LRESULT Invoke(std::function<LRESULT()> f)
    {
        return ::SendMessage(m_hWnd, WM_USER + 0x101, 1, (LPARAM) &f);
    }
    void AsyncInvoke(std::function<LRESULT()> f)
    {
        ::PostMessage(m_hWnd, WM_USER + 0x101, 0, (LPARAM) new decltype(f)(f));
    }
    template<typename Func, typename... Args>
    //typename std::enable_if<std::is_same<typename std::result_of<Func(Args...)>::type, bool>::value>::type
    //requires std::invocable<Func, Args...>&& std::same_as<std::invoke_result_t<Func, Args...>, bool>
    void AsyncInvoke(Func&& func, Args&&... args)
    {
        static_assert(std::is_invocable_v<Func, Args...>, "Func must be invocable with Args");
        static_assert(std::is_same_v<std::invoke_result_t<Func, Args...>, bool>, "Func must return a bool value");

        auto boundFunc = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        auto* funcPtr = new std::function<decltype(func(args...))()>(boundFunc);
        ::PostMessage(m_hWnd, WM_USER + 0x101, 0, reinterpret_cast<LPARAM>(funcPtr));
    }
    virtual faw::string_t QueryControlText(faw::string_t lpstrId, faw::string_t lpstrType);

    virtual std::optional<LRESULT> MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);

#if defined(WIN32) && !defined(UNDER_CE)
    virtual std::optional<LRESULT> OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
    virtual std::optional<LRESULT> OnTray(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual std::optional<LRESULT> OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual std::optional<LRESULT> HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT                HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LONG GetStyle();
};
}

#endif // WIN_IMPL_BASE_HPP
