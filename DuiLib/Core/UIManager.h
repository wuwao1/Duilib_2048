#ifndef __UIMANAGER_H__
#define __UIMANAGER_H__

#pragma once

#include <optional>

namespace DuiLib
{
using MAP_STR_RESURCE_CAHCE = std::unordered_map<faw::string_t, std::unordered_map<faw::string_t, std::shared_ptr<CacheZipEntry>>> ;
using MAP_STR_FONT_COLLECTION = std::map<faw::string_t, std::shared_ptr<Gdiplus::PrivateFontCollection>>;
/////////////////////////////////////////////////////////////////////////////////////
//
class GdiplusInitializer
{
public:
    GdiplusInitializer()
    {
        ULONG_PTR token;
        gdiplusStartupInput = MAKE_UNIQUE(Gdiplus::GdiplusStartupInput);
        Gdiplus::GdiplusStartup(&token, gdiplusStartupInput.get(), NULL);
        token_ = token;
    }

    ~GdiplusInitializer()
    {
        Gdiplus::GdiplusShutdown(token_);
    }

private:
    ULONG_PTR token_;
    std::unique_ptr<Gdiplus::GdiplusStartupInput> gdiplusStartupInput;

    // 禁止拷贝和赋值
    GdiplusInitializer(const GdiplusInitializer&) = delete;
    GdiplusInitializer& operator=(const GdiplusInitializer&) = delete;
};

class CControlUI;
class CRichEditUI;
class CIDropTarget;

//
typedef struct UILIB_API tagTimageArgs {
    std::variant<UINT, faw::string_t> bitmap;
    faw::string_t type = _T("");
    HINSTANCE instance = NULL;
}TimageArgs;

typedef struct UILIB_API tagTDataArgs {
    std::unique_ptr<unsigned char[]> pUniqueData = nullptr;
    DWORD dataSize = 0;
    unsigned char* pData = nullptr;
}TDataArgs;
/////////////////////////////////////////////////////////////////////////////////////
//
enum UILIB_RESTYPE
{
    UILIB_FILE = 1,		// 来自磁盘文件
    UILIB_ZIP,			// 来自磁盘zip压缩包
    UILIB_RESOURCE,		// 来自资源
    UILIB_ZIPRESOURCE,	// 来自资源的zip压缩包
};
/////////////////////////////////////////////////////////////////////////////////////
//

enum EVENTTYPE_UI
{
    UIEVENT__FIRST = 1,
    UIEVENT__KEYBEGIN,
    UIEVENT_KEYDOWN,
    UIEVENT_KEYUP,
    UIEVENT_CHAR,
    UIEVENT_SYSKEY,
    UIEVENT__KEYEND,
    UIEVENT__MOUSEBEGIN,
    UIEVENT_MOUSEMOVE,
    UIEVENT_MOUSELEAVE,
    UIEVENT_MOUSEENTER,
    UIEVENT_MOUSEHOVER,
    UIEVENT_BUTTONDOWN,
    UIEVENT_BUTTONUP,
    UIEVENT_RBUTTONDOWN,
    UIEVENT_RBUTTONUP,
    UIEVENT_MBUTTONDOWN,
    UIEVENT_MBUTTONUP,
    UIEVENT_DBLCLICK,
    UIEVENT_CONTEXTMENU,
    UIEVENT_SCROLLWHEEL,
    UIEVENT__MOUSEEND,
    UIEVENT_KILLFOCUS,
    UIEVENT_SETFOCUS,
    UIEVENT_WINDOWSIZE,
    UIEVENT_SETCURSOR,
    UIEVENT_TIMER,
    UIEVENT__LAST,
};

/////////////////////////////////////////////////////////////////////////////////////
//
// 内部保留的消息
enum MSGTYPE_UI
{
    UIMSG_TRAYICON = WM_USER + 1,// 托盘消息
    UIMSG_SET_DPI,				 // DPI
    WM_MENUCLICK,				 // 菜单消息
    UIMSG_USER = WM_USER + 100,	 // 程序自定义消息
};

/////////////////////////////////////////////////////////////////////////////////////
//

// Flags for CControlUI::GetControlFlags()
#define UIFLAG_TABSTOP       0x00000001
#define UIFLAG_SETCURSOR     0x00000002
#define UIFLAG_WANTRETURN    0x00000004

// Flags for FindControl()
#define UIFIND_ALL           0x00000000
#define UIFIND_VISIBLE       0x00000001
#define UIFIND_ENABLED       0x00000002
#define UIFIND_HITTEST       0x00000004
#define UIFIND_UPDATETEST    0x00000008
#define UIFIND_TOP_FIRST     0x00000010
#define UIFIND_ME_FIRST      0x80000000

// Flags used for controlling the paint
#define UISTATE_FOCUSED      0x00000001
#define UISTATE_SELECTED     0x00000002
#define UISTATE_DISABLED     0x00000004
#define UISTATE_HOT          0x00000008
#define UISTATE_PUSHED       0x00000010
#define UISTATE_READONLY     0x00000020
#define UISTATE_CAPTURED     0x00000040



/////////////////////////////////////////////////////////////////////////////////////
//

typedef struct UILIB_API tagTFontInfo
{
    HFONT hFont;
    faw::string_t sFontName;
    int iSize;
    bool bBold;
    bool bUnderline;
    bool bItalic;
    TEXTMETRIC tm;
    tagTFontInfo()
    {
        hFont = NULL;
        sFontName = _T("");
        iSize = 0;
        bBold = bUnderline = bItalic = false;
        memset(&tm, 0, sizeof(TEXTMETRIC));
    }
} TFontInfo;

typedef struct UILIB_API tagTImageInfo
{
    HBITMAP hBitmap			            = NULL;
    HBITMAP *phBitmap		            = nullptr;
    LPBYTE pBits			            = nullptr;
    std::shared_ptr<BYTE[]> pSrcBits    = nullptr;
    int nX					            = 0;
    int nY					            = 0;
    bool bAlpha				            = false;
    bool bUseHSL			            = false;
    faw::string_t sResType	            = _T("");
    DWORD dwMask			            = 0;
} TImageInfo;

typedef struct UILIB_API tagTDrawInfo
{
    tagTDrawInfo();
    void Parse(faw::string_t pStrImage, faw::string_t pStrModify, CPaintManagerUI *paintManager);
    void Clear();

    faw::string_t   sDrawString;
    faw::string_t   sDrawModify;
    faw::string_t   sImageName;
    faw::string_t   sResType;
    RECT            rcDest { 0 };
    RECT            rcSource { 0 };
    RECT            rcCorner { 0 };
    DWORD           dwMask;
    BYTE            uFade;
    bool            bHole;
    bool            bTiledX;
    bool            bTiledY;
    bool            bHSL;
    faw::string_t   sAlign;
    faw::string_t   imageFormat{L""};
    SIZE            szDesireDraw{0};
    DWORD           svgcolor = {0};		//SVG的填充颜色
} TDrawInfo;

typedef struct UILIB_API tagTPercentInfo
{
    double left;
    double top;
    double right;
    double bottom;
} TPercentInfo;

using MAP_STR_FONT_INFO = std::unordered_map<faw::string_t, std::shared_ptr<TFontInfo>>;
using MAP_STR_DRAW_INFO = std::unordered_map<faw::string_t, std::shared_ptr<TDrawInfo>>;
using MAP_STR_IMAGE_INFO = std::unordered_map<faw::string_t, std::shared_ptr<TImageInfo>>;
using MAP_STR_ATTRIBUTE_INFO = std::unordered_map<faw::string_t, std::shared_ptr<faw::string_t>>;
typedef struct UILIB_API tagTResInfo
{
    DWORD m_dwDefaultDisabledColor;
    DWORD m_dwDefaultFontColor;
    DWORD m_dwDefaultLinkFontColor;
    DWORD m_dwDefaultLinkHoverFontColor;
    DWORD m_dwDefaultSelectedBkColor;
    std::shared_ptr<TFontInfo> m_DefaultFontInfo;
    MAP_STR_FONT_INFO           m_CustomFonts;
    MAP_STR_DRAW_INFO           m_DrawInfoHash;
    MAP_STR_IMAGE_INFO          m_ImageHash;
    MAP_STR_ATTRIBUTE_INFO      m_AttrHash;
    MAP_STR_ATTRIBUTE_INFO      m_StyleHash;
} TResInfo;


typedef struct tagTIMERINFO
{
    std::shared_ptr<CControlUI> pSender;
    UINT nLocalID;
    HWND hWnd;
    UINT uWinTimer;
    bool bKilled;
} TIMERINFO;

// Structure for notifications from the system
// to the control implementation.
typedef struct UILIB_API tagTEventUI
{
    int Type;
    std::shared_ptr<CControlUI> pSender;
    DWORD dwTimestamp;
    POINT ptMouse = { 0 };
    TCHAR chKey;
    WORD wKeyState;
    WPARAM wParam;
    LPARAM lParam;
} TEventUI;

// Drag&Drop control
const TCHAR* const CF_MOVECONTROL = _T("CF_MOVECONTROL");

typedef struct UILIB_API tagTCFMoveUI
{
    std::shared_ptr<CControlUI> pControl;
} TCFMoveUI;

// Listener interface
class INotifyUI
{
public:
    virtual void Notify(TNotifyUI& msg) = 0;
};

// MessageFilter interface
class IMessageFilterUI
{
public:
    virtual std::optional<LRESULT> MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

class ITranslateAccelerator
{
public:
    virtual LRESULT TranslateAccelerator(MSG *pMsg) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//
using LPCREATECONTROL = std::shared_ptr<CControlUI>(*)(faw::string_t pstrType);

class UILIB_API CPaintManagerUI: public CIDropTarget
{
public:
    using ptr = std::shared_ptr<CPaintManagerUI>;
    CPaintManagerUI();
    virtual ~CPaintManagerUI();

public:
    void Init(HWND hWnd, faw::string_t pstrName = _T(""));
    bool IsUpdateNeeded() const;
    void NeedUpdate();
    void Invalidate();
    void Invalidate(RECT& rcItem);

