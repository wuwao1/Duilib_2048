#include "StdAfx.h"

#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_IMPLEMENTATION
#include "..\Utils\nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "..\Utils\nanosvgrast.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\Utils\stb_image.h"

#ifdef USE_XIMAGE_EFFECT
#	include "../3rd/CxImage/ximage.cpp"
#	include "../3rd/CxImage/ximaenc.cpp"
#	include "../3rd/CxImage/ximagif.cpp"
#	include "../3rd/CxImage/ximainfo.cpp"
#	include "../3rd/CxImage/ximalpha.cpp"
#	include "../3rd/CxImage/ximapal.cpp"
#	include "../3rd/CxImage/ximatran.cpp"
#	include "../3rd/CxImage/ximawnd.cpp"
#	include "../3rd/CxImage/xmemfile.cpp"
#endif

///////////////////////////////////////////////////////////////////////////////////////
namespace DuiLib
{
static int g_iFontID = MAX_FONT_ID;

/////////////////////////////////////////////////////////////////////////////////////
//
//

CRenderClip::~CRenderClip()
{
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    ASSERT(::GetObjectType(hRgn) == OBJ_REGION);
    ASSERT(::GetObjectType(hOldRgn) == OBJ_REGION);
    ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);
}

void CRenderClip::GenerateClip(HDC hDC, RECT rc, CRenderClip& clip)
{
    //首先，创建了一个名为 rcClip 的矩形结构体，用于存储获取到的设备上下文 hDC 的裁剪区域。
    RECT rcClip = { 0 };
    //调用 Windows API 函数 GetClipBox 获取 hDC 的裁剪区域，并将结果存储在 rcClip 中。
    ::GetClipBox(hDC, &rcClip);
    //通过调用 CreateRectRgnIndirect 函数创建一个矩形区域 clip.hOldRgn，其形状由 rcClip 定义。
    clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    //同样地，使用 CreateRectRgnIndirect 创建另一个矩形区域 clip.hRgn，其形状由参数 rc 定义。
    clip.hRgn = ::CreateRectRgnIndirect(&rc);
    //调用 CombineRgn 函数将 clip.hRgn 和 clip.hOldRgn 进行逻辑与操作（RGN_AND），结果保存在 clip.hRgn 中。
    ::CombineRgn(clip.hRgn, clip.hRgn, clip.hOldRgn, RGN_AND);
    //通过调用 SelectClipRgn 将 clip.hRgn 设置为 hDC 的裁剪区域。
    ::SelectClipRgn(hDC, clip.hRgn);
    //最后，将 hDC、rc 存储到 clip 对象的成员变量中，以便后续使用。
    clip.hDC = hDC;
    clip.rcItem = rc;
}

void CRenderClip::GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip)
{
    //首先，创建了一个名为 rcClip 的矩形结构体，用于存储获取到的设备上下文 hDC 的裁剪区域。
    RECT rcClip = { 0 };
    //调用 Windows API 函数 GetClipBox 获取 hDC 的裁剪区域，并将结果存储在 rcClip 中。
    ::GetClipBox(hDC, &rcClip);
    //通过调用 CreateRectRgnIndirect 函数创建一个矩形区域 clip.hOldRgn，其形状由 rcClip 定义。
    clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    //同样地，使用 CreateRectRgnIndirect 创建另一个矩形区域 clip.hRgn，其形状由参数 rc 定义。
    clip.hRgn = ::CreateRectRgnIndirect(&rc);
    //调用 CreateRoundRectRgn 函数创建一个圆角矩形的区域 hRgnItem，其参数定义了圆角矩形的位置、大小、以及圆角的宽度和高度。
    HRGN hRgnItem = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
    //通过两次调用 CombineRgn 函数，先将 hRgnItem 与 clip.hRgn 进行逻辑与操作（RGN_AND），
    ::CombineRgn(clip.hRgn, clip.hRgn, hRgnItem, RGN_AND);
    //再将结果与 clip.hOldRgn 进行逻辑与操作（RGN_AND），最终得到一个综合的裁剪区域，保存在 clip.hRgn 中。
    ::CombineRgn(clip.hRgn, clip.hRgn, clip.hOldRgn, RGN_AND);
    //使用 SelectClipRgn 函数将 clip.hRgn 设置为 hDC 的裁剪区域。
    ::SelectClipRgn(hDC, clip.hRgn);
    //将相关信息存储到 clip 对象的成员变量中，包括 hDC 和 rc。
    clip.hDC = hDC;
    clip.rcItem = rc;
    //最后，调用 DeleteObject 函数释放 hRgnItem 所占用的资源。
    ::DeleteObject(hRgnItem);
}

void CRenderClip::UseOldClipBegin(HDC hDC, CRenderClip& clip)
{
    ::SelectClipRgn(hDC, clip.hOldRgn);
}

