#ifndef __UILISTEX_H__
#define __UILISTEX_H__

#pragma once

#include "../Layout/UIVerticalLayout.h"
#include "../Layout/UIHorizontalLayout.h"

namespace DuiLib
{

class IListComboCallbackUI
{
public:
    virtual void GetItemComboTextArray(std::shared_ptr<CControlUI> pCtrl, int iItem, int iSubItem) = 0;
};

class CEditUI;
class CComboBoxUI;

class UILIB_API CListExUI: public CListUI, public INotifyUI
{
    DECLARE_DUICONTROL(CListExUI)

public:
    using ptr = std::shared_ptr<CListExUI>;

    IListComboCallbackUI *m_pXCallback	= nullptr;
    BOOL	m_bAddMessageFilter	= FALSE;
    int		m_nRow				= -1;
    int		m_nColum			= -1;

public:
    CListExUI();

    faw::string_t GetClass() const override;
    UINT GetControlFlags() const override;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName)override;

    virtual void DoEvent(TEventUI& event)override;

    void InitListCtrl();

    virtual BOOL CheckColumEditable(int nColum)override;
    virtual std::shared_ptr<CRichEditUI> GetEditUI()override;

    virtual BOOL CheckColumComboBoxable(int nColum)override;
    virtual std::shared_ptr<CComboBoxUI> GetComboBoxUI()override;

    virtual BOOL CheckColumCheckBoxable(int nColum);

    void	SetEditRowAndColum(int nRow, int nColum)
    {
        m_nRow = nRow;
        m_nColum = nColum;
    };
    virtual void Notify(TNotifyUI& msg)override;
    virtual IListComboCallbackUI* GetTextArrayCallback() const ;
    virtual void SetTextArrayCallback(IListComboCallbackUI* pCallback);

    void OnListItemClicked(int nIndex, int nColum, RECT* lpRCColum, faw::string_t lpstrText);
    void OnListItemChecked(int nIndex, int nColum, BOOL bChecked);

    void SetColumItemColor(int nIndex, int nColum, DWORD iBKColor);
    BOOL GetColumItemColor(int nIndex, int nColum, DWORD& iBKColor);

protected:
    std::shared_ptr<CRichEditUI> m_pEditUI = nullptr;
    std::shared_ptr<CComboBoxUI> m_pComboBoxUI = nullptr;

private:
    void HideEditAndComboCtrl();
};

/////////////////////////////////////////////////////////////////////////////////////
//
class UILIB_API CListContainerHeaderItemUI: public CHorizontalLayoutUI
{
    DECLARE_DUICONTROL(CListContainerHeaderItemUI)

public:
    using ptr = std::shared_ptr<CListContainerHeaderItemUI>;
    CListContainerHeaderItemUI();

    faw::string_t GetClass() const override;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName) override;
    UINT GetControlFlags() const override;

    void SetEnabled(BOOL bEnable = TRUE);

    BOOL IsDragable() const;
    void SetDragable(BOOL bDragable);
    DWORD GetSepWidth() const;
    void SetSepWidth(int iWidth);
    DWORD GetTextStyle() const;
    void SetTextStyle(UINT uStyle);
    DWORD GetTextColor() const;
    void SetTextColor(DWORD dwTextColor);
    void SetTextPadding(RECT rc);
    RECT GetTextPadding() const;
    void SetFont(int index);
    BOOL IsShowHtml();
    void SetShowHtml(BOOL bShowHtml = TRUE);
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

    void DoEvent(TEventUI& event) override;
    SIZE EstimateSize(SIZE szAvailable) override;
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue) override;
    RECT GetThumbRect() const;

    void PaintText(HDC hDC)override;
    void PaintStatusImage(HDC hDC)override;

protected:
    POINT ptLastMouse = { 0 };
    BOOL m_bDragable;
    UINT m_uButtonState;
    int m_iSepWidth;
    DWORD m_dwTextColor;
    int m_iFont;
    UINT m_uTextStyle;
    BOOL m_bShowHtml;
    RECT m_rcTextPadding = { 0 };
    faw::string_t m_sNormalImage;
    faw::string_t m_sHotImage;
    faw::string_t m_sPushedImage;
    faw::string_t m_sFocusedImage;
    faw::string_t m_sSepImage;
    faw::string_t m_sSepImageModify;

    //支持编辑
    BOOL m_bEditable;

    //支持组合框
    BOOL m_bComboable;

    //支持复选框
    BOOL m_bCheckBoxable;

public:
    BOOL GetColumeEditable();
    void SetColumeEditable(BOOL bEnable);

    BOOL GetColumeComboable();
    void SetColumeComboable(BOOL bEnable);

    BOOL GetColumeCheckable();
    void SetColumeCheckable(BOOL bEnable);

    void SetCheck(BOOL bCheck);
    BOOL GetCheck();

