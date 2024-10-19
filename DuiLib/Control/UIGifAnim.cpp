#include "StdAfx.h"
#include "UIGifAnim.h"

///////////////////////////////////////////////////////////////////////////////////////
//DECLARE_HANDLE (HZIP);	// An HZIP identifies a zip file that has been opened
//typedef DWORD ZRESULT;
//typedef struct {
//	int index;                 // index of this file within the zip
//	char name[MAX_PATH];       // filename within the zip
//	DWORD attr;                // attributes, as in GetFileAttributes.
//	FILETIME atime, ctime, mtime;// access, create, modify filetimes
//	long comp_size;            // sizes of item, compressed and uncompressed. These
//	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
//} ZIPENTRY;
//typedef struct {
//	int index;                 // index of this file within the zip
//	TCHAR name[MAX_PATH];      // filename within the zip
//	DWORD attr;                // attributes, as in GetFileAttributes.
//	FILETIME atime, ctime, mtime;// access, create, modify filetimes
//	long comp_size;            // sizes of item, compressed and uncompressed. These
//	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
//} ZIPENTRYW;
//#define OpenZip OpenZipU
//#define CloseZip(hz) CloseZipU(hz)
//extern HZIP OpenZipU (void *z, unsigned int len, DWORD flags);
//extern ZRESULT CloseZipU (HZIP hz);
//#ifdef _UNICODE
//#define ZIPENTRY ZIPENTRYW
//#define GetZipItem GetZipItemW
//#define FindZipItem FindZipItemW
//#else
//#define GetZipItem GetZipItemA
//#define FindZipItem FindZipItemA
//#endif
//extern ZRESULT GetZipItemA (HZIP hz, int index, ZIPENTRY *ze);
//extern ZRESULT GetZipItemW (HZIP hz, int index, ZIPENTRYW *ze);
//extern ZRESULT FindZipItemA (HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRY *ze);
//extern ZRESULT FindZipItemW (HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRYW *ze);
//extern ZRESULT UnzipItem (HZIP hz, int index, void *dst, unsigned int len, DWORD flags);
namespace DuiLib
{
IMPLEMENT_DUICONTROL(CGifAnimUI)
CGifAnimUI::CGifAnimUI(void) {}


CGifAnimUI::~CGifAnimUI(void)
{
    DeleteGif();
    m_pManager->KillTimer(shared_from_this(), GIF_TIMER_ID);

}

faw::string_t CGifAnimUI::GetClass() const
{
    return _T("GifAnimUI");
}

std::shared_ptr<CControlUI> CGifAnimUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_GIFANIM) return std::dynamic_pointer_cast<CGifAnimUI>(shared_from_this());
    return CControlUI::GetInterface(pstrName);
}

void CGifAnimUI::DoInit()
{
    InitGifImage();
}

bool CGifAnimUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    if(!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return true;
    if(!m_pGifImage)
    {
        InitGifImage();
    }
    DrawFrame(hDC);
    return true;
}

void CGifAnimUI::DoEvent(TEventUI& event)
{
    if(event.Type == UIEVENT_TIMER && event.wParam == GIF_TIMER_ID)
        OnTimer((UINT_PTR) event.wParam);
}

void CGifAnimUI::SetVisible(bool bVisible /* = true */)
{
    CControlUI::SetVisible(bVisible);
    if(bVisible)
        PlayGif();
    else
        StopGif();
}

void CGifAnimUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("bkimage")) SetBkImage(pstrValue);
    else if(pstrName == _T("autoplay"))
    {
        SetAutoPlay(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("autosize"))
    {
        SetAutoSize(FawTools::parse_bool(pstrValue));
    }
    else
        CControlUI::SetAttribute(pstrName, pstrValue);
}

void CGifAnimUI::SetBkImage(faw::string_t pStrImage)
{
    if(m_sBkImage == pStrImage || pStrImage.empty()) return;

    m_sBkImage = pStrImage;

    StopGif();
    DeleteGif();

    Invalidate();

}

faw::string_t CGifAnimUI::GetBkImage()
{
    return m_sBkImage;
}

