﻿#ifndef __UILIST_H__
#define __UILIST_H__

namespace DuiLib
{
/////////////////////////////////////////////////////////////////////////////////////
//

typedef int (CALLBACK *PULVCompareFunc)(UINT_PTR, UINT_PTR, UINT_PTR);

class CListHeaderUI;

#define UILIST_MAX_COLUMNS 32

typedef struct tagTListInfoUI
{
    int nColumns;
    RECT rcColumn[UILIST_MAX_COLUMNS];
    int nFont;
    UINT uTextStyle;
    RECT rcTextPadding = { 0 };
    DWORD dwTextColor;
    DWORD dwBkColor;
    faw::string_t sBkImage;
    bool bAlternateBk;
    DWORD dwSelectedTextColor;
    DWORD dwSelectedBkColor;
    faw::string_t sSelectedImage;
    DWORD dwHotTextColor;
    DWORD dwHotBkColor;
    faw::string_t sHotImage;
    DWORD dwDisabledTextColor;
    DWORD dwDisabledBkColor;
    faw::string_t sDisabledImage;
    DWORD dwLineColor;
    bool bShowRowLine;
    bool bShowColumnLine;
    bool bShowHtml;
    bool bMultiExpandable;
    bool bRSelected;
} TListInfoUI;


/////////////////////////////////////////////////////////////////////////////////////
//

class IListCallbackUI
{
public:
    virtual faw::string_t GetItemText(std::shared_ptr<CControlUI> pList, int iItem, int iSubItem) = 0;
};

class IListOwnerUI
{
public:
    virtual UINT GetListType() = 0;
    virtual TListInfoUI* GetListInfo() = 0;
    virtual int GetCurSel() const = 0;
    virtual bool SelectItem(int iIndex, bool bTakeFocus = false) = 0;
    virtual bool SelectMultiItem(int iIndex, bool bTakeFocus = false) = 0;
    virtual bool UnSelectItem(int iIndex, bool bOthers = false) = 0;
    virtual void DoEvent(TEventUI& event) = 0;
};

class IListUI: public IListOwnerUI
{
public:
    virtual std::shared_ptr<CListHeaderUI> GetHeader() const = 0;
    virtual std::shared_ptr<CContainerUI> GetList() const = 0;
    virtual IListCallbackUI* GetTextCallback() const = 0;
    virtual void SetTextCallback(IListCallbackUI* pCallback) = 0;
    virtual bool ExpandItem(int iIndex, bool bExpand = true) = 0;
    virtual int GetExpandedItem() const = 0;

    virtual void SetMultiSelect(bool bMultiSel) = 0;
    virtual bool IsMultiSelect() const = 0;
    virtual void SelectAllItems() = 0;
    virtual void UnSelectAllItems() = 0;
    virtual int GetSelectItemCount() const = 0;
    virtual int GetNextSelItem(int nItem) const = 0;
};

class IListItemUI
{
public:
    virtual int GetIndex() const = 0;
    virtual void SetIndex(int iIndex) = 0;
    virtual std::shared_ptr<IListOwnerUI> GetOwner() = 0;
    virtual void SetOwner(std::shared_ptr<CControlUI> pOwner) = 0;
    virtual bool IsSelected() const = 0;
    virtual bool Select(bool bSelect = true) = 0;
    virtual bool SelectMulti(bool bSelect = true) = 0;
    virtual bool IsExpanded() const = 0;
    virtual bool Expand(bool bExpand = true) = 0;
    virtual void DrawItemText(HDC hDC, const RECT& rcItem) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CListBodyUI;
class CListHeaderUI;
class CEditUI;
class CComboBoxUI;
class UILIB_API CListUI: public CVerticalLayoutUI, public IListUI
{
    DECLARE_DUICONTROL(CListUI)

public:
    using ptr = std::shared_ptr<CListUI>;
    CListUI();

    faw::string_t GetClass() const;
    UINT GetControlFlags() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    bool GetScrollSelect();
    void SetScrollSelect(bool bScrollSelect);
    int GetCurSel() const;
    int GetCurSelActivate() const;
    bool SelectItem(int iIndex, bool bTakeFocus = false);
    bool SelectItemActivate(int iIndex);     // 双击选中

    bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
    void SetMultiSelect(bool bMultiSel);
    bool IsMultiSelect() const;
    bool UnSelectItem(int iIndex, bool bOthers = false);
    void SelectAllItems();
    void UnSelectAllItems();
    int GetSelectItemCount() const;
    int GetNextSelItem(int nItem) const;

    bool IsFixedScrollbar();
    void SetFixedScrollbar(bool bFixed);
    RECT GetClientPos() const override;

    std::shared_ptr<CListHeaderUI> GetHeader() const;
    std::shared_ptr<CContainerUI> GetList() const;
    UINT GetListType();
    TListInfoUI* GetListInfo();

    std::shared_ptr<CControlUI> GetItemAt(int iIndex) const;
    int GetItemIndex(std::shared_ptr<CControlUI> pControl) const;
    bool SetItemIndex(std::shared_ptr<CControlUI> pControl, int iIndex);
    int GetCount() const;
    bool Add(std::shared_ptr<CControlUI> pControl);
    bool AddAt(std::shared_ptr<CControlUI> pControl, int iIndex);
    bool Remove(std::shared_ptr<CControlUI> pControl);
    bool Remove(CControlUI* pControl);
    bool RemoveAt(int iIndex);
    void RemoveAll();

    void EnsureVisible(int iIndex);
    void Scroll(int dx, int dy);

    bool IsDelayedDestroy() const;
    void SetDelayedDestroy(bool bDelayed);
    int GetChildPadding() const;
    void SetChildPadding(int iPadding);

    void SetItemFont(int index);
    void SetItemTextStyle(UINT uStyle);
    void SetItemTextPadding(RECT rc);
    void SetItemTextColor(DWORD dwTextColor);
    void SetItemBkColor(DWORD dwBkColor);
    void SetItemBkImage(faw::string_t pStrImage);
    void SetAlternateBk(bool bAlternateBk);
    void SetSelectedItemTextColor(DWORD dwTextColor);
    void SetSelectedItemBkColor(DWORD dwBkColor);
    void SetSelectedItemImage(faw::string_t pStrImage);
    void SetHotItemTextColor(DWORD dwTextColor);
    void SetHotItemBkColor(DWORD dwBkColor);
    void SetHotItemImage(faw::string_t pStrImage);
    void SetDisabledItemTextColor(DWORD dwTextColor);
    void SetDisabledItemBkColor(DWORD dwBkColor);
    void SetDisabledItemImage(faw::string_t pStrImage);
    void SetItemLineColor(DWORD dwLineColor);
    void SetItemShowRowLine(bool bShowLine = false);
    void SetItemShowColumnLine(bool bShowLine = false);
    bool IsItemShowHtml();
    void SetItemShowHtml(bool bShowHtml = true);
    bool IsItemRSelected();
    void SetItemRSelected(bool bSelected = true);
    RECT GetItemTextPadding() const;
    DWORD GetItemTextColor() const;
    DWORD GetItemBkColor() const;
    faw::string_t GetItemBkImage() const;
    bool IsAlternateBk() const;
    DWORD GetSelectedItemTextColor() const;
    DWORD GetSelectedItemBkColor() const;
    faw::string_t GetSelectedItemImage() const;
    DWORD GetHotItemTextColor() const;
    DWORD GetHotItemBkColor() const;
    faw::string_t GetHotItemImage() const;
    DWORD GetDisabledItemTextColor() const;
    DWORD GetDisabledItemBkColor() const;
    faw::string_t GetDisabledItemImage() const;
    DWORD GetItemLineColor() const;

    void SetMultiExpanding(bool bMultiExpandable);
    int GetExpandedItem() const;
    bool ExpandItem(int iIndex, bool bExpand = true);

    void SetPos(RECT rc, bool bNeedInvalidate = true);
    void Move(SIZE szOffset, bool bNeedInvalidate = true);
    void DoEvent(TEventUI& event);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    IListCallbackUI* GetTextCallback() const;
    void SetTextCallback(IListCallbackUI* pCallback);

    SIZE GetScrollPos() const;
    SIZE GetScrollRange() const;
    void SetScrollPos(SIZE szPos, bool bMsg = true);
    void LineUp();
    void LineDown();
    void PageUp();
    void PageDown();
    void HomeUp();
    void EndDown();
    void LineLeft();
    void LineRight();
    void PageLeft();
    void PageRight();
    void HomeLeft();
    void EndRight();
    void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
    virtual std::shared_ptr<CScrollBarUI> GetVerticalScrollBar() const;
    virtual std::shared_ptr<CScrollBarUI> GetHorizontalScrollBar() const;
    BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