    faw::string_t   GetName() const;
    HDC             GetPaintDC() const;
    HWND            GetPaintWindow() const;
    HWND            GetTooltipWindow() const;
    int             GetHoverTime() const;
    void            SetHoverTime(int iTime);

    POINT   GetMousePos() const;
    SIZE    GetClientSize() const;
    SIZE    GetInitSize();
    void    SetInitSize(int cx, int cy);
    RECT&   GetSizeBox();
    void    SetSizeBox(RECT& rcSizeBox);
    RECT&   GetCaptionRect();
    void    SetCaptionRect(RECT& rcCaption);
    SIZE    GetRoundCorner() const;
    void    SetRoundCorner(int cx, int cy);
    SIZE    GetMinInfo() const;
    void    SetMinInfo(int cx, int cy);
    SIZE    GetMaxInfo() const;
    void    SetMaxInfo(int cx, int cy);
    bool    IsShowUpdateRect() const;
    void    SetShowUpdateRect(bool show);
    bool    IsNoActivate();
    void    SetNoActivate(bool bNoActivate);

    BYTE    GetOpacity() const;
    void    SetOpacity(BYTE nOpacity);

    bool    IsLayered();
    void    SetLayered(bool bLayered);
    RECT&   GetLayeredInset();
    void    SetLayeredInset(RECT& rcLayeredInset);
    BYTE    GetLayeredOpacity();
    void    SetLayeredOpacity(BYTE nOpacity);
    faw::   string_t GetLayeredImage();
    void    SetLayeredImage(faw::string_t pstrImage);

