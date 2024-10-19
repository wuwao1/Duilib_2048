#ifndef __UIRICHEDIT_H__
#define __UIRICHEDIT_H__

#pragma once
#include <imm.h>
#include <textserv.h>
#include <richole.h>

namespace DuiLib
{

class CTxtWinHost : public ITextHost
{
public:
    using ptr = std::shared_ptr<CTxtWinHost>;
    CTxtWinHost();
    BOOL Init(std::shared_ptr<CRichEditUI> re, const CREATESTRUCT* pcs);
    virtual ~CTxtWinHost();

    ITextServices* GetTextServices(void)
    {
        return pserv;
    }
    void SetClientRect(RECT* prc);
    RECT* GetClientRect()
    {
        return &rcClient;
    }
    BOOL IsWordWrap(void)
    {
        return fWordWrap;
    }
    void SetWordWrap(BOOL fWordWrap);
    BOOL IsReadOnly();
    void SetReadOnly(BOOL fReadOnly);

    void SetFont(HFONT hFont);
    void SetColor(DWORD dwColor);
    SIZEL* GetExtent();
    void SetExtent(SIZEL* psizelExtent);
    void LimitText(LONG nChars);
    BOOL IsCaptured();
    BOOL IsShowCaret();
    void NeedFreshCaret();
    INT GetCaretWidth();
    INT GetCaretHeight();

    BOOL GetAllowBeep();
    void SetAllowBeep(BOOL fAllowBeep);
    WORD GetDefaultAlign();
    void SetDefaultAlign(WORD wNewAlign);
    BOOL GetRichTextFlag();
    void SetRichTextFlag(BOOL fNew);
    LONG GetDefaultLeftIndent();
    void SetDefaultLeftIndent(LONG lNewIndent);
    BOOL SetSaveSelection(BOOL fSaveSelection);
    HRESULT OnTxInPlaceDeactivate();
    HRESULT OnTxInPlaceActivate(LPCRECT prcClient);
    BOOL GetActiveState(void)
    {
        return fInplaceActive;
    }
    BOOL DoSetCursor(RECT* prc, POINT* pt);
    void SetTransparent(BOOL fTransparent);
    void GetControlRect(LPRECT prc);
    LONG SetAccelPos(LONG laccelpos);
    WCHAR SetPasswordChar(WCHAR chPasswordChar);
    void SetDisabled(BOOL fOn);
    LONG SetSelBarWidth(LONG lSelBarWidth);
    BOOL GetTimerState();

    void SetCharFormat(CHARFORMAT2W& c);
    void SetParaFormat(PARAFORMAT2& p);

    // -----------------------------
    //	IUnknown interface
    // -----------------------------
    virtual HRESULT _stdcall QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG _stdcall AddRef(void);
    virtual ULONG _stdcall Release(void);

    // -----------------------------
    //	ITextHost interface
    // -----------------------------
    virtual HDC TxGetDC();
    virtual INT TxReleaseDC(HDC hdc);
    virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
    virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
    virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
    virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
    virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode);
    virtual void TxViewChange(BOOL fUpdate);
    virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
    virtual BOOL TxShowCaret(BOOL fShow);
    virtual BOOL TxSetCaretPos(INT x, INT y);
    virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
    virtual void TxKillTimer(UINT idTimer);
    virtual void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
    virtual void TxSetCapture(BOOL fCapture);
    virtual void TxSetFocus();
    virtual void TxSetCursor(HCURSOR hcur, BOOL fText);
    virtual BOOL TxScreenToClient(LPPOINT lppt);
    virtual BOOL TxClientToScreen(LPPOINT lppt);
    virtual HRESULT TxActivate(LONG* plOldState);
    virtual HRESULT TxDeactivate(LONG lNewState);
    virtual HRESULT TxGetClientRect(LPRECT prc);
    virtual HRESULT TxGetViewInset(LPRECT prc);
    virtual HRESULT TxGetCharFormat(const CHARFORMATW** ppCF);
    virtual HRESULT TxGetParaFormat(const PARAFORMAT** ppPF);
    virtual COLORREF TxGetSysColor(int nIndex);
    virtual HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle);
    virtual HRESULT TxGetMaxLength(DWORD* plength);
    virtual HRESULT TxGetScrollBars(DWORD* pdwScrollBar);
    virtual HRESULT TxGetPasswordChar(TCHAR* pch);
    virtual HRESULT TxGetAcceleratorPos(LONG* pcp);
    virtual HRESULT TxGetExtent(LPSIZEL lpExtent);
    virtual HRESULT OnTxCharFormatChange(const CHARFORMATW* pcf);
    virtual HRESULT OnTxParaFormatChange(const PARAFORMAT* ppf);
    virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits);
    virtual HRESULT TxNotify(DWORD iNotify, void* pv);
    virtual HIMC TxImmGetContext(void);
    virtual void TxImmReleaseContext(HIMC himc);
    virtual HRESULT TxGetSelectionBarWidth(LONG* lSelBarWidth);

