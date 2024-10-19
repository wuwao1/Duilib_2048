#ifndef __UIHOTKEY_H__
#define __UIHOTKEY_H__
#pragma once

namespace DuiLib
{
class CHotKeyUI;

class UILIB_API CHotKeyWnd: public CWindowWnd
{
public:
    using ptr = std::shared_ptr<CHotKeyWnd>;
    CHotKeyWnd(void);

public:
    void        Init(std::shared_ptr<CHotKeyUI> pOwner);
    RECT        CalPos();
    LPCTSTR     GetWindowClassName() const;
    void        OnFinalMessage(HWND hWnd);
    LPCTSTR     GetSuperClassName() const;
    LRESULT     HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    std::optional<LRESULT> OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
    std::optional<LRESULT> OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
    void            SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);
    void            GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const;
    DWORD           GetHotKey(void) const;
    faw::string_t   GetHotKeyName();
    void            SetRules(WORD wInvalidComb, WORD wModifiers);
    faw::string_t   GetKeyName(UINT vk, BOOL fExtended);
protected:
    std::shared_ptr<CHotKeyUI> m_pOwner;
    HBRUSH m_hBkBrush;
    bool m_bInit;
};

class UILIB_API CHotKeyUI: public CLabelUI
{
    DECLARE_DUICONTROL(CHotKeyUI)
    friend CHotKeyWnd;
public:
    using ptr = std::shared_ptr<CHotKeyUI>;
    CHotKeyUI();
    faw::string_t GetClass() const override;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName)override;
    UINT GetControlFlags() const override;
    void SetEnabled(bool bEnable = true)override;
    void SetText(faw::string_t pstrText)override;
    faw::string_t GetNormalImage();
    void SetNormalImage(faw::string_t pStrImage);
    faw::string_t GetHotImage();
    void SetHotImage(faw::string_t pStrImage);
    faw::string_t GetFocusedImage();
    void SetFocusedImage(faw::string_t pStrImage);
    faw::string_t GetDisabledImage();
    void SetDisabledImage(faw::string_t pStrImage) ;

    void SetPos(RECT rc, bool bNeedInvalidate = true);
    void SetVisible(bool bVisible = true)override;
    void SetInternVisible(bool bVisible = true)override;
    SIZE EstimateSize(SIZE szAvailable)override;
    void DoEvent(TEventUI& event)override;
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)override;

    void PaintStatusImage(HDC hDC)override;
    void PaintText(HDC hDC)override;
public:
    void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const;
    DWORD GetHotKey(void) const;
    void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);

protected:
    CHotKeyWnd * m_pWindow;
    UINT m_uButtonState;
    faw::string_t m_sNormalImage;
    faw::string_t m_sHotImage;
    faw::string_t m_sFocusedImage;
    faw::string_t m_sDisabledImage;

protected:
    WORD m_wVirtualKeyCode;
    WORD m_wModifiers;
};
}


#endif