    CShadowUI* GetShadow();

    void SetUseGdiplusText(bool bUse);
    bool IsUseGdiplusText() const;
    void SetGdiplusTextRenderingHint(int trh);
    int  GetGdiplusTextRenderingHint() const;

    static HINSTANCE            GetInstance();
    static faw::string_t        GetInstancePath();
    static faw::string_t        GetCurrentPath();
    static HINSTANCE            GetResourceDll();
    static const faw::string_t  GetResourcePath();
    static const faw::string_t  GetResourceZip();
    static const faw::string_t  GetResourceZipPwd();
    static bool                 IsCachedResourceZip();
    static std::shared_ptr<CacheZipEntry> GetResourceZipCacheEntry(faw::string_t& name);
    static void                 SetInstance(HINSTANCE hInst);
    static void                 SetCurrentPath(faw::string_t pStrPath);
    static void                 SetResourceDll(HINSTANCE hInst);
    static void                 SetResourcePath(faw::string_t pStrPath);
    static void                 SetResourceZip(faw::string_t pStrPath, LPVOID pVoid, unsigned int len, faw::string_t password = _T(""));
    static void                 SetResourceZip(faw::string_t pstrZip, bool bCachedResourceZip = true, faw::string_t password = _T(""));
    static void                 SetResourceType(int nType);
    static int                  GetResourceType();
    static bool                 GetHSL(short* H, short* S, short* L);
    static void                 SetHSL(bool bUseHSL, short H, short S, short L);  // H:0~360, S:0~200, L:0~200
    static void                 ReloadSkin();
    static CPaintManagerUI*     GetPaintManager(faw::string_t pstrName);
    static CStdPtrArray*        GetPaintManagers();
    static bool                 LoadPlugin(faw::string_t pstrModuleName);
    static CStdPtrArray*        GetPlugins();