private:
    std::shared_ptr<CRichEditUI>  m_re = nullptr;
    ULONG	cRefs;					// Reference Count
    ITextServices* pserv;		    // pointer to Text Services object
    // Properties

    DWORD		dwStyle;				// style bits

    unsigned	fEnableAutoWordSel : 1;	// enable Word style auto word selection?
    unsigned	fWordWrap : 1;	// Whether control should word wrap
    unsigned	fAllowBeep : 1;	// Whether beep is allowed
    unsigned	fRich : 1;	// Whether control is rich text
    unsigned	fSaveSelection : 1;	// Whether to save the selection when inactive
    unsigned	fInplaceActive : 1; // Whether control is inplace active
    unsigned	fTransparent : 1; // Whether control is transparent
    unsigned	fTimer : 1;	// A timer is set
    unsigned    fCaptured : 1;
    unsigned    fShowCaret : 1;
    unsigned    fNeedFreshCaret : 1; // 修正改变大小后点击其他位置原来光标不能消除的问题

    INT         iCaretWidth;
    INT         iCaretHeight;
    INT         iCaretLastWidth;
    INT         iCaretLastHeight;
    LONG		lSelBarWidth;			// Width of the selection bar
    LONG  		cchTextMost;			// maximum text size
    DWORD		dwEventMask;			// DoEvent mask to pass on to parent window
    LONG		icf;
    LONG		ipf;
    RECT		rcClient;				// Client Rect for this control
    SIZEL		sizelExtent;			// Extent array
    CHARFORMAT2W cf;					// Default character format
    PARAFORMAT2	pf;					    // Default paragraph format
    LONG		laccelpos;				// Accelerator position
    WCHAR		chPasswordChar;		    // Password character
};

class UILIB_API CRichEditUI: public CContainerUI, public IMessageFilterUI
{
    DECLARE_DUICONTROL(CRichEditUI)
public:
    using ptr = std::shared_ptr<CRichEditUI>;
    CRichEditUI();
    virtual ~CRichEditUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    UINT GetControlFlags() const;

