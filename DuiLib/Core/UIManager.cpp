#include "StdAfx.h"
#include <zmouse.h>

namespace DuiLib
{

/////////////////////////////////////////////////////////////////////////////////////
//
//

static void GetChildWndRect(HWND hWnd, HWND hChildWnd, RECT& rcChildWnd)
{
    ::GetWindowRect(hChildWnd, &rcChildWnd);

    POINT pt = { 0 };
    pt.x = rcChildWnd.left;
    pt.y = rcChildWnd.top;
    ::ScreenToClient(hWnd, &pt);
    rcChildWnd.left = pt.x;
    rcChildWnd.top = pt.y;

    pt.x = rcChildWnd.right;
    pt.y = rcChildWnd.bottom;
    ::ScreenToClient(hWnd, &pt);
    rcChildWnd.right = pt.x;
    rcChildWnd.bottom = pt.y;
}

static UINT MapKeyState()
{
    UINT uState = 0;
    if(::GetKeyState(VK_CONTROL) < 0) uState |= MK_CONTROL;
    if(::GetKeyState(VK_LBUTTON) < 0) uState |= MK_LBUTTON;
    if(::GetKeyState(VK_RBUTTON) < 0) uState |= MK_RBUTTON;
    if(::GetKeyState(VK_SHIFT) < 0) uState |= MK_SHIFT;
    if(::GetKeyState(VK_MENU) < 0) uState |= MK_ALT;
    return uState;
}

typedef struct tagFINDTABINFO
{
    std::shared_ptr<CControlUI> pFocus;
    std::shared_ptr<CControlUI> pLast;
    bool bForward;
    bool bNextIsIt;
} FINDTABINFO;

typedef struct tagFINDSHORTCUT
{
    TCHAR ch;
    bool bPickNext;
} FINDSHORTCUT;


tagTDrawInfo::tagTDrawInfo()
{
    Clear();
}

void tagTDrawInfo::Parse(faw::string_t pStrImage, faw::string_t pStrModify, CPaintManagerUI *paintManager)
{
    // 1、aaa.jpg
    // 2、file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0'
    // ='0,0,0,0'
    // mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false'
    sDrawString = pStrImage;
    sDrawModify = pStrModify;
    sImageName = pStrImage;

    for(size_t i = 0; i < 2; ++i)
    {
        std::map<faw::string_t, faw::string_t> m = FawTools::parse_keyvalue_pairs(i == 0 ? pStrImage : pStrModify);
        for(auto[str_key, str_value] : m)
        {
            if(str_key == _T("file") || str_key == _T("res"))
            {
                sImageName = str_value;
            }
            else if(str_key == _T("restype"))
            {
                sResType = str_value;
            }
            else if(str_key == _T("dest"))
            {
                rcDest = FawTools::parse_rect(str_value);
                if(paintManager)
                    paintManager->GetDPIObj()->Scale(&rcDest);
            }
            else if(str_key == _T("source"))
            {
                rcSource = FawTools::parse_rect(str_value);
                if(paintManager)
                    paintManager->GetDPIObj()->Scale(&rcSource);
            }
            else if(str_key == _T("corner"))
            {
                rcCorner = FawTools::parse_rect(str_value);
                if(paintManager)
                    paintManager->GetDPIObj()->Scale(&rcCorner);
            }
            else if(str_key == _T("mask"))
            {
                dwMask = (DWORD) FawTools::parse_hex(str_value);
            }
            else if(str_key == _T("fade"))
            {
                uFade = (BYTE) _ttoi(str_value.c_str());
            }
            else if(str_key == _T("hole"))
            {
                bHole = FawTools::parse_bool(str_value);
            }
            else if(str_key == _T("xtiled"))
            {
                bTiledX = FawTools::parse_bool(str_value);
            }
            else if(str_key == _T("ytiled"))
            {
                bTiledY = FawTools::parse_bool(str_value);
            }
            else if(str_key == _T("hsl"))
            {
                bHSL = FawTools::parse_bool(str_value);
            }
            else if(str_key == _T("svgcolor"))
            {
                svgcolor = (DWORD)FawTools::parse_hex(str_value);
            }
            else if(str_key == _T("size"))
            {
                szDesireDraw = FawTools::parse_size(str_value);
            }
            else if(str_key == _T("align"))
            {
                sAlign = str_value;
            }
        }
    }

    // 调整DPI资源
    if(paintManager && paintManager->GetDPIObj()->GetScale() != 100)
    {
        faw::string_t OriginalImageName = sImageName;
        faw::string_t sScale = std::format(_T("@{}."), (int) paintManager->GetDPIObj()->GetScale());
        FawTools::replace_self(sImageName, _T("."), sScale);

        TDataArgs dataArgs;
        TimageArgs imageArgs;
        imageArgs.bitmap = sImageName;
        bool bDelete = CRenderEngine::LoadResource2Memory(dataArgs, imageArgs);
        if(!dataArgs.pData && dataArgs.dataSize == 0)
        {
            sImageName = OriginalImageName;
            paintManager->GetDPIObj()->ScaleBack(&rcSource);
        }
    }
}
void tagTDrawInfo::Clear()
{
    sDrawString.clear();
    sDrawModify.clear();
    sImageName.clear();

    memset(&rcDest, 0, sizeof(RECT));
    memset(&rcSource, 0, sizeof(RECT));
    memset(&rcCorner, 0, sizeof(RECT));
    dwMask = 0;
    uFade = 255;
    bHole = false;
    bTiledX = false;
    bTiledY = false;
    bHSL = false;
    szDesireDraw = { 0, 0 };
    sAlign.clear();
}

/////////////////////////////////////////////////////////////////////////////////////
using PFUNC_UPDATE_LAYERED_WINDOW = BOOL(__stdcall*)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
PFUNC_UPDATE_LAYERED_WINDOW g_fUpdateLayeredWindow = nullptr;

HPEN m_hUpdateRectPen = nullptr;

HINSTANCE CPaintManagerUI::m_hResourceInstance = nullptr;
faw::string_t CPaintManagerUI::m_pStrResourcePath;
faw::string_t CPaintManagerUI::m_pStrResourceZip;
faw::string_t CPaintManagerUI::m_pStrResourceZipPwd;  //Garfield 20160325 带密码zip包解密
bool CPaintManagerUI::m_bCachedResourceZip = true;
int CPaintManagerUI::m_nResType = UILIB_FILE;
TResInfo CPaintManagerUI::m_SharedResInfo;
HINSTANCE CPaintManagerUI::m_hInstance = nullptr;
bool CPaintManagerUI::m_bUseHSL = false;
short CPaintManagerUI::m_H = 180;
short CPaintManagerUI::m_S = 100;
short CPaintManagerUI::m_L = 100;
CStdPtrArray CPaintManagerUI::m_aPreMessages;
CStdPtrArray CPaintManagerUI::m_aPlugins;

CPaintManagerUI::CPaintManagerUI() :
    m_hWndPaint(nullptr),
    m_hDcPaint(nullptr),
    m_hDcOffscreen(nullptr),
    m_hDcBackground(nullptr),
    m_bOffscreenPaint(true),
    m_hbmpOffscreen(nullptr),
    m_pOffscreenBits(nullptr),
    m_hbmpBackground(nullptr),
    m_pBackgroundBits(nullptr),
    m_hwndTooltip(nullptr),
    m_uTimerID(0x1000),
    m_pRoot(nullptr),
    m_pFocus(nullptr),
    m_pEventHover(nullptr),
    m_pEventClick(nullptr),
    m_pEventKey(nullptr),
    m_bFirstLayout(true),
    m_bFocusNeeded(false),
    m_bUpdateNeeded(false),
    m_bMouseTracking(false),
    m_bMouseCapture(false),
    m_bUsedVirtualWnd(false),
    m_bAsyncNotifyPosted(false),
    m_bForceUseSharedRes(false),
    m_nOpacity(0xFF),
    m_bLayered(false),
    m_bLayeredChanged(false),
    m_bShowUpdateRect(false),
    m_bUseGdiplusText(true),
    m_trh(0),
    m_bDragMode(false),
    m_hDragBitmap(nullptr),
    m_pDPI(nullptr),
    m_iHoverTime(400UL)
{
    if(!m_SharedResInfo.m_DefaultFontInfo)
    {
        m_SharedResInfo.m_DefaultFontInfo = std::make_shared<TFontInfo>();
        m_SharedResInfo.m_dwDefaultDisabledColor = 0xFFA7A6AA;
        m_SharedResInfo.m_dwDefaultFontColor = 0xFF000000;
        m_SharedResInfo.m_dwDefaultLinkFontColor = 0xFF0000FF;
        m_SharedResInfo.m_dwDefaultLinkHoverFontColor = 0xFFD3215F;
        m_SharedResInfo.m_dwDefaultSelectedBkColor = 0xFFBAE4FF;

        LOGFONT lf = { 0 };
        ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
        lf.lfCharSet = DEFAULT_CHARSET;
        HFONT hDefaultFont = ::CreateFontIndirect(&lf);
        m_SharedResInfo.m_DefaultFontInfo->hFont = hDefaultFont;
        m_SharedResInfo.m_DefaultFontInfo->sFontName = lf.lfFaceName;
        m_SharedResInfo.m_DefaultFontInfo->iSize = -lf.lfHeight;
        m_SharedResInfo.m_DefaultFontInfo->bBold = (lf.lfWeight >= FW_BOLD);
        m_SharedResInfo.m_DefaultFontInfo->bUnderline = (lf.lfUnderline == TRUE);
        m_SharedResInfo.m_DefaultFontInfo->bItalic = (lf.lfItalic == TRUE);
        ::ZeroMemory(&m_SharedResInfo.m_DefaultFontInfo->tm, sizeof(m_SharedResInfo.m_DefaultFontInfo->tm));
    }

    m_ResInfo.m_DefaultFontInfo = std::make_shared<TFontInfo>();
    m_ResInfo.m_dwDefaultDisabledColor = m_SharedResInfo.m_dwDefaultDisabledColor;
    m_ResInfo.m_dwDefaultFontColor = m_SharedResInfo.m_dwDefaultFontColor;
    m_ResInfo.m_dwDefaultLinkFontColor = m_SharedResInfo.m_dwDefaultLinkFontColor;
    m_ResInfo.m_dwDefaultLinkHoverFontColor = m_SharedResInfo.m_dwDefaultLinkHoverFontColor;
    m_ResInfo.m_dwDefaultSelectedBkColor = m_SharedResInfo.m_dwDefaultSelectedBkColor;

    if(!m_hUpdateRectPen)
    {
        m_hUpdateRectPen = ::CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
        // Boot Windows Common Controls (for the ToolTip control)
        ::InitCommonControls();
        ::LoadLibrary(_T("msimg32.dll"));
    }

    m_diLayered = std::make_shared<TDrawInfo>();

    m_szMinWindow.cx = 0;
    m_szMinWindow.cy = 0;
    m_szMaxWindow.cx = 0;
    m_szMaxWindow.cy = 0;
    m_szInitWindowSize.cx = 0;
    m_szInitWindowSize.cy = 0;
    m_szRoundCorner.cx = m_szRoundCorner.cy = 0;
    ::ZeroMemory(&m_rcSizeBox, sizeof(m_rcSizeBox));
    ::ZeroMemory(&m_rcCaption, sizeof(m_rcCaption));
    ::ZeroMemory(&m_rcLayeredInset, sizeof(m_rcLayeredInset));
    ::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
    m_ptLastMousePos.x = m_ptLastMousePos.y = -1;

    static GdiplusInitializer gdiplusInitializer;

    CShadowUI::Initialize(m_hInstance);
}

CPaintManagerUI::~CPaintManagerUI()
{
    // Delete the control-tree structures
    m_aDelayedCleanup.clear();
    m_aAsyncNotify.clear();
    m_mNameHash.clear();

    if(m_ResInfo.m_DefaultFontInfo)::DeleteObject(m_ResInfo.m_DefaultFontInfo->hFont);
    RemoveAllFonts();
    RemoveAllImages();
    RemoveAllStyle();
    RemoveAllDefaultAttributeList();
    RemoveAllWindowCustomAttribute();
    RemoveAllOptionGroups();
    RemoveAllTimers();
    RemoveAllDrawInfos();

    if(m_hwndTooltip)
    {
        ::DestroyWindow(m_hwndTooltip);
        m_hwndTooltip = nullptr;
    }
    if(m_hDcOffscreen) ::DeleteDC(m_hDcOffscreen);
    if(m_hDcBackground) ::DeleteDC(m_hDcBackground);
    if(m_hbmpOffscreen) ::DeleteObject(m_hbmpOffscreen);
    if(m_hbmpBackground) ::DeleteObject(m_hbmpBackground);
    if(m_hDcPaint) ::ReleaseDC(m_hWndPaint, m_hDcPaint);
    m_aPreMessages.Remove(m_aPreMessages.Find(this));
    //// 销毁拖拽图片
    if(m_hDragBitmap) ::DeleteObject(m_hDragBitmap);
    if(m_pTaskBarList3) m_pTaskBarList3->Release();
}

void CPaintManagerUI::Init(HWND hWnd, faw::string_t pstrName)
{
    ASSERT(::IsWindow(hWnd));

    m_mNameHash.clear();
    RemoveAllFonts();
    RemoveAllImages();
    RemoveAllStyle();
    RemoveAllDefaultAttributeList();
    RemoveAllWindowCustomAttribute();
    RemoveAllOptionGroups();
    RemoveAllTimers();

    m_sName.clear();
    if(!pstrName.empty()) m_sName = pstrName;

    if(m_hWndPaint != hWnd)
    {
        m_hWndPaint = hWnd;
        m_hDcPaint = ::GetDC(hWnd);
        m_aPreMessages.Add(this);
    }

    SetTargetWnd(hWnd);
    InitDragDrop();
    InitTaskbarList();
}

void CPaintManagerUI::DeletePtr(void* ptr)
{
    if(ptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

HINSTANCE CPaintManagerUI::GetInstance()
{
    return m_hInstance;
}

faw::string_t CPaintManagerUI::GetInstancePath()
{
    if(!m_hInstance) return _T("");

    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(m_hInstance, tszModule, MAX_PATH);
    faw::string_t sInstancePath = tszModule;
    size_t pos = sInstancePath.rfind(_T('\\'));
    if(pos != faw::string_t::npos) sInstancePath = sInstancePath.substr(0, pos + 1);
    return sInstancePath;
}

faw::string_t CPaintManagerUI::GetCurrentPath()
{
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetCurrentDirectory(MAX_PATH, tszModule);
    return tszModule;
}

HINSTANCE CPaintManagerUI::GetResourceDll()
{
    return m_hResourceInstance ? m_hResourceInstance : m_hInstance;
}

const faw::string_t CPaintManagerUI::GetResourcePath()
{
    return m_pStrResourcePath;
}

const faw::string_t CPaintManagerUI::GetResourceZip()
{
    return m_pStrResourceZip;
}

const faw::string_t CPaintManagerUI::GetResourceZipPwd()
{
    return m_pStrResourceZipPwd;
}

bool CPaintManagerUI::IsCachedResourceZip()
{
    return m_bCachedResourceZip;
}

std::shared_ptr<CacheZipEntry> CPaintManagerUI::GetResourceZipCacheEntry(faw::string_t& name)
{
    if(m_mapNameToResurceCahce.empty())return nullptr;
    for(auto& MapNameToResurceEntery: m_mapNameToResurceCahce)
        if(MapNameToResurceEntery.second.contains(name))
            return MapNameToResurceEntery.second[name];
    return nullptr;
}

void CPaintManagerUI::SetInstance(HINSTANCE hInst)
{
    m_hInstance = hInst;
}

void CPaintManagerUI::SetCurrentPath(faw::string_t pStrPath)
{
    ::SetCurrentDirectory(pStrPath.data());
}

void CPaintManagerUI::SetResourceDll(HINSTANCE hInst)
{
    m_hResourceInstance = hInst;
}

void CPaintManagerUI::SetResourcePath(faw::string_t pStrPath)
{
    m_pStrResourcePath = pStrPath;
    if(m_pStrResourcePath.empty()) return;
    TCHAR cEnd = m_pStrResourcePath[m_pStrResourcePath.size() - 1];
    if(cEnd != _T('\\') && cEnd != _T('/')) m_pStrResourcePath += _T('\\');
}

void CPaintManagerUI::SetResourceZip(faw::string_t pStrPath,LPVOID pVoid, unsigned int len, faw::string_t password)
{
    m_pStrResourceZip = pStrPath;
    m_bCachedResourceZip = true;
    m_pStrResourceZipPwd = password;  //Garfield 20160325 带密码zip包解密
    if(m_bCachedResourceZip)
    {
        ScopeCostTime time("unzip memoryzip");
        std::string pwd = FawTools::T_to_gb18030(password);
        HZIP hz = (HZIP)OpenZip(pVoid, len, pwd.c_str());
        if(!hz) return;
        if(m_mapNameToResurceCahce.contains(pStrPath))
            m_mapNameToResurceCahce[pStrPath].clear();
        std::unordered_map<faw::string_t, std::shared_ptr<CacheZipEntry>> NameToResurceEntery;
        GetAllZipItem(hz, NameToResurceEntery);
        m_mapNameToResurceCahce[pStrPath] = std::move(NameToResurceEntery);
        CloseZip(hz);
    }
}

void CPaintManagerUI::SetResourceZip(faw::string_t pStrPath, bool bCachedResourceZip, faw::string_t password)
{
    m_pStrResourceZip = pStrPath;
    m_bCachedResourceZip = bCachedResourceZip;
    m_pStrResourceZipPwd = password;
    if(m_bCachedResourceZip)
    {
        ScopeCostTime time("unzip filezip");
        faw::string_t sFile = CPaintManagerUI::GetResourcePath() + CPaintManagerUI::GetResourceZip();
        std::ifstream file(sFile, std::ios::binary | std::ios::ate);						// 打开文件并将文件指针移动到文件末尾
        if(file.is_open())
        {
            std::streampos fileSize = file.tellg();											// 获取文件大小
            file.seekg(0, std::ios::beg);													// 将文件指针移回文件开头
            std::unique_ptr<char[]> fileData = std::make_unique<char[]>(fileSize);			// 使用 std::make_unique 分配内存空间
            file.read(fileData.get(), fileSize);											// 读取文件内容到内存中
            SetResourceZip(pStrPath, fileData.get(), fileSize, password);
            file.close();																	// 关闭文件
        }
        else
        {
            MessageBox(NULL, _T("file could not open!!"), _T("error"), NULL);
        }
    }
}

void CPaintManagerUI::SetResourceType(int nType)
{
    m_nResType = nType;
}

int CPaintManagerUI::GetResourceType()
{
    return m_nResType;
}

bool CPaintManagerUI::GetHSL(short* H, short* S, short* L)
{
    *H = m_H;
    *S = m_S;
    *L = m_L;
    return m_bUseHSL;
}

void CPaintManagerUI::SetHSL(bool bUseHSL, short H, short S, short L)
{
    if(m_bUseHSL || m_bUseHSL != bUseHSL)
    {
        m_bUseHSL = bUseHSL;
        if(H == m_H && S == m_S && L == m_L) return;
        m_H = CLAMP(H, 0, 360);
        m_S = CLAMP(S, 0, 200);
        m_L = CLAMP(L, 0, 200);
        AdjustSharedImagesHSL();
        for(int i = 0; i < m_aPreMessages.GetSize(); i++)
        {
            CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
            if(pManager) pManager->AdjustImagesHSL();
        }
    }
}

void CPaintManagerUI::ReloadSkin()
{
    ReloadSharedImages();
    for(int i = 0; i < m_aPreMessages.GetSize(); i++)
    {
        CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
        if(pManager) pManager->ReloadImages();
    }
}

CPaintManagerUI* CPaintManagerUI::GetPaintManager(faw::string_t pstrName)
{
    //if (pstrName.empty ()) return nullptr;
    for(int i = 0; i < m_aPreMessages.GetSize(); i++)
    {
        CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
        if(pManager && pstrName == pManager->GetName()) return pManager;
    }
    return nullptr;
}

CStdPtrArray* CPaintManagerUI::GetPaintManagers()
{
    return &m_aPreMessages;
}

bool CPaintManagerUI::LoadPlugin(faw::string_t pstrModuleName)
{
    ASSERT(!pstrModuleName.empty());
    if(pstrModuleName.empty()) return false;
    HMODULE hModule = ::LoadLibrary(pstrModuleName.data());
    if(hModule)
    {
        LPCREATECONTROL lpCreateControl = (LPCREATECONTROL)::GetProcAddress(hModule, "CreateControl");
        if(lpCreateControl)
        {
            if(m_aPlugins.Find(lpCreateControl) >= 0) return true;
            m_aPlugins.Add(lpCreateControl);
            return true;
        }
    }
    return false;
}

CStdPtrArray* CPaintManagerUI::GetPlugins()
{
    return &m_aPlugins;
}

HWND CPaintManagerUI::GetPaintWindow() const
{
    return m_hWndPaint;
}

HWND CPaintManagerUI::GetTooltipWindow() const
{
    return m_hwndTooltip;
}
int CPaintManagerUI::GetHoverTime() const
{
    return m_iHoverTime;
}

void CPaintManagerUI::SetHoverTime(int iTime)
{
    m_iHoverTime = iTime;
}

faw::string_t CPaintManagerUI::GetName() const
{
    return m_sName;
}

HDC CPaintManagerUI::GetPaintDC() const
{
    return m_hDcPaint;
}

POINT CPaintManagerUI::GetMousePos() const
{
    return m_ptLastMousePos;
}

SIZE CPaintManagerUI::GetClientSize() const
{
    RECT rcClient = { 0 };
    ::GetClientRect(m_hWndPaint, &rcClient);
    return { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
}

SIZE CPaintManagerUI::GetInitSize()
{
    return m_szInitWindowSize;
}

void CPaintManagerUI::SetInitSize(int cx, int cy)
{
    m_szInitWindowSize.cx = cx;
    m_szInitWindowSize.cy = cy;
    if(!m_pRoot && m_hWndPaint)
    {
        ::SetWindowPos(m_hWndPaint, nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
}

RECT& CPaintManagerUI::GetSizeBox()
{
    return m_rcSizeBox;
}

void CPaintManagerUI::SetSizeBox(RECT& rcSizeBox)
{
    m_rcSizeBox = rcSizeBox;
}

RECT& CPaintManagerUI::GetCaptionRect()
{
    return m_rcCaption;
}

void CPaintManagerUI::SetCaptionRect(RECT& rcCaption)
{
    m_rcCaption = rcCaption;
}

SIZE CPaintManagerUI::GetRoundCorner() const
{
    return m_szRoundCorner;
}

void CPaintManagerUI::SetRoundCorner(int cx, int cy)
{
    m_szRoundCorner.cx = cx;
    m_szRoundCorner.cy = cy;
}

SIZE CPaintManagerUI::GetMinInfo() const
{
    return m_szMinWindow;
}

void CPaintManagerUI::SetMinInfo(int cx, int cy)
{
    ASSERT(cx >= 0 && cy >= 0);
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

SIZE CPaintManagerUI::GetMaxInfo() const
{
    return m_szMaxWindow;
}

void CPaintManagerUI::SetMaxInfo(int cx, int cy)
{
    ASSERT(cx >= 0 && cy >= 0);
    m_szMaxWindow.cx = cx;
    m_szMaxWindow.cy = cy;
}

bool CPaintManagerUI::IsShowUpdateRect() const
{
    return m_bShowUpdateRect;
}

void CPaintManagerUI::SetShowUpdateRect(bool show)
{
    m_bShowUpdateRect = show;
}

bool CPaintManagerUI::IsNoActivate()
{
    return m_bNoActivate;
}

void CPaintManagerUI::SetNoActivate(bool bNoActivate)
{
    m_bNoActivate = bNoActivate;
}

BYTE CPaintManagerUI::GetOpacity() const
{
    return m_nOpacity;
}

void CPaintManagerUI::SetOpacity(BYTE nOpacity)
{
    m_nOpacity = nOpacity;
    if(m_hWndPaint)
    {
        typedef BOOL (__stdcall *PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
        PFUNCSETLAYEREDWINDOWATTR fSetLayeredWindowAttributes = nullptr;

        HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
        if(hUser32)
        {
            fSetLayeredWindowAttributes =
                (PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
            if(!fSetLayeredWindowAttributes) return;
        }

        DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
        DWORD dwNewStyle = dwStyle;
        if(nOpacity >= 0 && nOpacity < 256) dwNewStyle |= WS_EX_LAYERED;
        else dwNewStyle &= ~WS_EX_LAYERED;
        if(dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
        fSetLayeredWindowAttributes(m_hWndPaint, 0, nOpacity, LWA_ALPHA);
    }
}

bool CPaintManagerUI::IsLayered()
{
    return m_bLayered;
}

void CPaintManagerUI::SetLayered(bool bLayered)
{
    if(m_hWndPaint && bLayered != m_bLayered)
    {
        UINT uStyle = GetWindowStyle(m_hWndPaint);
        if((uStyle & WS_CHILD) != 0) return;
        if(!g_fUpdateLayeredWindow)
        {
            HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
            if(hUser32)
            {
                g_fUpdateLayeredWindow =
                    (PFUNC_UPDATE_LAYERED_WINDOW)::GetProcAddress(hUser32, "UpdateLayeredWindow");
                if(!g_fUpdateLayeredWindow) return;
            }
        }
        m_bLayered = bLayered;
        if(m_pRoot) m_pRoot->NeedUpdate();
        Invalidate();
    }
}

RECT& CPaintManagerUI::GetLayeredInset()
{
    return m_rcLayeredInset;
}

void CPaintManagerUI::SetLayeredInset(RECT& rcLayeredInset)
{
    m_rcLayeredInset = rcLayeredInset;
    m_bLayeredChanged = true;
    Invalidate();
}

BYTE CPaintManagerUI::GetLayeredOpacity()
{
    return m_nOpacity;
}

void CPaintManagerUI::SetLayeredOpacity(BYTE nOpacity)
{
    m_nOpacity = nOpacity;
    m_bLayeredChanged = true;
    Invalidate();
}

faw::string_t CPaintManagerUI::GetLayeredImage()
{
    return m_diLayered->sDrawString;
}

void CPaintManagerUI::SetLayeredImage(faw::string_t pstrImage)
{
    m_diLayered->sDrawString = pstrImage;
    RECT rcnullptr = { 0 };
    CRenderEngine::DrawImageInfo(nullptr, this, rcnullptr, rcnullptr, m_diLayered);
    m_bLayeredChanged = true;
    Invalidate();
}

CShadowUI* CPaintManagerUI::GetShadow()
{
    return &m_shadow;
}

void CPaintManagerUI::SetUseGdiplusText(bool bUse)
{
    m_bUseGdiplusText = bUse;
}

bool CPaintManagerUI::IsUseGdiplusText() const
{
    return m_bUseGdiplusText;
}

void CPaintManagerUI::SetGdiplusTextRenderingHint(int trh)
{
    m_trh = trh;
}

int CPaintManagerUI::GetGdiplusTextRenderingHint() const
{
    return m_trh;
}

std::optional<LRESULT> CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    for(int i = 0; i < m_aPreMessageFilters.GetSize(); i++)
    {
        const std::optional<LRESULT> _ret = static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i])->MessageHandler(uMsg, wParam, lParam);
        if(_ret.has_value())
            return 0;
    }
    switch(uMsg)
    {
    case WM_KEYDOWN:
    {
        // Tabbing between controls
        if(wParam == VK_TAB)
        {
            if(m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && m_pFocus->GetClass().find(_T("RichEdit")) != faw::string_t::npos)
            {
                if(std::dynamic_pointer_cast<CRichEditUI>(m_pFocus)->IsWantTab())
                    return std::nullopt;
            }
            if(m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && m_pFocus->GetClass().find(_T("WkeWebkitUI")) != faw::string_t::npos)
            {
                return std::nullopt;
            }
            SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
            return 0;
        }
    }
    break;
    case WM_SYSCHAR:
    {
        // Handle ALT-shortcut key-combinations
        FINDSHORTCUT fs = { 0 };
        fs.ch = (TCHAR) toupper((int) wParam);
        std::shared_ptr<CControlUI> pControl = m_pRoot->FindControl(__FindControlFromShortcut, &fs, UIFIND_ENABLED | UIFIND_ME_FIRST | UIFIND_TOP_FIRST);
        if(pControl)
        {
            pControl->SetFocus();
            pControl->Activate();
            return 0;
        }
    }
    break;
    case WM_SYSKEYDOWN:
    {
        if(m_pFocus)
        {
            TEventUI event = { 0 };
            event.Type = UIEVENT_SYSKEY;
            event.chKey = (TCHAR) wParam;
            event.ptMouse = m_ptLastMousePos;
            event.wKeyState = (WORD) MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            m_pFocus->Event(event);
        }
    }
    break;
    }
    return std::nullopt;
}

std::optional<LRESULT> CPaintManagerUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(!m_hWndPaint) return std::nullopt;
    // Cycle through listeners
    std::optional<LRESULT> lRes = std::nullopt;
    for(int i = 0; i < m_aMessageFilters.GetSize(); i++)
    {
        lRes = static_cast<IMessageFilterUI*>(m_aMessageFilters[i])->MessageHandler(uMsg, wParam, lParam);
        if(lRes.has_value())
        {
            switch(uMsg)
            {
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONUP:
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                m_ptLastMousePos = pt;
            }
            break;
            case WM_CONTEXTMENU:
            case WM_MOUSEWHEEL:
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ::ScreenToClient(m_hWndPaint, &pt);
                m_ptLastMousePos = pt;
            }
            break;
            }
            return lRes.value();
        }
    }

    if(m_bLayered)
    {
        switch(uMsg)
        {
        case WM_NCACTIVATE:
            if(!::IsIconic(m_hWndPaint))
            {
                return (wParam == 0) ? 1 : 0;
            }
            break;
        case WM_NCCALCSIZE:
        case WM_NCPAINT:
            return 0;
        }
    }
    // Custom handling of events
    switch(uMsg)
    {
    case WM_APP + 1:
    {
        m_aDelayedCleanup.clear();

        m_bAsyncNotifyPosted = false;

        std::shared_ptr<TNotifyUI> pMsg = nullptr;
        for(auto& pMsg :m_aAsyncNotify)
        {
            if(pMsg->pSender&&pMsg->pSender->OnNotify)
            {
                pMsg->pSender->OnNotify(pMsg.get());
            }
            for(int j = 0; j < m_aNotifiers.GetSize(); j++)
            {
                static_cast<INotifyUI*>(m_aNotifiers[j])->Notify(*pMsg);
            }
        }
        m_aAsyncNotify.clear();
    }
    break;
    case WM_CLOSE:
    {
        // Make sure all matching "closing" events are sent
        TEventUI event = { 0 };
        event.ptMouse = m_ptLastMousePos;
        event.wKeyState = (WORD) MapKeyState();
        event.dwTimestamp = ::GetTickCount();
        if(m_pEventHover)
        {
            event.Type = UIEVENT_MOUSELEAVE;
            event.pSender = m_pEventHover;
            m_pEventHover->Event(event);
        }
        if(m_pEventClick)
        {
            event.Type = UIEVENT_BUTTONUP;
            event.pSender = m_pEventClick;
            m_pEventClick->Event(event);
        }

        SetFocus(nullptr);

        if(::GetActiveWindow() == m_hWndPaint)
        {
            HWND hwndParent = GetWindowOwner(m_hWndPaint);
            if(hwndParent) ::SetFocus(hwndParent);
        }

        if(m_hwndTooltip)
        {
            ::DestroyWindow(m_hwndTooltip);
            m_hwndTooltip = nullptr;
        }
    }
    break;
    case WM_ERASEBKGND:
    {
        // We'll do the painting here...
    }
    return 1;
    case WM_PAINT:
    {
        if(!m_pRoot)
        {
            PAINTSTRUCT ps = { 0 };
            ::BeginPaint(m_hWndPaint, &ps);
            CRenderEngine::DrawColor(m_hDcPaint, ps.rcPaint, 0xFF000000);
            ::EndPaint(m_hWndPaint, &ps);
            return lRes;
        }

        RECT rcClient = { 0 };
        ::GetClientRect(m_hWndPaint, &rcClient);

        RECT rcPaint = { 0 };
        if(!::GetUpdateRect(m_hWndPaint, &rcPaint, FALSE)) return lRes;

        //if( m_bLayered ) {
        //	m_bOffscreenPaint = true;
        //	rcPaint = m_rcLayeredUpdate;
        //	if( ::IsRectEmpty(&m_rcLayeredUpdate) ) {
        //		PAINTSTRUCT ps = { 0 };
        //		::BeginPaint(m_hWndPaint, &ps);
        //		::EndPaint(m_hWndPaint, &ps);
        //		return lRes;
        //	}
        //	if( rcPaint.right > rcClient.right ) rcPaint.right = rcClient.right;
        //	if( rcPaint.bottom > rcClient.bottom ) rcPaint.bottom = rcClient.bottom;
        //	::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
        //}
        //else {
        //	if( !::GetUpdateRect(m_hWndPaint, &rcPaint, FALSE) ) return lRes;
        //}

        // Set focus to first control?
        if(m_bFocusNeeded)
        {
            SetNextTabControl();
        }

        SetPainting(true);

        bool bNeedSizeMsg = false;
        DWORD dwWidth = rcClient.right - rcClient.left;
        DWORD dwHeight = rcClient.bottom - rcClient.top;

        SetPainting(true);
        if(m_bUpdateNeeded)
        {
            m_bUpdateNeeded = false;
            if(!::IsRectEmpty(&rcClient) && !::IsIconic(m_hWndPaint))
            {
                if(m_pRoot->IsUpdateNeeded())
                {
                    RECT rcRoot = rcClient;
                    if(m_hDcOffscreen) ::DeleteDC(m_hDcOffscreen);
                    if(m_hDcBackground) ::DeleteDC(m_hDcBackground);
                    if(m_hbmpOffscreen) ::DeleteObject(m_hbmpOffscreen);
                    if(m_hbmpBackground) ::DeleteObject(m_hbmpBackground);
                    m_hDcOffscreen = nullptr;
                    m_hDcBackground = nullptr;
                    m_hbmpOffscreen = nullptr;
                    m_hbmpBackground = nullptr;
                    if(m_bLayered)
                    {
                        rcRoot.left += m_rcLayeredInset.left;
                        rcRoot.top += m_rcLayeredInset.top;
                        rcRoot.right -= m_rcLayeredInset.right;
                        rcRoot.bottom -= m_rcLayeredInset.bottom;
                    }
                    m_pRoot->SetPos(rcRoot, true);
                    bNeedSizeMsg = true;
                }
                else
                {
                    std::shared_ptr<CControlUI> pControl = nullptr;
                    m_aFoundControls.clear();
                    m_pRoot->FindControl(__FindControlsFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST | UIFIND_UPDATETEST);
                    for(auto& pControl: m_aFoundControls)
                    {
                        if(!pControl->IsFloat()) pControl->SetPos(pControl->GetPos(), true);
                        else pControl->SetPos(pControl->GetRelativePos(), true);
                    }
                    bNeedSizeMsg = true;
                }
                // We'll want to notify the window when it is first initialized
                // with the correct layout. The window form would take the time
                // to submit swipes/animations.
                if(m_bFirstLayout)
                {
                    m_bFirstLayout = false;
                    SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWINIT, 0, 0, false);
                    if(m_bLayered && m_bLayeredChanged)
                    {
                        Invalidate();
                        SetPainting(false);
                        //return lRes;
                    }
                    // 更新阴影窗口显示
                    m_shadow.Update(m_hWndPaint);
                }
            }
        }
        else if(m_bLayered && m_bLayeredChanged)
        {
            RECT rcRoot = rcClient;
            if(m_pOffscreenBits) ::ZeroMemory(m_pOffscreenBits, (rcRoot.right - rcRoot.left)
                                                  * (rcRoot.bottom - rcRoot.top) * 4);
            rcRoot.left += m_rcLayeredInset.left;
            rcRoot.top += m_rcLayeredInset.top;
            rcRoot.right -= m_rcLayeredInset.right;
            rcRoot.bottom -= m_rcLayeredInset.bottom;
            m_pRoot->SetPos(rcRoot, true);
        }

        if(m_bLayered)
        {
            DWORD dwExStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
            DWORD dwNewExStyle = dwExStyle | WS_EX_LAYERED;
            if(dwExStyle != dwNewExStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewExStyle);
            m_bOffscreenPaint = true;
            UnionRect(&rcPaint, &rcPaint, &m_rcLayeredUpdate);
            if(rcPaint.right > rcClient.right) rcPaint.right = rcClient.right;
            if(rcPaint.bottom > rcClient.bottom) rcPaint.bottom = rcClient.bottom;
            ::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
        }

        //
        // Render screen
        //
        // Prepare offscreen bitmap
        if(m_bOffscreenPaint && !m_hbmpOffscreen)
        {
            m_hDcOffscreen = ::CreateCompatibleDC(m_hDcPaint);
            m_hbmpOffscreen = CRenderEngine::CreateARGB32Bitmap(m_hDcPaint, dwWidth, dwHeight, (LPBYTE*) &m_pOffscreenBits);
            ASSERT(m_hDcOffscreen);
            ASSERT(m_hbmpOffscreen);
        }
        // Begin Windows paint
        PAINTSTRUCT ps = { 0 };
        ::BeginPaint(m_hWndPaint, &ps);
        if(m_bOffscreenPaint)
        {
            HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(m_hDcOffscreen, m_hbmpOffscreen);
            int iSaveDC = ::SaveDC(m_hDcOffscreen);
            if(m_bLayered)
            {
                for(LONG y = rcClient.bottom - rcPaint.bottom; y < rcClient.bottom - rcPaint.top; ++y)
                {
                    for(LONG x = rcPaint.left; x < rcPaint.right; ++x)
                    {
                        int i = (y * dwWidth + x) * 4;
                        *(DWORD*)(&m_pOffscreenBits[i]) = 0;
                    }
                }
            }
            m_pRoot->Paint(m_hDcOffscreen, rcPaint, nullptr);

            if(m_bLayered)
            {
                for(int i = 0; i < m_aNativeWindow.GetSize();)
                {
                    HWND hChildWnd = static_cast<HWND>(m_aNativeWindow[i]);
                    if(!::IsWindow(hChildWnd))
                    {
                        m_aNativeWindow.Remove(i);
                        auto it = m_aNativeWindowControl.begin();
                        std::advance(it,i);
                        m_aNativeWindowControl.erase(it);
                        continue;
                    }
                    ++i;
                    if(!::IsWindowVisible(hChildWnd)) continue;
                    RECT rcChildWnd = GetNativeWindowRect(hChildWnd);
                    RECT rcTemp = { 0 };
                    if(!::IntersectRect(&rcTemp, &rcPaint, &rcChildWnd)) continue;

                    COLORREF* pChildBitmapBits = nullptr;
                    HDC hChildMemDC = ::CreateCompatibleDC(m_hDcOffscreen);
                    HBITMAP hChildBitmap = CRenderEngine::CreateARGB32Bitmap(hChildMemDC, rcChildWnd.right - rcChildWnd.left, rcChildWnd.bottom - rcChildWnd.top, (BYTE**) &pChildBitmapBits);
                    ::ZeroMemory(pChildBitmapBits, (rcChildWnd.right - rcChildWnd.left)*(rcChildWnd.bottom - rcChildWnd.top) * 4);
                    HBITMAP hOldChildBitmap = (HBITMAP) ::SelectObject(hChildMemDC, hChildBitmap);
                    ::SendMessage(hChildWnd, WM_PRINT, (WPARAM) hChildMemDC, (LPARAM)(PRF_CHECKVISIBLE | PRF_CHILDREN | PRF_CLIENT | PRF_OWNED));
                    COLORREF* pChildBitmapBit;
                    for(LONG y = 0; y < rcChildWnd.bottom - rcChildWnd.top; y++)
                    {
                        for(LONG x = 0; x < rcChildWnd.right - rcChildWnd.left; x++)
                        {
                            pChildBitmapBit = pChildBitmapBits + y * (rcChildWnd.right - rcChildWnd.left) + x;
                            if(*pChildBitmapBit != 0x00000000) *pChildBitmapBit |= 0xff000000;
                        }
                    }
                    ::BitBlt(m_hDcOffscreen, rcChildWnd.left, rcChildWnd.top, rcChildWnd.right - rcChildWnd.left,
                             rcChildWnd.bottom - rcChildWnd.top, hChildMemDC, 0, 0, SRCCOPY);
                    ::SelectObject(hChildMemDC, hOldChildBitmap);
                    ::DeleteObject(hChildBitmap);
                    ::DeleteDC(hChildMemDC);
                }
            }

            for(auto& pPostPaintControl: m_aPostPaintControls)
                pPostPaintControl->DoPostPaint(m_hDcOffscreen, rcPaint);

            ::RestoreDC(m_hDcOffscreen, iSaveDC);

            if(m_bLayered)
            {
                RECT rcWnd = { 0 };
                ::GetWindowRect(m_hWndPaint, &rcWnd);
                if(!m_diLayered->sDrawString.empty())
                {
                    DWORD _dwWidth = rcClient.right - rcClient.left;
                    DWORD _dwHeight = rcClient.bottom - rcClient.top;
                    RECT rcLayeredClient = rcClient;
                    rcLayeredClient.left += m_rcLayeredInset.left;
                    rcLayeredClient.top += m_rcLayeredInset.top;
                    rcLayeredClient.right -= m_rcLayeredInset.right;
                    rcLayeredClient.bottom -= m_rcLayeredInset.bottom;

                    COLORREF* pOffscreenBits = (COLORREF*) m_pOffscreenBits;
                    COLORREF* pBackgroundBits = m_pBackgroundBits;
                    BYTE A = 0;
                    BYTE R = 0;
                    BYTE G = 0;
                    BYTE B = 0;
                    if(!m_diLayered->sDrawString.empty())
                    {
                        if(!m_hbmpBackground)
                        {
                            m_hDcBackground = ::CreateCompatibleDC(m_hDcPaint);
                            m_hbmpBackground = CRenderEngine::CreateARGB32Bitmap(m_hDcPaint, _dwWidth, _dwHeight, (BYTE**) &m_pBackgroundBits);
                            ::ZeroMemory(m_pBackgroundBits, _dwWidth * _dwHeight * 4);
                            ::SelectObject(m_hDcBackground, m_hbmpBackground);
                            CRenderClip clip;
                            CRenderClip::GenerateClip(m_hDcBackground, rcLayeredClient, clip);
                            CRenderEngine::DrawImageInfo(m_hDcBackground, this, rcLayeredClient, rcLayeredClient, m_diLayered);
                        }
                        else if(m_bLayeredChanged)
                        {
                            ::ZeroMemory(m_pBackgroundBits, _dwWidth * _dwHeight * 4);
                            CRenderClip clip;
                            CRenderClip::GenerateClip(m_hDcBackground, rcLayeredClient, clip);
                            CRenderEngine::DrawImageInfo(m_hDcBackground, this, rcLayeredClient, rcLayeredClient, m_diLayered);
                        }
                        for(LONG y = rcClient.bottom - rcPaint.bottom; y < rcClient.bottom - rcPaint.top; ++y)
                        {
                            for(LONG x = rcPaint.left; x < rcPaint.right; ++x)
                            {
                                pOffscreenBits = (COLORREF*)(m_pOffscreenBits + y * _dwWidth + x);
                                pBackgroundBits = m_pBackgroundBits + y * _dwWidth + x;
                                A = (BYTE)((*pBackgroundBits) >> 24);
                                R = (BYTE)((*pOffscreenBits) >> 16) * A / 255;
                                G = (BYTE)((*pOffscreenBits) >> 8) * A / 255;
                                B = (BYTE)(*pOffscreenBits) * A / 255;
                                *pOffscreenBits = RGB(B, G, R) + ((DWORD) A << 24);
                            }
                        }
                    }
                }
                else
                {
                    for(LONG y = rcClient.bottom - rcPaint.bottom; y < rcClient.bottom - rcPaint.top; ++y)
                    {
                        for(LONG x = rcPaint.left; x < rcPaint.right; ++x)
                        {
                            int i = (y * dwWidth + x) * 4;
                            if((m_pOffscreenBits[i + 3] == 0) && (m_pOffscreenBits[i + 0] != 0 || m_pOffscreenBits[i + 1] != 0 || m_pOffscreenBits[i + 2] != 0))
                                m_pOffscreenBits[i + 3] = 255;
                        }
                    }
                }

                BLENDFUNCTION bf = { AC_SRC_OVER, 0, m_nOpacity, AC_SRC_ALPHA };
                POINT ptPos = { rcWnd.left, rcWnd.top };
                SIZE sizeWnd = { (LONG) dwWidth, (LONG) dwHeight };
                POINT ptSrc = { 0, 0 };
                g_fUpdateLayeredWindow(m_hWndPaint, m_hDcPaint, &ptPos, &sizeWnd, m_hDcOffscreen, &ptSrc, 0, &bf, ULW_ALPHA);
            }
            else
            {
                ::BitBlt(m_hDcPaint, rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left, rcPaint.bottom - rcPaint.top, m_hDcOffscreen, rcPaint.left, rcPaint.top, SRCCOPY);
            }
            ::SelectObject(m_hDcOffscreen, hOldBitmap);

            if(m_bShowUpdateRect && !m_bLayered)
            {
                HPEN hOldPen = (HPEN)::SelectObject(m_hDcPaint, m_hUpdateRectPen);
                ::SelectObject(m_hDcPaint, ::GetStockObject(HOLLOW_BRUSH));
                ::Rectangle(m_hDcPaint, rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
                ::SelectObject(m_hDcPaint, hOldPen);
            }
        }
        else
        {
            // A standard paint job
            int iSaveDC = ::SaveDC(m_hDcPaint);
            m_pRoot->Paint(m_hDcPaint, rcPaint, nullptr);

            for(auto& pPostPaintControl : m_aPostPaintControls)
                pPostPaintControl->DoPostPaint(m_hDcPaint, rcPaint);

            ::RestoreDC(m_hDcPaint, iSaveDC);
        }
        // All Done!
        ::EndPaint(m_hWndPaint, &ps);

        // 绘制结束
        SetPainting(false);
        m_bLayeredChanged = false;
        if(m_bUpdateNeeded) Invalidate();

        // 发送窗口大小改变消息
        if(bNeedSizeMsg)
        {
            this->SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWSIZE, 0, 0, true);
        }
        return lRes;
    }
    case WM_PRINTCLIENT:
    {
        if(!m_pRoot) break;
        RECT rcClient = { 0 };
        ::GetClientRect(m_hWndPaint, &rcClient);
        HDC hDC = (HDC) wParam;
        int save = ::SaveDC(hDC);
        m_pRoot->Paint(hDC, rcClient, nullptr);
        if((lParam & PRF_CHILDREN) != 0)
        {
            HWND hWndChild = ::GetWindow(m_hWndPaint, GW_CHILD);
            while(hWndChild)
            {
                RECT rcPos = { 0 };
                ::GetWindowRect(hWndChild, &rcPos);
                ::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, reinterpret_cast<LPPOINT>(&rcPos), 2);
                ::SetWindowOrgEx(hDC, -rcPos.left, -rcPos.top, nullptr);
                ::SendMessage(hWndChild, WM_PRINT, wParam, lParam | PRF_NONCLIENT);
                hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
            }
        }
        ::RestoreDC(hDC, save);
    }
    break;
    case WM_GETMINMAXINFO:
    {
        MONITORINFO Monitor = {};
        Monitor.cbSize = sizeof(Monitor);
        ::GetMonitorInfo(::MonitorFromWindow(m_hWndPaint, MONITOR_DEFAULTTOPRIMARY), &Monitor);
        RECT rcWork = Monitor.rcWork;
        if(Monitor.dwFlags != MONITORINFOF_PRIMARY)
        {
            ::OffsetRect(&rcWork, -rcWork.left, -rcWork.top);
        }

        LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
        if(m_szMinWindow.cx > 0) lpMMI->ptMinTrackSize.x = m_szMinWindow.cx;
        if(m_szMinWindow.cy > 0) lpMMI->ptMinTrackSize.y = m_szMinWindow.cy;
        if(m_szMaxWindow.cx > 0) lpMMI->ptMaxTrackSize.x = m_szMaxWindow.cx;
        if(m_szMaxWindow.cy > 0) lpMMI->ptMaxTrackSize.y = m_szMaxWindow.cy;
        if(m_szMaxWindow.cx > 0) lpMMI->ptMaxSize.x = m_szMaxWindow.cx;
        if(m_szMaxWindow.cy > 0) lpMMI->ptMaxSize.y = m_szMaxWindow.cy;
    }
    break;
    case WM_SIZE:
    {
        if(m_pFocus)
        {
            TEventUI event = { 0 };
            event.Type = UIEVENT_WINDOWSIZE;
            event.pSender = m_pFocus;
            event.dwTimestamp = ::GetTickCount();
            m_pFocus->Event(event);
        }
        if(m_pRoot) m_pRoot->NeedUpdate();
    }
    return 0;
    case WM_TIMER:
    {
        for(size_t i = 0; i < m_aTimers.size(); i++)
        {
            const auto pTimer = m_aTimers[i];
            if(pTimer->hWnd == m_hWndPaint && pTimer->uWinTimer == LOWORD(wParam) && pTimer->bKilled == false)
            {
                TEventUI event = { 0 };
                event.Type = UIEVENT_TIMER;
                event.pSender = pTimer->pSender;
                event.dwTimestamp = ::GetTickCount();
                event.ptMouse = m_ptLastMousePos;
                event.wKeyState = (WORD) MapKeyState();
                event.wParam = pTimer->nLocalID;
                event.lParam = lParam;
                pTimer->pSender->Event(event);
                break;
            }
        }
    }
    break;
    case WM_MOUSEHOVER:
    {
        m_bMouseTracking = false;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        std::shared_ptr<CControlUI> pHover = FindControl(pt);
        if(!pHover) break;
        // Generate mouse hover event
        if(m_pEventHover)
        {
            TEventUI event = { 0 };
            event.Type = UIEVENT_MOUSEHOVER;
            event.pSender = m_pEventHover;
            event.wParam = wParam;
            event.lParam = lParam;
            event.dwTimestamp = ::GetTickCount();
            event.ptMouse = pt;
            event.wKeyState = (WORD) MapKeyState();
            m_pEventHover->Event(event);
        }
        // Create tooltip information
        faw::string_t sToolTip = pHover->GetToolTip();
        if(sToolTip.empty()) return 0;
        ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
        m_ToolTip.cbSize = sizeof(TOOLINFO);
        m_ToolTip.uFlags = TTF_IDISHWND;
        m_ToolTip.hwnd = m_hWndPaint;
        m_ToolTip.uId = (UINT_PTR) m_hWndPaint;
        m_ToolTip.hinst = m_hInstance;
        m_ToolTip.lpszText = &sToolTip[0];
        m_ToolTip.rect = pHover->GetPos();
        if(!m_hwndTooltip)
        {
            m_hwndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWndPaint, NULL, m_hInstance, nullptr);
            ::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM) &m_ToolTip);
            ::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, pHover->GetToolTipWidth());
        }
        if(!::IsWindowVisible(m_hwndTooltip))
        {
            // 设置工具提示显示在最前面,避免被其他窗体遮挡
            ::SetWindowPos(m_hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            ::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM) &m_ToolTip);
            ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM) &m_ToolTip);
        }
    }
    return 0;
    case WM_MOUSELEAVE:
    {
        if(m_hwndTooltip) ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
        if(m_bMouseTracking)
        {
            POINT pt = { 0 };
            RECT rcWnd = { 0 };
            ::GetCursorPos(&pt);
            ::GetWindowRect(m_hWndPaint, &rcWnd);
            if(!::IsIconic(m_hWndPaint) && ::GetActiveWindow() == m_hWndPaint && ::PtInRect(&rcWnd, pt))
            {
                if(::SendMessage(m_hWndPaint, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTCLIENT)
                {
                    ::ScreenToClient(m_hWndPaint, &pt);
                    ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
                }
                else
                    ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) -1);
            }
            else
                ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) -1);
        }
        m_bMouseTracking = false;
    }
    break;
    case WM_MOUSEMOVE:
    {
        // Start tracking this entire window again...
        if(!m_bMouseTracking)
        {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_HOVER | TME_LEAVE;
            tme.hwndTrack = m_hWndPaint;
            tme.dwHoverTime = !m_hwndTooltip ? m_iHoverTime : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
            _TrackMouseEvent(&tme);
            m_bMouseTracking = true;
        }

        // Generate the appropriate mouse messages
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        // 是否移动
        bool bNeedDrag = true;
        if(m_ptLastMousePos.x == pt.x && m_ptLastMousePos.y == pt.y)
        {
            bNeedDrag = false;
        }
        // 记录鼠标位置
        m_ptLastMousePos = pt;
        std::shared_ptr<CControlUI> pNewHover = FindControl(pt);
        if(pNewHover && pNewHover->GetManager() != this) break;

        // 拖拽事件
        if(bNeedDrag && m_bDragMode && wParam == MK_LBUTTON)
        {
            ::ReleaseCapture();
            auto pdsrc = std::make_shared<CIDropSource> ();
            if(!pdsrc) return std::nullopt;
            pdsrc->AddRef();

            auto pdobj = std::make_shared<CIDataObject>(pdsrc.get());
            if(!pdobj) return std::nullopt;
            pdobj->AddRef();

            FORMATETC fmtetc = { 0 };
            STGMEDIUM medium = { 0 };
            fmtetc.dwAspect = DVASPECT_CONTENT;
            fmtetc.lindex = -1;
            //////////////////////////////////////
            fmtetc.cfFormat = CF_BITMAP;
            fmtetc.tymed = TYMED_GDI;
            medium.tymed = TYMED_GDI;
            HBITMAP hBitmap = (HBITMAP) OleDuplicateData(m_hDragBitmap, fmtetc.cfFormat, NULL);
            medium.hBitmap = hBitmap;
            pdobj->SetData(&fmtetc, &medium, FALSE);
            //////////////////////////////////////
            BITMAP bmap;
            GetObject(hBitmap, sizeof(BITMAP), &bmap);
            RECT rc = { 0, 0, bmap.bmWidth, bmap.bmHeight };
            fmtetc.cfFormat = CF_ENHMETAFILE;
            fmtetc.tymed = TYMED_ENHMF;
            HDC hMetaDC = CreateEnhMetaFile(m_hDcPaint, nullptr, nullptr, nullptr);
            HDC hdcMem = CreateCompatibleDC(m_hDcPaint);
            HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBitmap);
            ::BitBlt(hMetaDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
            ::SelectObject(hdcMem, hOldBmp);
            medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
            DeleteDC(hdcMem);
            medium.tymed = TYMED_ENHMF;
            pdobj->SetData(&fmtetc, &medium, TRUE);
            //////////////////////////////////////
            CDragSourceHelper dragSrcHelper;
            POINT ptDrag = { 0 };
            ptDrag.x = bmap.bmWidth / 2;
            ptDrag.y = bmap.bmHeight / 2;
            dragSrcHelper.InitializeFromBitmap(hBitmap, ptDrag, rc, pdobj.get());  //will own the bmp
            DWORD dwEffect;
            HRESULT hr = ::DoDragDrop(pdobj.get(), pdsrc.get(), DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
            m_bDragMode = false;
            break;
        }
        TEventUI event = { 0 };
        event.ptMouse = pt;
        event.wParam = wParam;
        event.lParam = lParam;
        event.dwTimestamp = ::GetTickCount();
        event.wKeyState = (WORD) MapKeyState();
        if(!IsCaptured())
        {
            pNewHover = FindControl(pt);
            if(pNewHover && pNewHover->GetManager() != this) break;
            if(pNewHover != m_pEventHover && m_pEventHover)
            {
                event.Type = UIEVENT_MOUSELEAVE;
                event.pSender = m_pEventHover;

                auto aNeedMouseLeaveNeeded=m_aNeedMouseLeaveNeeded;
                for(auto pControl: aNeedMouseLeaveNeeded)
                {
                    pControl->Event(event);
                }

                m_pEventHover->Event(event);
                m_pEventHover = nullptr;
                if(m_hwndTooltip) ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
            }
            if(pNewHover != m_pEventHover && pNewHover)
            {
                event.Type = UIEVENT_MOUSEENTER;
                event.pSender = pNewHover;
                pNewHover->Event(event);
                m_pEventHover = pNewHover;
            }
        }
        if(m_pEventClick)
        {
            event.Type = UIEVENT_MOUSEMOVE;
            event.pSender = m_pEventClick;
            m_pEventClick->Event(event);
        }
        else if(pNewHover)
        {
            event.Type = UIEVENT_MOUSEMOVE;
            event.pSender = pNewHover;
            pNewHover->Event(event);
        }
    }
    break;
    case WM_LBUTTONDOWN:
    {
        // We alway set focus back to our app (this helps
        // when Win32 child windows are placed on the dialog
        // and we need to remove them on focus change).
        if(!m_bNoActivate) ::SetFocus(m_hWndPaint);
        if(!m_pRoot) break;
        // 查找控件
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        std::shared_ptr<CControlUI> pControl = FindControl(pt);
        if(!pControl) break;
        if(pControl->GetManager() != this) break;

        // 准备拖拽
        if(pControl->IsDragEnabled())
        {
            m_bDragMode = true;
            if(m_hDragBitmap)
            {
                ::DeleteObject(m_hDragBitmap);
                m_hDragBitmap = nullptr;
            }
            m_hDragBitmap = CRenderEngine::GenerateBitmap(this, pControl, pControl->GetPos());
        }

        // 开启捕获
        SetCapture();
        // 事件处理
        m_pEventClick = pControl;
        pControl->SetFocus();

        TEventUI event = { 0 };
        event.Type = UIEVENT_BUTTONDOWN;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();
        pControl->Event(event);
    }
    break;
    case WM_LBUTTONDBLCLK:
    {
        if(!m_bNoActivate) ::SetFocus(m_hWndPaint);

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        std::shared_ptr<CControlUI> pControl = FindControl(pt);
        if(!pControl) break;
        if(pControl->GetManager() != this) break;
        SetCapture();
        TEventUI event = { 0 };
        event.Type = UIEVENT_DBLCLICK;
        event.pSender = pControl;
        event.ptMouse = pt;
        event.wParam = wParam;
        event.lParam = lParam;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();
        pControl->Event(event);
        m_pEventClick = pControl;
    }
    break;
    case WM_LBUTTONUP:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        if(!m_pEventClick) break;
        ReleaseCapture();
        TEventUI event = { 0 };
        event.Type = UIEVENT_BUTTONUP;
        event.pSender = m_pEventClick;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();

        std::shared_ptr<CControlUI> pClick = m_pEventClick;
        m_pEventClick = nullptr;
        pClick->Event(event);
    }
    break;
    case WM_RBUTTONDOWN:
    {
        if(!m_bNoActivate) ::SetFocus(m_hWndPaint);
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        std::shared_ptr<CControlUI> pControl = FindControl(pt);
        if(!pControl) break;
        if(pControl->GetManager() != this) break;
        pControl->SetFocus();
        SetCapture();
        TEventUI event = { 0 };
        event.Type = UIEVENT_RBUTTONDOWN;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();
        pControl->Event(event);
        m_pEventClick = pControl;
    }
    break;
    case WM_RBUTTONUP:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        m_pEventClick = FindControl(pt);
        if(!m_pEventClick) break;
        ReleaseCapture();
        TEventUI event = { 0 };
        event.Type = UIEVENT_RBUTTONUP;
        event.pSender = m_pEventClick;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();
        m_pEventClick->Event(event);
    }
    break;
    case WM_MBUTTONDOWN:
    {
        if(!m_bNoActivate) ::SetFocus(m_hWndPaint);
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        std::shared_ptr<CControlUI> pControl = FindControl(pt);
        if(!pControl) break;
        if(pControl->GetManager() != this) break;
        pControl->SetFocus();
        SetCapture();
        TEventUI event = { 0 };
        event.Type = UIEVENT_MBUTTONDOWN;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();
        pControl->Event(event);
        m_pEventClick = pControl;
    }
    break;
    case WM_MBUTTONUP:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_ptLastMousePos = pt;
        m_pEventClick = FindControl(pt);
        if(!m_pEventClick) break;
        ReleaseCapture();

        TEventUI event = { 0 };
        event.Type = UIEVENT_MBUTTONUP;
        event.pSender = m_pEventClick;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.dwTimestamp = ::GetTickCount();
        m_pEventClick->Event(event);
    }
    break;
    case WM_CONTEXTMENU:
    {
        if(!m_pRoot) break;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ::ScreenToClient(m_hWndPaint, &pt);
        m_ptLastMousePos = pt;
        if(!m_pEventClick) break;
        ReleaseCapture();
        TEventUI event = { 0 };
        event.Type = UIEVENT_CONTEXTMENU;
        event.pSender = m_pEventClick;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) wParam;
        event.lParam = (LPARAM) m_pEventClick.get();
        event.dwTimestamp = ::GetTickCount();
        m_pEventClick->Event(event);
        m_pEventClick = nullptr;
    }
    break;
    case WM_MOUSEWHEEL:
    {
        if(!m_pRoot) break;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ::ScreenToClient(m_hWndPaint, &pt);
        m_ptLastMousePos = pt;
        std::shared_ptr<CControlUI> pControl = FindControl(pt);
        if(!pControl) break;
        if(pControl->GetManager() != this) break;
        int zDelta = (int)(short) HIWORD(wParam);
        TEventUI event = { 0 };
        event.Type = UIEVENT_SCROLLWHEEL;
        event.pSender = pControl;
        event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) MapKeyState();
        event.dwTimestamp = ::GetTickCount();
        pControl->Event(event);

        // Let's make sure that the scroll item below the cursor is the same as before...
        ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) MAKELPARAM(m_ptLastMousePos.x, m_ptLastMousePos.y));
    }
    break;
    case WM_CHAR:
    {
        if(!m_pRoot) break;
        if(!m_pFocus) break;
        TEventUI event = { 0 };
        event.Type = UIEVENT_CHAR;
        event.pSender = m_pFocus;
        event.wParam = wParam;
        event.lParam = lParam;
        event.chKey = (TCHAR) wParam;
        event.ptMouse = m_ptLastMousePos;
        event.wKeyState = (WORD) MapKeyState();
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
    }
    break;
    case WM_KEYDOWN:
    {
        if(!m_pRoot) break;
        if(!m_pFocus) break;
        TEventUI event = { 0 };
        event.Type = UIEVENT_KEYDOWN;
        event.pSender = m_pFocus;
        event.wParam = wParam;
        event.lParam = lParam;
        event.chKey = (TCHAR) wParam;
        event.ptMouse = m_ptLastMousePos;
        event.wKeyState = (WORD) MapKeyState();
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        m_pEventKey = m_pFocus;
    }
    break;
    case WM_KEYUP:
    {
        if(!m_pRoot) break;
        if(!m_pEventKey) break;
        TEventUI event = { 0 };
        event.Type = UIEVENT_KEYUP;
        event.pSender = m_pEventKey;
        event.wParam = wParam;
        event.lParam = lParam;
        event.chKey = (TCHAR) wParam;
        event.ptMouse = m_ptLastMousePos;
        event.wKeyState = (WORD) MapKeyState();
        event.dwTimestamp = ::GetTickCount();
        m_pEventKey->Event(event);
        m_pEventKey = nullptr;
    }
    break;
    case WM_SETCURSOR:
    {
        if(!m_pRoot) break;
        if(LOWORD(lParam) != HTCLIENT) break;
        if(m_bMouseCapture) return 0;

        POINT pt = { 0 };
        ::GetCursorPos(&pt);
        ::ScreenToClient(m_hWndPaint, &pt);
        std::shared_ptr<CControlUI> pControl = FindControl(pt);
        if(!pControl) break;
        if((pControl->GetControlFlags() & UIFLAG_SETCURSOR) == 0) break;
        TEventUI event = { 0 };
        event.Type = UIEVENT_SETCURSOR;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD) MapKeyState();
        event.dwTimestamp = ::GetTickCount();
        pControl->Event(event);
    }
    return 0;
    case WM_SETFOCUS:
    {
        if(m_pFocus)
        {
            TEventUI event = { 0 };
            event.Type = UIEVENT_SETFOCUS;
            event.wParam = wParam;
            event.lParam = lParam;
            event.pSender = m_pFocus;
            event.dwTimestamp = ::GetTickCount();
            m_pFocus->Event(event);
        }
        break;
    }
    //case WM_KILLFOCUS:
    //{
    //	if (IsCaptured ()) ReleaseCapture ();
    //	break;
    //}
    case WM_NOTIFY:
    {
        if(lParam == 0) break;
        LPNMHDR lpNMHDR = (LPNMHDR) lParam;
        if(lpNMHDR) return ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
        return 0;
    }
    break;
    case WM_COMMAND:
    {
        if(lParam == 0) break;
        HWND hWndChild = (HWND) lParam;
        lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
        if(lRes != std::nullopt) return lRes;
    }
    break;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
    {
        // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
        // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
        if(lParam == 0) break;
        HWND hWndChild = (HWND) lParam;
        lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
        if(lRes != std::nullopt) return lRes;
    }
    break;
    default:
        break;
    }
    return std::nullopt;
}