    bool IsForceUseSharedRes() const;
    void SetForceUseSharedRes(bool bForce);

    void DeletePtr(void* ptr);

    DWORD   GetDefaultDisabledColor() const;
    void    SetDefaultDisabledColor(DWORD dwColor, bool bShared = false);
    DWORD   GetDefaultFontColor() const;
    void    SetDefaultFontColor(DWORD dwColor, bool bShared = false);
    DWORD   GetDefaultLinkFontColor() const;
    void    SetDefaultLinkFontColor(DWORD dwColor, bool bShared = false);
    DWORD   GetDefaultLinkHoverFontColor() const;
    void    SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared = false);
    DWORD   GetDefaultSelectedBkColor() const;
    void    SetDefaultSelectedBkColor(DWORD dwColor, bool bShared = false);
    std::shared_ptr<TFontInfo>  GetDefaultFontInfo();
    void            SetDefaultFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared = false);
    DWORD           GetCustomFontCount(bool bShared = false) const;
    void            AddFontArray(faw::string_t pstrPath);
    HFONT           AddFont(int id, faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared = false);
    HFONT           GetFont(int id);
    HFONT           GetFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
    Gdiplus::Font*  GetResourceFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
    int             GetFontIndex(HFONT hFont, bool bShared = false);
    int             GetFontIndex(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared = false);
    void            RemoveFont(HFONT hFont, bool bShared = false);
    void            RemoveFont(int id, bool bShared = false);
    void            RemoveAllFonts(bool bShared = false);
    std::shared_ptr<TFontInfo>   GetFontInfo(int id);
    std::shared_ptr<TFontInfo>   GetFontInfo(HFONT hFont);

    const std::shared_ptr<TImageInfo> GetImage(faw::string_t bitmap);
    const std::shared_ptr<TImageInfo> GetImageEx(faw::string_t bitmap, faw::string_t type = _T(""), DWORD mask = 0, bool bUseHSL = false, HINSTANCE instance = NULL, const std::shared_ptr<TDrawInfo> pDrawInfo = nullptr);
    const std::shared_ptr<TImageInfo> AddImage(faw::string_t bitmap, faw::string_t type = _T(""), DWORD mask = 0, bool bUseHSL = false, bool bShared = false, HINSTANCE instance = NULL,const std::shared_ptr<TDrawInfo> pDrawInfo = nullptr);
    const std::shared_ptr<TImageInfo> AddImage(faw::string_t bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared = false);
    const std::shared_ptr<TImageInfo> AddImage(faw::string_t bitmap, HBITMAP *phBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared = false);
    const std::shared_ptr<TImageInfo> AddImage(faw::string_t bitmap, TDataArgs dataArgs,DWORD mask = 0, bool bUseHSL = false, bool bShared = false);
    void              RemoveImage(faw::string_t bitmap, bool bShared = false);
    void              RemoveAllImages(bool bShared = false);
    static void       ReloadSharedImages();
    void              ReloadImages();