void CRenderClip::UseOldClipEnd(HDC hDC, CRenderClip& clip)
{
    ::SelectClipRgn(hDC, clip.hRgn);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
//RGB的取值范围在0~255之间，HSL的取值H为0~240，SL在0~1之间，需要对RGB和HSL颜色
static const float OneThird = 1.0f / 3;

static void RGBtoHSL(DWORD ARGB, float* H, float* S, float* L)
{
    const float
    R = (float)GetRValue(ARGB),
        G = (float)GetGValue(ARGB),
            B = (float)GetBValue(ARGB),
                nR = (R < 0 ? 0 : (R > 255 ? 255 : R)) / 255,
                     nG = (G < 0 ? 0 : (G > 255 ? 255 : G)) / 255,
                          nB = (B < 0 ? 0 : (B > 255 ? 255 : B)) / 255,
                               m = min(min(nR, nG), nB),
                                   M = max(max(nR, nG), nB);
    *L = (m + M) / 2;
    if(M == m) *H = *S = 0;
    else
    {
        const float
        f = (nR == m) ? (nG - nB) : ((nG == m) ? (nB - nR) : (nR - nG)),
            i = (nR == m) ? 3.0f : ((nG == m) ? 5.0f : 1.0f);
        *H = (i - f / (M - m));
        if(*H >= 6) *H -= 6;
        *H *= 60;
        *S = (2 * (*L) <= 1) ? ((M - m) / (M + m)) : ((M - m) / (2 - M - m));
    }
}

static void HSLtoRGB(DWORD* ARGB, float H, float S, float L)
{
    const float
    q = 2 * L < 1 ? L * (1 + S) : (L + S - L * S),
        p = 2 * L - q,
            h = H / 360,
                tr = h + OneThird,
                     tg = h,
                          tb = h - OneThird,
                               ntr = tr < 0 ? tr + 1 : (tr > 1 ? tr - 1 : tr),
                                     ntg = tg < 0 ? tg + 1 : (tg > 1 ? tg - 1 : tg),
                                           ntb = tb < 0 ? tb + 1 : (tb > 1 ? tb - 1 : tb),
                                                   B = 255 * (6 * ntr < 1 ? p + (q - p) * 6 * ntr : (2 * ntr < 1 ? q : (3 * ntr < 2 ? p + (q - p) * 6 * (2.0f * OneThird - ntr) : p))),
                                                           G = 255 * (6 * ntg < 1 ? p + (q - p) * 6 * ntg : (2 * ntg < 1 ? q : (3 * ntg < 2 ? p + (q - p) * 6 * (2.0f * OneThird - ntg) : p))),
                                                                   R = 255 * (6 * ntb < 1 ? p + (q - p) * 6 * ntb : (2 * ntb < 1 ? q : (3 * ntb < 2 ? p + (q - p) * 6 * (2.0f * OneThird - ntb) : p)));
    *ARGB &= 0xFF000000;
    *ARGB |= RGB((BYTE)(R < 0 ? 0 : (R > 255 ? 255 : R)), (BYTE)(G < 0 ? 0 : (G > 255 ? 255 : G)), (BYTE)(B < 0 ? 0 : (B > 255 ? 255 : B)));
}

DWORD CRenderEngine::AdjustColor(DWORD dwColor, short H, short S, short L)
{
    if(H == 180 && S == 100 && L == 100) return dwColor;
    float fH, fS, fL;
    float S1 = S / 100.0f;
    float L1 = L / 100.0f;
    RGBtoHSL(dwColor, &fH, &fS, &fL);
    fH += (H - 180);
    fH = fH > 0 ? fH : fH + 360;
    fS *= S1;
    fL *= L1;
    HSLtoRGB(&dwColor, fH, fS, fL);
    return dwColor;
}

static COLORREF PixelAlpha(COLORREF clrSrc, double src_darken, COLORREF clrDest, double dest_darken)
{
    return RGB(GetRValue(clrSrc) * src_darken + GetRValue(clrDest) * dest_darken,
               GetGValue(clrSrc) * src_darken + GetGValue(clrDest) * dest_darken,
               GetBValue(clrSrc) * src_darken + GetBValue(clrDest) * dest_darken);
}

static BOOL WINAPI AlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, \
                               int nSrcX, int nSrcY, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    HDC hTempDC = ::CreateCompatibleDC(hDC);
    if(!hTempDC) return FALSE;
    ON_SCOPE_EXIT([&] {	::DeleteDC(hTempDC); });

    //Creates Source DIB
    std::unique_ptr<BITMAPINFO> lpbiSrc=std::make_unique<BITMAPINFO>();
    if (!lpbiSrc) return FALSE;
    lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiSrc->bmiHeader.biWidth = dwWidth;
    lpbiSrc->bmiHeader.biHeight = dwHeight;
    lpbiSrc->bmiHeader.biPlanes = 1;
    lpbiSrc->bmiHeader.biBitCount = 32;
    lpbiSrc->bmiHeader.biCompression = BI_RGB;
    lpbiSrc->bmiHeader.biSizeImage = dwWidth * dwHeight * 4;
    lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biClrUsed = 0;
    lpbiSrc->bmiHeader.biClrImportant = 0;

    COLORREF* pSrcBits = nullptr;
    HBITMAP hSrcDib = CreateDIBSection(hSrcDC, lpbiSrc.get(), DIB_RGB_COLORS, (void**)&pSrcBits, nullptr, 0);
    if(!hSrcDib || !pSrcBits) return FALSE;
    //不需要再单独删除 pSrcBits 指向的像素数组。这是因为在创建 DIB 对象时，操作系统会自动管理和释放与之关联的像素数组内存。
    //当调用::DeleteObject(hSrcDib) 时，系统会自动释放 DIB 对象所占用的内存，包括像素数组。
    ON_SCOPE_EXIT([&] {	::DeleteObject(hSrcDib); });

    HBITMAP hOldTempBmp = (HBITMAP)::SelectObject(hTempDC, hSrcDib);
    ::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
    ::SelectObject(hTempDC, hOldTempBmp);

    //Creates Destination DIB
    std::unique_ptr<BITMAPINFO> lpbiDest=std::make_unique<BITMAPINFO>();
    if (!lpbiDest) return FALSE;
    lpbiDest->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiDest->bmiHeader.biWidth = dwWidth;
    lpbiDest->bmiHeader.biHeight = dwHeight;
    lpbiDest->bmiHeader.biPlanes = 1;
    lpbiDest->bmiHeader.biBitCount = 32;
    lpbiDest->bmiHeader.biCompression = BI_RGB;
    lpbiDest->bmiHeader.biSizeImage = dwWidth * dwHeight * 4;
    lpbiDest->bmiHeader.biXPelsPerMeter = 0;
    lpbiDest->bmiHeader.biYPelsPerMeter = 0;
    lpbiDest->bmiHeader.biClrUsed = 0;
    lpbiDest->bmiHeader.biClrImportant = 0;

    COLORREF* pDestBits = nullptr;
    HBITMAP hDestDib = CreateDIBSection(hDC, lpbiDest.get(), DIB_RGB_COLORS, (void**)&pDestBits,nullptr, 0);

    if(!hDestDib || !pDestBits) return FALSE;
    ON_SCOPE_EXIT([&] {::DeleteObject(hDestDib); });

    ::SelectObject(hTempDC, hDestDib);
    ::BitBlt(hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
    ::SelectObject(hTempDC, hOldTempBmp);

    double src_darken;
    BYTE nAlpha;

    for(int pixel = 0; pixel < dwWidth * dwHeight; pixel++, pSrcBits++, pDestBits++)
    {
        nAlpha = LOBYTE(*pSrcBits >> 24);
        src_darken = (double)(nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
        if(src_darken < 0.0) src_darken = 0.0;
        *pDestBits = PixelAlpha(*pSrcBits, src_darken, *pDestBits, 1.0 - src_darken);
    } //for

    ::SelectObject(hTempDC, hDestDib);
    ::BitBlt(hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
    ::SelectObject(hTempDC, hOldTempBmp);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
static LPBYTE WINAPI svg_load_from_memory(const LPBYTE buffer, int& out_width, int& out_height, int hope_width, int hope_height, DWORD fillcolor, CPaintManagerUI* pManager)
{
    LPBYTE pImage = NULL;
    float dpi = 96.0f;
    float scale = 1.0f;
    if(pManager)
    {
        scale = (pManager->GetDPIObj()->GetScale() * 1.0) / 100;
    }
    //SVG
    NSVGimage* svg = nsvgParse((LPSTR)(LPVOID)buffer, "px", (float)dpi);
    if(svg == NULL)
    {
        return NULL;
    }

    if(fillcolor != 0)
    {
        NSVGshape* shapes = svg->shapes;
        while(shapes)
        {
            if(shapes->fill.type == NSVG_PAINT_COLOR)
                if(shapes->fill.color != 0)
                    shapes->fill.color = UIRGB(GetRValue(fillcolor), GetGValue(fillcolor), GetBValue(fillcolor));
            shapes = shapes->next;
        }
    }

    out_width = (int)svg->width;
    out_height = (int)svg->height;
    if(out_width == 0 || out_height == 0)
    {
        nsvgDelete(svg);
        return NULL;
    }

    //如果指定长宽，则使用当前的长宽比例，否则使用dpi适配
    if(hope_width > 0 && hope_height > 0)
    {
        out_width = hope_width;
        scale = hope_width * 1.0 / svg->width;
        out_height = svg->height * scale;
        if(svg->height * scale > out_height)
        {
            scale = out_height * 1.0 / svg->height;
            out_width = svg->width * scale;
        }
    }
    else
    {
        if(pManager)
        {
            out_width = pManager->GetDPIObj()->Scale(out_width);
            out_height = pManager->GetDPIObj()->Scale(out_height);
        }
    }

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if(rast == NULL)
    {
        nsvgDelete(svg);
        return NULL;
    }

    pImage = (LPBYTE)malloc((out_width + 1) * (out_height + 1) * 4);//由于使用了小数，防止被四舍五入舍去，因此都加1
    if(!pImage)
    {
        nsvgDeleteRasterizer(rast);
        nsvgDelete(svg);
        return NULL;
    }
    nsvgRasterize(rast, svg, 0, 0, scale, pImage, out_width, out_height, out_width * 4);
    nsvgDeleteRasterizer(rast);
    nsvgDelete(svg);
    return pImage;
}

bool DrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, const RECT& rcPaint, const faw::string_t& sImageName, \
               const faw::string_t& sImageResType, RECT rcItem, RECT rcBmpPart, RECT rcCorner, DWORD dwMask, BYTE bFade, \
               bool bHole, bool bTiledX, bool bTiledY, HINSTANCE instance = nullptr, const std::shared_ptr<TDrawInfo> pDrawInfo = nullptr)
{
    if(sImageName.empty()) return false;

    std::shared_ptr<TImageInfo> data = nullptr;
    if(sImageResType.empty())
    {
        data = pManager->GetImageEx(sImageName, _T(""), dwMask, false, instance, pDrawInfo);
    }
    else
    {
        data = pManager->GetImageEx(sImageName, sImageResType, dwMask, false, instance, pDrawInfo);
    }
    if(!data) return false;

    if(rcBmpPart.left == 0 && rcBmpPart.right == 0 && rcBmpPart.top == 0 && rcBmpPart.bottom == 0)
    {
        rcBmpPart.right = data->nX;
        rcBmpPart.bottom = data->nY;
    }
    if(rcBmpPart.right > data->nX) rcBmpPart.right = data->nX;
    if(rcBmpPart.bottom > data->nY) rcBmpPart.bottom = data->nY;

    RECT rcTemp = { 0 };
    if(!::IntersectRect(&rcTemp, &rcItem, &rc)) return true;
    if(!::IntersectRect(&rcTemp, &rcItem, &rcPaint)) return true;

    HBITMAP hBmp = (data->hBitmap ? data->hBitmap : *(data->phBitmap));
    CRenderEngine::DrawImage(hDC, hBmp, rcItem, rcPaint, rcBmpPart, rcCorner, pManager->IsLayered() ? true : data->bAlpha, bFade, bHole, bTiledX, bTiledY);

    return true;
}

std::shared_ptr<TImageInfo> CRenderEngine::LoadImage(faw::string_t pStrImage, faw::string_t type, DWORD mask, HINSTANCE instance, const std::shared_ptr<TDrawInfo> pDrawInfo)
{
    if(pStrImage.empty()) return nullptr;

    faw::string_t sStrPath = pStrImage;
    if(type.empty())
    {
        sStrPath = CResourceManager::GetInstance()->GetImagePath(pStrImage);
        if(sStrPath.empty()) sStrPath = pStrImage;
        else
        {
            //if (CResourceManager::GetInstance()->GetScale() != 100) {
            //faw::string_t sScale;
            //sScale = std::format(_T("@%d."), CResourceManager::GetInstance()->GetScale());
            //sStrPath.replace_self(_T("."), sScale);
            //}
        }
    }
    TimageArgs imageArgs;
    imageArgs.bitmap = sStrPath.data();
    imageArgs.type = type;
    imageArgs.instance = instance;
    return LoadImage(imageArgs, mask, pDrawInfo);
}

std::shared_ptr<TImageInfo> CRenderEngine::LoadImage(UINT nID, faw::string_t type, DWORD mask, HINSTANCE instance, const std::shared_ptr<TDrawInfo> pDrawInfo)
{
    TimageArgs imageArgs;
    imageArgs.bitmap = nID;
    imageArgs.type = type;
    imageArgs.instance = instance;
    return LoadImage(imageArgs, mask, pDrawInfo);
}

//传LPBYTE& pData可以赋值pData=new char* a[],使pData指向新的内存
//指针传递：传递的是指针的副本。函数可以修改指针所指向的对象，但不能修改指针本身（即不能改变指针指向的地址）。
//指针引用传递：传递的是指针的引用。函数可以修改指针所指向的对象，也可以修改指针本身，使其指向不同的地址。
std::shared_ptr<TImageInfo> CRenderEngine::LoadMemoryImage(OUT TDataArgs& dataArgs, DWORD mask, const std::shared_ptr<TDrawInfo> pDrawInfo, CPaintManagerUI* pManager)
{
    LPBYTE pImage = nullptr;
    int x, y, n;
    pImage = stbi_load_from_memory(dataArgs.pData, dataArgs.dataSize, &x, &y, &n, 4);
    if(!pImage)
    {
        pImage = DuiLib::svg_load_from_memory(dataArgs.pData, x, y, pDrawInfo->szDesireDraw.cx, pDrawInfo->szDesireDraw.cy, pDrawInfo->svgcolor, pManager);
    }
    if(!pImage) return FALSE;
    ON_SCOPE_EXIT([&] {	stbi_image_free(pImage); });

    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = x;
    bmi.bmiHeader.biHeight = -y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = x * y * 4;

    bool bAlphaChannel = false;
    LPBYTE pDest = nullptr;
    HBITMAP hBitmap = ::CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pDest, nullptr, 0);
    if(!hBitmap)
    {
        //::MessageBox(0, _T("CreateDIBSection失败"), _T("抓BUG"), MB_OK);
        return nullptr;
    }

    for(int i = 0; i < x * y; i++)
    {
        pDest[i * 4 + 3] = pImage[i * 4 + 3];
        if(pDest[i * 4 + 3] < 255)
        {
            pDest[i * 4] = (BYTE)(DWORD(pImage[i * 4 + 2]) * pImage[i * 4 + 3] / 255);
            pDest[i * 4 + 1] = (BYTE)(DWORD(pImage[i * 4 + 1]) * pImage[i * 4 + 3] / 255);
            pDest[i * 4 + 2] = (BYTE)(DWORD(pImage[i * 4]) * pImage[i * 4 + 3] / 255);
            bAlphaChannel = true;
        }
        else
        {
            pDest[i * 4] = pImage[i * 4 + 2];
            pDest[i * 4 + 1] = pImage[i * 4 + 1];
            pDest[i * 4 + 2] = pImage[i * 4];
        }

        if(*(DWORD*)(&pDest[i * 4]) == mask)
        {
            pDest[i * 4] = (BYTE)0;
            pDest[i * 4 + 1] = (BYTE)0;
            pDest[i * 4 + 2] = (BYTE)0;
            pDest[i * 4 + 3] = (BYTE)0;
            bAlphaChannel = true;
        }
    }

    std::shared_ptr<TImageInfo> data = std::make_shared<TImageInfo>();
    data->pBits = nullptr;
    data->pSrcBits = nullptr;
    data->hBitmap = hBitmap;
    data->phBitmap = nullptr;
    data->nX = x;
    data->nY = y;
    data->bAlpha = bAlphaChannel;
    return data;
}

std::shared_ptr<TImageInfo> CRenderEngine::LoadImage(TimageArgs imageArgs, DWORD mask, const std::shared_ptr<TDrawInfo> pDrawInfo)
{
    TDataArgs dataArgs;
    auto bDeleteData = LoadResource2Memory(dataArgs, imageArgs);
    if (dataArgs.dataSize > 0U && dataArgs.pData)
        return LoadMemoryImage(dataArgs, mask, pDrawInfo);
    else
        std::wcout << L"images parce failed :" + pDrawInfo->sDrawString << std::endl;
    return nullptr;
}


void CRenderEngine::FreeImage(std::shared_ptr<TImageInfo> bitmap, bool bDelete)
{
    if(!bitmap) return;
    if(bitmap->hBitmap)
    {
        ::DeleteObject(bitmap->hBitmap);
    }
    bitmap->hBitmap = nullptr;
    // bitmap->phBitmap 无需释放
    bitmap->phBitmap = nullptr;
    if(bitmap->pBits)
    {
        delete[] bitmap->pBits;
    }
    bitmap->pBits = nullptr;
}

void CRenderEngine::AdjustImage(bool bUseHSL, std::shared_ptr<TImageInfo> imageInfo, short H, short S, short L)
{
    if(!imageInfo || !imageInfo->bUseHSL || !imageInfo->pBits || !imageInfo->pSrcBits
            || (!imageInfo->hBitmap && !imageInfo->phBitmap && !*(imageInfo->phBitmap)))
        return;
    if(bUseHSL == false || (H == 180 && S == 100 && L == 100))
    {
        ::CopyMemory(imageInfo->pBits, imageInfo->pSrcBits.get(), imageInfo->nX * imageInfo->nY * 4);
        return;
    }

    float fH, fS, fL;
    float S1 = S / 100.0f;
    float L1 = L / 100.0f;
    for(int i = 0; i < imageInfo->nX * imageInfo->nY; i++)
    {
        RGBtoHSL(*(DWORD*)(imageInfo->pSrcBits.get() + i * 4), &fH, &fS, &fL);
        fH += (H - 180);
        fH = fH > 0 ? fH : fH + 360;
        fS *= S1;
        fL *= L1;
        HSLtoRGB((DWORD*)(imageInfo->pBits + i * 4), fH, fS, fL);
    }
}

bool CRenderEngine::LoadResource2Memory(OUT TDataArgs& dataArgs, TimageArgs imageArgs)
{
    do
    {
        if(imageArgs.type.empty())
        {
            faw::string_t sFile = CPaintManagerUI::GetResourcePath();
            faw::string_t key = std::get<1>(imageArgs.bitmap);
            FawTools::replace_self(key, _T("\\"), _T("/"));
            std::shared_ptr<CacheZipEntry> entry = CPaintManagerUI::GetResourceZipCacheEntry(key);
            if(entry)
            {
                dataArgs.pData = entry->dataPtr.get();
                dataArgs.dataSize = entry->dataSize;
                return false;
            }

            //std::locale::global(std::locale(""));//cause http server's function being blocked
            std::wcout << L"Resource not found in cache: " + std::get<1>(imageArgs.bitmap) << std::endl;
            //找压缩包
            while(!dataArgs.pData)
            {
                if(CPaintManagerUI::GetResourceZip().empty())break;
                if(std::filesystem::exists(std::get<1>(imageArgs.bitmap)))break;
                auto file = sFile;
                file += CPaintManagerUI::GetResourceZip();
                faw::string_t sFilePwd = CPaintManagerUI::GetResourceZipPwd();  //Garfield 20160325 带密码zip包解密
                HZIP hz = nullptr;
                std::string pwd = FawTools::T_to_gb18030(sFilePwd);
                hz = OpenZip(file.c_str(), pwd.data());
                if(!hz) break;
                ON_SCOPE_EXIT([&] {	CloseZip(hz); });
                ZIPENTRY ze;
                int i = 0;
                faw::string_t key = std::get<1>(imageArgs.bitmap);
                FawTools::replace_self(key, _T("\\"), _T("/"));
                if(FindZipItem(hz, key.c_str(), true, &i, &ze) != 0) break;
                dataArgs.dataSize = ze.unc_size;
                if(dataArgs.dataSize == 0) break;

                dataArgs.pUniqueData = std::make_unique<unsigned char[]>(dataArgs.dataSize);
                int res = UnzipItem(hz, i, dataArgs.pUniqueData.get(), dataArgs.dataSize);
                dataArgs.pData = dataArgs.pUniqueData.get();
                if(res != 0x00000000 && res != 0x00000600)
                {
                    dataArgs.pUniqueData.reset();
                    dataArgs.dataSize = 0;
                    dataArgs.pData = nullptr;
                }
                break;
            }
            //找路径
            while(!dataArgs.pUniqueData)
            {
                sFile += std::get<1>(imageArgs.bitmap);
                HANDLE hFile = ::CreateFile(sFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if(hFile == INVALID_HANDLE_VALUE) break;
                dataArgs.dataSize = ::GetFileSize(hFile, nullptr);
                if(dataArgs.dataSize == 0) break;
                DWORD dwRead = 0;
                dataArgs.pUniqueData = std::make_unique<unsigned char[]>(dataArgs.dataSize);
                ::ReadFile(hFile, dataArgs.pUniqueData.get(), dataArgs.dataSize, &dwRead, nullptr);
                dataArgs.pData = dataArgs.pUniqueData.get();
                ::CloseHandle(hFile);
                if(dwRead != dataArgs.dataSize)
                {
                    dataArgs.pUniqueData.reset();
                    dataArgs.dataSize = 0;
                    dataArgs.pData = nullptr;
                }
                break;
            }
        }
        else
        {
            //找dll
            HINSTANCE dllinstance = nullptr;
            if(imageArgs.instance) dllinstance = imageArgs.instance;
            else dllinstance = CPaintManagerUI::GetResourceDll();
            HRSRC hResource = ::FindResource(dllinstance, MAKEINTRESOURCE(std::get<0>(imageArgs.bitmap)), imageArgs.type.data());
            if(!hResource) break;
            ON_SCOPE_EXIT([&] {FreeResource(hResource); });
            HGLOBAL hGlobal = ::LoadResource(dllinstance, hResource);
            if(!hGlobal) break;
            dataArgs.dataSize = ::SizeofResource(dllinstance, hResource);
            if(dataArgs.dataSize == 0) break;
            dataArgs.pUniqueData = std::make_unique<unsigned char[]>(dataArgs.dataSize);
            ::CopyMemory(dataArgs.pUniqueData.get(), (LPBYTE)::LockResource(hGlobal), dataArgs.dataSize);
            dataArgs.pData = dataArgs.pUniqueData.get();
        }
    }
    while(0);

    while(!dataArgs.pUniqueData)
    {
        //读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
        HANDLE hFile = ::CreateFile(std::get<1>(imageArgs.bitmap).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if(hFile == INVALID_HANDLE_VALUE) break;
        dataArgs.dataSize = ::GetFileSize(hFile, nullptr);
        if(dataArgs.dataSize == 0) break;

        DWORD dwRead = 0;
        dataArgs.pUniqueData = std::make_unique<unsigned char[]>(dataArgs.dataSize);
        ::ReadFile(hFile, dataArgs.pUniqueData.get(), dataArgs.dataSize, &dwRead, nullptr);
        dataArgs.pData = dataArgs.pUniqueData.get();
        ::CloseHandle(hFile);

        if(dwRead != dataArgs.dataSize)
        {
            dataArgs.pUniqueData.reset();
            dataArgs.dataSize = 0;
            dataArgs.pData = nullptr;
        }
        break;
    }
    return true;
}

#ifdef USE_XIMAGE_EFFECT
static DWORD LoadImage2Memory(const std::variant<UINT, faw::string_t>& bitmap, faw::string_t type, LPBYTE& pData)
{
    //write by wangji
    LPBYTE pData = nullptr;
    DWORD dwSize = 0;
    auto bDeleteData = LoadResource2Memory(pData, dwSize, bitmap, type, instance);
    if(dwSize == 0U || !pData)
        return nullptr;
    CxImage* pImg = nullptr;
    if(pImg = new CxImage())
    {
        pImg->SetRetreiveAllFrames(TRUE);
        if(!pImg->Decode(pData, dwSize, CXIMAGE_FORMAT_GIF))
        {
            delete pImg;
            pImg = nullptr;
        }
    }
    ON_SCOPE_EXIT([&] { if(bDeleteData) delete[] pData; });
    return pImg;
}
#endif//USE_XIMAGE_EFFECT

Gdiplus::Image* CRenderEngine::GdiplusLoadImage(TimageArgs imageArgs)
{
    //通过 LoadResource2Memory 函数将资源加载到内存中，并获取资源的大小 dwSize 和数据指针 pData。
    TDataArgs dataArgs;
    auto bDeleteData = LoadResource2Memory(dataArgs, imageArgs);
    if (dataArgs.dataSize == 0U || !dataArgs.pData) return nullptr;
    Gdiplus::Image* pImg = nullptr;
    //分配一个固定大小的内存块 hMem，并将资源数据复制到这个内存块中。
    HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dataArgs.dataSize);
    unsigned char* pMem = (unsigned char*)::GlobalLock(hMem);
    memcpy(pMem, dataArgs.pData, dataArgs.dataSize);
    //创建一个 IStream 接口对象 pStm，关联上述内存块的数据。
    IStream* pStm = nullptr;
    ::CreateStreamOnHGlobal(hMem, TRUE, &pStm);
    //使用 Gdiplus::Image::FromStream 函数从流中创建 Gdiplus::Image 对象 pImg。
    pImg = Gdiplus::Image::FromStream(pStm);
    ON_SCOPE_EXIT([&] { pStm->Release(); });
    //检查 pImg 是否成功创建以及创建状态是否为 Ok，如果失败则释放内存并返回 nullptr。
    if(!pImg || pImg->GetLastStatus() != Gdiplus::Ok)
    {
        ON_SCOPE_EXIT([&]
        {
            ::GlobalUnlock(hMem);
            ::GlobalFree(hMem);
        });
        return nullptr;
    }
    return pImg;
}


HICON CRenderEngine::GdiplusLoadIcon(TimageArgs imageArgs)
{
    //通过 LoadResource2Memory 函数将资源加载到内存中，并获取资源的大小 dwSize 和数据指针 pData。
    TDataArgs dataArgs;
    auto bDeleteData = LoadResource2Memory(dataArgs, imageArgs);
    //检查 dwSize 是否为 0 或者 pData 是否为空，如果是则返回 nullptr。
    if (dataArgs.dataSize == 0U || !dataArgs.pData) return nullptr;
    HICON hIcon = nullptr;
    Gdiplus::Status stat = Gdiplus::Ok;
    //分配一个固定大小的内存块 hMem，并将资源数据复制到这个内存块中。
    HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dataArgs.dataSize);
    unsigned char* pMem = (unsigned char*)::GlobalLock(hMem);
    memcpy(pMem, dataArgs.pData, dataArgs.dataSize);
    //创建一个 IStream 接口对象 pStm，关联上述内存块的数据。
    IStream* pStm = nullptr;
    if(!SUCCEEDED(::CreateStreamOnHGlobal(hMem, TRUE, &pStm)))return nullptr;
    ON_SCOPE_EXIT([&] { pStm->Release(); });
    //使用 Gdiplus::Bitmap::FromStream 函数从流中创建 Gdiplus::Bitmap 对象 pBitmap。
    Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStm);
    ON_SCOPE_EXIT([&]
    {
        ::GlobalUnlock(hMem);
        ::GlobalFree(hMem);
    });
    if(!pBitmap || !pBitmap->GetLastStatus() == Gdiplus::Ok)return nullptr;
    ON_SCOPE_EXIT([&]
    {
        delete pBitmap;
        pBitmap = NULL;
    });
    stat = pBitmap->GetHICON(&hIcon);
    return hIcon;
}


bool CRenderEngine::DrawIconImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, const RECT& rcPaint, faw::string_t pStrImage, faw::string_t pStrModify)
{
    if(!pManager || !hDC)
        return false;

    RECT rcDest = rc;
    const std::shared_ptr<TDrawInfo> pDrawInfo = pManager->GetDrawInfo(pStrImage, pStrModify);
    if(!pDrawInfo->sAlign.empty())
    {
        MakeFitIconDest(rc, pDrawInfo->szDesireDraw, pDrawInfo->sAlign, rcDest);
    }

    bool bRet = DuiLib::DrawImage(hDC, pManager, rc, rcPaint, pDrawInfo->sImageName, pDrawInfo->sResType, rcDest, pDrawInfo->rcSource, pDrawInfo->rcCorner, pDrawInfo->dwMask, pDrawInfo->uFade, pDrawInfo->bHole, pDrawInfo->bTiledX, pDrawInfo->bTiledY);

    return true;
}