bool CPaintManagerUI::IsUpdateNeeded() const
{
    return m_bUpdateNeeded;
}

void CPaintManagerUI::NeedUpdate()
{
    m_bUpdateNeeded = true;
}

void CPaintManagerUI::Invalidate()
{
    RECT rcClient = { 0 };
    ::GetClientRect(m_hWndPaint, &rcClient);
    ::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcClient);
    ::InvalidateRect(m_hWndPaint, nullptr, FALSE);
}

void CPaintManagerUI::Invalidate(RECT& rcItem)
{
    if(rcItem.left < 0) rcItem.left = 0;
    if(rcItem.top < 0) rcItem.top = 0;
    if(rcItem.right < rcItem.left) rcItem.right = rcItem.left;
    if(rcItem.bottom < rcItem.top) rcItem.bottom = rcItem.top;
    ::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcItem);
    // TODO 不加这一段将使得InvalidateRect被多次调用
    if(rcItem.right - rcItem.left <= 0 || rcItem.bottom - rcItem.top <= 0)
        return;
    ::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
}

bool CPaintManagerUI::AttachDialog(std::shared_ptr<CControlUI> pControl)
{
    ASSERT(::IsWindow(m_hWndPaint));
    // 创建阴影窗口
    m_shadow.Create(this);

    // Reset any previous attachment
    SetFocus(nullptr);
    m_pEventKey = nullptr;
    m_pEventHover = nullptr;
    m_pEventClick = nullptr;
    // Remove the existing control-tree. We might have gotten inside this function as
    // a result of an event fired or similar, so we cannot just delete the objects and
    // pull the internal memory of the calling code. We'll delay the cleanup.
    if(m_pRoot)
    {
        m_aPostPaintControls.clear();
        AddDelayedCleanup(m_pRoot);
    }
    // Set the dialog root element
    m_pRoot = pControl;
    // Go ahead...
    m_bUpdateNeeded = true;
    m_bFirstLayout = true;
    m_bFocusNeeded = false;
    // Initiate all control
    return InitControls(pControl);
}