    const std::shared_ptr<TDrawInfo>  GetDrawInfo(faw::string_t pStrImage, faw::string_t pStrModify);
    void              RemoveDrawInfo(faw::string_t pStrImage, faw::string_t pStrModify);
    void              RemoveAllDrawInfos();

    static void     AddGlobalAttributeList(faw::string_t GlobalStyleFile);
    void            AddDefaultAttributeList(faw::string_t pStrControlName, faw::string_t pStrControlAttrList, bool bShared = false);
    faw::string_t   GetDefaultAttributeList(faw::string_t pStrControlName) const;
    bool            RemoveDefaultAttributeList(faw::string_t pStrControlName, bool bShared = false);
    void            RemoveAllDefaultAttributeList(bool bShared = false);

    void            AddWindowCustomAttribute(faw::string_t pstrName, faw::string_t pstrAttr);
    faw::string_t   GetWindowCustomAttribute(faw::string_t pstrName) const;
    bool            RemoveWindowCustomAttribute(faw::string_t pstrName);
    void            RemoveAllWindowCustomAttribute();

    // 样式管理
    void                            AddStyle(faw::string_t pName, faw::string_t pStyle, bool bShared = false);
    faw::string_t                   GetStyle(faw::string_t pName) const;
    BOOL                            RemoveStyle(faw::string_t pName, bool bShared = false);
    const MAP_STR_ATTRIBUTE_INFO&   GetStyles(bool bShared = false) const;
    void                            RemoveAllStyle(bool bShared = false);

    const std::shared_ptr<TImageInfo>       GetImageString(faw::string_t pStrImage, faw::string_t pStrModify = _T(""));

    // 初始化拖拽
    bool            InitDragDrop();
    virtual bool    OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect);
    void            OnDropCallCack(std::function<bool(HDROP)> callback);

    // 任务栏进度条
    bool            InitTaskbarList();
    bool            ResetTaskbarList();
    bool            SetTaskbarProgress(double lfProgress);
    bool            SetTaskbarIcon(HICON m_hIcon,faw::string_t Description);

    bool            AttachDialog(std::shared_ptr<CControlUI> pControl);
    bool            InitControls(std::shared_ptr<CControlUI> pControl, std::shared_ptr<CControlUI> pParent = nullptr);
    void            ReapObjects(std::shared_ptr<CControlUI> pControl);

    bool            AddOptionGroup(faw::string_t pStrGroupName, std::shared_ptr<CControlUI> pControl);
    std::vector<std::shared_ptr<CControlUI>>  GetOptionGroup(faw::string_t pStrGroupName);
    void            RemoveOptionGroup(faw::string_t pStrGroupName, std::shared_ptr<CControlUI> pControl);
    void            RemoveAllOptionGroups();

    std::shared_ptr<CControlUI>     GetFocus() const;
    void SetFocus(std::shared_ptr<CControlUI> pControl);
    void SetFocusNeeded(std::shared_ptr<CControlUI> pControl);

    bool SetNextTabControl(bool bForward = true);

    bool SetTimer(std::shared_ptr<CControlUI> pControl, UINT nTimerID, UINT uElapse);
    bool KillTimer(std::shared_ptr<CControlUI> pControl, UINT nTimerID);
    void KillTimer(std::shared_ptr<CControlUI> pControl);
    void RemoveAllTimers();

    void SetCapture();
    void ReleaseCapture();
    bool IsCaptured();

    bool IsPainting();
    void SetPainting(bool bIsPainting);

    bool AddNotifier(INotifyUI* pControl);
    bool RemoveNotifier(INotifyUI* pControl);
    void SendNotify(TNotifyUI& Msg, bool bAsync = false);
    void SendNotify(std::shared_ptr<CControlUI> pControl, faw::string_t pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false);

    bool AddPreMessageFilter(IMessageFilterUI* pFilter);
    bool RemovePreMessageFilter(IMessageFilterUI* pFilter);