private:
    UINT	m_uCheckBoxState;
    BOOL	m_bChecked;

    faw::string_t m_sCheckBoxNormalImage;
    faw::string_t m_sCheckBoxHotImage;
    faw::string_t m_sCheckBoxPushedImage;
    faw::string_t m_sCheckBoxFocusedImage;
    faw::string_t m_sCheckBoxDisabledImage;

    faw::string_t m_sCheckBoxSelectedImage;
    faw::string_t m_sCheckBoxForeImage;

    SIZE m_cxyCheckBox = { 0 };

    CContainerUI* m_pOwner;
public:
    BOOL DrawCheckBoxImage(HDC hDC, faw::string_t pStrImage, faw::string_t pStrModify = _T(""));
    faw::string_t GetCheckBoxNormalImage();
    void SetCheckBoxNormalImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxHotImage();
    void SetCheckBoxHotImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxPushedImage();
    void SetCheckBoxPushedImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxFocusedImage();
    void SetCheckBoxFocusedImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxDisabledImage();
    void SetCheckBoxDisabledImage(faw::string_t pStrImage);

    faw::string_t GetCheckBoxSelectedImage();
    void SetCheckBoxSelectedImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxForeImage();
    void SetCheckBoxForeImage(faw::string_t pStrImage);

    void GetCheckBoxRect(RECT &rc);

    int GetCheckBoxWidth() const;        // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    void SetCheckBoxWidth(int cx);       // 预设的参考值
    int GetCheckBoxHeight() const;       // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    void SetCheckBoxHeight(int cy);      // 预设的参考值

    void SetOwner(CContainerUI* pOwner);
    CContainerUI* GetOwner();



};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListTextExtElementUI: public CListLabelElementUI
{
    DECLARE_DUICONTROL(CListTextExtElementUI)

public:
    using ptr = std::shared_ptr<CListTextExtElementUI>;
    CListTextExtElementUI();
    virtual ~CListTextExtElementUI();

    faw::string_t GetClass() const override;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName) override;
    UINT GetControlFlags() const override;

    faw::string_t GetText(int iIndex) const;
    faw::string_t GetText() const override;
    void SetText(int iIndex, faw::string_t pstrText);
    void SetText(faw::string_t pstrText)override;

    void SetOwner(std::shared_ptr<CControlUI> pOwner) override;
    faw::string_t* GetLinkContent(int iIndex);

    void DoEvent(TEventUI& event)override;
    SIZE EstimateSize(SIZE szAvailable)override;

    void DrawItemText(HDC hDC, const RECT& rcItem)override;

    virtual bool DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)override;
    virtual void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)override;
    virtual void PaintStatusImage(HDC hDC)override;
    BOOL DrawCheckBoxImage(HDC hDC, faw::string_t pStrImage, faw::string_t pStrModify, RECT& rcCheckBox);
    faw::string_t GetCheckBoxNormalImage();
    void SetCheckBoxNormalImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxHotImage();
    void SetCheckBoxHotImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxPushedImage();
    void SetCheckBoxPushedImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxFocusedImage();
    void SetCheckBoxFocusedImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxDisabledImage();
    void SetCheckBoxDisabledImage(faw::string_t pStrImage);

    faw::string_t GetCheckBoxSelectedImage();
    void SetCheckBoxSelectedImage(faw::string_t pStrImage);
    faw::string_t GetCheckBoxForeImage();
    void SetCheckBoxForeImage(faw::string_t pStrImage);

    void GetCheckBoxRect(int nIndex, RECT &rc);
    void GetColumRect(int nColum, RECT &rc);

    int GetCheckBoxWidth() const;        // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    void SetCheckBoxWidth(int cx);       // 预设的参考值
    int GetCheckBoxHeight() const;       // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    void SetCheckBoxHeight(int cy);      // 预设的参考值

    void SetCheck(BOOL bCheck);
    BOOL GetCheck() const;

    int HitTestColum(POINT ptMouse);
    BOOL CheckColumEditable(int nColum);

    void SetColumItemColor(int nColum, DWORD iBKColor);
    BOOL GetColumItemColor(int nColum, DWORD& iBKColor);
private:
    typedef struct tagColumColorNode
    {
        BOOL  bEnable;
        DWORD iTextColor;
        DWORD iBKColor;
    } COLUMCOLORNODE;

    COLUMCOLORNODE ColumCorlorArray[UILIST_MAX_COLUMNS];
protected:
    enum
    {
        MAX_LINK = 8
    };
    int m_nLinks;
    RECT m_rcLinks[MAX_LINK];
    faw::string_t m_sLinks[MAX_LINK];
    int m_nHoverLink;
    std::shared_ptr<CListUI> m_pOwner;
    CStdPtrArray m_aTexts;

private:
    UINT	m_uCheckBoxState;
    BOOL	m_bChecked;

    faw::string_t m_sCheckBoxNormalImage;
    faw::string_t m_sCheckBoxHotImage;
    faw::string_t m_sCheckBoxPushedImage;
    faw::string_t m_sCheckBoxFocusedImage;
    faw::string_t m_sCheckBoxDisabledImage;

    faw::string_t m_sCheckBoxSelectedImage;
    faw::string_t m_sCheckBoxForeImage;

    SIZE m_cxyCheckBox = { 0 };


};
} // namespace DuiLib

#endif // __UILISTEX_H__