bool CPaintManagerUI::InitControls(std::shared_ptr<CControlUI> pControl, std::shared_ptr<CControlUI> pParent /*= nullptr*/)
{
    ASSERT(pControl);
    if(!pControl) return false;
    pControl->SetManager(this, pParent ? pParent : pControl->GetParent(), true);
    pControl->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
    return true;
}

void CPaintManagerUI::ReapObjects(std::shared_ptr<CControlUI> pControl)
{
    if(pControl == m_pEventKey) m_pEventKey = nullptr;
    if(pControl == m_pEventHover) m_pEventHover = nullptr;
    if(pControl == m_pEventClick) m_pEventClick = nullptr;
    if(pControl == m_pFocus) m_pFocus = nullptr;
    KillTimer(pControl);
    const faw::string_t sName = pControl->GetName();
    if(!sName.empty() && FindControl(sName))
    {
        // 找到要删除的元素
        auto it = m_mNameHash.find(sName);
        if(it != m_mNameHash.end())
        {
            // 通过迭代器移除元素
            m_mNameHash.erase(it);
        }
    }
    for(size_t i = 0; i < m_aAsyncNotify.size(); i++)
    {
        std::shared_ptr<TNotifyUI> pMsg = m_aAsyncNotify[i];
        if(pMsg->pSender == pControl)
            pMsg->pSender = nullptr;
    }
}