    virtual BOOL CheckColumEditable(int nColum)
    {
        return FALSE;
    };
    virtual std::shared_ptr<CRichEditUI> GetEditUI()
    {
        return nullptr;
    };
    virtual BOOL CheckColumComboBoxable(int nColum)
    {
        return FALSE;
    };
    virtual std::shared_ptr<CComboBoxUI> GetComboBoxUI()
    {
        return nullptr;
    };

protected:
    int GetMinSelItemIndex();
    int GetMaxSelItemIndex();

protected:
    bool				m_bFixedScrollbar	= false;
    bool				m_bScrollSelect		= false;
    int					m_iCurSel			= -1;
    bool				m_bMultiSel			= false;
    CStdPtrArray		m_aSelItems;
    int					m_iCurSelActivate	= 0;	// 双击的列
    int					m_iExpandedItem		= -1;
    IListCallbackUI		*m_pCallback		= nullptr;
    std::shared_ptr<CListBodyUI>    m_pList;
    std::shared_ptr<CListHeaderUI>  m_pHeader;
    TListInfoUI			m_ListInfo;

};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListBodyUI: public CVerticalLayoutUI
{
public:
    using ptr = std::shared_ptr<CListBodyUI>;
    CListBodyUI(CListUI* pOwner);


    int GetScrollStepSize() const;
    void SetScrollPos(SIZE szPos, bool bMsg = true);
    void SetPos(RECT rc, bool bNeedInvalidate = true);
    void DoEvent(TEventUI& event);
    BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);
protected:
    static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
    int __cdecl ItemComareFunc(const void *item1, const void *item2);
protected:
    CListUI* m_pOwner;
    PULVCompareFunc m_pCompareFunc;
    UINT_PTR m_compareData;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListHeaderUI: public CHorizontalLayoutUI
{
    DECLARE_DUICONTROL(CListHeaderUI)
public:
    using ptr = std::shared_ptr<CListHeaderUI>;
    CListHeaderUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    SIZE EstimateSize(SIZE szAvailable);
    void SetPos(RECT rc, bool bNeedInvalidate = true);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    void SetScaleHeader(bool bIsScale);
    bool IsScaleHeader() const;

private:
    bool m_bIsScaleHeader;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListHeaderItemUI: public CContainerUI
{
    DECLARE_DUICONTROL(CListHeaderItemUI)

public:
    using ptr = std::shared_ptr<CListHeaderItemUI>;
    CListHeaderItemUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    UINT GetControlFlags() const;

    void SetEnabled(bool bEnable = true);

    bool IsDragable() const;
    void SetDragable(bool bDragable);
    DWORD GetSepWidth() const;
    void SetSepWidth(int iWidth);
    DWORD GetTextStyle() const;
    void SetTextStyle(UINT uStyle);
    DWORD GetTextColor() const;
    void SetTextColor(DWORD dwTextColor);
    void SetTextPadding(RECT rc);
    RECT GetTextPadding() const;
    void SetFont(int index);
    bool IsShowHtml();
    void SetShowHtml(bool bShowHtml = true);
    faw::string_t GetNormalImage() const;
    void SetNormalImage(faw::string_t pStrImage);
    faw::string_t GetHotImage() const;
    void SetHotImage(faw::string_t pStrImage);
    faw::string_t GetPushedImage() const;
    void SetPushedImage(faw::string_t pStrImage);
    faw::string_t GetFocusedImage() const;
    void SetFocusedImage(faw::string_t pStrImage);
    faw::string_t GetSepImage() const;
    void SetSepImage(faw::string_t pStrImage);
    void SetScale(int nScale);
    int GetScale() const;

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);
    RECT GetThumbRect() const;

    void PaintText(HDC hDC);
    void PaintStatusImage(HDC hDC);

protected:
    POINT ptLastMouse = { 0 };
    bool m_bDragable;
    UINT m_uButtonState;
    int m_iSepWidth;
    DWORD m_dwTextColor;
    int m_iFont;
    UINT m_uTextStyle;
    bool m_bShowHtml;
    RECT m_rcTextPadding = { 0 };
    faw::string_t m_sNormalImage;
    faw::string_t m_sHotImage;
    faw::string_t m_sPushedImage;
    faw::string_t m_sFocusedImage;
    faw::string_t m_sSepImage;
    faw::string_t m_sSepImageModify;
    int m_nScale;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListElementUI: public CControlUI, public IListItemUI
{
public:
    using ptr = std::shared_ptr<CListElementUI>;
    CListElementUI();

    faw::string_t GetClass() const;
    UINT GetControlFlags() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    void SetEnabled(bool bEnable = true);

    int GetIndex() const;
    void SetIndex(int iIndex);

    std::shared_ptr<IListOwnerUI> GetOwner();
    void SetOwner(std::shared_ptr<CControlUI> pOwner);
    void SetVisible(bool bVisible = true);

    bool IsSelected() const;
    bool Select(bool bSelect = true);
    bool SelectMulti(bool bSelect = true);
    bool IsExpanded() const;
    bool Expand(bool bExpand = true);

    void Invalidate();  // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
    bool Activate();

    void DoEvent(TEventUI& event);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    void DrawItemBk(HDC hDC, const RECT& rcItem);
    void DrawItemText(HDC hDC, const RECT& rcItem) {};

protected:
    int m_iIndex;
    bool m_bSelected;
    UINT m_uButtonState;
    std::shared_ptr<IListOwnerUI> m_pOwner;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListLabelElementUI: public CListElementUI
{
    DECLARE_DUICONTROL(CListLabelElementUI)
public:
    using ptr = std::shared_ptr<CListLabelElementUI>;
    CListLabelElementUI();
    CListLabelElementUI(faw::string_t text, int height = 20);

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);
    bool DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl);

    void DrawItemText(HDC hDC, const RECT& rcItem);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListTextElementUI: public CListLabelElementUI
{
    DECLARE_DUICONTROL(CListTextElementUI)
public:
    using ptr = std::shared_ptr<CListTextElementUI>;
    CListTextElementUI();
    virtual ~CListTextElementUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    UINT GetControlFlags() const;

    faw::string_t GetText(int iIndex) const;
    faw::string_t GetText() const
    {
        return _T("");
    };
    void SetText(int iIndex, faw::string_t pstrText);
    void SetText(faw::string_t pstrText) {};

    void SetOwner(std::shared_ptr<CControlUI> pOwner);
    faw::string_t* GetLinkContent(int iIndex);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);

    void DrawItemText(HDC hDC, const RECT& rcItem);

protected:
    enum
    {
        MAX_LINK = 8
    };
    int m_nLinks;
    RECT m_rcLinks[MAX_LINK];
    faw::string_t m_sLinks[MAX_LINK];
    int m_nHoverLink;
    std::shared_ptr<IListUI>  m_pOwner;
    CStdPtrArray m_aTexts;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListContainerElementUI: public CHorizontalLayoutUI, public IListItemUI
{
    DECLARE_DUICONTROL(CListContainerElementUI)
public:
    using ptr = std::shared_ptr<CListContainerElementUI>;
    CListContainerElementUI();

    faw::string_t GetClass() const;
    UINT GetControlFlags() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    int GetIndex() const;
    void SetIndex(int iIndex);

    std::shared_ptr<IListOwnerUI> GetOwner();
    void SetOwner(std::shared_ptr<CControlUI> pOwner);
    void SetVisible(bool bVisible = true);
    void SetEnabled(bool bEnable = true);

    bool IsSelected() const;
    bool Select(bool bSelect = true);
    bool SelectMulti(bool bSelect = true);
    bool IsExpanded() const;
    bool Expand(bool bExpand = true);

    void Invalidate();  // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
    bool Activate();

    void DoEvent(TEventUI& event);
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);
    bool DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl);

    virtual void DrawItemText(HDC hDC, const RECT& rcItem);
    virtual void DrawItemBk(HDC hDC, const RECT& rcItem);

    void SetPos(RECT rc, bool bNeedInvalidate = true);

protected:
    int m_iIndex;
    bool m_bSelected;
    UINT m_uButtonState;
    std::shared_ptr<IListOwnerUI> m_pOwner;
};

} // namespace DuiLib

#endif // __UILIST_H__