bool CRenderEngine::MakeFitIconDest(const RECT& rcControl, const SIZE& szDesireDraw, const faw::string_t& sAlign, RECT& rcDest)
{
    ASSERT(!sAlign.empty());
    if(sAlign == _T("left"))
    {
        rcDest.left = rcControl.left;
        rcDest.top = rcControl.top;
        rcDest.right = rcDest.left + szDesireDraw.cx;
        rcDest.bottom = rcDest.top + szDesireDraw.cy;
    }
    else if(sAlign == _T("center"))
    {
        rcDest.left = rcControl.left + ((rcControl.right - rcControl.left) - szDesireDraw.cx) / 2;
        rcDest.top = rcControl.top + ((rcControl.bottom - rcControl.top) - szDesireDraw.cy) / 2;
        rcDest.right = rcDest.left + szDesireDraw.cx;
        rcDest.bottom = rcDest.top + szDesireDraw.cy;
    }
    else if(sAlign == _T("vcenter"))
    {
        rcDest.left = rcControl.left;
        rcDest.top = rcControl.top + ((rcControl.bottom - rcControl.top) - szDesireDraw.cy) / 2;
        rcDest.right = rcDest.left + szDesireDraw.cx;
        rcDest.bottom = rcDest.top + szDesireDraw.cy;
    }
    else if(sAlign == _T("hcenter"))
    {
        rcDest.left = rcControl.left + ((rcControl.right - rcControl.left) - szDesireDraw.cx) / 2;
        rcDest.top = rcControl.top;
        rcDest.right = rcDest.left + szDesireDraw.cx;
        rcDest.bottom = rcDest.top + szDesireDraw.cy;
    }

    if(rcDest.right > rcControl.right)
        rcDest.right = rcControl.right;

    if(rcDest.bottom > rcControl.bottom)
        rcDest.bottom = rcControl.bottom;

    return true;
}