bool CPaintManagerUI::AddOptionGroup(faw::string_t pStrGroupName, std::shared_ptr<CControlUI> pControl)
{
    auto lp = m_mOptionGroup.find(pStrGroupName);
    if(lp != m_mOptionGroup.end())
    {
        auto& aOptionGroup = lp->second;
        auto it = std::find(aOptionGroup.begin(), aOptionGroup.end(), pControl);
        if(it != aOptionGroup.end())
        {
            return false; // 找到了相同的控制对象
        }
        aOptionGroup.push_back(pControl);
    }
    else
    {
        std::vector<std::shared_ptr<CControlUI>> aOptionGroup;
        aOptionGroup.push_back(pControl);
        m_mOptionGroup[pStrGroupName] = std::move(aOptionGroup);
    }
    return true;
}

std::vector<std::shared_ptr<CControlUI>> CPaintManagerUI::GetOptionGroup(faw::string_t pStrGroupName)
{
    auto lp = m_mOptionGroup.find(pStrGroupName);
    if(lp != m_mOptionGroup.end())
        return lp->second;
    return std::vector<std::shared_ptr<CControlUI>>();
}

void CPaintManagerUI::RemoveOptionGroup(faw::string_t pStrGroupName, std::shared_ptr<CControlUI> pControl)
{
    auto lp = m_mOptionGroup.find(pStrGroupName);
    if(lp != m_mOptionGroup.end())
    {
        auto& aOptionGroup = lp->second;
        if(aOptionGroup.empty()) return;
        aOptionGroup.clear();
        if(aOptionGroup.empty())
        {
            m_mOptionGroup.erase(lp);
        }
    }
}

void CPaintManagerUI::RemoveAllOptionGroups()
{
    m_mOptionGroup.clear();
}

void CPaintManagerUI::MessageLoop()
{
    MSG msg = { 0 };
    while(::GetMessage(&msg, nullptr, 0, 0))
    {
        if(!CPaintManagerUI::TranslateMessage(&msg))
        {
            ::TranslateMessage(&msg);
            //try {
            ::DispatchMessage(&msg);
//				} catch (...) {
//					DUITRACE (_T ("EXCEPTION: %s(%d)\n"), __FILET__, __LINE__);
//#ifdef _DEBUG
//					throw "CPaintManagerUI::MessageLoop";
//#endif
//				}
        }
    }
}

void CPaintManagerUI::Term()
{
    // 销毁资源管理器
    CResourceManager::GetInstance()->Release();
    //CMenuWnd::DestroyMenu();

    // 清理共享资源
    // 图片
    for(auto& [k,v]: m_SharedResInfo.m_ImageHash)
    {
        if(!k.empty()&&v)
        {
            CRenderEngine::FreeImage(v);
        }
    }
    m_SharedResInfo.m_ImageHash.clear();
    // 字体
    for (auto& [k, v] : m_SharedResInfo.m_CustomFonts)
    {
        if (!k.empty()&&v)
        {
            auto& pFontInfo = v;
            ::DeleteObject(pFontInfo->hFont);
        }
    }
    m_SharedResInfo.m_CustomFonts.clear();
    // 默认字体
    if(m_SharedResInfo.m_DefaultFontInfo->hFont) ::DeleteObject(m_SharedResInfo.m_DefaultFontInfo->hFont);
    // 样式
    m_SharedResInfo.m_StyleHash.clear();
    m_SharedResInfo.m_AttrHash.clear();
    // 释放zip资源
    m_mapNameToResurceCahce.clear();
    // 释放共享字体
    m_mapNameToFontCollection.clear();
    
    for(auto& hFont: m_aFonts)
        ::RemoveFontMemResourceEx(hFont);
}

std::shared_ptr<CDPI> DuiLib::CPaintManagerUI::GetDPIObj()
{
    if(!m_pDPI)
    {
        m_pDPI = std::make_shared<CDPI>() ;
    }
    return m_pDPI;
}

void DuiLib::CPaintManagerUI::SetDPI(int iDPI)
{
    int scale1 = GetDPIObj()->GetScale();
    GetDPIObj()->SetScale(iDPI);
    int scale2 = GetDPIObj()->GetScale();
    ResetDPIAssets();
    RECT rcWnd = { 0 };
    ::GetWindowRect(GetPaintWindow(), &rcWnd);
    RECT*  prcNewWindow = &rcWnd;
    if(!::IsZoomed(GetPaintWindow()))
    {
        RECT rc = rcWnd;
        rc.right = rcWnd.left + (rcWnd.right - rcWnd.left) * scale2 / scale1;
        rc.bottom = rcWnd.top + (rcWnd.bottom - rcWnd.top) * scale2 / scale1;
        prcNewWindow = &rc;
    }
    //SetWindowPos (GetPaintWindow (), NULL, prcNewWindow->left, prcNewWindow->top, prcNewWindow->right - prcNewWindow->left, prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);
    if(GetRoot()) GetRoot()->NeedUpdate();
    ::PostMessage(GetPaintWindow(), UIMSG_SET_DPI, 0, 0);
}

void DuiLib::CPaintManagerUI::SetAllDPI(int iDPI)
{
    for(int i = 0; i < m_aPreMessages.GetSize(); i++)
    {
        CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
        pManager->SetDPI(iDPI);
    }
}