    bool AddMessageFilter(IMessageFilterUI* pFilter);
    bool RemoveMessageFilter(IMessageFilterUI* pFilter);

    int  GetPostPaintCount() const;
    bool IsPostPaint(std::shared_ptr<CControlUI> pControl);
    bool AddPostPaint(std::shared_ptr<CControlUI> pControl);
    bool RemovePostPaint(std::shared_ptr<CControlUI> pControl);
    bool SetPostPaintIndex(std::shared_ptr<CControlUI> pControl, int iIndex);

    int  GetNativeWindowCount() const;
    RECT GetNativeWindowRect(HWND hChildWnd);
    bool AddNativeWindow(std::shared_ptr<CControlUI> pControl, HWND hChildWnd);
    bool RemoveNativeWindow(HWND hChildWnd);

    void AddDelayedCleanup(std::shared_ptr<CControlUI> pControl);
    void AddMouseLeaveNeeded(std::shared_ptr<CControlUI> pControl);
    bool RemoveMouseLeaveNeeded(std::shared_ptr<CControlUI> pControl);

    bool AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
    bool RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
    bool TranslateAccelerator(LPMSG pMsg);

    std::shared_ptr<CControlUI> GetRoot() const;
    std::shared_ptr<CControlUI> FindControl(POINT pt) const;
    std::shared_ptr<CControlUI> FindControl(faw::string_t pstrName) const;
    std::shared_ptr<CControlUI> FindSubControlByPoint(std::shared_ptr<CControlUI> pParent, POINT pt) const;
    std::shared_ptr<CControlUI> FindSubControlByName(std::shared_ptr<CControlUI> pParent, faw::string_t pstrName) const;
    std::shared_ptr<CControlUI> FindSubControlByClass(std::shared_ptr<CControlUI> pParent, faw::string_t pstrClass, int iIndex = 0);
    std::vector<std::shared_ptr<CControlUI>>& FindSubControlsByClass(std::shared_ptr<CControlUI> pParent, faw::string_t pstrClass);

    static void MessageLoop();
    static bool TranslateMessage(const LPMSG pMsg);
    static void Term();

    std::shared_ptr<CDPI>       GetDPIObj();
    void                        ResetDPIAssets();
    void                        RebuildFont(std::shared_ptr<TFontInfo> pFontInfo);
    void                        SetDPI(int iDPI);
    static void                 SetAllDPI(int iDPI);

    std::optional<LRESULT> MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);
    std::optional<LRESULT> PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void                   UsedVirtualWnd(bool bUsed);