    void SetEnabled(bool bEnabled);
    bool IsMultiLine();
    void SetMultiLine(bool bMultiLine);
    bool IsWantTab();
    void SetWantTab(bool bWantTab = true);
    bool IsWantReturn();
    void SetWantReturn(bool bWantReturn = true);
    bool IsWantCtrlReturn();
    void SetWantCtrlReturn(bool bWantCtrlReturn = true);
    bool IsTransparent();
    void SetTransparent(bool bTransparent = true);
    bool IsRich();
    void SetRich(bool bRich = true);
    bool IsReadOnly();
    void SetReadOnly(bool bReadOnly = true);
    bool IsWordWrap();
    void SetWordWrap(bool bWordWrap = true);
    int GetFont();
    void SetFont(int index);
    void SetFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
    LONG GetWinStyle();
    void SetWinStyle(LONG lStyle);
    DWORD GetTextColor();
    void SetTextColor(DWORD dwTextColor);
    int GetLimitText();
    void SetLimitText(int iChars);
    long GetTextLength(DWORD dwFlags = GTL_DEFAULT) const;
    faw::string_t GetText() const;
    void SetText(faw::string_t pstrText);
    bool IsModify() const;
    void SetModify(bool bModified = true) const;
    void GetSel(CHARRANGE &cr) const;
    void GetSel(long& nStartChar, long& nEndChar) const;
    int SetSel(CHARRANGE &cr);
    int SetSel(long nStartChar, long nEndChar);
    void ReplaceSel(faw::string_t lpszNewText, bool bCanUndo);
    void ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo = false);
    faw::string_t GetSelText() const;
    int SetSelAll();
    int SetSelNone();
    WORD GetSelectionType() const;
    bool GetZoom(int& nNum, int& nDen) const;
    bool SetZoom(int nNum, int nDen);
    bool SetZoomOff();
    bool GetAutoURLDetect() const;
    bool SetAutoURLDetect(bool bAutoDetect = true);
    DWORD GetEventMask() const;
    DWORD SetEventMask(DWORD dwEventMask);
    faw::string_t GetTextRange(long nStartChar, long nEndChar) const;
    void HideSelection(bool bHide = true, bool bChangeStyle = false);
    void ScrollCaret();
    int InsertText(long nInsertAfterChar, faw::string_t lpstrText, bool bCanUndo = false);
    int AppendText(faw::string_t lpstrText, bool bCanUndo = false);
    DWORD GetDefaultCharFormat(CHARFORMAT2 &cf) const;
    bool SetDefaultCharFormat(CHARFORMAT2 &cf);
    DWORD GetSelectionCharFormat(CHARFORMAT2 &cf) const;
    bool SetSelectionCharFormat(CHARFORMAT2 &cf);
    bool SetWordCharFormat(CHARFORMAT2 &cf);
    DWORD GetParaFormat(PARAFORMAT2 &pf) const;
    bool SetParaFormat(PARAFORMAT2 &pf);
    bool CanUndo();
    bool CanRedo();
    bool CanPaste();
    bool Redo();
    bool Undo();
    void Clear();
    void Copy();
    void Cut();
    void Paste();
    int GetLineCount() const;
    faw::string_t GetLine(int nIndex, int nMaxLength) const;
    int LineIndex(int nLine = -1) const;
    int LineLength(int nLine = -1) const;
    bool LineScroll(int nLines, int nChars = 0);
    POINT GetCharPos(long lChar) const;
    long LineFromChar(long nIndex) const;
    POINT PosFromChar(UINT nChar) const;
    int CharFromPos(POINT pt) const;
    void EmptyUndoBuffer();
    UINT SetUndoLimit(UINT nLimit);
    long StreamIn(int nFormat, EDITSTREAM &es);
    long StreamOut(int nFormat, EDITSTREAM &es);
    void SetAccumulateDBCMode(bool bDBCMode);
    bool IsAccumulateDBCMode();

    RECT GetTextPadding() const;
    void SetTextPadding(RECT rc);
    faw::string_t GetNormalImage();
    void SetNormalImage(faw::string_t pStrImage);
    faw::string_t GetHotImage();
    void SetHotImage(faw::string_t pStrImage);
    faw::string_t GetFocusedImage();
    void SetFocusedImage(faw::string_t pStrImage);
    faw::string_t GetDisabledImage();
    void SetDisabledImage(faw::string_t pStrImage);
    void PaintStatusImage(HDC hDC);

    void SetTipValue(faw::string_t pStrTipValue);
    faw::string_t GetTipValue();
    void SetTipValueColor(faw::string_t pStrColor);
    DWORD GetTipValueColor();
    void SetTipValueAlign(UINT uAlign);
    UINT GetTipValueAlign();

    void DoInit();
    bool SetDropAcceptFile(bool bAccept);
    // 注意：TxSendMessage和SendMessage是有区别的，TxSendMessage没有multibyte和unicode自动转换的功能，
    // 而richedit2.0内部是以unicode实现的，在multibyte程序中，必须自己处理unicode到multibyte的转换
    virtual HRESULT TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const;
    IDropTarget* GetTxDropTarget();
    virtual bool OnTxViewChanged();
    virtual void OnTxNotify(DWORD iNotify, void *pv);

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

    SIZE EstimateSize(SIZE szAvailable);
    void SetPos(RECT rc, bool bNeedInvalidate = true);
    void Move(SIZE szOffset, bool bNeedInvalidate = true);
    void DoEvent(TEventUI& event);
    bool DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl);

    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);


    bool SetOLECallback(IRichEditOleCallback* pCallback);
    IRichEditOleCallback *GetOLECallback();
    LPRICHEDITOLE GetRichEditOle();

    std::optional<LRESULT> MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:


    CTxtWinHost	*m_pTwh				= nullptr;
    bool		m_bVScrollBarFixing	= false;
    bool		m_bWantTab			= true;
    bool		m_bWantReturn		= true;
    bool		m_bWantCtrlReturn	= true;
    bool		m_bTransparent		= true;
    bool		m_bRich				= true;
    bool		m_bReadOnly			= false;
    bool		m_bWordWrap			= false;
    DWORD		m_dwTextColor		= 0;
    int			m_iFont				= -1;
    int			m_iLimitText;
    LONG		m_lTwhStyle;
    bool		m_bDrawCaret		= true;
    bool		m_bInited			= false;

    bool		m_fAccumulateDBC; // TRUE - need to cumulate ytes from 2 WM_CHAR msgs
    // we are in this mode when we receive VK_PROCESSKEY
    UINT		m_chLeadByte		= 0; // use when we are in _fAccumulateDBC mode

    RECT		m_rcTextPadding;
    UINT		m_uButtonState		= 0;
    faw::string_t	m_sNormalImage;
    faw::string_t	m_sHotImage;
    faw::string_t	m_sFocusedImage;
    faw::string_t	m_sDisabledImage;
    faw::string_t	m_sTipValue;
    DWORD		m_dwTipValueColor	= 0xFFBAC0C5;
    UINT		m_uTipValueAlign	= DT_SINGLELINE | DT_LEFT;

    LPRICHEDITOLE m_pRichEditOle = NULL;
    IRichEditOleCallback* m_pCallback;
};

} // namespace DuiLib

#endif // __UIRICHEDIT_H__