void DuiLib::CPaintManagerUI::ResetDPIAssets()
{
    RemoveAllDrawInfos();
    RemoveAllImages();

    for(auto& [k, pFontInfo]: m_ResInfo.m_CustomFonts)
    {
        RebuildFont(pFontInfo);
    }
    RebuildFont(m_ResInfo.m_DefaultFontInfo);

    for (auto& [k, pFontInfo] : m_SharedResInfo.m_CustomFonts)
    {
        RebuildFont(pFontInfo);
    }
    RebuildFont(m_SharedResInfo.m_DefaultFontInfo);

    auto& richEditList = FindSubControlsByClass(GetRoot(), _T("RichEdit"));
    for(auto& pT: richEditList)
    {
        std::shared_ptr<CRichEditUI> pRD = std::dynamic_pointer_cast<CRichEditUI>(pT);
        pRD->SetFont(pRD->GetFont());
    }
}

void DuiLib::CPaintManagerUI::RebuildFont(std::shared_ptr<TFontInfo> pFontInfo)
{
    if (!pFontInfo)return;
    ::DeleteObject(pFontInfo->hFont);
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    _tcsncpy(lf.lfFaceName, pFontInfo->sFontName.c_str(), LF_FACESIZE);
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -GetDPIObj()->Scale(pFontInfo->iSize);
    lf.lfQuality = CLEARTYPE_QUALITY;
    if(pFontInfo->bBold) lf.lfWeight += FW_BOLD;
    if(pFontInfo->bUnderline) lf.lfUnderline = TRUE;
    if(pFontInfo->bItalic) lf.lfItalic = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    pFontInfo->hFont = hFont;
    ::ZeroMemory(&(pFontInfo->tm), sizeof(pFontInfo->tm));
    if(m_hDcPaint)
    {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
}

std::shared_ptr<CControlUI> CPaintManagerUI::GetFocus() const
{
    return m_pFocus;
}

void CPaintManagerUI::SetFocus(std::shared_ptr<CControlUI> pControl)
{
    // Paint manager window has focus?
    HWND hFocusWnd = ::GetFocus();
    if(hFocusWnd != m_hWndPaint && pControl != m_pFocus) ::SetFocus(m_hWndPaint);
    // Already has focus?
    if(pControl == m_pFocus) return;
    // Remove focus from old control
    if(m_pFocus)
    {
        TEventUI event = { 0 };
        event.Type = UIEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, DUI_MSGTYPE_KILLFOCUS);
        m_pFocus = nullptr;
    }
    if(!pControl) return;
    // Set focus to new control
    if(pControl
            && pControl->GetManager() == this
            && pControl->IsVisible()
            && pControl->IsEnabled())
    {
        m_pFocus = pControl;
        TEventUI event = { 0 };
        event.Type = UIEVENT_SETFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, DUI_MSGTYPE_SETFOCUS);
    }
}

void CPaintManagerUI::SetFocusNeeded(std::shared_ptr<CControlUI> pControl)
{
    ::SetFocus(m_hWndPaint);
    if(!pControl) return;
    if(m_pFocus)
    {
        TEventUI event = { 0 };
        event.Type = UIEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, DUI_MSGTYPE_KILLFOCUS);
        m_pFocus = nullptr;
    }
    FINDTABINFO info = { 0 };
    info.pFocus = pControl;
    info.bForward = false;
    m_pFocus = m_pRoot->FindControl(__FindControlFromTab, &info, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    m_bFocusNeeded = true;
    if(m_pRoot) m_pRoot->NeedUpdate();
}

bool CPaintManagerUI::SetTimer(std::shared_ptr<CControlUI> pControl, UINT nTimerID, UINT uElapse)
{
    ASSERT(pControl);
    ASSERT(uElapse > 0);
    for(size_t i = 0; i < m_aTimers.size(); i++)
    {
        auto pTimer = m_aTimers[i];
        if(pTimer->pSender == pControl&& pTimer->hWnd == m_hWndPaint&& pTimer->nLocalID == nTimerID)
        {
            if(pTimer->bKilled == true)
            {
                if(::SetTimer(m_hWndPaint, pTimer->uWinTimer, uElapse, nullptr))
                {
                    pTimer->bKilled = false;
                    return true;
                }
                return false;
            }
            return false;
        }
    }

    m_uTimerID = (++m_uTimerID) % 0xF0; //0xf1-0xfe特殊用途
    if(!::SetTimer(m_hWndPaint, m_uTimerID, uElapse, nullptr)) return FALSE;
    auto pTimer = std::make_shared<TIMERINFO>() ;
    if(!pTimer) return FALSE;
    pTimer->hWnd = m_hWndPaint;
    pTimer->pSender = pControl;
    pTimer->nLocalID = nTimerID;
    pTimer->uWinTimer = m_uTimerID;
    pTimer->bKilled = false;
    m_aTimers.push_back(pTimer);
    return true;
}

bool CPaintManagerUI::KillTimer(std::shared_ptr<CControlUI> pControl, UINT nTimerID)
{
    ASSERT(pControl);
    for(size_t i = 0; i < m_aTimers.size(); i++)
    {
        auto pTimer = m_aTimers[i];
        if(pTimer->pSender == pControl
                && pTimer->hWnd == m_hWndPaint
                && pTimer->nLocalID == nTimerID)
        {
            if(pTimer->bKilled == false)
            {
                if(::IsWindow(m_hWndPaint)) ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
                pTimer->bKilled = true;
                return true;
            }
        }
    }
    return false;
}

void CPaintManagerUI::KillTimer(std::shared_ptr<CControlUI> pControl)
{
    ASSERT(pControl);
    for (int i = m_aTimers.size() - 1; i >= 0; --i)
    {
        auto pTimer = m_aTimers[i];
        if (pTimer->pSender == pControl && pTimer->hWnd == m_hWndPaint)
        {
            if (!pTimer->bKilled)
                ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
            m_aTimers.erase(m_aTimers.begin() + i);
        }
    }
}

void CPaintManagerUI::RemoveAllTimers()
{
    for(size_t i = 0; i < m_aTimers.size(); i++)
    {
        auto pTimer = m_aTimers[i];
        if(pTimer->hWnd == m_hWndPaint&& pTimer->bKilled == false)
        {
            if(::IsWindow(m_hWndPaint))
                ::KillTimer(m_hWndPaint, pTimer->uWinTimer);
        }
    }
    m_aTimers.clear();
}

void CPaintManagerUI::SetCapture()
{
    ::SetCapture(m_hWndPaint);
    m_bMouseCapture = true;
}

void CPaintManagerUI::ReleaseCapture()
{
    ::ReleaseCapture();
    m_bMouseCapture = false;
    m_bDragMode = false;
}

bool CPaintManagerUI::IsCaptured()
{
    return m_bMouseCapture;
}

bool CPaintManagerUI::IsPainting()
{
    return m_bIsPainting;
}

void CPaintManagerUI::SetPainting(bool bIsPainting)
{
    m_bIsPainting = bIsPainting;
}