private:
    std::vector<std::shared_ptr<CControlUI>>&    GetFoundControls();
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromNameHash(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromCount(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromPoint(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromTab(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromShortcut(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromName(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlFromClass(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlsFromClass(std::shared_ptr<CControlUI> pThis, LPVOID pData);
    static std::shared_ptr<CControlUI> CALLBACK __FindControlsFromUpdate(std::shared_ptr<CControlUI> pThis, LPVOID pData);

    static void AdjustSharedImagesHSL();
    void        AdjustImagesHSL();
    void        PostAsyncNotify();

private:
    faw::string_t       m_sName;
    HWND                m_hWndPaint;	//所附加的窗体的句柄
    HDC                 m_hDcPaint;
    HDC                 m_hDcOffscreen;
    HDC                 m_hDcBackground;
    HBITMAP             m_hbmpOffscreen;
    BYTE*               m_pOffscreenBits;
    HBITMAP             m_hbmpBackground;
    COLORREF*           m_pBackgroundBits;

    // 提示信息
    HWND                m_hwndTooltip;
    TOOLINFO            m_ToolTip;
    int                 m_iHoverTime;
    bool                m_bNoActivate;
    bool                m_bShowUpdateRect;

    //
    std::shared_ptr<CControlUI>         m_pRoot;
    std::shared_ptr<CControlUI>         m_pFocus;
    std::shared_ptr<CControlUI>         m_pEventHover;
    std::shared_ptr<CControlUI>         m_pEventClick;
    std::shared_ptr<CControlUI>         m_pEventKey;
    std::shared_ptr<CControlUI>         m_pLastToolTip;
    //
    POINT               m_ptLastMousePos = { 0 };
    SIZE                m_szMinWindow = { 0 };
    SIZE                m_szMaxWindow = { 0 };
    SIZE                m_szInitWindowSize = { 0 };
    RECT                m_rcSizeBox = { 0 };
    SIZE                m_szRoundCorner = { 0 };
    RECT                m_rcCaption = { 0 };
    UINT                m_uTimerID;
    bool                m_bFirstLayout;
    bool                m_bUpdateNeeded;
    bool                m_bFocusNeeded;
    bool                m_bOffscreenPaint;

    BYTE                m_nOpacity;
    bool                m_bLayered;
    RECT                m_rcLayeredInset = { 0 };
    bool                m_bLayeredChanged;
    RECT                m_rcLayeredUpdate = { 0 };
    std::shared_ptr<TDrawInfo>           m_diLayered;

    bool                m_bMouseTracking;
    bool                m_bMouseCapture;
    bool                m_bIsPainting;
    bool                m_bUsedVirtualWnd;
    bool                m_bAsyncNotifyPosted;

    //
    CStdPtrArray        m_aNotifiers;
    CStdPtrArray        m_aTranslateAccelerator;
    CStdPtrArray        m_aPreMessageFilters;
    CStdPtrArray        m_aMessageFilters;
    CStdPtrArray        m_aNativeWindow;
    MAP_STR_ATTRIBUTE_INFO                                                              m_mWindowCustomAttrHash;
    std::vector<std::shared_ptr<TIMERINFO>>                                             m_aTimers;

    std::vector<std::shared_ptr<CControlUI>>                                            m_aDelayedCleanup;
    std::vector<std::shared_ptr<TNotifyUI>>                                             m_aAsyncNotify;
    std::vector<std::shared_ptr<CControlUI>>                                            m_aPostPaintControls;
    std::vector<std::shared_ptr<CControlUI>>                                            m_aNativeWindowControl;
    std::vector<std::shared_ptr<CControlUI>>                                            m_aFoundControls;
    std::vector<std::shared_ptr<CControlUI>>                                            m_aNeedMouseLeaveNeeded;
    std::unordered_map<faw::string_t,std::shared_ptr<CControlUI>>                       m_mNameHash;
    std::unordered_map<faw::string_t, std::vector<std::shared_ptr<CControlUI>>>         m_mOptionGroup;

    bool                                m_bForceUseSharedRes;
    TResInfo                            m_ResInfo;

    // 窗口阴影
    CShadowUI                           m_shadow;

    // DPI管理器
    std::shared_ptr<CDPI>               m_pDPI;
    // 是否开启Gdiplus
    bool                                m_bUseGdiplusText;
    int                                 m_trh;
    ULONG_PTR                           m_gdiplusToken;

    // 拖拽
    bool                                m_bDragMode;
    HBITMAP                             m_hDragBitmap;

    std::function<bool(HDROP)>          m_DropCallback;

    double			                    m_lfProgress;	                       //进度条百分比
    ITaskbarList3*                      m_pTaskBarList3 = nullptr;	           //任务栏列表接口

    //
    static HINSTANCE                        m_hInstance;
    static HINSTANCE                        m_hResourceInstance;
    static faw::string_t                    m_pStrResourcePath;
    static faw::string_t                    m_pStrResourceZip;
    static faw::string_t                    m_pStrResourceZipPwd;
    static bool                             m_bCachedResourceZip;
    static int                              m_nResType;
    static TResInfo                         m_SharedResInfo;
    static bool                             m_bUseHSL;
    static short                            m_H;
    static short                            m_S;
    static short                            m_L;
    static CStdPtrArray                     m_aPreMessages;
    static CStdPtrArray                     m_aPlugins;
    static inline std::vector<HANDLE>       m_aFonts;
    static inline MAP_STR_RESURCE_CAHCE     m_mapNameToResurceCahce;
    static inline MAP_STR_FONT_COLLECTION   m_mapNameToFontCollection;
};

} // namespace DuiLib

#endif // __UIMANAGER_H__