void CRenderEngine::DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, faw::string_t pstrText, DWORD dwTextColor, \
                             int iFont, UINT uStyle, DWORD dwTextBKColor)
{
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    if(pstrText.empty() || !pManager) return;
    DrawColor(hDC, rc, dwTextBKColor);
    DrawText(hDC, pManager, rc, pstrText, dwTextColor, iFont, uStyle);
}

void CRenderEngine::DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint,
                              const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha,
                              BYTE uFade, bool hole, bool xtiled, bool ytiled)
{
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);

    typedef BOOL(WINAPI* LPALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
    static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");

    if(!lpAlphaBlend) lpAlphaBlend = AlphaBitBlt;
    if(!hBitmap) return;

    HDC hCloneDC = ::CreateCompatibleDC(hDC);
    HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
    ON_SCOPE_EXIT([&]
    {
        ::SelectObject(hCloneDC, hOldBitmap);
        ::DeleteDC(hCloneDC);
    });

    ::SetStretchBltMode(hDC, HALFTONE);

    RECT rcTemp = { 0 };
    RECT rcDest = { 0 };
    if(lpAlphaBlend && (bAlpha || uFade < 255))
    {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
        // middle
        if(!hole)
        {
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                if(!xtiled && !ytiled)
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                 rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, \
                                 rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
                }
                else if(xtiled && ytiled)
                {
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                    int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for(int j = 0; j < iTimesY; ++j)
                    {
                        LONG lDestTop = rcDest.top + lHeight * j;
                        LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                        LONG lDrawHeight = lHeight;
                        if(lDestBottom > rcDest.bottom)
                        {
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        for(int i = 0; i < iTimesX; ++i)
                        {
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if(lDestRight > rcDest.right)
                            {
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            lpAlphaBlend(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j,
                                         lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC,
                                         rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, lDrawWidth, lDrawHeight, bf);
                        }
                    }
                }
                else if(xtiled)
                {
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                    int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    for(int i = 0; i < iTimes; ++i)
                    {
                        LONG lDestLeft = rcDest.left + lWidth * i;
                        LONG lDestRight = rcDest.left + lWidth * (i + 1);
                        LONG lDrawWidth = lWidth;
                        if(lDestRight > rcDest.right)
                        {
                            lDrawWidth -= lDestRight - rcDest.right;
                            lDestRight = rcDest.right;
                        }
                        lpAlphaBlend(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
                                     hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                     lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
                    }
                }
                else   // ytiled
                {
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                    int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for(int i = 0; i < iTimes; ++i)
                    {
                        LONG lDestTop = rcDest.top + lHeight * i;
                        LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                        LONG lDrawHeight = lHeight;
                        if(lDestBottom > rcDest.bottom)
                        {
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        lpAlphaBlend(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
                                     hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                     rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, bf);
                    }
                }
            }
        }

        // left-top
        if(rcCorners.left > 0 && rcCorners.top > 0)
        {
            rcDest.left = rc.left;
            rcDest.top = rc.top;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, bf);
            }
        }
        // top
        if(rcCorners.top > 0)
        {
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.top;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
                             rcCorners.left - rcCorners.right, rcCorners.top, bf);
            }
        }
        // right-top
        if(rcCorners.right > 0 && rcCorners.top > 0)
        {
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.top;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, bf);
            }
        }
        // left
        if(rcCorners.left > 0)
        {
            rcDest.left = rc.left;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - \
                             rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
            }
        }
        // right
        if(rcCorners.right > 0)
        {
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, \
                             rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
            }
        }
        // left-bottom
        if(rcCorners.left > 0 && rcCorners.bottom > 0)
        {
            rcDest.left = rc.left;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, bf);
            }
        }
        // bottom
        if(rcCorners.bottom > 0)
        {
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, \
                             rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, bf);
            }
        }
        // right-bottom
        if(rcCorners.right > 0 && rcCorners.bottom > 0)
        {
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
            {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                             rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, \
                             rcCorners.bottom, bf);
            }
        }
    }
    else
    {
        if(rc.right - rc.left == rcBmpPart.right - rcBmpPart.left \
                && rc.bottom - rc.top == rcBmpPart.bottom - rcBmpPart.top \
                && rcCorners.left == 0 && rcCorners.right == 0 && rcCorners.top == 0 && rcCorners.bottom == 0)
        {
            if(::IntersectRect(&rcTemp, &rcPaint, &rc))
            {
                ::BitBlt(hDC, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, \
                         hCloneDC, rcBmpPart.left + rcTemp.left - rc.left, rcBmpPart.top + rcTemp.top - rc.top, SRCCOPY);
            }
        }
        else
        {
            // middle
            if(!hole)
            {
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    if(!xtiled && !ytiled)
                    {
                        rcDest.right -= rcDest.left;
                        rcDest.bottom -= rcDest.top;
                        ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                     rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                     rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, \
                                     rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                    }
                    else if(xtiled && ytiled)
                    {
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                        int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for(int j = 0; j < iTimesY; ++j)
                        {
                            LONG lDestTop = rcDest.top + lHeight * j;
                            LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                            LONG lDrawHeight = lHeight;
                            if(lDestBottom > rcDest.bottom)
                            {
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            for(int i = 0; i < iTimesX; ++i)
                            {
                                LONG lDestLeft = rcDest.left + lWidth * i;
                                LONG lDestRight = rcDest.left + lWidth * (i + 1);
                                LONG lDrawWidth = lWidth;
                                if(lDestRight > rcDest.right)
                                {
                                    lDrawWidth -= lDestRight - rcDest.right;
                                    lDestRight = rcDest.right;
                                }
                                ::BitBlt(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, \
                                         lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, \
                                         rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, SRCCOPY);
                            }
                        }
                    }
                    else if(xtiled)
                    {
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                        int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        for(int i = 0; i < iTimes; ++i)
                        {
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if(lDestRight > rcDest.right)
                            {
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            ::StretchBlt(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
                                         hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                         lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                        }
                    }
                    else   // ytiled
                    {
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                        int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for(int i = 0; i < iTimes; ++i)
                        {
                            LONG lDestTop = rcDest.top + lHeight * i;
                            LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                            LONG lDrawHeight = lHeight;
                            if(lDestBottom > rcDest.bottom)
                            {
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            ::StretchBlt(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
                                         hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                                         rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, SRCCOPY);
                        }
                    }
                }
            }

            // left-top
            if(rcCorners.left > 0 && rcCorners.top > 0)
            {
                rcDest.left = rc.left;
                rcDest.top = rc.top;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, SRCCOPY);
                }
            }
            // top
            if(rcCorners.top > 0)
            {
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.top;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
                                 rcCorners.left - rcCorners.right, rcCorners.top, SRCCOPY);
                }
            }
            // right-top
            if(rcCorners.right > 0 && rcCorners.top > 0)
            {
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.top;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, SRCCOPY);
                }
            }
            // left
            if(rcCorners.left > 0)
            {
                rcDest.left = rc.left;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - \
                                 rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                }
            }
            // right
            if(rcCorners.right > 0)
            {
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, \
                                 rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                }
            }
            // left-bottom
            if(rcCorners.left > 0 && rcCorners.bottom > 0)
            {
                rcDest.left = rc.left;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, SRCCOPY);
                }
            }
            // bottom
            if(rcCorners.bottom > 0)
            {
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, \
                                 rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, SRCCOPY);
                }
            }
            // right-bottom
            if(rcCorners.right > 0 && rcCorners.bottom > 0)
            {
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if(::IntersectRect(&rcTemp, &rcPaint, &rcDest))
                {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                                 rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, \
                                 rcCorners.bottom, SRCCOPY);
                }
            }
        }
    }
}

bool CRenderEngine::DrawImageInfo(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, const std::shared_ptr<TDrawInfo> pDrawInfo, HINSTANCE instance)
{
    if(!pManager || !hDC || !pDrawInfo) return false;
    RECT rcDest = rcItem;
    if(pDrawInfo->rcDest.left != 0 || 
       pDrawInfo->rcDest.top != 0 ||
       pDrawInfo->rcDest.right != 0 || 
       pDrawInfo->rcDest.bottom != 0)
    {
        rcDest.left = rcItem.left + pDrawInfo->rcDest.left;
        rcDest.top = rcItem.top + pDrawInfo->rcDest.top;
        rcDest.right = rcItem.left + pDrawInfo->rcDest.right;
        if(rcDest.right > rcItem.right) rcDest.right = rcItem.right;
        rcDest.bottom = rcItem.top + pDrawInfo->rcDest.bottom;
        if(rcDest.bottom > rcItem.bottom) rcDest.bottom = rcItem.bottom;

        if(rcDest.right < rcItem.left)
        {
            rcDest.right += (rcItem.right - rcItem.left);
            rcDest.left += (rcItem.right - rcItem.left);
        }
        if(rcDest.bottom < rcItem.top)
        {
            rcDest.bottom += (rcItem.bottom - rcItem.top);
            rcDest.top += (rcItem.bottom - rcItem.top);
        }
    }
    bool bRet = DuiLib::DrawImage(hDC, pManager, rcItem, rcPaint, pDrawInfo->sImageName, pDrawInfo->sResType, rcDest, \
                                  pDrawInfo->rcSource, pDrawInfo->rcCorner, pDrawInfo->dwMask, pDrawInfo->uFade, pDrawInfo->bHole, pDrawInfo->bTiledX, pDrawInfo->bTiledY, instance, pDrawInfo);

    return bRet;
}

bool CRenderEngine::DrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, faw::string_t pStrImage, faw::string_t pStrModify, HINSTANCE instance)
{
    if(!pManager || !hDC) return false;
    const std::shared_ptr<TDrawInfo> pDrawInfo = pManager->GetDrawInfo(pStrImage, pStrModify);
    return DrawImageInfo(hDC, pManager, rcItem, rcPaint, pDrawInfo, instance);
}

void CRenderEngine::DrawColor(HDC hDC, const RECT& rc, DWORD color)
{
    if(color <= 0x00FFFFFF) return;

    Gdiplus::Graphics graphics(hDC);
    Gdiplus::SolidBrush brush(Gdiplus::Color((LOBYTE((color) >> 24)), GetBValue(color), GetGValue(color), GetRValue(color)));
    graphics.FillRectangle(&brush, (INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left), (INT)(rc.bottom - rc.top));
}