bool CPaintManagerUI::SetNextTabControl(bool bForward)
{
    // If we're in the process of restructuring the layout we can delay the
    // focus calulation until the next repaint.
    if(m_bUpdateNeeded && bForward)
    {
        m_bFocusNeeded = true;
        ::InvalidateRect(m_hWndPaint, nullptr, FALSE);
        return true;
    }
    // Find next/previous tabbable control
    FINDTABINFO info1 = { 0 };
    info1.pFocus = m_pFocus;
    info1.bForward = bForward;
    std::shared_ptr<CControlUI> pControl = m_pRoot->FindControl(__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    if(!pControl)
    {
        if(bForward)
        {
            // Wrap around
            FINDTABINFO info2 = { 0 };
            info2.pFocus = bForward ? nullptr : info1.pLast;
            info2.bForward = bForward;
            pControl = m_pRoot->FindControl(__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
        }
        else
        {
            pControl = info1.pLast;
        }
    }
    if(pControl) SetFocus(pControl);
    m_bFocusNeeded = false;
    return true;
}

bool CPaintManagerUI::AddNotifier(INotifyUI* pNotifier)
{
    ASSERT(m_aNotifiers.Find(pNotifier) < 0);
    return m_aNotifiers.Add(pNotifier);
}

bool CPaintManagerUI::RemoveNotifier(INotifyUI* pNotifier)
{
    for(int i = 0; i < m_aNotifiers.GetSize(); i++)
    {
        if(static_cast<INotifyUI*>(m_aNotifiers[i]) == pNotifier)
        {
            return m_aNotifiers.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::AddPreMessageFilter(IMessageFilterUI* pFilter)
{
    ASSERT(m_aPreMessageFilters.Find(pFilter) < 0);
    return m_aPreMessageFilters.Add(pFilter);
}

bool CPaintManagerUI::RemovePreMessageFilter(IMessageFilterUI* pFilter)
{
    for(int i = 0; i < m_aPreMessageFilters.GetSize(); i++)
    {
        if(static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i]) == pFilter)
        {
            return m_aPreMessageFilters.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::AddMessageFilter(IMessageFilterUI* pFilter)
{
    ASSERT(m_aMessageFilters.Find(pFilter) < 0);
    return m_aMessageFilters.Add(pFilter);
}

bool CPaintManagerUI::RemoveMessageFilter(IMessageFilterUI* pFilter)
{
    for(int i = 0; i < m_aMessageFilters.GetSize(); i++)
    {
        if(static_cast<IMessageFilterUI*>(m_aMessageFilters[i]) == pFilter)
        {
            return m_aMessageFilters.Remove(i);
        }
    }
    return false;
}

int CPaintManagerUI::GetPostPaintCount() const
{
    return m_aPostPaintControls.size();
}

bool CPaintManagerUI::IsPostPaint(std::shared_ptr<CControlUI> pControl)
{
    // 使用 std::find 查找第一个匹配的元素
    auto it = std::find(m_aPostPaintControls.begin(), m_aPostPaintControls.end(), pControl);
    // 如果找到了，使用 erase 移除该元素
    if(it != m_aPostPaintControls.end())
        return true;
    return false;
}

bool CPaintManagerUI::AddPostPaint(std::shared_ptr<CControlUI> pControl)
{
    auto it = std::find(m_aPostPaintControls.begin(), m_aPostPaintControls.end(), pControl);
    ASSERT(it == m_aPostPaintControls.end());
    m_aPostPaintControls.push_back(pControl);
    return true;
}

bool CPaintManagerUI::RemovePostPaint(std::shared_ptr<CControlUI> pControl)
{
    // 使用 std::find 查找第一个匹配的元素
    auto it = std::find(m_aPostPaintControls.begin(), m_aPostPaintControls.end(), pControl);

    // 如果找到了，使用 erase 移除该元素
    if(it != m_aPostPaintControls.end())
    {
        m_aPostPaintControls.erase(it);
        return true;
    }
    return false;
}

bool CPaintManagerUI::SetPostPaintIndex(std::shared_ptr<CControlUI> pControl, int iIndex)
{
    RemovePostPaint(pControl);
    auto it = m_aPostPaintControls.begin() + iIndex; // 指向第3个元素
    m_aPostPaintControls.insert(it, pControl); // 在位置2处插入99
    return true;
}

int CPaintManagerUI::GetNativeWindowCount() const
{
    return m_aNativeWindow.GetSize();
}

bool CPaintManagerUI::AddNativeWindow(std::shared_ptr<CControlUI> pControl, HWND hChildWnd)
{
    if(!pControl || !hChildWnd) return false;

    RECT rcChildWnd = GetNativeWindowRect(hChildWnd);
    Invalidate(rcChildWnd);

    if(m_aNativeWindow.Find(hChildWnd) >= 0) return false;
    if(m_aNativeWindow.Add(hChildWnd))
    {
        m_aNativeWindowControl.push_back(pControl);
        return true;
    }
    return false;
}

bool CPaintManagerUI::RemoveNativeWindow(HWND hChildWnd)
{
    for(int i = 0; i < m_aNativeWindow.GetSize(); i++)
    {
        if(static_cast<HWND>(m_aNativeWindow[i]) == hChildWnd)
        {
            if(m_aNativeWindow.Remove(i))
            {
                auto it = m_aNativeWindowControl.begin();
                std::advance(it, i);
                m_aNativeWindowControl.erase(it);
                return true;
            }
            return false;
        }
    }
    return false;
}

RECT CPaintManagerUI::GetNativeWindowRect(HWND hChildWnd)
{
    RECT rcChildWnd = { 0 };
    ::GetWindowRect(hChildWnd, &rcChildWnd);
    ::ScreenToClient(m_hWndPaint, (LPPOINT)(&rcChildWnd));
    ::ScreenToClient(m_hWndPaint, (LPPOINT)(&rcChildWnd) + 1);
    return rcChildWnd;
}

void CPaintManagerUI::AddDelayedCleanup(std::shared_ptr<CControlUI> pControl)
{
    if(!pControl) return;
    pControl->SetManager(this, nullptr, false);
    m_aDelayedCleanup.push_back(pControl);
    PostAsyncNotify();
}

void CPaintManagerUI::AddMouseLeaveNeeded(std::shared_ptr<CControlUI> pControl)
{
    if(!pControl) return;
    auto it = std::find(m_aNeedMouseLeaveNeeded.begin(), m_aNeedMouseLeaveNeeded.end(), pControl);
    if(it != m_aNeedMouseLeaveNeeded.end())
        return;

    m_aNeedMouseLeaveNeeded.push_back(pControl);
}

bool CPaintManagerUI::RemoveMouseLeaveNeeded(std::shared_ptr<CControlUI> pControl)
{
    if(!pControl) return false;
    // 使用 std::remove 移动匹配的元素到末尾
    auto new_end = std::remove(m_aNeedMouseLeaveNeeded.begin(), m_aNeedMouseLeaveNeeded.end(), pControl);
    // 使用 std::vector::erase 删除从 new_end 到末尾的所有元素
    m_aNeedMouseLeaveNeeded.erase(new_end, m_aNeedMouseLeaveNeeded.end());

    return false;
}

void CPaintManagerUI::SendNotify(std::shared_ptr<CControlUI> pControl, faw::string_t pstrMessage, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/, bool bAsync /*= false*/)
{
    TNotifyUI Msg;
    Msg.pSender = pControl;
    Msg.sType = pstrMessage;
    Msg.wParam = wParam;
    Msg.lParam = lParam;
    SendNotify(Msg, bAsync);
}

void CPaintManagerUI::SendNotify(TNotifyUI& Msg, bool bAsync /*= false*/)
{
    Msg.ptMouse = m_ptLastMousePos;
    Msg.dwTimestamp = ::GetTickCount();
    if(m_bUsedVirtualWnd)
    {
        Msg.sVirtualWnd = Msg.pSender->GetVirtualWnd();
    }

    if(!bAsync)
    {
        // Send to all listeners
        if(Msg.pSender && Msg.pSender->OnNotify)
        {
            Msg.pSender->OnNotify(&Msg);
        }
        for(int i = 0; i < m_aNotifiers.GetSize(); i++)
        {
            static_cast<INotifyUI*>(m_aNotifiers[i])->Notify(Msg);
        }
    }
    else
    {
        std::shared_ptr<TNotifyUI>  pMsg = std::make_shared<TNotifyUI> ();
        pMsg->pSender = Msg.pSender;
        pMsg->sType = Msg.sType;
        pMsg->wParam = Msg.wParam;
        pMsg->lParam = Msg.lParam;
        pMsg->ptMouse = Msg.ptMouse;
        pMsg->dwTimestamp = Msg.dwTimestamp;
        m_aAsyncNotify.push_back(pMsg);

        PostAsyncNotify();
    }
}

bool CPaintManagerUI::IsForceUseSharedRes() const
{
    return m_bForceUseSharedRes;
}

void CPaintManagerUI::SetForceUseSharedRes(bool bForce)
{
    m_bForceUseSharedRes = bForce;
}

DWORD CPaintManagerUI::GetDefaultDisabledColor() const
{
    return m_ResInfo.m_dwDefaultDisabledColor;
}

void CPaintManagerUI::SetDefaultDisabledColor(DWORD dwColor, bool bShared)
{
    if(bShared)
    {
        if(m_ResInfo.m_dwDefaultDisabledColor == m_SharedResInfo.m_dwDefaultDisabledColor)
            m_ResInfo.m_dwDefaultDisabledColor = dwColor;
        m_SharedResInfo.m_dwDefaultDisabledColor = dwColor;
    }
    else
    {
        m_ResInfo.m_dwDefaultDisabledColor = dwColor;
    }
}

DWORD CPaintManagerUI::GetDefaultFontColor() const
{
    return m_ResInfo.m_dwDefaultFontColor;
}

void CPaintManagerUI::SetDefaultFontColor(DWORD dwColor, bool bShared)
{
    if(bShared)
    {
        if(m_ResInfo.m_dwDefaultFontColor == m_SharedResInfo.m_dwDefaultFontColor)
            m_ResInfo.m_dwDefaultFontColor = dwColor;
        m_SharedResInfo.m_dwDefaultFontColor = dwColor;
    }
    else
    {
        m_ResInfo.m_dwDefaultFontColor = dwColor;
    }
}

DWORD CPaintManagerUI::GetDefaultLinkFontColor() const
{
    return m_ResInfo.m_dwDefaultLinkFontColor;
}

void CPaintManagerUI::SetDefaultLinkFontColor(DWORD dwColor, bool bShared)
{
    if(bShared)
    {
        if(m_ResInfo.m_dwDefaultLinkFontColor == m_SharedResInfo.m_dwDefaultLinkFontColor)
            m_ResInfo.m_dwDefaultLinkFontColor = dwColor;
        m_SharedResInfo.m_dwDefaultLinkFontColor = dwColor;
    }
    else
    {
        m_ResInfo.m_dwDefaultLinkFontColor = dwColor;
    }
}

DWORD CPaintManagerUI::GetDefaultLinkHoverFontColor() const
{
    return m_ResInfo.m_dwDefaultLinkHoverFontColor;
}

void CPaintManagerUI::SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared)
{
    if(bShared)
    {
        if(m_ResInfo.m_dwDefaultLinkHoverFontColor == m_SharedResInfo.m_dwDefaultLinkHoverFontColor)
            m_ResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
        m_SharedResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
    }
    else
    {
        m_ResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
    }
}

DWORD CPaintManagerUI::GetDefaultSelectedBkColor() const
{
    return m_ResInfo.m_dwDefaultSelectedBkColor;
}

void CPaintManagerUI::SetDefaultSelectedBkColor(DWORD dwColor, bool bShared)
{
    if(bShared)
    {
        if(m_ResInfo.m_dwDefaultSelectedBkColor == m_SharedResInfo.m_dwDefaultSelectedBkColor)
            m_ResInfo.m_dwDefaultSelectedBkColor = dwColor;
        m_SharedResInfo.m_dwDefaultSelectedBkColor = dwColor;
    }
    else
    {
        m_ResInfo.m_dwDefaultSelectedBkColor = dwColor;
    }
}

std::shared_ptr<TFontInfo> CPaintManagerUI::GetDefaultFontInfo()
{
    if(m_ResInfo.m_DefaultFontInfo->sFontName.empty())
    {
        if(m_SharedResInfo.m_DefaultFontInfo->tm.tmHeight == 0)
        {
            HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_SharedResInfo.m_DefaultFontInfo->hFont);
            ::GetTextMetrics(m_hDcPaint, &m_SharedResInfo.m_DefaultFontInfo->tm);
            ::SelectObject(m_hDcPaint, hOldFont);
        }
        return m_SharedResInfo.m_DefaultFontInfo;
    }
    else
    {
        if(m_ResInfo.m_DefaultFontInfo->tm.tmHeight == 0)
        {
            HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_ResInfo.m_DefaultFontInfo->hFont);
            ::GetTextMetrics(m_hDcPaint, &m_ResInfo.m_DefaultFontInfo->tm);
            ::SelectObject(m_hDcPaint, hOldFont);
        }
        return m_ResInfo.m_DefaultFontInfo;
    }
}

void CPaintManagerUI::SetDefaultFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared)
{
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    if(pStrFontName.length() > 0)
    {
        _tcsncpy(lf.lfFaceName, pStrFontName.data(), lengthof(lf.lfFaceName));
    }
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -GetDPIObj()->Scale(nSize);;
    if(bBold) lf.lfWeight += FW_BOLD;
    if(bUnderline) lf.lfUnderline = TRUE;
    if(bItalic) lf.lfItalic = TRUE;

    HFONT hFont = ::CreateFontIndirect(&lf);
    if(!hFont) return;

    if(bShared)
    {
        if(m_SharedResInfo.m_DefaultFontInfo)::DeleteObject(m_SharedResInfo.m_DefaultFontInfo->hFont);
        m_SharedResInfo.m_DefaultFontInfo->hFont = hFont;
        m_SharedResInfo.m_DefaultFontInfo->sFontName = lf.lfFaceName;
        m_SharedResInfo.m_DefaultFontInfo->iSize = nSize;
        m_SharedResInfo.m_DefaultFontInfo->bBold = bBold;
        m_SharedResInfo.m_DefaultFontInfo->bUnderline = bUnderline;
        m_SharedResInfo.m_DefaultFontInfo->bItalic = bItalic;
        ::ZeroMemory(&m_SharedResInfo.m_DefaultFontInfo->tm, sizeof(m_SharedResInfo.m_DefaultFontInfo->tm));
        if(m_hDcPaint)
        {
            HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
            ::GetTextMetrics(m_hDcPaint, &m_SharedResInfo.m_DefaultFontInfo->tm);
            ::SelectObject(m_hDcPaint, hOldFont);
        }
    }
    else
    {
        if(m_ResInfo.m_DefaultFontInfo)::DeleteObject(m_ResInfo.m_DefaultFontInfo->hFont);
        m_ResInfo.m_DefaultFontInfo->hFont = hFont;
        m_ResInfo.m_DefaultFontInfo->sFontName = lf.lfFaceName;
        m_ResInfo.m_DefaultFontInfo->iSize = nSize;
        m_ResInfo.m_DefaultFontInfo->bBold = bBold;
        m_ResInfo.m_DefaultFontInfo->bUnderline = bUnderline;
        m_ResInfo.m_DefaultFontInfo->bItalic = bItalic;
        ::ZeroMemory(&m_ResInfo.m_DefaultFontInfo->tm, sizeof(m_ResInfo.m_DefaultFontInfo->tm));
        if(m_hDcPaint)
        {
            HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
            ::GetTextMetrics(m_hDcPaint, &m_ResInfo.m_DefaultFontInfo->tm);
            ::SelectObject(m_hDcPaint, hOldFont);
        }
    }
}

DWORD CPaintManagerUI::GetCustomFontCount(bool bShared) const
{
    if(bShared)
        return m_SharedResInfo.m_CustomFonts.size();
    else
        return m_ResInfo.m_CustomFonts.size();
}

static int CALLBACK _enum_font_cb(CONST LOGFONTA *, CONST TEXTMETRICA *, DWORD, LPARAM lParam)
{
    return 42;
}

static faw::string_t _CheckFontExist(HDC _dc, faw::string_t _fonts)
{
    auto _v = FawTools::split(_fonts, _T(','));
    LOGFONT _lf = { 0 };
    for(auto _font : _v)
    {
        lstrcpy(_lf.lfFaceName, _font.data());
        if(EnumFontFamiliesEx(_dc, &_lf, (FONTENUMPROC) _enum_font_cb, 0, 0) == 42)
            return _font;
    }
    return _T("Microsoft YaHei");
}

HFONT CPaintManagerUI::AddFont(int id, faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared)
{
    pStrFontName = m_mapNameToFontCollection.contains(pStrFontName) ? pStrFontName : _CheckFontExist(m_hDcPaint, pStrFontName);
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    if(pStrFontName.length() > 0)
    {
        _tcsncpy(lf.lfFaceName, pStrFontName.data(), lengthof(lf.lfFaceName));
    }
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -GetDPIObj()->Scale(nSize);
    if(bBold) lf.lfWeight = FW_BOLD;
    if(bUnderline) lf.lfUnderline = TRUE;
    if(bItalic) lf.lfItalic = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    if(!hFont) return nullptr;

    auto pFontInfo = std::make_shared<TFontInfo>() ;
    if(!pFontInfo) return nullptr;
    pFontInfo->hFont = hFont;
    pFontInfo->sFontName = lf.lfFaceName;
    pFontInfo->iSize = nSize;
    pFontInfo->bBold = bBold;
    pFontInfo->bUnderline = bUnderline;
    pFontInfo->bItalic = bItalic;
    if(!m_hDcPaint) m_hDcPaint = GetDC(NULL);
    if(m_hDcPaint)
    {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    faw::string_t idBuffer = std::to_wstring(id);;
    if(bShared || m_bForceUseSharedRes)
    {
        auto it = m_SharedResInfo.m_CustomFonts.find(idBuffer);
        if (it != m_SharedResInfo.m_CustomFonts.end())
        {
            ::DeleteObject(it->second->hFont);
            m_SharedResInfo.m_CustomFonts.erase(it);
        }
        m_SharedResInfo.m_CustomFonts[idBuffer] = pFontInfo;
    }
    else
    {
        auto it = m_ResInfo.m_CustomFonts.find(idBuffer);
        if (it != m_ResInfo.m_CustomFonts.end())
        {
            ::DeleteObject(it->second->hFont);
            m_ResInfo.m_CustomFonts.erase(it);
        }
        m_ResInfo.m_CustomFonts[idBuffer] = pFontInfo;
    }

    return hFont;
}

void CPaintManagerUI::AddFontArray(faw::string_t pstrPath)
{
    bool bDeleteData = true;
    std::unique_ptr<BYTE[]> pData;
    DWORD dwSize = 0;
    faw::string_t sFile = CPaintManagerUI::GetResourcePath();

    // 先查找缓存
    faw::string_t key = pstrPath;
    FawTools::replace_self(key, _T("\\"), _T("/"));
    std::shared_ptr<CacheZipEntry> entry = CPaintManagerUI::GetResourceZipCacheEntry(key);
   
    // 否则查找路径
    while (!pData && !entry) {
        sFile += pstrPath;
        HANDLE hFile = ::CreateFile(sFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        ON_SCOPE_EXIT([&] { ::CloseHandle(hFile); });
        if (hFile == INVALID_HANDLE_VALUE) break;
        dwSize = ::GetFileSize(hFile, nullptr);
        if (dwSize == 0) break;

        DWORD dwRead = 0;
        pData = std::make_unique<BYTE[]>(dwSize);
        if (::ReadFile(hFile, pData.get(), dwSize, &dwRead, nullptr) && dwRead == dwSize) 
            break;
        pData.reset();
        break;
    }

    while (!pData && !entry) {
        if (CPaintManagerUI::GetResourceZip().empty()) break;
        sFile += CPaintManagerUI::GetResourceZip();
        HZIP hz = nullptr;
        faw::string_t sFilePwd = CPaintManagerUI::GetResourceZipPwd();
        std::string pwd = FawTools::T_to_gb18030(sFilePwd);
        hz = OpenZip(sFile.c_str(), pwd.c_str());
        ON_SCOPE_EXIT([&] { CloseZip(hz); });
        if (!hz) break;
        ZIPENTRY ze;
        int i = 0;
        faw::string_t key = pstrPath;
        FawTools::replace_self(key, _T("\\"), _T("/"));
        if (FindZipItem(hz, key.c_str(), true, &i, &ze) != 0) break;
        dwSize = ze.unc_size;
        if (dwSize == 0) break;
        pData = std::make_unique<BYTE[]>(dwSize);
        int res = UnzipItem(hz, i, pData.get(), dwSize);
        if (res != 0x00000000 && res != 0x00000600) 
            pData.reset();
        break;
    }

    while (!pData && !entry) {
        HANDLE hFile = ::CreateFile(pstrPath.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        ON_SCOPE_EXIT([&] { ::CloseHandle(hFile); });
        if (hFile == INVALID_HANDLE_VALUE) break;
        dwSize = ::GetFileSize(hFile, nullptr);
        if (dwSize == 0) break;

        DWORD dwRead = 0;
        pData = std::make_unique<BYTE[]>(dwSize);
        if (::ReadFile(hFile, pData.get(), dwSize, &dwRead, nullptr) && dwRead == dwSize) 
            break;    
        pData.reset();
        break;
    }

    if (!pData && !entry) return ;

    LPBYTE data_ptr = nullptr;
    DWORD data_size = 0;
    if (entry)
    {
        data_ptr = entry->dataPtr.get();
        data_size = entry->dataSize;
    }
    else
    {
        data_ptr = pData.get();
        data_size = dwSize;
    }

    DWORD nFonts;
    HANDLE hFont = ::AddFontMemResourceEx(data_ptr, data_size, nullptr, &nFonts);
    m_aFonts.push_back(hFont);

    // 添加字体
    std::shared_ptr<Gdiplus::PrivateFontCollection> pCollection = std::make_shared<Gdiplus::PrivateFontCollection>();
    pCollection->AddMemoryFont(data_ptr, data_size);
    int count = pCollection->GetFamilyCount();
    if (count == 0)return;

    int found = 0;
    Gdiplus::FontFamily pFontFamily;
    pCollection->GetFamilies(count, &pFontFamily, &found);
    if (found == 0) return;

    // 获取字体名称
    WCHAR  fontFamilyName[MAX_PATH] = { 0 };
    pFontFamily.GetFamilyName(fontFamilyName);
    auto FontFamilyName = FawTools::T_to_utf16(fontFamilyName);
    if (m_mapNameToFontCollection.contains(FontFamilyName))
        m_mapNameToFontCollection.erase(FontFamilyName);
    m_mapNameToFontCollection[FontFamilyName] = pCollection;
}


HFONT CPaintManagerUI::GetFont(int id)
{
    if(id < 0) return GetDefaultFontInfo()->hFont;

    faw::string_t idBuffer = std::to_wstring(id);

    auto it = m_ResInfo.m_CustomFonts.find(idBuffer);
    if (it != m_ResInfo.m_CustomFonts.end())
        return it->second->hFont;
    auto it2 = m_SharedResInfo.m_CustomFonts.find(idBuffer);
    if (it2 != m_SharedResInfo.m_CustomFonts.end())
        return it2->second->hFont;
    return GetDefaultFontInfo()->hFont;
}

HFONT CPaintManagerUI::GetFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    std::shared_ptr<TFontInfo> pFontInfo = nullptr;
    for(auto& [k,v]: m_ResInfo.m_CustomFonts)
    {
        if(!k.empty())
        {
            pFontInfo = v;
            if(pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize &&
                    pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic)
                return pFontInfo->hFont;
        }
    }
    for(auto& [k, v] : m_SharedResInfo.m_CustomFonts)
    {
        if(!k.empty())
        {
            pFontInfo = v;
            if(pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize &&
                    pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic)
                return pFontInfo->hFont;
        }
    }

    return nullptr;
}

Gdiplus::Font* CPaintManagerUI::GetResourceFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    auto& pCollection = m_mapNameToFontCollection[pStrFontName];
    if(pCollection)
    {
        Gdiplus::FontFamily fontFamily;
        int nNumFound = 0;
        pCollection->GetFamilies(1, &fontFamily, &nNumFound);

        if(nNumFound > 0)
        {
            int fontStyle = Gdiplus::FontStyle::FontStyleRegular;
            if(bBold && bItalic)
                fontStyle = Gdiplus::FontStyle::FontStyleBoldItalic;
            else if(bBold)
                fontStyle = Gdiplus::FontStyle::FontStyleBold;
            else if(bUnderline)
                fontStyle = Gdiplus::FontStyle::FontStyleUnderline;
            else if(bItalic)
                fontStyle = Gdiplus::FontStyle::FontStyleItalic;
            Gdiplus::Font* font = Gdiplus::Font(&fontFamily, nSize, fontStyle, Gdiplus::Unit::UnitPixel).Clone();
            return font;
        }
    }
    return Gdiplus::Font(GetDC(NULL), GetFont(-1)).Clone();
}

int CPaintManagerUI::GetFontIndex(HFONT hFont, bool bShared)
{
    std::shared_ptr<TFontInfo> pFontInfo = nullptr;
    if(bShared)
    {
        for (auto& [k, v] : m_SharedResInfo.m_CustomFonts)
        {
            if (!k.empty())
            {
                pFontInfo = v;
                if(pFontInfo && pFontInfo->hFont == hFont) 
                    return _ttoi(k.c_str());
            }
        }
    }
    else
    {
        for (auto& [k, v] : m_ResInfo.m_CustomFonts)
        {
            if (!k.empty())
            {
                pFontInfo = v;
                if(pFontInfo && pFontInfo->hFont == hFont) 
                    return _ttoi(k.c_str());
            }
        }
    }

    return -1;
}

int CPaintManagerUI::GetFontIndex(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bShared)
{
    std::shared_ptr<TFontInfo> pFontInfo = nullptr;
    if(bShared)
    {
        for (auto& [k, v] : m_SharedResInfo.m_CustomFonts)
        {
            if (!k.empty())
            {
                pFontInfo = v;
                if(pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize &&
                        pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic)
                    return _ttoi(k.c_str());
            }
        }
    }
    else
    {
        for (auto& [k, v] : m_ResInfo.m_CustomFonts)
        {
            if (!k.empty())
            {
                pFontInfo = v;
                if(pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize &&
                        pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic)
                    return _ttoi(k.c_str());
            }
        }
    }

    return -1;
}

void CPaintManagerUI::RemoveFont(HFONT hFont, bool bShared)
{
    std::shared_ptr<TFontInfo> pFontInfo = nullptr;
    if(bShared)
    {
        auto it = m_SharedResInfo.m_CustomFonts.begin();
        while (it != m_SharedResInfo.m_CustomFonts.end()) {
            if (!it->first.empty()) {
                auto& pFontInfo = it->second;
                if (pFontInfo && pFontInfo->hFont == hFont) {
                    ::DeleteObject(pFontInfo->hFont);
                    it = m_SharedResInfo.m_CustomFonts.erase(it); // Erase and get the next iterator
                    return;
                }
                else {
                    ++it; // Only increment if not erasing
                }
            }
            else {
                ++it; // Skip empty keys
            }
        }
    }
    else
    {
        auto it = m_ResInfo.m_CustomFonts.begin();
        while (it != m_ResInfo.m_CustomFonts.end()) {
            if (!it->first.empty()) {
                auto& pFontInfo = it->second;
                if (pFontInfo && pFontInfo->hFont == hFont) {
                    ::DeleteObject(pFontInfo->hFont);
                    it = m_ResInfo.m_CustomFonts.erase(it); // Erase and get the next iterator
                    return;
                }
                else {
                    ++it; // Only increment if not erasing
                }
            }
            else {
                ++it; // Skip empty keys
            }
        }
    }
}

void CPaintManagerUI::RemoveFont(int id, bool bShared)
{
    faw::string_t idBuffer = std::to_wstring(id);
    std::shared_ptr<TFontInfo> pFontInfo = nullptr;
    if(bShared)
    {
        auto it = m_SharedResInfo.m_CustomFonts.find(idBuffer);
        if(it!= m_SharedResInfo.m_CustomFonts.end())
        {
            ::DeleteObject(it->second->hFont);
            m_SharedResInfo.m_CustomFonts.erase(it);
        }
    }
    else
    {
        auto it = m_ResInfo.m_CustomFonts.find(idBuffer);
        if (it != m_ResInfo.m_CustomFonts.end())
        {
            ::DeleteObject(it->second->hFont);
            m_ResInfo.m_CustomFonts.erase(it);
        }
    }
}

void CPaintManagerUI::RemoveAllFonts(bool bShared)
{
    std::shared_ptr<TFontInfo> pFontInfo;
    if(bShared)
    {
        for (auto& [k, pFontInfo] : m_SharedResInfo.m_CustomFonts)
        {
            if (!k.empty() && pFontInfo)
                ::DeleteObject(pFontInfo->hFont);
        }
        m_SharedResInfo.m_CustomFonts.clear();
    }
    else
    {
        for (auto& [k, pFontInfo] : m_ResInfo.m_CustomFonts)
        {
            if (!k.empty()&& pFontInfo)
                 ::DeleteObject(pFontInfo->hFont);          
        }
        m_ResInfo.m_CustomFonts.clear();
    }
}


std::shared_ptr<TFontInfo> CPaintManagerUI::GetFontInfo(int id)
{
    if(id < 0) return GetDefaultFontInfo();

    faw::string_t idBuffer = std::to_wstring(id);

    auto it = m_ResInfo.m_CustomFonts.find(idBuffer);
    if (it != m_ResInfo.m_CustomFonts.end())
        return it->second;
    auto it2 = m_SharedResInfo.m_CustomFonts.find(idBuffer);
    if (it2 != m_SharedResInfo.m_CustomFonts.end())
        return it2->second;
    std::shared_ptr<TFontInfo> pFontInfo = GetDefaultFontInfo();
    if(pFontInfo->tm.tmHeight == 0)
    {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    return pFontInfo;
}

std::shared_ptr<TFontInfo> CPaintManagerUI::GetFontInfo(HFONT hFont)
{
    std::shared_ptr<TFontInfo> pFontInfo = nullptr;
    for (auto& [k, v] : m_ResInfo.m_CustomFonts)
    {
        if (!k.empty() && v)
        {
            pFontInfo = v;
            if(pFontInfo->hFont == hFont) break;
        }
    }
    if(!pFontInfo)
    {
        for (auto& [k, v] : m_SharedResInfo.m_CustomFonts)
        {
            if (!k.empty() && v)
            {
                pFontInfo = v;
                if(pFontInfo->hFont == hFont) break;
            }
        }
    }
    if(!pFontInfo) pFontInfo = GetDefaultFontInfo();
    if(pFontInfo->tm.tmHeight == 0)
    {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    return pFontInfo;
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::GetImage(faw::string_t bitmap)
{
    std::shared_ptr<TImageInfo> data = m_ResInfo.m_ImageHash[bitmap];
    if(!data) data = m_SharedResInfo.m_ImageHash[bitmap];
    return data;
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::GetImageEx(faw::string_t bitmap, faw::string_t type, DWORD mask, bool bUseHSL, HINSTANCE instance,const std::shared_ptr<TDrawInfo> pDrawInfo)
{
    std::shared_ptr<TImageInfo> data = GetImage(bitmap);
    if(!data)
    {
        if(AddImage(bitmap, type, mask, bUseHSL, false, instance, pDrawInfo))
        {
            if(m_bForceUseSharedRes) data = m_SharedResInfo.m_ImageHash[bitmap];
            else data = m_ResInfo.m_ImageHash[bitmap];
        }
    }
    return data;
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::AddImage(faw::string_t bitmap, faw::string_t type, DWORD mask, bool bUseHSL, bool bShared, HINSTANCE instance, const std::shared_ptr<TDrawInfo> pDrawInfo)
{
    if(bitmap.empty()) return nullptr;

    std::shared_ptr<TImageInfo> data = nullptr;
    if(type.length() > 0)
    {
        if(isdigit(bitmap[0]))
        {
            int iIndex = _ttoi(bitmap.data());
            data = CRenderEngine::LoadImage(iIndex, type.data(), mask, instance, pDrawInfo);
        }
    }
    else
    {
        data = CRenderEngine::LoadImage(bitmap, _T(""), mask, instance, pDrawInfo);
    }

    if(!data)
    {
        return nullptr;
    }
    data->bUseHSL = bUseHSL;
    if(!type.empty()) data->sResType = type;
    data->dwMask = mask;
    if(data->bUseHSL)
    {
        data->pSrcBits = std::make_shared<BYTE[]>(data->nX * data->nY * 4); 
        ::CopyMemory(data->pSrcBits.get(), data->pBits, data->nX * data->nY * 4);
    }
    else data->pSrcBits = nullptr;
    if(m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);
    if(data)
    {
        if(bShared || m_bForceUseSharedRes)
        {
            auto it = m_SharedResInfo.m_ImageHash.find(bitmap);
            if(it!= m_SharedResInfo.m_ImageHash.end())
            {
                CRenderEngine::FreeImage(it->second);
                m_SharedResInfo.m_ImageHash.erase(it);
            }
            m_SharedResInfo.m_ImageHash[bitmap] = data;
        }
        else
        {
            auto it = m_ResInfo.m_ImageHash.find(bitmap);
            if (it != m_ResInfo.m_ImageHash.end())
            {
                CRenderEngine::FreeImage(it->second);
                m_ResInfo.m_ImageHash.erase(it);
            }
            m_ResInfo.m_ImageHash[bitmap] = data;
        }
    }

    return data;
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::AddImage(faw::string_t bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared)
{
    // 因无法确定外部HBITMAP格式，不能使用hsl调整
    if(bitmap.empty()) return nullptr;
    if(!hBitmap || iWidth <= 0 || iHeight <= 0) return nullptr;

    std::shared_ptr<TImageInfo> data = std::make_shared<TImageInfo>() ;
    data->pBits = nullptr;
    data->pSrcBits = nullptr;
    data->hBitmap = hBitmap;
    data->pBits = nullptr;
    data->nX = iWidth;
    data->nY = iHeight;
    data->bAlpha = bAlpha;
    data->bUseHSL = false;
    data->pSrcBits = nullptr;
    data->dwMask = 0;

    if (bShared || m_bForceUseSharedRes)
    {
        auto it = m_SharedResInfo.m_ImageHash.find(bitmap);
        if (it != m_SharedResInfo.m_ImageHash.end())
        {
            CRenderEngine::FreeImage(it->second);
            m_SharedResInfo.m_ImageHash.erase(it);
        }
        m_SharedResInfo.m_ImageHash[bitmap] = data;
    }
    else
    {
        auto it = m_ResInfo.m_ImageHash.find(bitmap);
        if (it != m_ResInfo.m_ImageHash.end())
        {
            CRenderEngine::FreeImage(it->second);
            m_ResInfo.m_ImageHash.erase(it);
        }
        m_ResInfo.m_ImageHash[bitmap] = data;
    }

    return data;
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::AddImage(faw::string_t bitmap, HBITMAP *phBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared)
{
    // 因无法确定外部HBITMAP格式，不能使用hsl调整
    if(bitmap.empty()) return nullptr;
    if(!phBitmap || !*phBitmap || iWidth <= 0 || iHeight <= 0) return nullptr;

    std::shared_ptr<TImageInfo> data = std::make_shared<TImageInfo>() ;
    data->hBitmap = nullptr;
    data->pSrcBits = nullptr;
    data->phBitmap = phBitmap;
    data->pBits = nullptr;
    data->nX = iWidth;
    data->nY = iHeight;
    data->bAlpha = bAlpha;
    data->bUseHSL = false;
    data->pSrcBits = nullptr;
    data->dwMask = 0;
    if (bShared || m_bForceUseSharedRes)
    {
        auto it = m_SharedResInfo.m_ImageHash.find(bitmap);
        if (it != m_SharedResInfo.m_ImageHash.end())
        {
            CRenderEngine::FreeImage(it->second);
            m_SharedResInfo.m_ImageHash.erase(it);
        }
        m_SharedResInfo.m_ImageHash[bitmap] = data;
    }
    else
    {
        auto it = m_ResInfo.m_ImageHash.find(bitmap);
        if (it != m_ResInfo.m_ImageHash.end())
        {
            CRenderEngine::FreeImage(it->second);
            m_ResInfo.m_ImageHash.erase(it);
        }
        m_ResInfo.m_ImageHash[bitmap] = data;
    }

    return data;
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::AddImage(faw::string_t bitmap, TDataArgs dataArgs, DWORD mask, bool bUseHSL, bool bShared)
{
    if(bitmap.empty()) return nullptr;
    std::shared_ptr<TImageInfo> data = nullptr;
    data = CRenderEngine::LoadMemoryImage(dataArgs, mask);

    if(!data) return nullptr;

    data->bUseHSL = bUseHSL;
    data->dwMask = mask;
    if(data->bUseHSL)
    {
        data->pSrcBits = std::make_shared<BYTE[]>(data->nX * data->nY * 4);
        ::CopyMemory(data->pSrcBits.get(), data->pBits, data->nX * data->nY * 4);
    }
    else data->pSrcBits = nullptr;
    if(m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);
    if(data)
    {
        if (bShared || m_bForceUseSharedRes)
        {
            auto it = m_SharedResInfo.m_ImageHash.find(bitmap);
            if (it != m_SharedResInfo.m_ImageHash.end())
            {
                CRenderEngine::FreeImage(it->second);
                m_SharedResInfo.m_ImageHash.erase(it);
            }
            m_SharedResInfo.m_ImageHash[bitmap] = data;
        }
        else
        {
            auto it = m_ResInfo.m_ImageHash.find(bitmap);
            if (it != m_ResInfo.m_ImageHash.end())
            {
                CRenderEngine::FreeImage(it->second);
                m_ResInfo.m_ImageHash.erase(it);
            }
            m_ResInfo.m_ImageHash[bitmap] = data;
        }
    }

    return data;
}

void CPaintManagerUI::RemoveImage(faw::string_t bitmap, bool bShared)
{
    std::shared_ptr<TImageInfo> data = nullptr;
    if(bShared)
    {
        auto it = m_SharedResInfo.m_ImageHash.find(bitmap);
        if (it != m_SharedResInfo.m_ImageHash.end())
        {
            CRenderEngine::FreeImage(it->second);
            m_SharedResInfo.m_ImageHash.erase(it);
        }
    }
    else
    {
        auto it = m_ResInfo.m_ImageHash.find(bitmap);
        if (it != m_ResInfo.m_ImageHash.end())
        {
            CRenderEngine::FreeImage(it->second);
            m_ResInfo.m_ImageHash.erase(it);
        }
    }
}

void CPaintManagerUI::RemoveAllImages(bool bShared)
{
    if(bShared)
    {
        for(auto& [k,v]: m_SharedResInfo.m_ImageHash)
        {
            if(!k.empty()&& v)
                CRenderEngine::FreeImage(v);
        }
        m_SharedResInfo.m_ImageHash.clear();
    }
    else
    {
        for (auto& [k, v] : m_ResInfo.m_ImageHash)
        {
            if (!k.empty() && v)
                CRenderEngine::FreeImage(v);
        }
        m_ResInfo.m_ImageHash.clear();
    }
}

void CPaintManagerUI::AdjustSharedImagesHSL()
{
    for (auto& [k, v] : m_SharedResInfo.m_ImageHash)
    {
        if (!k.empty() && v&& v->bUseHSL)
            CRenderEngine::AdjustImage(m_bUseHSL, v, m_H, m_S, m_L);        
    }
}

void CPaintManagerUI::AdjustImagesHSL()
{
    for (auto& [k, v] : m_ResInfo.m_ImageHash)
    {
        if (!k.empty() && v && v->bUseHSL)
            CRenderEngine::AdjustImage(m_bUseHSL, v, m_H, m_S, m_L);
    }
    std::shared_ptr<TImageInfo> data;
    Invalidate();
}

void CPaintManagerUI::PostAsyncNotify()
{
    if(!m_bAsyncNotifyPosted)
    {
        ::PostMessage(m_hWndPaint, WM_APP + 1, 0, 0L);
        m_bAsyncNotifyPosted = true;
    }
}
void CPaintManagerUI::ReloadSharedImages()
{
    std::shared_ptr<TImageInfo> pNewData = nullptr;
    for (auto& [k, v] : m_SharedResInfo.m_ImageHash)
    {
        if (!k.empty() && v)
        {
            auto& bitmap = k;
            auto& data = v;
            if(!data->sResType.empty()&& isdigit(bitmap[0]))
            {
                int iIndex = _ttoi(bitmap.c_str());
                pNewData = CRenderEngine::LoadImage(iIndex, data->sResType.c_str(), data->dwMask);      
            }
            else
            {
                pNewData = CRenderEngine::LoadImage(bitmap, _T(""), data->dwMask);
            }
            if (!pNewData)
            {
                //std::locale::global(std::locale(""));
                std::wcout << L"Reload Shared Image not found: " + bitmap << std::endl;
                continue;
            }

            CRenderEngine::FreeImage(data, false);
            data->hBitmap = pNewData->hBitmap;
            data->phBitmap = pNewData->phBitmap;
            data->pBits = pNewData->pBits;
            data->nX = pNewData->nX;
            data->nY = pNewData->nY;
            data->bAlpha = pNewData->bAlpha;
            data->pSrcBits = nullptr;
            if(data->bUseHSL)
            {
                data->pSrcBits = std::make_shared<BYTE[]>(data->nX * data->nY * 4);
                ::CopyMemory(data->pSrcBits.get(), data->pBits, data->nX * data->nY * 4);
            }
            else data->pSrcBits = nullptr;
            if(m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);
        }
    }
}

void CPaintManagerUI::ReloadImages()
{
    RemoveAllDrawInfos();

    std::shared_ptr<TImageInfo> pNewData = nullptr;
    for (auto& [k, v] : m_ResInfo.m_ImageHash)
    {
        if (!k.empty() && v)
        {
            auto& bitmap = k;
            auto& data = v;
            if (!data->sResType.empty() && isdigit(bitmap[0]))
            {
                int iIndex = _ttoi(bitmap.c_str());
                pNewData = CRenderEngine::LoadImage(iIndex, data->sResType.c_str(), data->dwMask);
            }
            else
            {
                pNewData = CRenderEngine::LoadImage(bitmap, _T(""), data->dwMask);
            }

            if(!pNewData)
            {
                //std::locale::global(std::locale(""));
                std::wcout << L"Reload Image not found : " + bitmap << std::endl;
                continue;
            }
           CRenderEngine::FreeImage(data, false);
            data->hBitmap = pNewData->hBitmap;
            data->phBitmap = pNewData->phBitmap;
            data->pBits = pNewData->pBits;
            data->nX = pNewData->nX;
            data->nY = pNewData->nY;
            data->bAlpha = pNewData->bAlpha;
            data->pSrcBits = nullptr;
            if(data->bUseHSL)
            {
                data->pSrcBits = std::make_shared<BYTE[]>(data->nX * data->nY * 4);
                ::CopyMemory(data->pSrcBits.get(), data->pBits, data->nX * data->nY * 4);
            }
            else data->pSrcBits = nullptr;
            if(m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);
        }
    }

    if(m_pRoot) m_pRoot->Invalidate();
}

const std::shared_ptr<TDrawInfo> CPaintManagerUI::GetDrawInfo(faw::string_t pStrImage, faw::string_t pStrModify)
{
    faw::string_t sStrImage = pStrImage;
    faw::string_t sStrModify = pStrModify;
    faw::string_t sKey = sStrImage + sStrModify;
    std::shared_ptr<TDrawInfo> pDrawInfo = nullptr;
    auto it = m_ResInfo.m_DrawInfoHash.find(sKey);
    if( it == m_ResInfo.m_DrawInfoHash.end() && !sKey.empty())
    {
        pDrawInfo = std::make_shared<TDrawInfo>();
        pDrawInfo->Parse(pStrImage, pStrModify, this);
        m_ResInfo.m_DrawInfoHash[sKey] = pDrawInfo;
        return pDrawInfo;
    }
    return it->second;
}

void CPaintManagerUI::RemoveDrawInfo(faw::string_t pStrImage, faw::string_t pStrModify)
{
    faw::string_t sStrImage = pStrImage;
    faw::string_t sStrModify = pStrModify;
    faw::string_t sKey = sStrImage + sStrModify;
    auto it = m_ResInfo.m_DrawInfoHash.find(sKey);
    if (it == m_ResInfo.m_DrawInfoHash.end()){
        m_ResInfo.m_DrawInfoHash.erase(it);
    }
}

void CPaintManagerUI::RemoveAllDrawInfos()
{
    std::shared_ptr<TDrawInfo> pDrawInfo = nullptr;
    m_ResInfo.m_DrawInfoHash.clear();
}

void CPaintManagerUI::AddGlobalAttributeList(faw::string_t GlobalStyleFile)
{
    CDialogBuilder builder;
    CPaintManagerUI pm;
    builder.Create(GlobalStyleFile, L"", nullptr, &pm);
}

void CPaintManagerUI::AddDefaultAttributeList(faw::string_t pStrControlName, faw::string_t pStrControlAttrList, bool bShared)
{
    if(bShared || m_bForceUseSharedRes)
    {
        auto pDefaultAttr = std::make_shared<faw::string_t>(pStrControlAttrList);
        m_SharedResInfo.m_AttrHash[pStrControlName] = pDefaultAttr;
    }
    else
    {
        auto pDefaultAttr = std::make_shared<faw::string_t>(pStrControlAttrList);
        m_ResInfo.m_AttrHash[pStrControlName] = pDefaultAttr;
    }
}

faw::string_t CPaintManagerUI::GetDefaultAttributeList(faw::string_t pStrControlName) const
{
    auto it = m_ResInfo.m_AttrHash.find(pStrControlName);
    if (it != m_ResInfo.m_AttrHash.end())
        return *(it->second);
    auto it2 = m_SharedResInfo.m_AttrHash.find(pStrControlName);
    if (it2 != m_SharedResInfo.m_AttrHash.end())
        return *(it2->second);
    return _T("");
}

bool CPaintManagerUI::RemoveDefaultAttributeList(faw::string_t pStrControlName, bool bShared)
{
    if(bShared)
    {
        auto it = m_SharedResInfo.m_AttrHash.find(pStrControlName);
        if (it != m_SharedResInfo.m_AttrHash.end())
        {
            m_SharedResInfo.m_AttrHash.erase(it);
            return true;
        }
    }
    else{
        auto it = m_ResInfo.m_AttrHash.find(pStrControlName);
        if (it != m_ResInfo.m_AttrHash.end())
        {
            m_ResInfo.m_AttrHash.erase(it);
            return true;
        }
    }
    return false;
}

void CPaintManagerUI::RemoveAllDefaultAttributeList(bool bShared)
{
    if(bShared)
    {
        m_SharedResInfo.m_AttrHash.clear();
    }
    else
    {
        m_ResInfo.m_AttrHash.clear();
    }
}

void CPaintManagerUI::AddWindowCustomAttribute(faw::string_t pstrName, faw::string_t pstrAttr)
{
    if(pstrName.empty() || pstrAttr.empty()) return;
    m_mWindowCustomAttrHash[pstrName] = std::make_shared< faw::string_t>(pstrAttr);
}

faw::string_t CPaintManagerUI::GetWindowCustomAttribute(faw::string_t pstrName) const
{
    if(pstrName.empty()) return _T("");
    auto it = m_mWindowCustomAttrHash.find(pstrName);
    if (it != m_mWindowCustomAttrHash.end())
        return *(it->second);
    return _T("");
}

bool CPaintManagerUI::RemoveWindowCustomAttribute(faw::string_t pstrName)
{
    if(pstrName.empty()) return false;
    auto it = m_mWindowCustomAttrHash.find(pstrName);
    if (it != m_mWindowCustomAttrHash.end())
        m_ResInfo.m_AttrHash.erase(it);
    return true;
}

void CPaintManagerUI::RemoveAllWindowCustomAttribute()
{
    m_mWindowCustomAttrHash.clear();
}

std::shared_ptr<CControlUI> CPaintManagerUI::GetRoot() const
{
    ASSERT(m_pRoot);
    return m_pRoot;
}

std::shared_ptr<CControlUI> CPaintManagerUI::FindControl(POINT pt) const
{
    ASSERT(m_pRoot);
    return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
}

std::shared_ptr<CControlUI> CPaintManagerUI::FindControl(faw::string_t pstrName) const
{
    ASSERT(m_pRoot);
    auto it = m_mNameHash.find(pstrName);
    if(it != m_mNameHash.end())
    {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<CControlUI> CPaintManagerUI::FindSubControlByPoint(std::shared_ptr<CControlUI> pParent, POINT pt) const
{
    if(!pParent) pParent = GetRoot();
    ASSERT(pParent);
    return pParent->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
}

std::shared_ptr<CControlUI> CPaintManagerUI::FindSubControlByName(std::shared_ptr<CControlUI> pParent, faw::string_t pstrName) const
{
    if(!pParent) pParent = GetRoot();
    ASSERT(pParent);
    return pParent->FindControl(__FindControlFromName, (LPVOID) pstrName.data(), UIFIND_ALL);
}

std::shared_ptr<CControlUI> CPaintManagerUI::FindSubControlByClass(std::shared_ptr<CControlUI> pParent, faw::string_t pstrClass, int iIndex)
{
    if(!pParent) pParent = GetRoot();
    ASSERT(pParent);
    return pParent->FindControl(__FindControlFromClass, (LPVOID) pstrClass.data(), UIFIND_ALL);
}

std::vector<std::shared_ptr<CControlUI>>& CPaintManagerUI::FindSubControlsByClass(std::shared_ptr<CControlUI> pParent, faw::string_t pstrClass)
{
    if(!pParent) pParent = GetRoot();
    ASSERT(pParent);
    m_aFoundControls.clear();
    pParent->FindControl(__FindControlsFromClass, (LPVOID) pstrClass.data(), UIFIND_ALL);
    return m_aFoundControls;
}

std::vector<std::shared_ptr<CControlUI>>& CPaintManagerUI::GetFoundControls()
{
    return m_aFoundControls;
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromNameHash(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(pData);
    const faw::string_t sName = pThis->GetName();
    if(sName.empty()) return nullptr;
    // Add this control to the hash list
    pManager->m_mNameHash[sName] = pThis;
    return nullptr; // Attempt to add all controls
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromCount(std::shared_ptr<CControlUI> /*pThis*/, LPVOID pData)
{
    int* pnCount = static_cast<int*>(pData);
    (*pnCount)++;
    return nullptr;  // Count all controls
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromPoint(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    LPPOINT pPoint = static_cast<LPPOINT>(pData);
    return ::PtInRect(&pThis->GetPos(), *pPoint) ? pThis : nullptr;
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromTab(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    FINDTABINFO* pInfo = static_cast<FINDTABINFO*>(pData);
    if(pInfo->pFocus == pThis)
    {
        if(pInfo->bForward) pInfo->bNextIsIt = true;
        return pInfo->bForward ? nullptr : pInfo->pLast;
    }
    if((pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0) return nullptr;
    pInfo->pLast = pThis;
    if(pInfo->bNextIsIt) return pThis;
    if(!pInfo->pFocus) return pThis;
    return nullptr;  // Examine all controls
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromShortcut(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    if(!pThis->IsVisible()) return nullptr;
    FINDSHORTCUT* pFS = static_cast<FINDSHORTCUT*>(pData);
    if(pFS->ch == toupper(pThis->GetShortcut())) pFS->bPickNext = true;
    if(pThis->GetClass().find(_T("LabelUI")) != faw::string_t::npos) return nullptr;       // Labels never get focus!
    return pFS->bPickNext ? pThis : nullptr;
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromName(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    faw::string_t pstrName = static_cast<LPCTSTR>(pData);
    const faw::string_t sName = pThis->GetName();
    if(sName.empty()) return nullptr;
    return (sName == pstrName ? pThis : nullptr);
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlFromClass(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    faw::string_t pstrType = static_cast<LPCTSTR>(pData);
    faw::string_t pType = pThis->GetClass();
    std::vector<std::shared_ptr<CControlUI>>& pFoundControls = pThis->GetManager()->GetFoundControls();
    if(pstrType == _T("*") || pstrType == pType)
    {
        pFoundControls.push_back(pThis);
        return pThis;
    }
    return nullptr;
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlsFromClass(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    faw::string_t pstrType = static_cast<LPCTSTR>(pData);
    faw::string_t pType = pThis->GetClass();
    if(pstrType == _T("*") || pstrType == pType)
        pThis->GetManager()->GetFoundControls().push_back(pThis);
    return nullptr;
}

std::shared_ptr<CControlUI> CALLBACK CPaintManagerUI::__FindControlsFromUpdate(std::shared_ptr<CControlUI> pThis, LPVOID pData)
{
    if(pThis->IsUpdateNeeded())
    {
        pThis->GetManager()->GetFoundControls().push_back(pThis);
        return pThis;
    }
    return nullptr;
}

bool CPaintManagerUI::TranslateAccelerator(LPMSG pMsg)
{
    for(int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
    {
        LRESULT lResult = static_cast<ITranslateAccelerator *>(m_aTranslateAccelerator[i])->TranslateAccelerator(pMsg);
        if(lResult == S_OK) return true;
    }
    return false;
}

bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
{
    // Pretranslate Message takes care of system-wide messages, such as
    // tabbing and shortcut key-combos. We'll look for all messages for
    // each window and any child control attached.
    UINT uStyle = GetWindowStyle(pMsg->hwnd);
    UINT uChildRes = uStyle & WS_CHILD;
    std::optional<LRESULT> lRes = 0;
    if(uChildRes != 0)
    {
        HWND hWndParent = ::GetParent(pMsg->hwnd);

        for(int i = 0; i < m_aPreMessages.GetSize(); i++)
        {
            CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
            HWND hTempParent = hWndParent;
            while(hTempParent)
            {
                if(pMsg->hwnd == pT->GetPaintWindow() || hTempParent == pT->GetPaintWindow())
                {
                    if(pT->TranslateAccelerator(pMsg))
                        return true;

                    lRes = pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam);
                }
                hTempParent = GetParent(hTempParent);
            }
        }
    }
    else
    {
        for(int i = 0; i < m_aPreMessages.GetSize(); i++)
        {
            CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
            if(pMsg->hwnd == pT->GetPaintWindow())
            {
                if(pT->TranslateAccelerator(pMsg))
                    return true;

                lRes = pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam);
                if(lRes.has_value())
                    return true;

                return false;
            }
        }
    }
    return false;
}

bool CPaintManagerUI::AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator)
{
    ASSERT(m_aTranslateAccelerator.Find(pTranslateAccelerator) < 0);
    return m_aTranslateAccelerator.Add(pTranslateAccelerator);
}

bool CPaintManagerUI::RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator)
{
    for(int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
    {
        if(static_cast<ITranslateAccelerator *>(m_aTranslateAccelerator[i]) == pTranslateAccelerator)
        {
            return m_aTranslateAccelerator.Remove(i);
        }
    }
    return false;
}

void CPaintManagerUI::UsedVirtualWnd(bool bUsed)
{
    m_bUsedVirtualWnd = bUsed;
}

// 样式管理
void CPaintManagerUI::AddStyle(faw::string_t pName, faw::string_t pDeclarationList, bool bShared)
{
    if(bShared || m_bForceUseSharedRes)
    {
        m_SharedResInfo.m_StyleHash[pName] = std::make_shared<faw::string_t>(pDeclarationList);
    }
    else
    {
        m_ResInfo.m_StyleHash[pName] = std::make_shared<faw::string_t>(pDeclarationList);
    }
}

faw::string_t CPaintManagerUI::GetStyle(faw::string_t pName) const
{
    auto it = m_ResInfo.m_StyleHash.find(pName);
    if (it != m_ResInfo.m_StyleHash.end())
        return *(it->second);
    auto it2 = m_SharedResInfo.m_StyleHash.find(pName);
    if (it2 != m_SharedResInfo.m_StyleHash.end())
        return *(it2->second);
    return _T("");
}

BOOL CPaintManagerUI::RemoveStyle(faw::string_t pName, bool bShared)
{
    if(bShared)
    {
        auto it = m_SharedResInfo.m_StyleHash.find(pName);
        if (it != m_SharedResInfo.m_StyleHash.end())
            m_SharedResInfo.m_StyleHash.erase(it);
    }
    else
    {
        auto it = m_ResInfo.m_StyleHash.find(pName);
        if (it != m_ResInfo.m_StyleHash.end())
            m_ResInfo.m_StyleHash.erase(it);
    }
    return true;
}

const MAP_STR_ATTRIBUTE_INFO& CPaintManagerUI::GetStyles(bool bShared) const
{
    if(bShared) return m_SharedResInfo.m_StyleHash;
    else return m_ResInfo.m_StyleHash;
}

void CPaintManagerUI::RemoveAllStyle(bool bShared)
{
    if(bShared)
    {
        m_SharedResInfo.m_StyleHash.clear();
    }
    else
    {
        m_ResInfo.m_StyleHash.clear();
    }
}

const std::shared_ptr<TImageInfo> CPaintManagerUI::GetImageString(faw::string_t pStrImage, faw::string_t pStrModify)
{
    faw::string_t sImageName = pStrImage;
    faw::string_t sImageResType = _T("");
    DWORD dwMask = 0;
    for(size_t i = 0; i < 2; ++i)
    {
        std::map<faw::string_t, faw::string_t> m = FawTools::parse_keyvalue_pairs(i == 0 ? pStrImage : pStrModify);
        for(auto[str_key, str_value] : m)
        {
            if(str_key == _T("file") || str_key == _T("res"))
            {
                sImageName = str_value;
            }
            else if(str_key == _T("restype"))
            {
                sImageResType = str_value;
            }
            else if(str_key == _T("mask"))
            {
                dwMask = static_cast<decltype(dwMask)> (FawTools::parse_hex(str_value));
            }
        }
    }
    return GetImageEx(sImageName, sImageResType, dwMask);
}

bool CPaintManagerUI::InitDragDrop()
{
    AddRef();

    HRESULT hr = RegisterDragDrop(m_hWndPaint, this);
    if(FAILED(hr))
    {
        DWORD dwError = GetLastError();
        // 可能需要将 hr 转换为更具体的错误信息
        // 使用 FormatMessage 函数可以获得更详细的错误描述
        TCHAR szError[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, dwError,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      szError,
                      sizeof(szError) / sizeof(TCHAR),
                      NULL);
        // 打印错误信息
        _tprintf(_T("RegisterDragDrop failed with error %d: %s\n"), dwError, szError);
        return false;
    }
    else
        Release();    //i decided to AddRef explicitly after new

    FORMATETC ftetc = { 0 };
    ftetc.cfFormat = CF_BITMAP;
    ftetc.dwAspect = DVASPECT_CONTENT;
    ftetc.lindex = -1;
    ftetc.tymed = TYMED_GDI;
    AddSuportedFormat(ftetc);
    ftetc.cfFormat = CF_DIB;
    ftetc.tymed = TYMED_HGLOBAL;
    AddSuportedFormat(ftetc);
    ftetc.cfFormat = CF_HDROP;
    ftetc.tymed = TYMED_HGLOBAL;
    AddSuportedFormat(ftetc);
    ftetc.cfFormat = CF_ENHMETAFILE;
    ftetc.tymed = TYMED_ENHMF;
    AddSuportedFormat(ftetc);
    return true;
}
static WORD DIBNumColors(void* pv)
{
    int bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;
    lpbi = ((LPBITMAPINFOHEADER) pv);
    lpbc = ((LPBITMAPCOREHEADER) pv);
    /*  With the BITMAPINFO format headers, the size of the palette
    *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
    *  is dependent on the bits per pixel ( = 2 raised to the power of
    *  bits/pixel).
    */
    if(lpbi->biSize != sizeof(BITMAPCOREHEADER))
    {
        if(lpbi->biClrUsed != 0)
            return (WORD) lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;
    switch(bits)
    {
    case 1:
        return 2;
    case 4:
        return 16;
    case 8:
        return 256;
    default:
        /* A 24 bitcount DIB has no color table */
        return 0;
    }
}
//code taken from SEEDIB MSDN sample
static WORD ColorTableSize(LPVOID lpv)
{
    LPBITMAPINFOHEADER lpbih = (LPBITMAPINFOHEADER) lpv;

    if(lpbih->biSize != sizeof(BITMAPCOREHEADER))
    {
        if(((LPBITMAPINFOHEADER)(lpbih))->biCompression == BI_BITFIELDS)
            /* Remember that 16/32bpp dibs can still have a color table */
            return (sizeof(DWORD) * 3) + (DIBNumColors(lpbih) * sizeof(RGBQUAD));
        else
            return (WORD)(DIBNumColors(lpbih) * sizeof(RGBQUAD));
    }
    else
        return (WORD)(DIBNumColors(lpbih) * sizeof(RGBTRIPLE));
}

bool CPaintManagerUI::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
{
    POINT ptMouse = { 0 };
    GetCursorPos(&ptMouse);
    ::SendMessage(m_hTargetWnd, WM_LBUTTONUP, 0, MAKELPARAM(ptMouse.x, ptMouse.y));

    if(pFmtEtc->cfFormat == CF_DIB && medium.tymed == TYMED_HGLOBAL)
    {
        if(medium.hGlobal)
        {
            LPBITMAPINFOHEADER  lpbi = (BITMAPINFOHEADER*) GlobalLock(medium.hGlobal);
            if(lpbi)
            {
                HBITMAP hbm = NULL;
                HDC hdc = GetDC(nullptr);
                if(hdc)
                {
                    int i = ((BITMAPFILEHEADER *) lpbi)->bfOffBits;
                    hbm = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER) lpbi,
                                         (LONG) CBM_INIT,
                                         (LPSTR) lpbi + lpbi->biSize + ColorTableSize(lpbi),
                                         (LPBITMAPINFO) lpbi, DIB_RGB_COLORS);

                    ::ReleaseDC(nullptr, hdc);
                }
                GlobalUnlock(medium.hGlobal);
                if(hbm != NULL)
                    hbm = (HBITMAP) SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbm);
                if(hbm != NULL)
                    DeleteObject(hbm);
                return true; //release the medium
            }
        }
    }
    if(pFmtEtc->cfFormat == CF_BITMAP && medium.tymed == TYMED_GDI)
    {
        if(medium.hBitmap)
        {
            HBITMAP hBmp = (HBITMAP) SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) medium.hBitmap);
            if(hBmp)
                DeleteObject(hBmp);
            return false; //don't free the bitmap
        }
    }
    if(pFmtEtc->cfFormat == CF_ENHMETAFILE && medium.tymed == TYMED_ENHMF)
    {
        ENHMETAHEADER emh;
        GetEnhMetaFileHeader(medium.hEnhMetaFile, sizeof(ENHMETAHEADER), &emh);
        RECT rc = { 0 };//={0,0,EnhMetaHdr.rclBounds.right-EnhMetaHdr.rclBounds.left, EnhMetaHdr.rclBounds.bottom-EnhMetaHdr.rclBounds.top};
        HDC hDC = GetDC(m_hTargetWnd);
        //start code: taken from ENHMETA.EXE MSDN Sample
        //*ALSO NEED to GET the pallete (select and RealizePalette it, but i was too lazy*
        // Get the characteristics of the output device
        float PixelsX = (float) GetDeviceCaps(hDC, HORZRES);
        float PixelsY = (float) GetDeviceCaps(hDC, VERTRES);
        float MMX = (float) GetDeviceCaps(hDC, HORZSIZE);
        float MMY = (float) GetDeviceCaps(hDC, VERTSIZE);
        // Calculate the rect in which to draw the metafile based on the
        // intended size and the current output device resolution
        // Remember that the intended size is given in 0.01mm units, so
        // convert those to device units on the target device
        rc.top = (int)((float)(emh.rclFrame.top) * PixelsY / (MMY*100.0f));
        rc.left = (int)((float)(emh.rclFrame.left) * PixelsX / (MMX*100.0f));
        rc.right = (int)((float)(emh.rclFrame.right) * PixelsX / (MMX*100.0f));
        rc.bottom = (int)((float)(emh.rclFrame.bottom) * PixelsY / (MMY*100.0f));
        //end code: taken from ENHMETA.EXE MSDN Sample

        HDC hdcMem = CreateCompatibleDC(hDC);
        HGDIOBJ hBmpMem = CreateCompatibleBitmap(hDC, emh.rclBounds.right, emh.rclBounds.bottom);
        HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBmpMem);
        PlayEnhMetaFile(hdcMem, medium.hEnhMetaFile, &rc);
        HBITMAP hBmp = (HBITMAP)::SelectObject(hdcMem, hOldBmp);
        DeleteDC(hdcMem);
        ReleaseDC(m_hTargetWnd, hDC);
        hBmp = (HBITMAP) SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBmp);
        if(hBmp)
            DeleteObject(hBmp);
        return true;
    }
    if(pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL)
    {
        HDROP hDrop = (HDROP) GlobalLock(medium.hGlobal);
        if(hDrop)
        {
            if(m_DropCallback && m_DropCallback(hDrop))
            {
                ::DragFinish(hDrop);
                return true;
            }
            TCHAR szFileName[MAX_PATH];
            UINT cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
            if(cFiles > 0)
            {
                DragQueryFile(hDrop, 0, szFileName, sizeof(szFileName));
                HBITMAP hBitmap = (HBITMAP) LoadImage(nullptr, szFileName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
                if(hBitmap)
                {
                    HBITMAP hBmp = (HBITMAP) SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);
                    if(hBmp)
                        DeleteObject(hBmp);
                }
            }
            //DragFinish(hDrop); // base class calls ReleaseStgMedium
        }
        GlobalUnlock(medium.hGlobal);
    }
    return true; //let base free the medium
}

void CPaintManagerUI::OnDropCallCack(std::function<bool(HDROP)> callback)
{
    m_DropCallback = callback;
}

bool CPaintManagerUI::InitTaskbarList()
{
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER,
                                  __uuidof(ITaskbarList3), (void**)&m_pTaskBarList3);
    return SUCCEEDED(hr);
}

bool CPaintManagerUI::ResetTaskbarList()
{
    if(m_pTaskBarList3)
    {
        m_lfProgress = 0.0;
        m_pTaskBarList3->SetOverlayIcon(m_hWndPaint, NULL, _T(""));
        m_pTaskBarList3->SetProgressState(m_hWndPaint, TBPF_NOPROGRESS);
        m_pTaskBarList3->SetProgressState(m_hWndPaint, TBPF_NORMAL);
        return true;
    }
    return false;
}

bool CPaintManagerUI::SetTaskbarProgress(double lfProgress)
{
    if(m_pTaskBarList3)
    {
        m_lfProgress = lfProgress;
        m_pTaskBarList3->SetProgressState(m_hWndPaint, TBPF_NORMAL);
        m_pTaskBarList3->SetProgressValue(m_hWndPaint, (ULONGLONG)(100 * lfProgress), (ULONGLONG)100);
        return true;
    }
    return false;
}

bool CPaintManagerUI::SetTaskbarIcon(HICON m_hIcon, faw::string_t Description)
{
    if(m_pTaskBarList3)
    {
        m_pTaskBarList3->SetOverlayIcon(m_hWndPaint, m_hIcon, Description.c_str());
        return true;
    }
    return false;
}


} // namespace DuiLib
