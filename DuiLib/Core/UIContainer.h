#ifndef __UICONTAINER_H__
#define __UICONTAINER_H__

#pragma once

namespace DuiLib
{
/////////////////////////////////////////////////////////////////////////////////////
//

class IContainerUI
{
public:
    virtual std::shared_ptr<CControlUI> GetItemAt(int iIndex) const = 0;
    virtual int         GetItemIndex(std::shared_ptr<CControlUI> pControl) const = 0;
    virtual bool        SetItemIndex(std::shared_ptr<CControlUI> pControl, int iIndex) = 0;
    virtual int         GetCount() const = 0;
    virtual bool        Add(std::shared_ptr<CControlUI> pControl) = 0;
    virtual bool        AddAt(std::shared_ptr<CControlUI> pControl, int iIndex) = 0;
    virtual bool        Remove(std::shared_ptr<CControlUI> pControl) = 0;
    virtual bool        Remove(CControlUI* pControl) = 0;
    virtual bool        RemoveAt(int iIndex) = 0;
    virtual void        RemoveAll() = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//
class CScrollBarUI;

class UILIB_API CContainerUI: public CControlUI, public IContainerUI
{
    DECLARE_DUICONTROL(CContainerUI)

public:
    using ptr = std::shared_ptr<CContainerUI>;
    CContainerUI();
    virtual ~CContainerUI();

public:
    faw::string_t          GetClass() const;
    std::shared_ptr<CControlUI>  GetInterface(faw::string_t pstrName);

    std::shared_ptr<CControlUI>  GetItemAt(int iIndex) const;
    int                  GetItemIndex(std::shared_ptr<CControlUI> pControl) const;
    bool                 SetItemIndex(std::shared_ptr<CControlUI> pControl, int iIndex);
    int                  GetCount() const;
    bool                 Add(std::shared_ptr<CControlUI> pControl);
    bool                 AddAt(std::shared_ptr<CControlUI> pControl, int iIndex);
    bool                 Remove(std::shared_ptr<CControlUI> pControl);
    bool                 Remove(CControlUI* pControl);
    bool                 RemoveAt(int iIndex);
    void                 RemoveAll();

    void                 DoEvent(TEventUI& event);
    void                 SetVisible(bool bVisible = true);
    void                 SetInternVisible(bool bVisible = true);
    void                 SetEnabled(bool bEnabled);
    void                 SetMouseEnabled(bool bEnable = true);

    virtual RECT         GetInset() const;
    virtual void         SetInset(RECT rcInset);  // 设置内边距，相当于设置客户区
    virtual int          GetChildPadding() const;
    virtual void         SetChildPadding(int iPadding);
    virtual UINT         GetChildAlign(std::shared_ptr<CControlUI>  pControl = nullptr) const;
    virtual void         SetChildAlign(UINT iAlign);
    virtual UINT         GetChildVAlign() const;
    virtual void         SetChildVAlign(UINT iVAlign);
    virtual bool         IsAutoDestroy() const;
    virtual void         SetAutoDestroy(bool bAuto);
    virtual bool         IsDelayedDestroy() const;
    virtual void         SetDelayedDestroy(bool bDelayed);
    virtual bool         IsMouseChildEnabled() const;
    virtual void         SetMouseChildEnabled(bool bEnable = true);

    virtual int         FindSelectable(int iIndex, bool bForward = true) const;

    RECT                GetClientPos() const override;
    void                SetPos(RECT rc, bool bNeedInvalidate = true);
    void                Move(SIZE szOffset, bool bNeedInvalidate = true);
    bool                DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl);
    virtual bool        PaintItem(HDC hDC, RECT rcPaint, std::shared_ptr<CControlUI> pStopControl, RECT rcIntersect, bool bTranslate = false);
    void                EnableTranslate(bool bTranslate);

    void                SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    void                SetManager(CPaintManagerUI* pManager, std::shared_ptr<CControlUI> pParent, bool bInit = true);
    std::shared_ptr<CControlUI>         FindControl(FIND_CONTROL_PROC Proc, LPVOID pData, UINT uFlags);

    bool                SetSubControlText(faw::string_t pstrSubControlName, faw::string_t pstrText);
    bool                SetSubControlFixedHeight(faw::string_t pstrSubControlName, int cy);
    bool                SetSubControlFixedWdith(faw::string_t pstrSubControlName, int cx);
    bool                SetSubControlUserData(faw::string_t pstrSubControlName, faw::string_t pstrText);

    faw::string_t       GetSubControlText(faw::string_t pstrSubControlName);
    int                 GetSubControlFixedHeight(faw::string_t pstrSubControlName);
    int                 GetSubControlFixedWdith(faw::string_t pstrSubControlName);
    const faw::string_t GetSubControlUserData(faw::string_t pstrSubControlName);
    std::shared_ptr<CControlUI>         FindSubControl(faw::string_t pstrSubControlName);

    virtual SIZE        GetScrollPos() const;
    virtual SIZE        GetScrollRange() const;
    virtual void        SetScrollPos(SIZE szPos, bool bMsg = true);
    virtual void        SetScrollStepSize(int nSize);
    virtual int         GetScrollStepSize() const;
    virtual void        LineUp();
    virtual void        LineDown();
    virtual void        PageUp();
    virtual void        PageDown();
    virtual void        HomeUp();
    virtual void        EndDown();
    virtual void        LineLeft();
    virtual void        LineRight();
    virtual void        PageLeft();
    virtual void        PageRight();
    virtual void        HomeLeft();
    virtual void        EndRight();
    virtual void        EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
    virtual std::shared_ptr<CScrollBarUI> GetVerticalScrollBar() const;
    virtual std::shared_ptr<CScrollBarUI> GetHorizontalScrollBar() const;

    virtual void        AnimateShow(bool show);
    virtual void        UnderAnimateShow();
    virtual void        AnimateHide(bool show);
    virtual void        UnderAnimateHide();

protected:
    virtual void        SetFloatPos(int iIndex);
    virtual void        ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);

protected:
    std::list<std::shared_ptr<CControlUI>>	        m_items;
    RECT			                m_rcInset					= { 0 };
    int				                m_iChildPadding				= 0;
    UINT			                m_iChildAlign				= DT_LEFT;
    UINT			                m_iChildVAlign				= DT_TOP;
    bool			                m_bAutoDestroy				= true;
    bool			                m_bDelayedDestroy			= true;
    bool			                m_bMouseChildEnabled		= true;
    int				                m_nScrollStepSize			= 60;

    int                             m_nOldWidth                 = 0;
    int                             m_nOldHeight                = 0;

    std::shared_ptr<CScrollBarUI>   m_pVerticalScrollBar;
    std::shared_ptr<CScrollBarUI>   m_pHorizontalScrollBar;
    faw::string_t	                m_sVerticalScrollBarStyle;
    faw::string_t	                m_sHorizontalScrollBarStyle;

    bool                            m_bAnimate                  = false;

    bool                            m_bTranslate                = true;
    RECT			                m_rcPreItem                 = { 0 };
};

} // namespace DuiLib

#endif // __UICONTAINER_H__