void CRenderEngine::DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
{
    typedef BOOL(WINAPI* LPALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
    static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");
    if(!lpAlphaBlend) lpAlphaBlend = AlphaBitBlt;
    typedef BOOL(WINAPI* PGradientFill)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
    static PGradientFill lpGradientFill = (PGradientFill) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "GradientFill");

    BYTE bAlpha = (BYTE)(((dwFirst >> 24) + (dwSecond >> 24)) >> 1);
    if(bAlpha == 0) return;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    RECT rcPaint = rc;
    HDC hPaintDC = hDC;
    HBITMAP hPaintBitmap = nullptr;
    HBITMAP hOldPaintBitmap = nullptr;
    if(bAlpha < 255)
    {
        rcPaint.left = rcPaint.top = 0;
        rcPaint.right = cx;
        rcPaint.bottom = cy;
        hPaintDC = ::CreateCompatibleDC(hDC);
        hPaintBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
        ASSERT(hPaintDC);
        ASSERT(hPaintBitmap);
        hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    }
    if(lpGradientFill)
    {
        TRIVERTEX triv[2] =
            {
                {
                    rcPaint.left, rcPaint.top,
                    static_cast<COLOR16>(GetBValue(dwFirst) << 8),
                    static_cast<COLOR16>(GetGValue(dwFirst) << 8),
                    static_cast<COLOR16>(GetRValue(dwFirst) << 8), 0xFF00
            },
                {
                    rcPaint.right, rcPaint.bottom,
                    static_cast<COLOR16>(GetBValue(dwSecond) << 8),
                    static_cast<COLOR16>(GetGValue(dwSecond) << 8),
                    static_cast<COLOR16>(GetRValue(dwSecond) << 8), 0xFF00
            }
            };
        GRADIENT_RECT grc = { 0, 1 };
        lpGradientFill(hPaintDC, triv, 2, &grc, 1, bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
    }
    else
    {
        // Determine how many shades
        int nShift = 1;
        if(nSteps >= 64) nShift = 6;
        else if(nSteps >= 32) nShift = 5;
        else if(nSteps >= 16) nShift = 4;
        else if(nSteps >= 8) nShift = 3;
        else if(nSteps >= 4) nShift = 2;
        int nLines = 1 << nShift;
        for(int i = 0; i < nLines; i++)
        {
            // Do a little alpha blending
            BYTE bR = (BYTE)((GetBValue(dwSecond) * (nLines - i) + GetBValue(dwFirst) * i) >> nShift);
            BYTE bG = (BYTE)((GetGValue(dwSecond) * (nLines - i) + GetGValue(dwFirst) * i) >> nShift);
            BYTE bB = (BYTE)((GetRValue(dwSecond) * (nLines - i) + GetRValue(dwFirst) * i) >> nShift);
            // ... then paint with the resulting color
            HBRUSH hBrush = ::CreateSolidBrush(RGB(bR, bG, bB));
            RECT r2 = rcPaint;
            if(bVertical)
            {
                r2.bottom = rc.bottom - ((i * (rc.bottom - rc.top)) >> nShift);
                r2.top = rc.bottom - (((i + 1) * (rc.bottom - rc.top)) >> nShift);
                if((r2.bottom - r2.top) > 0) ::FillRect(hDC, &r2, hBrush);
            }
            else
            {
                r2.left = rc.right - (((i + 1) * (rc.right - rc.left)) >> nShift);
                r2.right = rc.right - ((i * (rc.right - rc.left)) >> nShift);
                if((r2.right - r2.left) > 0) ::FillRect(hPaintDC, &r2, hBrush);
            }
            ::DeleteObject(hBrush);
        }
    }
    if(bAlpha < 255)
    {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, bAlpha, AC_SRC_ALPHA };
        lpAlphaBlend(hDC, rc.left, rc.top, cx, cy, hPaintDC, 0, 0, cx, cy, bf);
        ::SelectObject(hPaintDC, hOldPaintBitmap);
        ::DeleteObject(hPaintBitmap);
        ::DeleteDC(hPaintDC);
    }
}

void CRenderEngine::DrawLine(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle /*= PS_SOLID*/)
{
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);

    LOGPEN lg;
    lg.lopnColor = RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor));
    lg.lopnStyle = nStyle;
    lg.lopnWidth.x = nSize;
    HPEN hPen = CreatePenIndirect(&lg);
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    POINT ptTemp = { 0 };
    ::MoveToEx(hDC, rc.left, rc.top, &ptTemp);
    ::LineTo(hDC, rc.right, rc.bottom);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}