void CGifAnimUI::SetAutoPlay(bool bIsAuto)
{
    m_bIsAutoPlay = bIsAuto;
}

bool CGifAnimUI::IsAutoPlay() const
{
    return m_bIsAutoPlay;
}

void CGifAnimUI::SetAutoSize(bool bIsAuto)
{
    m_bIsAutoSize = bIsAuto;
}

bool CGifAnimUI::IsAutoSize() const
{
    return m_bIsAutoSize;
}

void CGifAnimUI::PlayGif()
{
    if(m_bIsPlaying || !m_pGifImage || m_nFrameCount <= 1)
    {
        return;
    }

    long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
    if(lPause == 0) lPause = 100;
    m_pManager->SetTimer(shared_from_this(), GIF_TIMER_ID, lPause);

    m_bIsPlaying = true;
}

void CGifAnimUI::PauseGif()
{
    if(!m_bIsPlaying || !m_pGifImage)
    {
        return;
    }

    m_pManager->KillTimer(shared_from_this(), GIF_TIMER_ID);
    this->Invalidate();
    m_bIsPlaying = false;
}

void CGifAnimUI::StopGif()
{
    if(!m_bIsPlaying)
    {
        return;
    }

    m_pManager->KillTimer(shared_from_this(), GIF_TIMER_ID);
    m_nFramePosition = 0;
    this->Invalidate();
    m_bIsPlaying = false;
}

void CGifAnimUI::InitGifImage()
{
    TimageArgs imageArgs;
    imageArgs.bitmap = GetBkImage();
    m_pGifImage = CRenderEngine::GdiplusLoadImage(imageArgs);
    if(!m_pGifImage) return;
    UINT nCount = 0;
    nCount = m_pGifImage->GetFrameDimensionsCount();
    auto pDimensionIDs = std::make_shared<GUID>(nCount);
    m_pGifImage->GetFrameDimensionsList(pDimensionIDs.get(), nCount);
    m_nFrameCount = m_pGifImage->GetFrameCount(&pDimensionIDs.get()[0]);
    if(m_nFrameCount > 1)
    {
        int nSize = m_pGifImage->GetPropertyItemSize(PropertyTagFrameDelay);
        m_pPropertyItem = (Gdiplus::PropertyItem*) malloc(nSize);
        m_pGifImage->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);
    }

    if(m_bIsAutoSize)
    {
        SetFixedWidth(m_pGifImage->GetWidth());
        SetFixedHeight(m_pGifImage->GetHeight());
    }
    if(m_bIsAutoPlay)
    {
        PlayGif();
    }
}

void CGifAnimUI::DeleteGif()
{
    if(m_pStream)
    {
        m_pStream->Release();
        m_pStream = nullptr;
    }
    if(m_pGifImage)
    {
        delete m_pGifImage;
        m_pGifImage = nullptr;
    }

    if(m_pPropertyItem)
    {
        free(m_pPropertyItem);
        m_pPropertyItem = nullptr;
    }
    m_nFrameCount = 0;
    m_nFramePosition = 0;
}

void CGifAnimUI::OnTimer(UINT_PTR idEvent)
{
    if(idEvent != GIF_TIMER_ID)
        return;
    m_pManager->KillTimer(shared_from_this(), GIF_TIMER_ID);
    this->Invalidate();

    m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;

    long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
    if(lPause == 0) lPause = 100;
    m_pManager->SetTimer(shared_from_this(), GIF_TIMER_ID, lPause);
}

void CGifAnimUI::DrawFrame(HDC hDC)
{
    if(!hDC || !m_pGifImage) return;
    GUID pageGuid = Gdiplus::FrameDimensionTime;
    Gdiplus::Graphics graphics(hDC);
    graphics.DrawImage(m_pGifImage, (INT) m_rcItem.left, (INT) m_rcItem.top, (INT)(m_rcItem.right - m_rcItem.left), (INT)(m_rcItem.bottom - m_rcItem.top));
    m_pGifImage->SelectActiveFrame(&pageGuid, m_nFramePosition);
}
}