void CRenderEngine::DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle /*= PS_SOLID*/)
{
#ifdef USE_GDI_RENDER
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    HPEN hPen = ::CreatePen(PS_SOLID | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
#else
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
    pen.SetAlignment(Gdiplus::PenAlignmentInset);
    graphics.DrawRectangle(&pen, (INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left - 1), (INT)(rc.bottom - rc.top - 1));
#endif
}

// 绘制及填充圆角矩形
void CRenderEngine::DrawRoundRectangle(HDC hDC, float x, float y, float width, float height, float arcSize, float lineWidth, Gdiplus::Color lineColor, bool fillPath, DWORD fillColor)
{
    float arcDiameter = arcSize * 2;
    //首先，通过创建 Gdiplus::Graphics 对象 g，将绘图操作与指定的设备上下文 hDC 相关联。
    Gdiplus::Graphics  g(hDC);
    //设置绘图时的滤波模式为消除锯齿现象，即使用 SmoothingModeHighQuality 来获得更平滑的绘图效果。
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    //创建一个 Gdiplus::GraphicsPath 对象 roundRectPath，用于定义矩形的路径。在路径中添加直线和弧线，以实现四个圆角。
    Gdiplus::GraphicsPath roundRectPath;
    //在路径中依次添加顶部横线、右上圆角、右侧竖线、右下圆角、底部横线、左下圆角、左侧竖线和左上圆角，完成整个矩形的路径。
    roundRectPath.AddLine(x + arcSize, y, x + width - arcSize, y);  // 顶部横线
    roundRectPath.AddArc(x + width - arcDiameter, y, arcDiameter, arcDiameter, 270, 90); // 右上圆角
    roundRectPath.AddLine(x + width, y + arcSize, x + width, y + height - arcSize);  // 右侧竖线
    roundRectPath.AddArc(x + width - arcDiameter, y + height - arcDiameter, arcDiameter, arcDiameter, 0, 90); // 右下圆角
    roundRectPath.AddLine(x + width - arcSize, y + height, x + arcSize, y + height);  // 底部横线
    roundRectPath.AddArc(x, y + height - arcDiameter, arcDiameter, arcDiameter, 90, 90); // 左下圆角
    roundRectPath.AddLine(x, y + height - arcSize, x, y + arcSize);  // 左侧竖线
    roundRectPath.AddArc(x, y, arcDiameter, arcDiameter, 180, 90); // 左上圆角
    //如果 lineWidth 大于 0，则创建一个指定颜色和宽度的 Gdiplus::Pen 对象 pen，并使用 DrawPath 方法在画布上绘制路径，实现矩形的边框线条绘制。
    if(lineWidth > 0)
    {
        Gdiplus::Pen pen(lineColor, lineWidth);
        g.DrawPath(&pen, &roundRectPath);
    }
    //如果 fillPath 为真且 fillColor 大于 0，则创建一个指定颜色的 Gdiplus::SolidBrush 对象 brush，并使用 FillPath 方法填充整个矩形路径，实现矩形的填充效果。
    if(fillPath && fillColor > 0)
    {
        Gdiplus::SolidBrush brush(fillColor);
        g.FillPath(&brush, &roundRectPath);
    }
}

void CRenderEngine::DrawRoundRect(HDC hDC, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle /*= PS_SOLID*/)
{
    //#ifdef USE_GDI_RENDER
    //		ASSERT (::GetObjectType (hDC) == OBJ_DC || ::GetObjectType (hDC) == OBJ_MEMDC);
    //		HPEN hPen = ::CreatePen (nStyle, nSize, RGB (GetBValue (dwPenColor), GetGValue (dwPenColor), GetRValue (dwPenColor)));
    //		HPEN hOldPen = (HPEN)::SelectObject (hDC, hPen);
    //		::SelectObject (hDC, ::GetStockObject (HOLLOW_BRUSH));
    //		::RoundRect (hDC, rc.left, rc.top, rc.right, rc.bottom, width, height);
    //		::SelectObject (hDC, hOldPen);
    //		::DeleteObject (hPen);
    //#else
    CRenderEngine::DrawRoundRectangle(hDC, rc.left, rc.top, rc.right - rc.left - 1, rc.bottom - rc.top - 1, width, nSize, Gdiplus::Color(dwPenColor), false, dwPenColor);
    //#endif
}

void CRenderEngine::DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, faw::string_t pstrText, DWORD dwTextColor, int iFont, UINT uStyle)
{
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    if(pstrText.empty() || !pManager)
    {
        rc.right = rc.left + 1;
        rc.bottom = rc.top + 1;
        return;
    }

    if(pManager->IsLayered() || pManager->IsUseGdiplusText())
    {
        HFONT hOldFont = (HFONT)::SelectObject(hDC, pManager->GetFont(iFont));
        Gdiplus::Graphics graphics(hDC);
        Gdiplus::Font* font = Gdiplus::Font(hDC, pManager->GetFont(iFont)).Clone();
        if(font->GetLastStatus() != Gdiplus::Ok)
        {
            delete font;
            auto info = pManager->GetFontInfo(iFont);
            font = pManager->GetResourceFont(info->sFontName, pManager->GetDPIObj()->Scale(info->iSize), info->bBold, info->bUnderline, info->bItalic);
        }
        Gdiplus::TextRenderingHint trh = Gdiplus::TextRenderingHintSystemDefault;
        switch(pManager->GetGdiplusTextRenderingHint())
        {
        case 0:
            trh = Gdiplus::TextRenderingHintSystemDefault;
            break;
        case 1:
            trh = Gdiplus::TextRenderingHintSingleBitPerPixelGridFit;
            break;
        case 2:
            trh = Gdiplus::TextRenderingHintSingleBitPerPixel;
            break;
        case 3:
            trh = Gdiplus::TextRenderingHintAntiAliasGridFit;
            break;
        case 4:
            trh = Gdiplus::TextRenderingHintAntiAlias;
            break;
        case 5:
            trh = Gdiplus::TextRenderingHintClearTypeGridFit;
            break;
        }
        graphics.SetTextRenderingHint(trh);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        Gdiplus::RectF rectF((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
        Gdiplus::SolidBrush brush(Gdiplus::Color(254, GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));

        Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();

        if((uStyle & DT_END_ELLIPSIS) != 0)
        {
            stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
        }

        int formatFlags = 0;
        if((uStyle & DT_NOCLIP) != 0)
        {
            formatFlags |= Gdiplus::StringFormatFlagsNoClip;
        }
        if((uStyle & DT_SINGLELINE) != 0)
        {
            formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
        }

        stringFormat.SetFormatFlags(formatFlags);

        if((uStyle & DT_LEFT) != 0)
        {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
        }
        else if((uStyle & DT_CENTER) != 0)
        {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if((uStyle & DT_RIGHT) != 0)
        {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
        }
        else
        {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
        }
        stringFormat.GenericTypographic();
        if((uStyle & DT_TOP) != 0)
        {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }
        else if((uStyle & DT_VCENTER) != 0)
        {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if((uStyle & DT_BOTTOM) != 0)
        {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
        }
        else
        {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }
        std::wstring pcwszDest = FawTools::T_to_utf16(pstrText);
        if((uStyle & DT_CALCRECT) != 0)
        {
            Gdiplus::RectF bounds;
            graphics.MeasureString(pcwszDest.data(), -1, font, rectF, &stringFormat, &bounds);
            // MeasureString存在计算误差，这里加一像素
            rc.bottom = rc.top + (long)bounds.Height + 1;
            rc.right = rc.left + (long)bounds.Width + 1;
        }
        else
        {
            //在GDI+中，DrawString 函数不会自动将文本垂直居中于矩形区域内。
            //DrawString 函数按照你指定的位置开始绘制文本，而文本在行内的对齐方式则由传递给它的 StringFormat 对象确定。
            //为了垂直居中整个文本块，你需要自己计算文本的起始绘制位置。
            //在使用 DrawString 时，你需要通过计算矩形的垂直中心位置，并减去文本高度的一半来设置文本的起始 Y 坐标。
            //这样，文本就会在矩形区域内垂直居中。
            if((uStyle & DT_SINGLELINE) != 0 && (uStyle & DT_VCENTER) != 0)
            {
                Gdiplus::RectF bounds;
                graphics.MeasureString(pcwszDest.data(), -1, font, rectF, &stringFormat, &bounds);
                //rectF.Y = rectF.Y + (rectF.Height - bounds.Height) / 2;
                rectF = bounds;
                //rectF.Y = rectF.Y - 1;
                rectF.Height += 2;
                //rectF.Width += 2;
            }
            graphics.DrawString(pcwszDest.data(), -1, font, rectF, &stringFormat, &brush);
        }
        delete font;
        ::SelectObject(hDC, hOldFont);
    }
    else
    {
        ::SetBkMode(hDC, TRANSPARENT);
        ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
        HFONT hOldFont = (HFONT)::SelectObject(hDC, pManager->GetFont(iFont));
        //1.找到标识符&#x
        size_t fonticonpos = faw::string_t(pstrText).find(_T("&#x"));
        if(fonticonpos != faw::string_t::npos)
        {
            //2.跳过标识符
            faw::string_t strUnicode = faw::string_t(pstrText).substr(fonticonpos + 3);
            //3.字串大小大于4，取前4位
            if(strUnicode.size() > 4) strUnicode = strUnicode.substr(0, 4);
            //4.右1位不是空格符，取前3位
            if(FawTools::right(strUnicode, 1) != _T(" "))
            {
                strUnicode = strUnicode.substr(0, strUnicode.size() - 1);
            }
            //5.右1位不是;，取前3位
            if(FawTools::right(strUnicode, 1) != _T(";"))
            {
                strUnicode = strUnicode.substr(0, strUnicode.size() - 1);
            }
            wchar_t wch[2] = { 0 };
            wch[0] = static_cast<wchar_t>(FawTools::parse_hex(strUnicode));
            ::DrawTextW(hDC, wch, -1, &rc, uStyle);
        }
        else
        {
            ::DrawText(hDC, pstrText.data(), -1, &rc, uStyle);
        }
        ::SelectObject(hDC, hOldFont);
    }
}

void CRenderEngine::DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, faw::string_t pstrText, DWORD dwTextColor, RECT* prcLinks, faw::string_t* sLinks, int& nLinkRects, int iFont, UINT uStyle)
{
    // 考虑到在xml编辑器中使用<>符号不方便，可以使用{}符号代替
    // 支持标签嵌套（如<l><b>text</b></l>），但是交叉嵌套是应该避免的（如<l><b>text</l></b>）
    // The string formatter supports a kind of "mini-html" that consists of various short tags:
    //
    //   Bold:             <b>text</b>
    //   Color:            <c #xxxxxx>text</c>  where x = RGB in hex
    //   Font:             <f x>text</f>        where x = font id
    //   Italic:           <i>text</i>
    //   Image:            <i x y z>            where x = image name and y = imagelist num and z(optional) = imagelist id
    //   Link:             <a x>text</a>        where x(optional) = link content, normal like app:notepad or http:www.xxx.com
    //   NewLine           <n>
    //   Paragraph:        <p x>text</p>        where x = extra pixels indent in p
    //   Raw Text:         <r>text</r>
    //   Selected:         <s>text</s>
    //   Underline:        <u>text</u>
    //   X Indent:         <x i>                where i = hor indent in pixels
    //   Y Indent:         <y i>                where i = ver indent in pixels

    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    if(pstrText.empty() || !pManager) return;
    if(::IsRectEmpty(&rc)) return;

    bool bDraw = (uStyle & DT_CALCRECT) == 0;

    std::vector<std::shared_ptr<TFontInfo>> aFontArray;
    CStdPtrArray aColorArray(10);
    CStdPtrArray aPIndentArray(10);

    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    HRGN hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    HRGN hRgn = ::CreateRectRgnIndirect(&rc);
    if(bDraw) ::ExtSelectClipRgn(hDC, hRgn, RGN_AND);

    std::shared_ptr<TFontInfo> pDefFontInfo = pManager->GetFontInfo(iFont);
    if(!pDefFontInfo)
    {
        pDefFontInfo = pManager->GetDefaultFontInfo();
    }
    TEXTMETRIC* pTm = &pDefFontInfo->tm;
    HFONT hOldFont = (HFONT) ::SelectObject(hDC, pDefFontInfo->hFont);
    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
    DWORD dwBkColor = pManager->GetDefaultSelectedBkColor();
    ::SetBkColor(hDC, RGB(GetBValue(dwBkColor), GetGValue(dwBkColor), GetRValue(dwBkColor)));

    // If the drawstyle include a alignment, we'll need to first determine the text-size so
    // we can draw it at the correct position...
    if(((uStyle & DT_CENTER) != 0 || (uStyle & DT_RIGHT) != 0 || (uStyle & DT_VCENTER) != 0 || (uStyle & DT_BOTTOM) != 0) && (uStyle & DT_CALCRECT) == 0)
    {
        RECT rcText = { 0, 0, 9999, 100 };
        int nLinks = 0;
        DrawHtmlText(hDC, pManager, rcText, pstrText, dwTextColor, nullptr, nullptr, nLinks, iFont, uStyle | DT_CALCRECT);
        if((uStyle & DT_SINGLELINE) != 0)
        {
            if((uStyle & DT_CENTER) != 0)
            {
                rc.left = rc.left + ((rc.right - rc.left) / 2) - ((rcText.right - rcText.left) / 2);
                rc.right = rc.left + (rcText.right - rcText.left);
            }
            if((uStyle & DT_RIGHT) != 0)
            {
                rc.left = rc.right - (rcText.right - rcText.left);
            }
        }
        if((uStyle & DT_VCENTER) != 0)
        {
            rc.top = rc.top + ((rc.bottom - rc.top) / 2) - ((rcText.bottom - rcText.top) / 2);
            rc.bottom = rc.top + (rcText.bottom - rcText.top);
        }
        if((uStyle & DT_BOTTOM) != 0)
        {
            rc.top = rc.bottom - (rcText.bottom - rcText.top);
        }
    }

    bool bHoverLink = false;
    faw::string_t sHoverLink;
    POINT ptMouse = pManager->GetMousePos();
    for(int i = 0; !bHoverLink && i < nLinkRects; i++)
    {
        if(::PtInRect(prcLinks + i, ptMouse))
        {
            sHoverLink = *(faw::string_t*)(sLinks + i);
            bHoverLink = true;
        }
    }

    POINT pt = { rc.left, rc.top };
    int iLinkIndex = 0;
    int cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
    int cyMinHeight = 0;
    int cxMaxWidth = 0;
    POINT ptLinkStart = { 0 };
    bool bLineEnd = false;
    bool bInRaw = false;
    bool bInLink = false;
    bool bInSelected = false;
    int iLineLinkIndex = 0;

    // 排版习惯是图文底部对齐，所以每行绘制都要分两步，先计算高度，再绘制
    std::vector<std::shared_ptr<TFontInfo>> aLineFontArray;
    CStdPtrArray aLineColorArray;
    CStdPtrArray aLinePIndentArray;
    faw::string_t pstrLineBegin = pstrText;
    bool bLineInRaw = false;
    bool bLineInLink = false;
    bool bLineInSelected = false;
    int cyLineHeight = 0;
    bool bLineDraw = false; // 行的第二阶段：绘制
    while(!pstrText.empty())
    {
        if(pt.x >= rc.right || pstrText[0] == _T('\n') || bLineEnd)
        {
            if(pstrText[0] == _T('\n')) pstrText = pstrText.substr(1);
            if(bLineEnd) bLineEnd = false;
            if(!bLineDraw)
            {
                if(bInLink && iLinkIndex < nLinkRects)
                {
                    ::SetRect(&prcLinks[iLinkIndex++], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + cyLine);
                    faw::string_t* pStr1 = (faw::string_t*)(sLinks + iLinkIndex - 1);
                    faw::string_t* pStr2 = (faw::string_t*)(sLinks + iLinkIndex);
                    *pStr2 = *pStr1;
                }
                for(int i = iLineLinkIndex; i < iLinkIndex; i++)
                {
                    prcLinks[i].bottom = pt.y + cyLine;
                }
                if(bDraw)
                {
                    bInLink = bLineInLink;
                    iLinkIndex = iLineLinkIndex;
                }
            }
            else
            {
                if(bInLink && iLinkIndex < nLinkRects) iLinkIndex++;
                bLineInLink = bInLink;
                iLineLinkIndex = iLinkIndex;
            }
            if((uStyle & DT_SINGLELINE) != 0 && (!bDraw || bLineDraw)) break;
            if(bDraw) bLineDraw = !bLineDraw;  // !
            pt.x = rc.left;
            if(!bLineDraw) pt.y += cyLine;
            if(pt.y > rc.bottom && bDraw) break;
            ptLinkStart = pt;
            cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
            if(pt.x >= rc.right) break;
        }
        else if(!bInRaw && (pstrText[0] == _T('<') || pstrText[0] == _T('{'))
                && (pstrText[1] >= _T('a') && pstrText[1] <= _T('z'))
                && (pstrText[2] == _T(' ') || pstrText[2] == _T('>') || pstrText[2] == _T('}')))
        {
            pstrText = pstrText.substr(1);
            faw::string_t pstrNextStart = _T("");
            switch(pstrText[0])
            {
            case _T('a') :
                // Link
            {
                pstrText = pstrText.substr(2);
                if(iLinkIndex < nLinkRects && !bLineDraw)
                {
                    faw::string_t* pStr = (faw::string_t*)(sLinks + iLinkIndex);
                    pStr->clear();
                    while(!pstrText.empty() && pstrText[0] != _T('>') && pstrText[0] != _T('}'))
                    {
                        //faw::string_t pstrTemp = pstrText.substr(1);
                        //while (pstrText < pstrTemp) {
                        *pStr += pstrText[0];
                        pstrText = pstrText.substr(1);
                        //}
                    }
                }

                DWORD clrColor = dwTextColor;
                if(clrColor == 0) pManager->GetDefaultLinkFontColor();
                if(bHoverLink && iLinkIndex < nLinkRects)
                {
                    faw::string_t* pStr = (faw::string_t*)(sLinks + iLinkIndex);
                    if(sHoverLink == *pStr) clrColor = pManager->GetDefaultLinkHoverFontColor();
                }
                //else if (!prcLinks) {
                //    if (::PtInRect (&rc, ptMouse))
                //        clrColor = pManager->GetDefaultLinkHoverFontColor ();
                //}
                aColorArray.Add((LPVOID)clrColor);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                std::shared_ptr<TFontInfo> pFontInfo = pDefFontInfo;
                if(aFontArray.size() > 0) pFontInfo = (std::shared_ptr<TFontInfo>)aFontArray[aFontArray.size() - 1];
                if(pFontInfo->bUnderline == false)
                {
                    HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                    if(!hFont) hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                    pFontInfo = pManager->GetFontInfo(hFont);
                    aFontArray.push_back(pFontInfo);
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                ptLinkStart = pt;
                bInLink = true;
            }
            break;
            case _T('b') :
                // Bold
            {
                pstrText = pstrText.substr(1);
                std::shared_ptr<TFontInfo> pFontInfo = pDefFontInfo;
                if(aFontArray.size() > 0) pFontInfo = (std::shared_ptr<TFontInfo>)aFontArray[aFontArray.size() - 1];
                if(pFontInfo->bBold == false)
                {
                    HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
                    if(!hFont) hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
                    pFontInfo = pManager->GetFontInfo(hFont);
                    aFontArray.push_back(pFontInfo);
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
            }
            break;
            case _T('c') :
                // Color
            {
                pstrText = pstrText.substr(2);
                DWORD clrColor = (DWORD)(FawTools::parse_hex(pstrText));
                aColorArray.Add((LPVOID)clrColor);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
            }
            break;
            case _T('f') :
                // Font
            {
                pstrText = pstrText.substr(2);
                faw::string_t pstrTemp = pstrText;
                int _iFont = _ttoi(pstrText.data());
                if(pstrTemp != pstrText)
                {
                    std::shared_ptr<TFontInfo> pFontInfo = pManager->GetFontInfo(_iFont);
                    aFontArray.push_back(pFontInfo);
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                }
                else
                {
                    faw::string_t sFontName;
                    int iFontSize = 10;
                    faw::string_t sFontAttr;
                    bool bBold = false;
                    bool bUnderline = false;
                    bool bItalic = false;
                    while(!pstrText.empty() && pstrText[0] != _T('>') && pstrText[0] != _T('}') && pstrText[0] != _T(' '))
                    {
                        //pstrTemp = pstrText.substr(1);
                        //while (pstrText < pstrTemp) {
                        sFontName += pstrText[0];
                        pstrText = pstrText.substr(1);
                        //}
                    }
                    if(isdigit(pstrText[0]))
                    {
                        iFontSize = FawTools::parse_dec(pstrText);
                    }
                    while(!pstrText.empty() && pstrText[0] != _T('>') && pstrText[0] != _T('}'))
                    {
                        //pstrTemp = pstrText.substr(1);
                        //while (pstrText < pstrTemp) {
                        sFontAttr += pstrText[0];
                        pstrText = pstrText.substr(1);
                        //}
                    }
                    FawTools::lower(sFontAttr);
                    if(sFontAttr.find(_T("bold")) != faw::string_t::npos) bBold = true;
                    if(sFontAttr.find(_T("underline")) != faw::string_t::npos) bUnderline = true;
                    if(sFontAttr.find(_T("italic")) != faw::string_t::npos) bItalic = true;
                    HFONT hFont = pManager->GetFont(sFontName, iFontSize, bBold, bUnderline, bItalic);
                    if(!hFont) hFont = pManager->AddFont(g_iFontID, sFontName, iFontSize, bBold, bUnderline, bItalic);
                    std::shared_ptr<TFontInfo> pFontInfo = pManager->GetFontInfo(hFont);
                    aFontArray.push_back(pFontInfo);
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                }
                cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
            }
            break;
            case _T('i') :
                // Italic or Image
            {
                pstrNextStart = &pstrText.data()[-1];
                pstrText = pstrText.substr(2);
                faw::string_t sImageString = pstrText;
                int iWidth = 0;
                int iHeight = 0;
                std::shared_ptr<TImageInfo> pImageInfo = nullptr;
                faw::string_t sName;
                while(!pstrText.empty() && pstrText[0] != _T('>') && pstrText[0] != _T('}') && pstrText[0] != _T(' '))
                {
                    //faw::string_t pstrTemp = pstrText.substr(1);
                    //while (!pstrText.empty()) {
                    sName += pstrText[0];
                    pstrText = pstrText.substr(1);
                    //}
                }
                if(sName.empty())    // Italic
                {
                    pstrNextStart = _T("");
                    std::shared_ptr<TFontInfo> pFontInfo = pDefFontInfo;
                    if (aFontArray.size() > 0) pFontInfo = aFontArray[aFontArray.size() - 1];
                    if(pFontInfo->bItalic == false)
                    {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
                        if(!hFont) hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.push_back(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                else
                {
                    int iImageListNum = (int)FawTools::parse_dec(pstrText);
                    if(iImageListNum <= 0) iImageListNum = 1;
                    int iImageListIndex = (int)FawTools::parse_dec(pstrText);
                    if(iImageListIndex < 0 || iImageListIndex >= iImageListNum) iImageListIndex = 0;

                    if(sImageString.find(_T("file=\'")) != faw::string_t::npos || sImageString.find(_T("res=\'")) != faw::string_t::npos)
                    {
                        faw::string_t sImageResType;
                        faw::string_t sImageName;
                        faw::string_t pStrImage = sImageString;
                        faw::string_t sItem;
                        faw::string_t sValue;
                        while(!pStrImage.empty())
                        {
                            sItem.clear();
                            sValue.clear();
                            while(pStrImage[0] != _T('\0') && pStrImage[0] != _T('=') && pStrImage[0] > _T(' '))
                            {
                                //faw::string_t pstrTemp = pStrImage.substr(1);
                                //while (pStrImage < pstrTemp) {
                                sItem += pStrImage[0];
                                pStrImage = pStrImage.substr(1);
                                //}
                            }
                            if(pStrImage[0] != _T('=')) break;
                            pStrImage = pStrImage.substr(1);
                            if(pStrImage[0] != _T('\'')) break;
                            pStrImage = pStrImage.substr(1);
                            while(pStrImage[0] != _T('\0') && pStrImage[0] != _T('\''))
                            {
                                //faw::string_t pstrTemp = pStrImage.substr(1);
                                //while (pStrImage < pstrTemp) {
                                sValue += pStrImage[0];
                                pStrImage = pStrImage.substr(1);
                                //}
                            }
                            if(pStrImage[0] != _T('\'')) break;
                            pStrImage = pStrImage.substr(1);
                            if(!sValue.empty())
                            {
                                if(sItem == _T("file") || sItem == _T("res"))
                                {
                                    sImageName = sValue;
                                }
                                else if(sItem == _T("restype"))
                                {
                                    sImageResType = sValue;
                                }
                            }
                            if(pStrImage[0] != _T(' ')) break;
                            pStrImage = pStrImage.substr(1);
                        }

                        pImageInfo = pManager->GetImageEx(sImageName, sImageResType);
                    }
                    else
                        pImageInfo = pManager->GetImageEx(sName);

                    if(pImageInfo)
                    {
                        iWidth = pImageInfo->nX;
                        iHeight = pImageInfo->nY;
                        if(iImageListNum > 1) iWidth /= iImageListNum;

                        if(pt.x + iWidth > rc.right && pt.x > rc.left && (uStyle & DT_SINGLELINE) == 0)
                        {
                            bLineEnd = true;
                        }
                        else
                        {
                            pstrNextStart = _T("");
                            if(bDraw && bLineDraw)
                            {
                                RECT rcImage{ pt.x, pt.y + cyLineHeight - iHeight, pt.x + iWidth, pt.y + cyLineHeight };
                                if(iHeight < cyLineHeight)
                                {
                                    rcImage.bottom -= (cyLineHeight - iHeight) / 2;
                                    rcImage.top = rcImage.bottom - iHeight;
                                }
                                RECT rcBmpPart{ 0, 0, iWidth, iHeight };
                                rcBmpPart.left = iWidth * iImageListIndex;
                                rcBmpPart.right = iWidth * (iImageListIndex + 1);
                                RECT rcCorner{ 0, 0, 0, 0 };
                                HBITMAP hBmp = (pImageInfo->hBitmap ? pImageInfo->hBitmap : *(pImageInfo->phBitmap));
                                DrawImage(hDC, hBmp, rcImage, rcImage, rcBmpPart, rcCorner, pImageInfo->bAlpha, 255);
                            }

                            cyLine = MAX(iHeight, cyLine);
                            pt.x += iWidth;
                            cyMinHeight = pt.y + iHeight;
                            cxMaxWidth = MAX(cxMaxWidth, pt.x);
                        }
                    }
                    else pstrNextStart = _T("");
                }
            }
            break;
            case _T('n') :
                // Newline
            {
                pstrText = pstrText.substr(1);
                if((uStyle & DT_SINGLELINE) != 0) break;
                bLineEnd = true;
            }
            break;
            case _T('p') :
                // Paragraph
            {
                pstrText = pstrText.substr(1);
                if(pt.x > rc.left) bLineEnd = true;
                int cyLineExtra = (int)FawTools::parse_dec(pstrText);
                aPIndentArray.Add((LPVOID)cyLineExtra);
                cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + cyLineExtra);
            }
            break;
            case _T('r') :
                // Raw Text
            {
                pstrText = pstrText.substr(1);
                bInRaw = true;
            }
            break;
            case _T('s') :
                // Selected text background color
            {
                pstrText = pstrText.substr(1);
                bInSelected = !bInSelected;
                if(bDraw && bLineDraw)
                {
                    if(bInSelected) ::SetBkMode(hDC, OPAQUE);
                    else ::SetBkMode(hDC, TRANSPARENT);
                }
            }
            break;
            case _T('u') :
                // Underline text
            {
                pstrText = pstrText.substr(1);
                std::shared_ptr<TFontInfo> pFontInfo = pDefFontInfo;
                if (aFontArray.size() > 0) pFontInfo = aFontArray[aFontArray.size() - 1];
                if(pFontInfo->bUnderline == false)
                {
                    HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                    if(!hFont) hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
                    pFontInfo = pManager->GetFontInfo(hFont);
                    aFontArray.push_back(pFontInfo);
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
            }
            break;
            case _T('x') :
                // X Indent
            {
                pstrText = pstrText.substr(2);
                int iWidth = (int)FawTools::parse_dec(pstrText);
                pt.x += iWidth;
                cxMaxWidth = MAX(cxMaxWidth, pt.x);
            }
            break;
            case _T('y') :
                // Y Indent
            {
                pstrText = pstrText.substr(2);
                int iWidth = (int)FawTools::parse_dec(pstrText);
            }
            break;
            }
            if(!pstrNextStart.empty()) pstrText = pstrNextStart;
            else
            {
                while(!pstrText.empty() && pstrText[0] != _T('>') && pstrText[0] != _T('}')) pstrText = pstrText.substr(1);
                pstrText = pstrText.substr(1);
            }
        }
        else if(!bInRaw && (pstrText[0] == _T('<') || pstrText[0] == _T('{')) && pstrText[1] == _T('/'))
        {
            pstrText = pstrText.substr(2);
            switch(pstrText[0])
            {
            case _T('c') :
            {
                pstrText = pstrText.substr(1);
                aColorArray.Remove(aColorArray.GetSize() - 1);
                DWORD clrColor = dwTextColor;
                if(aColorArray.GetSize() > 0) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
            }
            break;
            case _T('p') :
                pstrText = pstrText.substr(1);
                if(pt.x > rc.left) bLineEnd = true;
                aPIndentArray.Remove(aPIndentArray.GetSize() - 1);
                cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                break;
            case _T('s') :
            {
                pstrText = pstrText.substr(1);
                bInSelected = !bInSelected;
                if(bDraw && bLineDraw)
                {
                    if(bInSelected) ::SetBkMode(hDC, OPAQUE);
                    else ::SetBkMode(hDC, TRANSPARENT);
                }
            }
            break;
            case _T('a') :
            {
                if(iLinkIndex < nLinkRects)
                {
                    if(!bLineDraw) ::SetRect(&prcLinks[iLinkIndex], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + pTm->tmHeight + pTm->tmExternalLeading);
                    iLinkIndex++;
                }
                aColorArray.Remove(aColorArray.GetSize() - 1);
                DWORD clrColor = dwTextColor;
                if(aColorArray.GetSize() > 0) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                bInLink = false;
            }
            case _T('b') :
            case _T('f') :
            case _T('i') :
            case _T('u') :
            {
                pstrText = pstrText.substr(1);
                aFontArray.pop_back();
                std::shared_ptr<TFontInfo> pFontInfo = (std::shared_ptr<TFontInfo>)aFontArray[aFontArray.size() - 1];

                if(!pFontInfo) pFontInfo = pDefFontInfo;
                if(pTm->tmItalic && pFontInfo->bItalic == false)
                {
                    ABC abc;
                    ::GetCharABCWidths(hDC, _T(' '), _T(' '), &abc);
                    pt.x += abc.abcC / 2; // 简单修正一下斜体混排的问题, 正确做法应该是http://support.microsoft.com/kb/244798/en-us
                }
                pTm = &pFontInfo->tm;
                ::SelectObject(hDC, pFontInfo->hFont);
                cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
            }
            break;
            }
            while(!pstrText.empty() && pstrText[0] != _T('>') && pstrText[0] != _T('}')) pstrText = pstrText.substr(1);
            pstrText = pstrText.substr(1);
        }
        else if(!bInRaw && pstrText[0] == _T('<') && pstrText[2] == _T('>') && (pstrText[1] == _T('{') || pstrText[1] == _T('}')))
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if(bDraw && bLineDraw) ::TextOut(hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
            pt.x += szSpace.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText = pstrText.substr(3);
        }
        else if(!bInRaw && pstrText[0] == _T('{') && pstrText[2] == _T('}') && (pstrText[1] == _T('<') || pstrText[1] == _T('>')))
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if(bDraw && bLineDraw) ::TextOut(hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
            pt.x += szSpace.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText = pstrText.substr(3);
        }
        else if(!bInRaw && pstrText[0] == _T(' '))
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, _T(" "), 1, &szSpace);
            // Still need to paint the space because the font might have
            // underline formatting.
            if(bDraw && bLineDraw) ::TextOut(hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, _T(" "), 1);
            pt.x += szSpace.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText = pstrText.substr(1);
        }
        else
        {
            POINT ptPos = pt;
            int cchChars = 0;
            int cchSize = 0;
            int cchLastGoodWord = 0;
            int cchLastGoodSize = 0;
            faw::string_t p = pstrText;
            faw::string_t pstrNext;
            SIZE szText = { 0 };
            if(!bInRaw && p[0] == _T('<') || p[0] == _T('{')) p = p.substr(1), cchChars++, cchSize++;
            while(p.size() > 0 && p[0] != _T('\n'))
            {
                // This part makes sure that we're word-wrapping if needed or providing support
                // for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
                // slow when repeated so often.
                // TODO: Rewrite and use GetTextExtentExPoint() instead!
                if(bInRaw)
                {
                    if((p[0] == _T('<') || p[0] == _T('{')) && p[1] == _T('/')
                            && p[2] == _T('r') && (p[3] == _T('>') || p[3] == _T('}')))
                    {
                        p = p.substr(4);
                        bInRaw = false;
                        break;
                    }
                }
                else
                {
                    if(p[0] == _T('<') || p[0] == _T('{')) break;
                }
                pstrNext = p.substr(1);
                cchChars++;
                //cchSize += (int)(pstrNext.data() - p.data());
                cchSize += (int)(p.size() - pstrNext.size());
                szText.cx = cchChars * pTm->tmMaxCharWidth;
                if(pt.x + szText.cx >= rc.right)
                {
                    ::GetTextExtentPoint32(hDC, pstrText.data(), cchSize, &szText);
                }
                if(pt.x + szText.cx > rc.right)
                {
                    if(pt.x + szText.cx > rc.right && pt.x != rc.left)
                    {
                        cchChars--;
                        //cchSize -= (int)(pstrNext.data() - p.data());
                        cchSize -= (int)(p.size() - pstrNext.size());
                    }
                    if((uStyle & DT_WORDBREAK) != 0 && cchLastGoodWord > 0)
                    {
                        cchChars = cchLastGoodWord;
                        cchSize = cchLastGoodSize;
                    }
                    if((uStyle & DT_END_ELLIPSIS) != 0 && cchChars > 0)
                    {
                        cchChars -= 1;
                        faw::string_t pstrPrev = &p.data()[-1];
                        if(cchChars > 0)
                        {
                            cchChars -= 1;
                            pstrPrev = &pstrPrev.data()[-1];
                            cchSize -= (int)(p.size() - pstrNext.size());
                        }
                        else
                            cchSize -= (int)(p.size() - pstrNext.size());
                        pt.x = rc.right;
                    }
                    bLineEnd = true;
                    cxMaxWidth = MAX(cxMaxWidth, pt.x);
                    break;
                }
                if(!((p[0] >= _T('a') && p[0] <= _T('z')) || (p[0] >= _T('A') && p[0] <= _T('Z'))))
                {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                if(p[0] == _T(' '))
                {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                p = p.substr(1);
            }

            ::GetTextExtentPoint32(hDC, pstrText.data(), cchSize, &szText);
            if(bDraw && bLineDraw)
            {
                if((uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_CENTER) != 0)
                {
                    ptPos.x += (rc.right - rc.left - szText.cx) / 2;
                }
                else if((uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_RIGHT) != 0)
                {
                    ptPos.x += (rc.right - rc.left - szText.cx);
                }
                ::TextOut(hDC, ptPos.x, ptPos.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, pstrText.data(), cchSize);
                if(pt.x >= rc.right && (uStyle & DT_END_ELLIPSIS) != 0)
                    ::TextOut(hDC, ptPos.x + szText.cx, ptPos.y, _T("..."), 3);
            }
            pt.x += szText.cx;
            cxMaxWidth = MAX(cxMaxWidth, pt.x);
            pstrText = pstrText.substr(cchSize);
        }

        if(pt.x >= rc.right || pstrText.empty() || pstrText[0] == _T('\n')) bLineEnd = true;
        if(bDraw && bLineEnd)
        {
            if(!bLineDraw)
            {
                aFontArray.resize(aLineFontArray.size());
                std::copy(aLineFontArray.begin(), aLineFontArray.end(), aFontArray.begin());
                aColorArray.Resize(aLineColorArray.GetSize());
                ::CopyMemory(aColorArray.GetData(), aLineColorArray.GetData(), aLineColorArray.GetSize() * sizeof(LPVOID));
                aPIndentArray.Resize(aLinePIndentArray.GetSize());
                ::CopyMemory(aPIndentArray.GetData(), aLinePIndentArray.GetData(), aLinePIndentArray.GetSize() * sizeof(LPVOID));

                cyLineHeight = cyLine;
                pstrText = pstrLineBegin;
                bInRaw = bLineInRaw;
                bInSelected = bLineInSelected;

                DWORD clrColor = dwTextColor;
                if(aColorArray.GetSize() > 0) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                std::shared_ptr<TFontInfo> pFontInfo = aFontArray[aFontArray.size() - 1];
                if(!pFontInfo) pFontInfo = pDefFontInfo;
                pTm = &pFontInfo->tm;
                ::SelectObject(hDC, pFontInfo->hFont);
                if(bInSelected) ::SetBkMode(hDC, OPAQUE);
            }
            else
            {
                aLineFontArray.resize(aFontArray.size());
                std::copy(aFontArray.begin(), aFontArray.end(), aLineFontArray.begin());
                aLineColorArray.Resize(aColorArray.GetSize());
                ::CopyMemory(aLineColorArray.GetData(), aColorArray.GetData(), aColorArray.GetSize() * sizeof(LPVOID));
                aLinePIndentArray.Resize(aPIndentArray.GetSize());
                ::CopyMemory(aLinePIndentArray.GetData(), aPIndentArray.GetData(), aPIndentArray.GetSize() * sizeof(LPVOID));
                pstrLineBegin = pstrText;
                bLineInSelected = bInSelected;
                bLineInRaw = bInRaw;
            }
        }

        ASSERT(iLinkIndex <= nLinkRects);
    }

    nLinkRects = iLinkIndex;

    // Return size of text when requested
    if((uStyle & DT_CALCRECT) != 0)
    {
        rc.bottom = MAX(cyMinHeight, pt.y + cyLine);
        rc.right = MIN(rc.right, cxMaxWidth);
    }

    if(bDraw) ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);

    ::SelectObject(hDC, hOldFont);
}

HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, RECT rc, std::shared_ptr<CControlUI> pStopControl, DWORD dwFilterColor)
{
    if(!pManager) return NULL;
    const int cx = rc.right - rc.left;
    const int cy = rc.bottom - rc.top;

    HDC hPaintDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    if(!hPaintDC) return NULL;
    ON_SCOPE_EXIT([&] {	::DeleteDC(hPaintDC); });

    HBITMAP hPaintBitmap = ::CreateCompatibleBitmap(pManager->GetPaintDC(), rc.right, rc.bottom);
    if(!hPaintBitmap) return NULL;
    ON_SCOPE_EXIT([&] { ::DeleteObject(hPaintBitmap); });

    HBITMAP hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);

    std::shared_ptr<CControlUI> pRoot = pManager->GetRoot();
    pRoot->Paint(hPaintDC, rc, pStopControl);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = cx * cy * sizeof(DWORD);

    LPDWORD pDest = nullptr;
    HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    if(!hCloneDC) return nullptr;
    ON_SCOPE_EXIT([&] {::DeleteDC(hCloneDC); });

    HBITMAP hBitmap = ::CreateDIBSection(pManager->GetPaintDC(), &bmi, DIB_RGB_COLORS, (LPVOID*)&pDest, nullptr, 0);
    if(!hBitmap || !pDest) return nullptr;

    HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
    ::BitBlt(hCloneDC, 0, 0, cx, cy, hPaintDC, rc.left, rc.top, SRCCOPY);
    const RECT rcClone{ 0, 0, cx, cy };
    if(dwFilterColor > 0x00FFFFFF) DrawColor(hCloneDC, rcClone, dwFilterColor);
    ::SelectObject(hCloneDC, hOldBitmap);
    ::GdiFlush();
    ::SelectObject(hPaintDC, hOldPaintBitmap);
    return hBitmap;
}

HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, std::shared_ptr<CControlUI> pControl, RECT rc, DWORD dwFilterColor)
{
    if(!pManager || !pControl) return NULL;
    const int cx = rc.right - rc.left;
    const int cy = rc.bottom - rc.top;

    HDC hPaintDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    if(!hPaintDC) return NULL;
    ON_SCOPE_EXIT([&] {	::DeleteDC(hPaintDC); });

    HBITMAP hPaintBitmap = ::CreateCompatibleBitmap(pManager->GetPaintDC(), rc.right, rc.bottom);
    if(!hPaintBitmap) return NULL;
    ON_SCOPE_EXIT([&] { ::DeleteObject(hPaintBitmap); });

    HBITMAP hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    pControl->Paint(hPaintDC, rc, nullptr);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = cx * cy * sizeof(DWORD);

    LPDWORD pDest = nullptr;
    HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    if(!hCloneDC) return nullptr;
    ON_SCOPE_EXIT([&] {::DeleteDC(hCloneDC); });

    HBITMAP hBitmap = ::CreateDIBSection(pManager->GetPaintDC(), &bmi, DIB_RGB_COLORS, (LPVOID*)&pDest, nullptr, 0);
    if(!hBitmap || !pDest) return nullptr;

    HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
    ::BitBlt(hCloneDC, 0, 0, cx, cy, hPaintDC, rc.left, rc.top, SRCCOPY);
    const RECT rcClone = { 0, 0, cx, cy };
    if(dwFilterColor > 0x00FFFFFF) DrawColor(hCloneDC, rcClone, dwFilterColor);
    ::SelectObject(hCloneDC, hOldBitmap);
    ::GdiFlush();
    ::SelectObject(hPaintDC, hOldPaintBitmap);
    return hBitmap;
}

SIZE CRenderEngine::GetTextSize(HDC hDC, CPaintManagerUI* pManager, faw::string_t pstrText, int iFont, UINT uStyle)
{
    SIZE size = { 0, 0 };
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    if(pstrText.empty() || !pManager) return size;
    ::SetBkMode(hDC, TRANSPARENT);
    HFONT hOldFont = (HFONT)::SelectObject(hDC, pManager->GetFont(iFont));
    GetTextExtentPoint32(hDC, pstrText.data(), (int)pstrText.length(), &size);
    ::SelectObject(hDC, hOldFont);
    return size;
}

HBITMAP CRenderEngine::CreateARGB32Bitmap(HDC hDC, int cx, int cy, BYTE** pBits)
{
    // 使用 unique_ptr 替代原生指针
    auto lpbiSrc = std::make_unique<BYTE[]>(sizeof(BITMAPINFOHEADER));
    if (!lpbiSrc) return nullptr;

    // 将 BYTE* 转换为 LPBITMAPINFO* 类型
    LPBITMAPINFO pbiSrc = reinterpret_cast<LPBITMAPINFO>(lpbiSrc.get());

    pbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbiSrc->bmiHeader.biWidth = cx;
    pbiSrc->bmiHeader.biHeight = cy;
    pbiSrc->bmiHeader.biPlanes = 1;
    pbiSrc->bmiHeader.biBitCount = 32;
    pbiSrc->bmiHeader.biCompression = BI_RGB;
    pbiSrc->bmiHeader.biSizeImage = cx * cy;
    pbiSrc->bmiHeader.biXPelsPerMeter = 0;
    pbiSrc->bmiHeader.biYPelsPerMeter = 0;
    pbiSrc->bmiHeader.biClrUsed = 0;
    pbiSrc->bmiHeader.biClrImportant = 0;

    HBITMAP hBitmap = CreateDIBSection(hDC, pbiSrc, DIB_RGB_COLORS, (void**)pBits, nullptr, 0);
    return hBitmap;
}


void CRenderEngine::CheckAlphaColor(DWORD& dwColor)
{
    //RestoreAlphaColor认为0x00000000是真正的透明(会穿透)，其它都是GDI绘制导致的
    //所以在GDI绘制中不能用0xFF000000这个颜色值，现在处理是让它变成RGB(0,0,1)
    //RGB(0,0,1)与RGB(0,0,0)很难分出来
    if((0x00FFFFFF & dwColor) == 0)
    {
        dwColor += 1;
    }
}



} // namespace DuiLib
