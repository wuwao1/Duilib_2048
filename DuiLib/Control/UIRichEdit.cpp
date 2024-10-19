﻿#include "StdAfx.h"
#include "UIRichEdit.h"
#include "../Utils/FawTools.hpp"

#include <oledlg.h>
#pragma comment(lib,"OleDlg.lib")

//#ifdef _USEIMM
#include <imm.h>
#pragma comment(lib, "imm32.lib")
//#endif
// These constants are for backward compatibility. They are the
// sizes used for initialization and reset in RichEdit 1.0

namespace DuiLib
{

#define IDM_VERBMIN                     40200
#define IDM_VERBMAX                     40300
#define ID_EDIT_CONVERT                 40013
#define CRICHEDITUI_ID_EDIT_CUT         40006
#define CRICHEDITUI_ID_EDIT_COPY        40007
#define CRICHEDITUI_ID_EDIT_PASTE       40008

#define ID_RICH_UNDO			101
#define ID_RICH_CUT				102
#define ID_RICH_COPY			103
#define ID_RICH_PASTE			104
#define ID_RICH_CLEAR			105
#define ID_RICH_SELECTALL		106
#define ID_RICH_REDO			107

const LONG cInitTextMax = (32 * 1024) - 1;

EXTERN_C const IID IID_ITextServices =   // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    {
        0x8d33f740,
        0xcf58,
        0x11ce,
        { 0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
    };

EXTERN_C const IID IID_ITextHost =   /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
    {
        0xc5bdd8d0,
        0xd26e,
        0x11ce,
        { 0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
    };

#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

#ifndef HIMETRIC_PER_INCH
#define HIMETRIC_PER_INCH 2540
#endif



class CRichEditOleCallback : public IRichEditOleCallback
{
protected:
    ULONG m_cRef;			// Object Reference Count
    int m_iNumStorages;
    IStorage* pStorage;
    CRichEditUI *m_pRichEditUI;
public:
    // Constructors and Destructor
    CRichEditOleCallback(CRichEditUI *pRichEditUI);
    virtual ~CRichEditOleCallback();

public:
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* lplpObj) ;
    STDMETHOD_(ULONG, AddRef)(THIS)  ;
    STDMETHOD_(ULONG, Release)(THIS) ;

    // *** IRichEditOleCallback methods ***
    STDMETHOD(GetNewStorage)(THIS_ LPSTORAGE FAR* lplpstg) ;
    STDMETHOD(GetInPlaceContext)(THIS_ LPOLEINPLACEFRAME FAR* lplpFrame,
                                 LPOLEINPLACEUIWINDOW FAR* lplpDoc,
                                 LPOLEINPLACEFRAMEINFO lpFrameInfo) ;
    STDMETHOD(ShowContainerUI)(THIS_ BOOL fShow) ;
    STDMETHOD(QueryInsertObject)(THIS_ LPCLSID lpclsid, LPSTORAGE lpstg,
                                 LONG cp) ;
    STDMETHOD(DeleteObject)(THIS_ LPOLEOBJECT lpoleobj) ;
    STDMETHOD(QueryAcceptData)(THIS_ LPDATAOBJECT lpdataobj,
                               CLIPFORMAT FAR* lpcfFormat, DWORD reco,
                               BOOL fReally, HGLOBAL hMetaPict) ;
    STDMETHOD(ContextSensitiveHelp)(THIS_ BOOL fEnterMode) ;
    STDMETHOD(GetClipboardData)(THIS_ CHARRANGE FAR* lpchrg, DWORD reco,
                                LPDATAOBJECT FAR* lplpdataobj) ;
    STDMETHOD(GetDragDropEffect)(THIS_ BOOL fDrag, DWORD grfKeyState,
                                 LPDWORD pdwEffect) ;
    STDMETHOD(GetContextMenu)(THIS_ WORD seltype, LPOLEOBJECT lpoleobj,
                              CHARRANGE FAR* lpchrg,
                              HMENU FAR* lphmenu) ;

};


CRichEditOleCallback::CRichEditOleCallback(CRichEditUI *pRichEditUI) :
    m_cRef(0), m_pRichEditUI(NULL)

{
    pStorage = NULL;
    m_iNumStorages = 0;
    m_cRef = 0;

    // set up OLE storage

    HRESULT hResult = ::StgCreateDocfile(NULL,
                                         STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE /*| STGM_DELETEONRELEASE */ | STGM_CREATE,
                                         0, &pStorage);

    if(pStorage == NULL ||
            hResult != S_OK)
    {
        ;//AfxThrowOleException( hResult );
    }

    m_pRichEditUI = pRichEditUI;
}

CRichEditOleCallback::~CRichEditOleCallback()
{

}
/////////////////////////////////////////////////////////////////////////////
// IUnknown Interface members
//

HRESULT CRichEditOleCallback::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    //ÌáÊ¾ÖØ¸´ÉùÃ÷,Ö±½Ó¸ÄÃû×ÖËãÁË
    GUID IID_IRichEditOleCallback2 = { 0x00020D03, 0x0, 0x0, { 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46 } };

    if(IID_IUnknown == riid)
    {
        *ppv = (LPUNKNOWN) this;
    }
    else if(IID_IRichEditOleCallback2 == riid)
    {
        *ppv = (LPRICHEDITOLECALLBACK) this;
    }
    else
    {
        //		DebugTrace(TEXT("E_NOINTERFACE.\r\n"));
    }

    if(NULL == *ppv)
        return ResultFromScode(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
}

ULONG CRichEditOleCallback::AddRef(void)
{
    m_cRef++;
    //DebugCount(TEXT("CRichEditOleCallback::AddRef().  Count = %d.\r\n"),m_cRef);
    return m_cRef;
}

ULONG CRichEditOleCallback::Release(void)
{
    m_cRef--;
    //	DebugCount(TEXT("CRichEditOleCallback::Release().  Count = %d.\r\n"),m_cRef);

    if(0 == m_cRef)
    {
        delete this;
        return m_cRef;
    }
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// IRichEditOleCallback Interface Members
//

//
//  FUNCTION:   CRichEditOleCallback::GetNewStorage
//
//  PURPOSE:    Gets a storage for a new object.
//
//  PARAMETERS:
//      ppStg - Where to return the storage.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//
//  COMMENTS:
//      Implemented in the document object.
//

HRESULT CRichEditOleCallback::GetNewStorage(LPSTORAGE* ppStg)
{
    /*
    if (!ppStg)
    return E_INVALIDARG;

    *ppStg = NULL;

    //
    // We need to create a new storage for an object to occupy.  We're going
    // to do this the easy way and just create a storage on an HGLOBAL and let
    // OLE do the management.  When it comes to saving things we'll just let
    // the RichEdit control do the work.  Keep in mind this is not efficient,
    // but this program is just for demonstration.
    //

    LPLOCKBYTES pLockBytes;
    HRESULT hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pLockBytes);
    if (FAILED(hr))
    return hr;

    hr = StgCreateDocfileOnILockBytes(pLockBytes,
    STGM_SHARE_EXCLUSIVE | STGM_CREATE |
    STGM_READWRITE,
    0,
    ppStg);
    pLockBytes->Release();
    return (hr);
    */

    m_iNumStorages++;
    faw::string_t tName = std::format(_T("REOLEStorage{}."), (int)m_iNumStorages);
    HRESULT hResult = pStorage->CreateStorage(tName.c_str(),
                      STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
                      0, 0, ppStg);

    if(hResult != S_OK)
    {
        ;//::AfxThrowOleException( hResult );
    }

    return hResult;
}


//
//  FUNCTION:   CRichEditOleCallback::GetInPlaceContext
//
//  PURPOSE:    Gets the context information for an in place object.
//
//  PARAMETERS:
//      ppFrame    - Pointer to the frame window object.
//		ppDoc      - Pointer to the document window object.
//		pFrameInfo - Pointer to the frame window information.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//
//

HRESULT CRichEditOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME* ppFrame,
        LPOLEINPLACEUIWINDOW* ppDoc,
        LPOLEINPLACEFRAMEINFO pFrameInfo)
{
    //	DebugTrace(TEXT("CRichEditOleCallback::GetInPlaceContext()\r\n"));

    //
    // Validate the arguments
    //
    if(!ppFrame || !ppDoc || !pFrameInfo)
    {
        //DebugMsg(TEXT("CRichEditOleCallback::GetInPlaceContext() received an") TEXT(" invalid argument.\r\n"));
        return E_INVALIDARG;
    }

    /*
    //
    // Return the appropriate interface pointers.
    //
    *ppDoc = (LPOLEINPLACEUIWINDOW) m_pDoc->GetInPlaceFrame();
    if (*ppDoc)
    (*ppDoc)->AddRef();

    *ppFrame = m_pDoc->GetInPlaceFrame();
    if (*ppFrame)
    (*ppFrame)->AddRef();
    */


    //
    // Fill in the default frame window information.
    //
    pFrameInfo->fMDIApp = FALSE;
    //pFrameInfo->hwndFrame = m_pDoc->GetFrameWindow();
    //pFrameInfo->haccel = m_pDoc->GetInPlaceAccel();

    //
    // Note: CopyAcceleratorTable() returns the number of items in the table
    // 	     if the	lpAccelDest is NULL.
    //
    pFrameInfo->cAccelEntries = CopyAcceleratorTable(pFrameInfo->haccel,
                                NULL, 0);

    return S_OK;
}


//
//  FUNCTION:	CRichEditOleCallback::ShowContainerUI
//
//  PURPOSE:	Handles the showing or hiding of the container UI.
//
//  PARAMETERS:
//      fShow - TRUE if the function should display the UI, FALSE to hide it.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT CRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
    //	DebugTrace(TEXT("CRichEditOleCallback::ShowContainerUI()\r\n"));

    /*
    if (!m_pDoc->GetInPlaceFrame()) 	// This should never fail, but just in case...
    return E_FAIL;

    if (fShow)
    {
    (m_pDoc->GetInPlaceFrame())->ReinstateUI();
    //SetFocus(m_hwndEdit);
    }
    else
    (m_pDoc->GetInPlaceFrame())->ShowUIAndTools(FALSE, FALSE);
    */

    return S_OK;
}


//
//  FUNCTION:	CRichEditOleCallback::QueryInsertObject
//
//  PURPOSE:    Called to ask whether an object should be inserted.  We're
//				going to allow everything to be inserted.
//

HRESULT CRichEditOleCallback::QueryInsertObject(LPCLSID /* pclsid */,
        LPSTORAGE /* pStg */,
        LONG /* cp */)
{
    return S_OK;
}


//
//  FUNCTION:   CRichEditOleCallback::DeleteObject
//
//  PURPOSE:    Notification that an object is going to be deleted.
//

HRESULT CRichEditOleCallback::DeleteObject(LPOLEOBJECT /* pOleObj */)
{
    return S_OK;
}


//
//  FUNCTION:   CRichEditOleCallback::QueryAcceptData
//
//  PURPOSE:    Called to ask whether the data that is being pasted or
//				dragged should be accepted.  We accept everything.
//

HRESULT CRichEditOleCallback::QueryAcceptData(LPDATAOBJECT /* pDataObj */,
        CLIPFORMAT* /* pcfFormat */,
        DWORD /* reco */,
        BOOL /* fReally */,
        HGLOBAL /* hMetaPict */)
{
    return S_OK;
}


//
//  FUNCTION:   CRichEditOleCallback::ContextSensitiveHelp
//
//  PURPOSE:    Tells the application that it should transition into or out of
//				context sensitive help mode.  We don't implement help at all
//				so blow this off.
//

HRESULT CRichEditOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}


//
//  FUNCTION:   CRichEditOleCallback::GetClipboardData
//
//  PURPOSE:    Called when the rich edit needs to provide clipboard data.
//				We'll let the rich edit control handle this one.
//

HRESULT CRichEditOleCallback::GetClipboardData(CHARRANGE* /* pchrg */,
        DWORD /* reco */,
        LPDATAOBJECT* ppDataObject)

{
    *ppDataObject = NULL;
    return E_NOTIMPL;
}


//
//  FUNCTION:   CRichEditOleCallback::GetDragDropEffect
//
//  PURPOSE:    Allows us to determine the drag cursor effect when the user
//				is dragging something over us.  We'll let the rich edit control
//				handle this one as well.
//

HRESULT CRichEditOleCallback::GetDragDropEffect(BOOL /* fDrag */,
        DWORD /* grfKeyState */,
        LPDWORD /* pdwEffect */)
{
    return E_NOTIMPL;
}


//
//  FUNCTION:   CRichEditOleCallback::GetContextMenu
//
//  PURPOSE:    Creates the context menu for alternate mouse clicks in the
//				RichEdit control.
//
//  PARAMETERS:
//      seltype - Selection type
//		pOleObj - IOleObject interface of the selected object, if any
//		pchrg   - Selection range
//		phMenu  - Place to return the constructed menu
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT CRichEditOleCallback::GetContextMenu(WORD seltype,
        LPOLEOBJECT pOleObj,
        CHARRANGE* pchrg,
        HMENU* phMenu)
{
    HMENU hMenuVerbs = NULL;

    //
    // First create the menu.
    //
    *phMenu = CreatePopupMenu();
    if(!*phMenu)
        return E_FAIL;

    //
    // Now put the verbs on the menu if we have a selected object.
    //
    if(pOleObj)
    {
        OleUIAddVerbMenu(pOleObj, NULL, *phMenu, 0, IDM_VERBMIN, IDM_VERBMAX, TRUE, ID_EDIT_CONVERT, &hMenuVerbs);

        AppendMenu(*phMenu, MF_SEPARATOR, 0, NULL);
    }

    //
    // If there isn't anything selected, then gray out the cut and copy
    // menu items.
    //
    UINT mf = MF_STRING | MF_BYCOMMAND;
    mf |= (pchrg->cpMin == pchrg->cpMax ? MF_GRAYED : MF_ENABLED);

    //
    // Add the cut, copy, and paste verbs.
    //
    AppendMenu(*phMenu, mf, CRICHEDITUI_ID_EDIT_CUT, TEXT("Cu&t"));
    AppendMenu(*phMenu, mf, CRICHEDITUI_ID_EDIT_COPY, TEXT("&Copy"));

    //
    // Now decide if there is something that can be pasted into the Rich Edit
    // control.
    //
    mf = MF_STRING | MF_BYCOMMAND;

    LRESULT result;
    mf |= (m_pRichEditUI->TxSendMessage(EM_CANPASTE, 0, 0, &result) ? MF_ENABLED : MF_GRAYED);
    AppendMenu(*phMenu, mf, CRICHEDITUI_ID_EDIT_PASTE, TEXT("&Paste"));

    return S_OK;
}




//
// Description - This class implements the application's IOleWindow,
// IOleInPlaceUIWindow, and IOleInPlaceFrame interfaces.
//


class COleInPlaceFrame : public IOleInPlaceFrame
{
private:
    ULONG m_cRef;	  			// Object reference count
    HWND  m_hwndFrame;			// Application frame window
    BOOL  m_fHelpMode;			// Context-Sensitive help mode flag

public:
    //
    // Constructors and Destructors
    //
    COleInPlaceFrame(HWND hwndFrame = NULL) :
        m_hwndFrame(hwndFrame), m_cRef(0), m_fHelpMode(FALSE) {}
    ~COleInPlaceFrame();

    //
    // IUnknown Interface members
    //
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    //
    // IOleWindow Interface members
    //
    STDMETHODIMP GetWindow(HWND* pHwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

    //
    // IOleInPlaceUIWindow Interface members
    //
    STDMETHODIMP GetBorder(LPRECT prcBorder);
    STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pbw);
    STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pbw);
    STDMETHODIMP SetActiveObject(LPOLEINPLACEACTIVEOBJECT pInPlaceActiveObject,
                                 LPCOLESTR szObjName);

    //
    // IOleInPlaceFrame Interface members
    //
    STDMETHODIMP InsertMenus(HMENU hmenuShared,
                             LPOLEMENUGROUPWIDTHS pMenuWidths);
    STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu,
                         HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus(HMENU hmenuShared);
    STDMETHODIMP SetStatusText(LPCOLESTR szStatusText);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP TranslateAccelerator(LPMSG pMsg, WORD wID);

    //
    // UI Helpers
    //
    void ShowUIAndTools(BOOL fShow, BOOL fMenu);
    void ReinstateUI(void);
public:
    static LPOLEINPLACEACTIVEOBJECT g_pActiveObject;
};



COleInPlaceFrame::~COleInPlaceFrame()
{
    // if (m_cRef != 0) DebugMsg(TEXT("WARNING!!! Object destroyed with m_cRef = %d!\r\n"),m_cRef);
}


/////////////////////////////////////////////////////////////////////////////
// IUnknown Interface members
//

HRESULT COleInPlaceFrame::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    //DebugTrace(TEXT("COleInPlaceFrame::QueryInterface() returns "));

    if(IID_IUnknown == riid)
    {
        //		DebugTrace(TEXT("IUnknown.\r\n"));
        *ppv = (LPUNKNOWN) this;
    }
    else if(IID_IOleWindow == riid)
    {
        //		DebugTrace(TEXT("IOleWindow.\r\n"));
        *ppv = (LPOLEWINDOW) this;
    }
    else if(IID_IOleInPlaceUIWindow == riid)
    {
        //		DebugTrace(TEXT("IOleInPlaceUIWindow.\r\n"));
        *ppv = (LPOLEINPLACEUIWINDOW) this;
    }
    else if(IID_IOleInPlaceFrame == riid)
    {
        //		DebugTrace(TEXT("IOleInPlaceFrame.\r\n"));
        *ppv = (LPOLEINPLACEFRAME) this;
    }
    else
    {
        //		DebugTrace(TEXT(" E_NOINTERFACE\r\n"));
    }



    if(NULL == *ppv)
        return ResultFromScode(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
}

ULONG COleInPlaceFrame::AddRef()
{
    m_cRef++;
    //DebugCount(TEXT("COleInPlaceFrame::AddRef().  Count = %d.\r\n"),m_cRef);
    return m_cRef;
}

ULONG COleInPlaceFrame::Release()
{
    m_cRef--;
    //DebugCount(TEXT("COleInPlaceFrame::Release().  Count = %d.\r\n"),m_cRef);

    if(0 == m_cRef)
        delete this;

    return m_cRef;
}



/////////////////////////////////////////////////////////////////////////////
// IOleWindow Interface members
//

//
//  FUNCTION:   COleInPlaceFrame::GetWindow
//
//  PURPOSE:    Returns the window handle of the top level application window.
//
//  PARAMETERS:
//      (out) pHwnd - Pointer where we return the handle of the frame window.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::GetWindow(HWND* pHwnd)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::GetWindow.\r\n"));
    if(!pHwnd)
        return E_INVALIDARG;

    *pHwnd = m_hwndFrame;
    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::ContextSensitiveHelp
//
//  PURPOSE:    Notifies the frame that the object has entered context help
//				mode.
//
//  PARAMETERS:
//      (in)  fEnterMode - TRUE if the object is entering help mode, FALSE if
//					 	   leaving help mode.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::ContextSensitiveHelp.\r\n"));
    if(m_fHelpMode != fEnterMode)
    {
        m_fHelpMode = fEnterMode;

        //
        // We're just going to forward the context sensitive help stuff
        // to the inplace active object since we don't support context
        // help ourselves.
        //
        if(g_pActiveObject)
        {
            LPOLEINPLACEOBJECT pInPlaceObject;
            g_pActiveObject->QueryInterface(IID_IOleInPlaceObject,
                                            (LPVOID*)&pInPlaceObject);
            if(pInPlaceObject)
            {
                pInPlaceObject->ContextSensitiveHelp(fEnterMode);
                pInPlaceObject->Release();
            }
        }
    }

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IOleInPlaceUIWindow Interface members
//

//
//  FUNCTION:   COleInPlaceFrame::GetBorder
//
//  PURPOSE:    The object is asking for a rectangle in which it can put
//				toolbars and similar controls while active in place.
//
//  PARAMETERS:
//      (out) prcBorder - Pointer to a rectangle containing the area that the
//						  object can use.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::GetBorder(LPRECT prcBorder)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::GetBorder.\r\n"));
    if(!prcBorder)
        return E_INVALIDARG;

    //
    // Get the client area of the frame window.  We need to subtract the
    // area occupied by any toolbars or status bars we want to keep while
    // the object is active.
    //

    GetClientRect(m_hwndFrame, prcBorder);
    //prcBorder->bottom -= g_pStatusBar->GetHeight();

    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::RequestBorderSpace
//
//  PURPOSE:    Determines whether tools can be installed around the objects
//				window frame while the object is active in place.
//
//  PARAMETERS:
//      (in) pbw - Requested border space
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS pbw)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::RequestBorderSpace.\r\n"));
    // Accept all requests
    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::SetBorderSpace
//
//  PURPOSE:    Allocates space for the border requested by the active
//				in place object.
//
//  PARAMETERS:
//      (in) pbw - Pointer to the structure containing the requested widths
//				   in pixels of the tools.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//
//  COMMENTS:
//		If the pbw is NULL, then the object is saying it doesn't have any of
//		it's own tools and we can leave our tools visible.  If pbw =
//		{0, 0, 0, 0), then the object has no tools, but it doesn't want our
//		tools visible either.
//

HRESULT COleInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS pbw)
{
    RECT rcClient;			// Rectangle of the current frame client area
    RECT rcOldEdit;			// Old rectangle for the edit control
    //	DebugTrace(TEXT("COleInPlaceFrame::SetBorderSpace.\r\n"));

    //
    // Remember the previous position of the client window in coordinates
    // relative to the frame window.
    //
    //GetWindowRect(GetDlgItem(m_hwndFrame, IDC_EDIT), &rcOldEdit);
    MapWindowPoints(NULL, m_hwndFrame, (LPPOINT)&rcOldEdit, 2);

    //
    // Get the client window rect.  We also need to subtract the area of any
    // status bars or tools that shouldn't disappear when the object is
    // active.
    //
    GetClientRect(m_hwndFrame, &rcClient);
    //rcClient.bottom -= g_pStatusBar->GetHeight();

    if(pbw)
    {
        //
        // Set the space for the object tools.
        //
        rcClient.left += pbw->left;
        rcClient.top += pbw->top;
        rcClient.right -= pbw->right;
        rcClient.bottom -= pbw->bottom;

        //
        // Save the new border widths.  We also need to add space for any
        // status bars or tools that are still visible.
        //
        //CopyRect(&g_rcBorderSpace, pbw);
    }
    else
    {
        //
        // No tools, reset the border space.  If we have any of our own tools
        // such as a status bar we need to add it to this rect.
        //
        //SetRect(&g_rcBorderSpace, 0, 0, 0, 0);
    }

    //
    // Only move the window if the rectangles have changed.
    //
    if(memcmp(&rcClient, &rcOldEdit, sizeof(RECT)))
    {
        //::SetWindowPos(m_hwndEdit, 0, rcClient.left,  rcClient.top,  rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_NOZORDER | SWP_NOACTIVATE);

        //DebugTrace(TEXT("Move Window in SetBorderSpace.\r\n"));
    }

    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::SetActiveObject
//
//  PURPOSE:	Called by the object to provide the frame window a direct
// 				channel of communication with the active in-place object.
//
//  PARAMETERS:
//      (in) pInPlaceActiveObject - New active object, or NULL if none.
//		(in) szObjName            - Name of the new active object.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::SetActiveObject(LPOLEINPLACEACTIVEOBJECT pInPlaceActiveObject,
        LPCOLESTR szObjName)
{

    //	DebugTrace(TEXT("COleInPlaceFrame::SetActiveObject.\r\n"));

    //
    // If object we have is same as the object passed then return S_OK.
    //
    if(pInPlaceActiveObject == g_pActiveObject)
        return S_OK;

    //
    // If we already have an active object, free that first.
    //
    if(g_pActiveObject)
        g_pActiveObject->Release();

    //
    // If we're given an object, AddRef it, and also update our global
    // pointer as well.
    //
    if(pInPlaceActiveObject)
        pInPlaceActiveObject->AddRef();

    g_pActiveObject = pInPlaceActiveObject;

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IOleInPlaceFrame Interface members
//

//
//  FUNCTION:   COleInPlaceFrame::InsertMenus
//
//  PURPOSE:    Called by the object server to allow the container to insert
//				its menu groups in the composite menu that will be used
//				during the in-place session.
//
//  PARAMETERS:
//      (in) hmenuShared      - Specifies a handle to an empty menu.
//		(in, out) pMenuWidths - Pointers to an array of six LONG values.  The
//								container fills elemts 0, 2, and 4 to reflect
//								the number of menu elements it provies in the
//								File, View, and Window menu groups.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::InsertMenus(HMENU hmenuShared,
                                      LPOLEMENUGROUPWIDTHS pMenuWidths)
{
    //DebugTrace(TEXT("COleInPlaceFrame::InsertMenus.\r\n"));
    if(!hmenuShared || !pMenuWidths)
        return E_INVALIDARG;

    //TCHAR szMenuString[64];

    //
    // Add the file menu
    //
    //GetMenuString(g_hmenuMain, 0, szMenuString, sizeof(szMenuString), MF_BYPOSITION);
    //AppendMenu(hmenuShared, MF_POPUP, (UINT) GetSubMenu(g_hmenuMain, 0),szMenuString);
    pMenuWidths->width[0] = 1;

    //
    // Don't want to add the view menu.
    //
    pMenuWidths->width[2] = 0;

    //
    // Don't have a window menu either.
    //
    pMenuWidths->width[4] = 0;

    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::SetMenu
//
//  PURPOSE:    Installs the composite menu into the window frame containing
//				the object that is being activated in place.
//
//  PARAMETERS:
//      (in) hmenuShared 	  - Handle to the composite menu.
//		(in) holemenu 		  - Handle to the menu descriptor.
//		(in) hwndActiveObject - Handle to the object's active window.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu,
                                  HWND hwndActiveObject)
{
    //::SetMenu(m_hwndFrame, (holemenu ? hmenuShared : g_hmenuMain));
    DrawMenuBar(m_hwndFrame);

    //
    // Pass the menu descriptor to OLE.
    //
    return OleSetMenuDescriptor(holemenu, m_hwndFrame, hwndActiveObject,
                                (LPOLEINPLACEFRAME) this, g_pActiveObject);
}


//
//  FUNCTION:   COleInPlaceFrame::RemoveMenus
//
//  PURPOSE:    Called by the object server to give the container a chance
//				to remove it's menu elements from the in-place composite menu.
//
//  PARAMETERS:
//      (in) hmenuShared - Handle to the composite menu.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::RemoveMenus(HMENU hmenuShared)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::RemoveMenus.\r\n"));
    if(!hmenuShared)
        return E_INVALIDARG;

    int nResult;

    while((nResult = GetMenuItemCount(hmenuShared)) && (nResult != -1))
        RemoveMenu(hmenuShared, 0, MF_BYPOSITION);

    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::SetStatusText
//
//  PURPOSE:	Sets and displays status text about the in-place object in
//				the containers frame window status bar.
//
//  PARAMETERS:
//      (in) szStatusText -	String containing the message to display.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::SetStatusText(LPCOLESTR szStatusText)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::SetStatusText.\r\n"));

    //
    // This string is sent to us in Unicode.  We need to decide if we should
    // translate it first.
    //
#ifndef UNICODE
    UINT cch = lstrlenW(szStatusText) + 1;
    LPSTR pszMessage = new char[cch];

    ZeroMemory(pszMessage, cch);

    WideCharToMultiByte(CP_ACP, 0, szStatusText, -1, pszMessage, cch, NULL, NULL);
    //	g_pStatusBar->ShowMessage(pszMessage);

    delete[] pszMessage;
#else
    //g_pStatusBar->ShowMessage(szStatusText);
#endif

    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::EnableModeless
//
//  PURPOSE:    Enables or disables modeless dialogs that are owned by the
//				frame window.
//
//  PARAMETERS:
//      fEnable - TRUE if the dialogs should be enabled, or FALSE to disable.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::EnableModeless(BOOL fEnable)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::EnableModeless.\r\n"));

    //
    // We don't have any modeless dialogs.
    //
    return S_OK;
}


//
//  FUNCTION:   COleInPlaceFrame::TranslateAccelerator
//
//  PURPOSE:    Translates keystrokes intended for the container frame while
//				an object is active in place.
//
//  PARAMETERS:
//      (in) pMsg - Pointer to the message to translate.
//		(in) wID  - Command ID value corresponding to the keystroke in the
//					container provided accelerator table.
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//

HRESULT COleInPlaceFrame::TranslateAccelerator(LPMSG pMsg, WORD wID)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::TranslateAccelerator.\r\n"));
    if(!pMsg)
        return E_INVALIDARG;

    HACCEL m_hAccelInPlace = NULL;

    if(!::TranslateAccelerator(m_hwndFrame, m_hAccelInPlace, pMsg))
        return (S_FALSE);

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// UI Helpers
//


//
//  FUNCTION:   COleInPlaceFrame::ShowUIAndTools
//
//  PURPOSE:    Shows or hides the application's menu and/or tools.  This
//				is called as in-place objects are activated and deactivated.
//
//  PARAMETERS:
//      (in) fShow - TRUE if the app should show it's own UI and tools
//		(in) fMenu - TRUE if the app should show it's own menu
//
//  RETURN VALUE:
//      Returns an HRESULT signifying success or failure.
//
//  COMMENTS:
//		fShow will be FALSE if we should hide our own tools.  We don't
//		currently have any tools so for now we just ignore this.
//

void COleInPlaceFrame::ShowUIAndTools(BOOL fShow, BOOL fMenu)
{
    //	DebugTrace(TEXT("COleInPlaceFrame::ShowUIAndTools.\r\n"));
    if(fShow)
    {
        //
        // First reset the menu if necessary.
        //
        if(fMenu)
            this->SetMenu(NULL, NULL, NULL);

        //
        // Also redraw any tools we had hidden.
        //
        //g_pToolbar->Show(TRUE);
    }
    else
    {
        //g_pToolbar->Show(FALSE);
    }
}


//
//  FUNCTION:   COleInPlaceFrame::ReinstateUI
//
//  PURPOSE:    Restores the applications tools after an object has
//				been deactivated.
//

void COleInPlaceFrame::ReinstateUI(void)
{
    RECT rc;
    //	DebugTrace(TEXT("COleInPlaceFrame::ReinstateUI.\r\n"));

    //
    // Set the default border widths
    //
    //SetRect(&rc, 0, g_pToolbar->GetHeight(), 0, 0);
    SetBorderSpace(&rc);

    //
    // Show our menu and tools
    //
    ShowUIAndTools(TRUE, TRUE);
};

// Convert

// Convert Pixels on the X axis to Himetric
LONG DXtoHimetricX(LONG dx, LONG xPerInch)
{
    return (LONG) MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
}

// Convert Pixels on the Y axis to Himetric
LONG DYtoHimetricY(LONG dy, LONG yPerInch)
{
    return (LONG) MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
}

HRESULT InitDefaultCharFormat(CRichEditUI* re, CHARFORMAT2W* pcf, HFONT hfont)
{
    memset(pcf, 0, sizeof(CHARFORMAT2W));
    if(!hfont)
    {
        hfont = re->GetManager()->GetFont(re->GetFont());
    }
    LOGFONT lf;
    ::GetObject(hfont, sizeof(LOGFONT), &lf);

    DWORD dwColor = re->GetTextColor();
    if(re->GetManager()->IsLayered())
    {
        CRenderEngine::CheckAlphaColor(dwColor);
    }
    pcf->cbSize = sizeof(CHARFORMAT2W);
    pcf->crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
    LONG yPixPerInch = GetDeviceCaps(re->GetManager()->GetPaintDC(), LOGPIXELSY);
    pcf->yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
    pcf->yOffset = 0;
    pcf->dwEffects = 0;
    pcf->dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
    if(lf.lfWeight >= FW_BOLD)
        pcf->dwEffects |= CFE_BOLD;
    if(lf.lfItalic)
        pcf->dwEffects |= CFE_ITALIC;
    if(lf.lfUnderline)
        pcf->dwEffects |= CFE_UNDERLINE;
    pcf->bCharSet = lf.lfCharSet;
    pcf->bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
    _tcscpy(pcf->szFaceName, lf.lfFaceName);
#else
    //need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
    MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE);
#endif

    return S_OK;
}

HRESULT InitDefaultParaFormat(CRichEditUI* re, PARAFORMAT2* ppf)
{
    memset(ppf, 0, sizeof(PARAFORMAT2));
    ppf->cbSize = sizeof(PARAFORMAT2);
    ppf->dwMask = PFM_ALL;
    ppf->wAlignment = PFA_LEFT;
    ppf->cTabCount = 1;
    ppf->rgxTabs[0] = lDefaultTab;

    return S_OK;
}

HRESULT CreateHost(std::shared_ptr<CRichEditUI> re, const CREATESTRUCT *pcs, CTxtWinHost **pptec)
{
    HRESULT hr = E_FAIL;
    CTxtWinHost *phost = new CTxtWinHost();
    if(phost)
    {
        if(phost->Init(re, pcs))
        {
            *pptec = phost;
            hr = S_OK;
        }
    }

    if(FAILED(hr))
    {
        delete phost;
    }

    return TRUE;
}

CTxtWinHost::CTxtWinHost() :
    m_re(nullptr)
{
    ::ZeroMemory(&cRefs, sizeof(CTxtWinHost) - offsetof(CTxtWinHost, cRefs));
    cchTextMost = cInitTextMax;
    laccelpos = -1;
}

CTxtWinHost::~CTxtWinHost()
{
    pserv->OnTxInPlaceDeactivate();
    pserv->Release();
}

////////////////////// Create/Init/Destruct Commands ///////////////////////

BOOL CTxtWinHost::Init(std::shared_ptr<CRichEditUI> re, const CREATESTRUCT *pcs)
{
    IUnknown *pUnk = nullptr;
    HRESULT hr;

    m_re = re;
    // Initialize Reference count
    cRefs = 1;

    // Create and cache CHARFORMAT for this control
    if(FAILED(InitDefaultCharFormat(re.get(), &cf, nullptr)))
        return FALSE;

    // Create and cache PARAFORMAT for this control
    if(FAILED(InitDefaultParaFormat(re.get(), &pf)))
        return FALSE;

    // edit controls created without a window are multiline by default
    // so that paragraph formats can be
    dwStyle = ES_MULTILINE;

    // edit controls are rich by default
    fRich = re->IsRich();

    cchTextMost = re->GetLimitText();

    if(pcs)
    {
        dwStyle = pcs->style;

        if(!(dwStyle & (ES_AUTOHSCROLL | WS_HSCROLL)))
        {
            fWordWrap = TRUE;
        }
    }

    if(!(dwStyle & ES_LEFT))
    {
        if(dwStyle & ES_CENTER)
            pf.wAlignment = PFA_CENTER;
        else if(dwStyle & ES_RIGHT)
            pf.wAlignment = PFA_RIGHT;
    }

    fInplaceActive = TRUE;

    PCreateTextServices TextServicesProc = nullptr;
#ifdef _UNICODE
    HMODULE hmod = LoadLibrary(_T("Msftedit.dll"));
#else
    HMODULE hmod = LoadLibrary(_T("Riched20.dll"));
#endif
    if(hmod)
    {
        TextServicesProc = (PCreateTextServices) GetProcAddress(hmod, "CreateTextServices");
    }
    if(TextServicesProc)
    {
        hr = TextServicesProc(nullptr, this, &pUnk);
    }

    hr = pUnk->QueryInterface(IID_ITextServices, (void **) &pserv);

    // Whether the previous call succeeded or failed we are done
    // with the private interface.
    pUnk->Release();

    if(FAILED(hr))
        return FALSE;

    // Set window text
    if(pcs && pcs->lpszName)
    {
        std::wstring _text = FawTools::T_to_utf16(pcs->lpszName);
        if(FAILED(pserv->TxSetText(_text.c_str())))
            return FALSE;
    }

    return TRUE;
}

/////////////////////////////////  IUnknown ////////////////////////////////


HRESULT CTxtWinHost::QueryInterface(REFIID riid, void **ppvObject)
{
    HRESULT hr = E_NOINTERFACE;
    *ppvObject = nullptr;

    if(IsEqualIID(riid, IID_IUnknown)
            || IsEqualIID(riid, IID_ITextHost))
    {
        AddRef();
        *ppvObject = (ITextHost *) this;
        hr = S_OK;
    }

    return hr;
}

ULONG CTxtWinHost::AddRef(void)
{
    return ++cRefs;
}

ULONG CTxtWinHost::Release(void)
{
    ULONG c_Refs = --cRefs;

    if(c_Refs == 0)
    {
        delete this;
    }

    return c_Refs;
}

/////////////////////////////////  Far East Support  //////////////////////////////////////

HIMC CTxtWinHost::TxImmGetContext(void)
{
    return nullptr;
}

void CTxtWinHost::TxImmReleaseContext(HIMC himc)
{
    //::ImmReleaseContext( hwnd, himc );
}

//////////////////////////// ITextHost Interface  ////////////////////////////

HDC CTxtWinHost::TxGetDC()
{
    return m_re->GetManager()->GetPaintDC();
}

int CTxtWinHost::TxReleaseDC(HDC hdc)
{
    return 1;
}

BOOL CTxtWinHost::TxShowScrollBar(INT fnBar, BOOL fShow)
{
    std::shared_ptr<CScrollBarUI>  pVerticalScrollBar = m_re->GetVerticalScrollBar();
    std::shared_ptr<CScrollBarUI>  pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
    if(fnBar == SB_VERT && pVerticalScrollBar)
    {
        pVerticalScrollBar->SetVisible(fShow == TRUE);
    }
    else if(fnBar == SB_HORZ && pHorizontalScrollBar)
    {
        pHorizontalScrollBar->SetVisible(fShow == TRUE);
    }
    else if(fnBar == SB_BOTH)
    {
        if(pVerticalScrollBar) pVerticalScrollBar->SetVisible(fShow == TRUE);
        if(pHorizontalScrollBar) pHorizontalScrollBar->SetVisible(fShow == TRUE);
    }
    return TRUE;
}

BOOL CTxtWinHost::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
    if(fuSBFlags == SB_VERT)
    {
        m_re->EnableScrollBar(true, bool(m_re->GetHorizontalScrollBar()));
        m_re->GetVerticalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
    }
    else if(fuSBFlags == SB_HORZ)
    {
        m_re->EnableScrollBar(bool(m_re->GetVerticalScrollBar()), true);
        m_re->GetHorizontalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
    }
    else if(fuSBFlags == SB_BOTH)
    {
        m_re->EnableScrollBar(true, true);
        m_re->GetVerticalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
        m_re->GetHorizontalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
    }
    return TRUE;
}

BOOL CTxtWinHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
    std::shared_ptr<CScrollBarUI>  pVerticalScrollBar = m_re->GetVerticalScrollBar();
    std::shared_ptr<CScrollBarUI>  pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
    if(fnBar == SB_VERT && pVerticalScrollBar)
    {
        if(nMaxPos - nMinPos - rcClient.bottom + rcClient.top <= 0)
        {
            pVerticalScrollBar->SetVisible(false);
        }
        else
        {
            pVerticalScrollBar->SetVisible(true);
            pVerticalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.bottom + rcClient.top);
        }
    }
    else if(fnBar == SB_HORZ && pHorizontalScrollBar)
    {
        if(nMaxPos - nMinPos - rcClient.right + rcClient.left <= 0)
        {
            pHorizontalScrollBar->SetVisible(false);
        }
        else
        {
            pHorizontalScrollBar->SetVisible(true);
            pHorizontalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.right + rcClient.left);
        }
    }
    return TRUE;
}

BOOL CTxtWinHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
    std::shared_ptr<CScrollBarUI>  pVerticalScrollBar = m_re->GetVerticalScrollBar();
    std::shared_ptr<CScrollBarUI>  pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
    if(fnBar == SB_VERT && pVerticalScrollBar)
    {
        pVerticalScrollBar->SetScrollPos(nPos);
    }
    else if(fnBar == SB_HORZ && pHorizontalScrollBar)
    {
        pHorizontalScrollBar->SetScrollPos(nPos);
    }
    return TRUE;
}

void CTxtWinHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
    if(!prc)
    {
        m_re->GetManager()->Invalidate(rcClient);
        return;
    }
    RECT rc = *prc;
    m_re->GetManager()->Invalidate(rc);
}

void CTxtWinHost::TxViewChange(BOOL fUpdate)
{
    if(m_re->OnTxViewChanged()) m_re->Invalidate();
}

BOOL CTxtWinHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
    iCaretWidth = xWidth;
    iCaretHeight = yHeight;
    return ::CreateCaret(m_re->GetManager()->GetPaintWindow(), hbmp, xWidth, yHeight);
}

BOOL CTxtWinHost::TxShowCaret(BOOL fShow)
{
    fShowCaret = fShow;
    if(fShow)
        return ::ShowCaret(m_re->GetManager()->GetPaintWindow());
    else
        return ::HideCaret(m_re->GetManager()->GetPaintWindow());
}

BOOL CTxtWinHost::TxSetCaretPos(INT x, INT y)
{
    POINT ptCaret = { 0 };
    ::GetCaretPos(&ptCaret);
    RECT rcCaret = { ptCaret.x, ptCaret.y, ptCaret.x + iCaretLastWidth, ptCaret.y + iCaretLastHeight };
    if(m_re->GetManager()->IsLayered()) m_re->GetManager()->Invalidate(rcCaret);
    else if(fNeedFreshCaret == TRUE)
    {
        m_re->GetManager()->Invalidate(rcCaret);
        fNeedFreshCaret = FALSE;
    }
    rcCaret.left = x;
    rcCaret.top = y;
    rcCaret.right = x + iCaretWidth;
    rcCaret.bottom = y + iCaretHeight;
    if(m_re->GetManager()->IsLayered()) m_re->GetManager()->Invalidate(rcCaret);
    iCaretLastWidth = iCaretWidth;
    iCaretLastHeight = iCaretHeight;
    return ::SetCaretPos(x, y);
}

BOOL CTxtWinHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
    fTimer = TRUE;
    return m_re->GetManager()->SetTimer(m_re, idTimer, uTimeout) == TRUE;
}

void CTxtWinHost::TxKillTimer(UINT idTimer)
{
    m_re->GetManager()->KillTimer(m_re, idTimer);
    fTimer = FALSE;
}

void CTxtWinHost::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
    return;
}

void CTxtWinHost::TxSetCapture(BOOL fCapture)
{
    if(fCapture) m_re->GetManager()->SetCapture();
    else m_re->GetManager()->ReleaseCapture();
    fCaptured = fCapture;
}

void CTxtWinHost::TxSetFocus()
{
    m_re->SetFocus();
}

void CTxtWinHost::TxSetCursor(HCURSOR hcur, BOOL fText)
{
    ::SetCursor(hcur);
}

BOOL CTxtWinHost::TxScreenToClient(LPPOINT lppt)
{
    return ::ScreenToClient(m_re->GetManager()->GetPaintWindow(), lppt);
}

BOOL CTxtWinHost::TxClientToScreen(LPPOINT lppt)
{
    return ::ClientToScreen(m_re->GetManager()->GetPaintWindow(), lppt);
}

HRESULT CTxtWinHost::TxActivate(LONG *plOldState)
{
    return S_OK;
}

HRESULT CTxtWinHost::TxDeactivate(LONG lNewState)
{
    return S_OK;
}

HRESULT CTxtWinHost::TxGetClientRect(LPRECT prc)
{
    *prc = rcClient;
    GetControlRect(prc);
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetViewInset(LPRECT prc)
{
    prc->left = prc->right = prc->top = prc->bottom = 0;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetCharFormat(const CHARFORMATW **ppCF)
{
    *ppCF = &cf;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetParaFormat(const PARAFORMAT **ppPF)
{
    *ppPF = &pf;
    return NOERROR;
}

COLORREF CTxtWinHost::TxGetSysColor(int nIndex)
{
    return ::GetSysColor(nIndex);
}

HRESULT CTxtWinHost::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
    *pstyle = !fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetMaxLength(DWORD *pLength)
{
    *pLength = cchTextMost;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetScrollBars(DWORD *pdwScrollBar)
{
    *pdwScrollBar = dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL |
                               ES_AUTOHSCROLL | ES_DISABLENOSCROLL);

    return NOERROR;
}

HRESULT CTxtWinHost::TxGetPasswordChar(TCHAR *pch)
{
#ifdef _UNICODE
    *pch = chPasswordChar;
#else
    ::WideCharToMultiByte(CP_ACP, 0, &chPasswordChar, 1, pch, 1, nullptr, nullptr);
#endif
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetAcceleratorPos(LONG *pcp)
{
    *pcp = laccelpos;
    return S_OK;
}

HRESULT CTxtWinHost::OnTxCharFormatChange(const CHARFORMATW *pcf)
{
    return S_OK;
}

HRESULT CTxtWinHost::OnTxParaFormatChange(const PARAFORMAT *ppf)
{
    return S_OK;
}

HRESULT CTxtWinHost::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
    DWORD dwProperties = 0;

    if(fRich)
    {
        dwProperties = TXTBIT_RICHTEXT;
    }

    if(dwStyle & ES_MULTILINE)
    {
        dwProperties |= TXTBIT_MULTILINE;
    }

    if(dwStyle & ES_READONLY)
    {
        dwProperties |= TXTBIT_READONLY;
    }

    if(dwStyle & ES_PASSWORD)
    {
        dwProperties |= TXTBIT_USEPASSWORD;
    }

    if(!(dwStyle & ES_NOHIDESEL))
    {
        dwProperties |= TXTBIT_HIDESELECTION;
    }

    if(fEnableAutoWordSel)
    {
        dwProperties |= TXTBIT_AUTOWORDSEL;
    }

    if(fWordWrap)
    {
        dwProperties |= TXTBIT_WORDWRAP;
    }

    if(fAllowBeep)
    {
        dwProperties |= TXTBIT_ALLOWBEEP;
    }

    if(fSaveSelection)
    {
        dwProperties |= TXTBIT_SAVESELECTION;
    }

    *pdwBits = dwProperties & dwMask;
    return NOERROR;
}


HRESULT CTxtWinHost::TxNotify(DWORD iNotify, void *pv)
{
    if(iNotify == EN_REQUESTRESIZE)
    {
        RECT rc = { 0 };
        REQRESIZE *preqsz = (REQRESIZE *) pv;
        GetControlRect(&rc);
        rc.bottom = rc.top + preqsz->rc.bottom;
        rc.right = rc.left + preqsz->rc.right;
        SetClientRect(&rc);
        return S_OK;
    }
    m_re->OnTxNotify(iNotify, pv);
    return S_OK;
}

HRESULT CTxtWinHost::TxGetExtent(LPSIZEL lpExtent)
{
    *lpExtent = sizelExtent;
    return S_OK;
}

HRESULT	CTxtWinHost::TxGetSelectionBarWidth(LONG *plSelBarWidth)
{
    *plSelBarWidth = lSelBarWidth;
    return S_OK;
}

void CTxtWinHost::SetWordWrap(BOOL _fWordWrap)
{
    fWordWrap = _fWordWrap;
    pserv->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, _fWordWrap ? TXTBIT_WORDWRAP : 0);
}

BOOL CTxtWinHost::IsReadOnly()
{
    return (dwStyle & ES_READONLY) != 0;
}

void CTxtWinHost::SetReadOnly(BOOL fReadOnly)
{
    if(fReadOnly)
    {
        dwStyle |= ES_READONLY;
    }
    else
    {
        dwStyle &= ~ES_READONLY;
    }

    pserv->OnTxPropertyBitsChange(TXTBIT_READONLY, fReadOnly ? TXTBIT_READONLY : 0);
}

void CTxtWinHost::SetFont(HFONT hFont)
{
    if(!hFont) return;
    LOGFONT lf;
    ::GetObject(hFont, sizeof(LOGFONT), &lf);
    LONG yPixPerInch = ::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSY);
    cf.yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
    if(lf.lfWeight >= FW_BOLD) cf.dwEffects |= CFE_BOLD;
    else cf.dwEffects &= ~CFE_BOLD;
    if(lf.lfItalic) cf.dwEffects |= CFE_ITALIC;
    else cf.dwEffects &= ~CFE_ITALIC;
    if(lf.lfUnderline) cf.dwEffects |= CFE_UNDERLINE;
    else cf.dwEffects &= ~CFE_UNDERLINE;
    cf.bCharSet = lf.lfCharSet;
    cf.bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
    _tcscpy(cf.szFaceName, lf.lfFaceName);
#else
    //need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
    MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, cf.szFaceName, LF_FACESIZE);
#endif

    pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE,
                                  TXTBIT_CHARFORMATCHANGE);
}

void CTxtWinHost::SetColor(DWORD dwColor)
{
    cf.crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
    pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
}

SIZEL* CTxtWinHost::GetExtent()
{
    return &sizelExtent;
}

void CTxtWinHost::SetExtent(SIZEL *psizelExtent)
{
    sizelExtent = *psizelExtent;
    pserv->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
}

void CTxtWinHost::LimitText(LONG nChars)
{
    cchTextMost = nChars;
    if(cchTextMost <= 0) cchTextMost = cInitTextMax;
    pserv->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
}

BOOL CTxtWinHost::IsCaptured()
{
    return fCaptured;
}

BOOL CTxtWinHost::IsShowCaret()
{
    return fShowCaret;
}

void CTxtWinHost::NeedFreshCaret()
{
    fNeedFreshCaret = TRUE;
}

INT CTxtWinHost::GetCaretWidth()
{
    return iCaretWidth;
}

INT CTxtWinHost::GetCaretHeight()
{
    return iCaretHeight;
}

BOOL CTxtWinHost::GetAllowBeep()
{
    return fAllowBeep;
}

void CTxtWinHost::SetAllowBeep(BOOL _fAllowBeep)
{
    fAllowBeep = _fAllowBeep;

    pserv->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, _fAllowBeep ? TXTBIT_ALLOWBEEP : 0);
}

WORD CTxtWinHost::GetDefaultAlign()
{
    return pf.wAlignment;
}

void CTxtWinHost::SetDefaultAlign(WORD wNewAlign)
{
    pf.wAlignment = wNewAlign;

    // Notify control of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL CTxtWinHost::GetRichTextFlag()
{
    return fRich;
}

void CTxtWinHost::SetRichTextFlag(BOOL fNew)
{
    fRich = fNew;

    pserv->OnTxPropertyBitsChange(TXTBIT_RICHTEXT, fNew ? TXTBIT_RICHTEXT : 0);
}

LONG CTxtWinHost::GetDefaultLeftIndent()
{
    return pf.dxOffset;
}

void CTxtWinHost::SetDefaultLeftIndent(LONG lNewIndent)
{
    pf.dxOffset = lNewIndent;

    pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

void CTxtWinHost::SetClientRect(RECT *prc)
{
    rcClient = *prc;

    LONG xPerInch = ::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSX);
    LONG yPerInch = ::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSY);
    sizelExtent.cx = DXtoHimetricX(rcClient.right - rcClient.left, xPerInch);
    sizelExtent.cy = DYtoHimetricY(rcClient.bottom - rcClient.top, yPerInch);

    pserv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, TXTBIT_VIEWINSETCHANGE);
}

BOOL CTxtWinHost::SetSaveSelection(BOOL f_SaveSelection)
{
    BOOL fResult = f_SaveSelection;

    fSaveSelection = f_SaveSelection;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION, fSaveSelection ? TXTBIT_SAVESELECTION : 0);

    return fResult;
}

HRESULT	CTxtWinHost::OnTxInPlaceDeactivate()
{
    HRESULT hr = pserv->OnTxInPlaceDeactivate();

    if(SUCCEEDED(hr))
    {
        fInplaceActive = FALSE;
    }

    return hr;
}

HRESULT	CTxtWinHost::OnTxInPlaceActivate(LPCRECT prcClient)
{
    fInplaceActive = TRUE;

    HRESULT hr = pserv->OnTxInPlaceActivate(prcClient);

    if(FAILED(hr))
    {
        fInplaceActive = FALSE;
    }

    return hr;
}

BOOL CTxtWinHost::DoSetCursor(RECT *prc, POINT *pt)
{
    RECT rc = prc ? *prc : rcClient;

    // Is this in our rectangle?
    if(PtInRect(&rc, *pt))
    {
        RECT *prcClient = (!fInplaceActive || prc) ? &rc : nullptr;
        pserv->OnTxSetCursor(DVASPECT_CONTENT, -1, nullptr, nullptr, m_re->GetManager()->GetPaintDC(), NULL, prcClient, pt->x, pt->y);

        return TRUE;
    }

    return FALSE;
}

void CTxtWinHost::GetControlRect(LPRECT prc)
{
    prc->top = rcClient.top;
    prc->bottom = rcClient.bottom;
    prc->left = rcClient.left;
    prc->right = rcClient.right;
}

void CTxtWinHost::SetTransparent(BOOL f_Transparent)
{
    fTransparent = f_Transparent;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
}

LONG CTxtWinHost::SetAccelPos(LONG l_accelpos)
{
    LONG laccelposOld = l_accelpos;

    laccelpos = l_accelpos;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_SHOWACCELERATOR, 0);

    return laccelposOld;
}

WCHAR CTxtWinHost::SetPasswordChar(WCHAR ch_PasswordChar)
{
    WCHAR chOldPasswordChar = chPasswordChar;

    chPasswordChar = ch_PasswordChar;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, (chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);

    return chOldPasswordChar;
}

void CTxtWinHost::SetDisabled(BOOL fOn)
{
    cf.dwMask |= CFM_COLOR | CFM_DISABLED;
    cf.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;

    if(!fOn)
    {
        cf.dwEffects &= ~CFE_DISABLED;
    }

    pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
}

LONG CTxtWinHost::SetSelBarWidth(LONG l_SelBarWidth)
{
    LONG lm_nOldSelBarWidth = lSelBarWidth;

    lSelBarWidth = l_SelBarWidth;

    if(lSelBarWidth)
    {
        dwStyle |= ES_SELECTIONBAR;
    }
    else
    {
        dwStyle &= (~ES_SELECTIONBAR);
    }

    pserv->OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);

    return lm_nOldSelBarWidth;
}

BOOL CTxtWinHost::GetTimerState()
{
    return fTimer;
}

void CTxtWinHost::SetCharFormat(CHARFORMAT2W &c)
{
    cf = c;
}

void CTxtWinHost::SetParaFormat(PARAFORMAT2 &p)
{
    pf = p;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
IMPLEMENT_DUICONTROL(CRichEditUI)
CRichEditUI::CRichEditUI() :
    m_lTwhStyle(ES_MULTILINE)
{
    m_iLimitText = cInitTextMax;
#ifndef _UNICODE
    m_fAccumulateDBC = true;
#else
    m_fAccumulateDBC = false;
#endif
    ::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
}

CRichEditUI::~CRichEditUI()
{
    if(m_pTwh)
    {
        m_pTwh->Release();
        m_pManager->RemoveMessageFilter(this);
    }
    if(m_pRichEditOle)
    {
        m_pRichEditOle->Release();
    }

    if(m_pCallback)
    {
        m_pCallback->Release();
    }
}

faw::string_t CRichEditUI::GetClass() const
{
    return DUI_CTRL_RICHEDIT;
}

std::shared_ptr<CControlUI> CRichEditUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_RICHEDIT) return std::dynamic_pointer_cast<CRichEditUI>(shared_from_this());
    return CContainerUI::GetInterface(pstrName);
}

UINT CRichEditUI::GetControlFlags() const
{
    if(!IsEnabled()) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CRichEditUI::SetEnabled(bool bEnabled)
{
    CContainerUI::SetEnabled(bEnabled);
    if(m_pTwh)
    {
        if(IsEnabled())
        {
            m_pTwh->SetColor(GetTextColor());
        }
        else
        {
            m_pTwh->SetColor(m_pManager->GetDefaultDisabledColor());
        }
    }
}

bool CRichEditUI::IsMultiLine()
{
    return (m_lTwhStyle & ES_MULTILINE) == ES_MULTILINE;
}

void CRichEditUI::SetMultiLine(bool bMultiLine)
{
    if(!bMultiLine) m_lTwhStyle &= ~ES_MULTILINE;
    else  m_lTwhStyle |= ES_MULTILINE;
}

bool CRichEditUI::IsWantTab()
{
    return m_bWantTab;
}

void CRichEditUI::SetWantTab(bool bWantTab)
{
    m_bWantTab = bWantTab;
}

bool CRichEditUI::IsWantReturn()
{
    return m_bWantReturn;
}

void CRichEditUI::SetWantReturn(bool bWantReturn)
{
    m_bWantReturn = bWantReturn;
}

bool CRichEditUI::IsWantCtrlReturn()
{
    return m_bWantCtrlReturn;
}

void CRichEditUI::SetWantCtrlReturn(bool bWantCtrlReturn)
{
    m_bWantCtrlReturn = bWantCtrlReturn;
}

bool CRichEditUI::IsTransparent()
{
    return m_bTransparent;
}

void CRichEditUI::SetTransparent(bool bTransparent)
{
    m_bTransparent = bTransparent;
    if(m_pTwh) m_pTwh->SetTransparent(bTransparent);
}

bool CRichEditUI::IsRich()
{
    return m_bRich;
}

void CRichEditUI::SetRich(bool bRich)
{
    m_bRich = bRich;
    if(m_pTwh) m_pTwh->SetRichTextFlag(bRich);
}

bool CRichEditUI::IsReadOnly()
{
    return m_bReadOnly;
}

void CRichEditUI::SetReadOnly(bool bReadOnly)
{
    m_bReadOnly = bReadOnly;
    if(m_pTwh) m_pTwh->SetReadOnly(bReadOnly);
}

bool CRichEditUI::IsWordWrap()
{
    return m_bWordWrap;
}

void CRichEditUI::SetWordWrap(bool bWordWrap)
{
    m_bWordWrap = bWordWrap;
    if(m_pTwh) m_pTwh->SetWordWrap(bWordWrap);
}

int CRichEditUI::GetFont()
{
    return m_iFont;
}

void CRichEditUI::SetFont(int index)
{
    m_iFont = index;
    if(m_pTwh)
    {
        m_pTwh->SetFont(GetManager()->GetFont(m_iFont));
    }
}

void CRichEditUI::SetFont(faw::string_t pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    if(m_pTwh)
    {
        LOGFONT lf = { 0 };
        ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
        _tcsncpy(lf.lfFaceName, pStrFontName.data(), LF_FACESIZE);
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfHeight = -nSize;
        if(bBold) lf.lfWeight += FW_BOLD;
        if(bUnderline) lf.lfUnderline = TRUE;
        if(bItalic) lf.lfItalic = TRUE;
        HFONT hFont = ::CreateFontIndirect(&lf);
        if(!hFont) return;
        m_pTwh->SetFont(hFont);
        ::DeleteObject(hFont);
    }
}

LONG CRichEditUI::GetWinStyle()
{
    return m_lTwhStyle;
}

void CRichEditUI::SetWinStyle(LONG lStyle)
{
    m_lTwhStyle = lStyle;
}

DWORD CRichEditUI::GetTextColor()
{
    return m_dwTextColor;
}

void CRichEditUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
    if(m_pTwh)
    {
        m_pTwh->SetColor(dwTextColor);
    }
}

int CRichEditUI::GetLimitText()
{
    return m_iLimitText;
}

void CRichEditUI::SetLimitText(int iChars)
{
    m_iLimitText = iChars;
    if(m_pTwh)
    {
        m_pTwh->LimitText(m_iLimitText);
    }
}

long CRichEditUI::GetTextLength(DWORD dwFlags) const
{
    GETTEXTLENGTHEX textLenEx;
    textLenEx.flags = dwFlags;
#ifdef _UNICODE
    textLenEx.codepage = 1200;
#else
    textLenEx.codepage = CP_ACP;
#endif
    LRESULT lResult;
    TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM) &textLenEx, 0, &lResult);
    return (long) lResult;
}

faw::string_t CRichEditUI::GetText() const
{
    long lLen = GetTextLength(GTL_DEFAULT);
    LPTSTR lpText = nullptr;
    GETTEXTEX gt;
    gt.flags = GT_DEFAULT;
#ifdef _UNICODE
    gt.cb = sizeof(TCHAR) * (lLen + 1);
    gt.codepage = 1200;
    lpText = new TCHAR[lLen + 1];
    ::ZeroMemory(lpText, (lLen + 1) * sizeof(TCHAR));
#else
    gt.cb = sizeof(TCHAR) * lLen * 2 + 1;
    gt.codepage = CP_ACP;
    lpText = new TCHAR[lLen * 2 + 1];
    ::ZeroMemory(lpText, (lLen * 2 + 1) * sizeof(TCHAR));
#endif
    gt.lpDefaultChar = nullptr;
    gt.lpUsedDefChar = nullptr;
    TxSendMessage(EM_GETTEXTEX, (WPARAM) &gt, (LPARAM) lpText, 0);
    faw::string_t sText(lpText);
    delete[] lpText;
    return sText;
}

void CRichEditUI::SetText(faw::string_t pstrText)
{
    m_sText = pstrText;
    if(!m_pTwh) return;
    SetSel(0, -1);
    ReplaceSel(pstrText, FALSE);
}

bool CRichEditUI::IsModify() const
{
    if(!m_pTwh) return false;
    LRESULT lResult;
    TxSendMessage(EM_GETMODIFY, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

void CRichEditUI::SetModify(bool bModified) const
{
    TxSendMessage(EM_SETMODIFY, bModified, 0, 0);
}

void CRichEditUI::GetSel(CHARRANGE &cr) const
{
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM) &cr, 0);
}

void CRichEditUI::GetSel(long& nStartChar, long& nEndChar) const
{
    CHARRANGE cr;
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM) &cr, 0);
    nStartChar = cr.cpMin;
    nEndChar = cr.cpMax;
}

int CRichEditUI::SetSel(CHARRANGE &cr)
{
    LRESULT lResult;
    TxSendMessage(EM_EXSETSEL, 0, (LPARAM) &cr, &lResult);
    return (int) lResult;
}

int CRichEditUI::SetSel(long nStartChar, long nEndChar)
{
    CHARRANGE cr;
    cr.cpMin = nStartChar;
    cr.cpMax = nEndChar;
    LRESULT lResult;
    TxSendMessage(EM_EXSETSEL, 0, (LPARAM) &cr, &lResult);
    return (int) lResult;
}

void CRichEditUI::ReplaceSel(faw::string_t lpszNewText, bool bCanUndo)
{
#ifdef _UNICODE
    TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM) lpszNewText.data(), 0);
#else
    size_t iLen = _tcslen(lpszNewText.data());
    LPWSTR lpText = new WCHAR[iLen + 1];
    ::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
    ::MultiByteToWideChar(CP_ACP, 0, lpszNewText.data(), -1, (LPWSTR) lpText, (int) iLen);
    TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM) lpText, 0);
    delete[] lpText;
#endif
}

void CRichEditUI::ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo)
{
    TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM) lpszNewText, 0);
}

faw::string_t CRichEditUI::GetSelText() const
{
    if(!m_pTwh) return faw::string_t ();
    CHARRANGE cr;
    cr.cpMin = cr.cpMax = 0;
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM) &cr, 0);
    LPWSTR lpText = nullptr;
    lpText = new WCHAR[cr.cpMax - cr.cpMin + 1];
    ::ZeroMemory(lpText, (cr.cpMax - cr.cpMin + 1) * sizeof(WCHAR));
    TxSendMessage(EM_GETSELTEXT, 0, (LPARAM) lpText, 0);
    faw::string_t sText = FawTools::utf16_to_T(lpText);
    delete[] lpText;
    return sText;
}

int CRichEditUI::SetSelAll()
{
    return SetSel(0, -1);
}

int CRichEditUI::SetSelNone()
{
    return SetSel(-1, 0);
}

bool CRichEditUI::GetZoom(int& nNum, int& nDen) const
{
    LRESULT lResult;
    TxSendMessage(EM_GETZOOM, (WPARAM) &nNum, (LPARAM) &nDen, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::SetZoom(int nNum, int nDen)
{
    if(nNum < 0 || nNum > 64) return false;
    if(nDen < 0 || nDen > 64) return false;
    LRESULT lResult;
    TxSendMessage(EM_SETZOOM, nNum, nDen, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::SetZoomOff()
{
    LRESULT lResult;
    TxSendMessage(EM_SETZOOM, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

WORD CRichEditUI::GetSelectionType() const
{
    LRESULT lResult;
    TxSendMessage(EM_SELECTIONTYPE, 0, 0, &lResult);
    return (WORD) lResult;
}

bool CRichEditUI::GetAutoURLDetect() const
{
    LRESULT lResult;
    TxSendMessage(EM_GETAUTOURLDETECT, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::SetAutoURLDetect(bool bAutoDetect)
{
    LRESULT lResult;
    TxSendMessage(EM_AUTOURLDETECT, bAutoDetect, 0, &lResult);
    return (BOOL) lResult == FALSE;
}

DWORD CRichEditUI::GetEventMask() const
{
    LRESULT lResult;
    TxSendMessage(EM_GETEVENTMASK, 0, 0, &lResult);
    return (DWORD) lResult;
}

DWORD CRichEditUI::SetEventMask(DWORD dwEventMask)
{
    LRESULT lResult;
    TxSendMessage(EM_SETEVENTMASK, 0, dwEventMask, &lResult);
    return (DWORD) lResult;
}

faw::string_t CRichEditUI::GetTextRange(long nStartChar, long nEndChar) const
{
    TEXTRANGEW tr = { 0 };
    tr.chrg.cpMin = nStartChar;
    tr.chrg.cpMax = nEndChar;
    LPWSTR lpText = nullptr;
    lpText = new WCHAR[nEndChar - nStartChar + 1];
    ::ZeroMemory(lpText, (nEndChar - nStartChar + 1) * sizeof(WCHAR));
    tr.lpstrText = lpText;
    TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM) &tr, 0);
    faw::string_t sText = FawTools::utf16_to_T(lpText);
    delete[] lpText;
    return sText;
}

void CRichEditUI::HideSelection(bool bHide, bool bChangeStyle)
{
    TxSendMessage(EM_HIDESELECTION, bHide, bChangeStyle, 0);
}

void CRichEditUI::ScrollCaret()
{
    TxSendMessage(EM_SCROLLCARET, 0, 0, 0);
}

int CRichEditUI::InsertText(long nInsertAfterChar, faw::string_t lpstrText, bool bCanUndo)
{
    int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
    ReplaceSel(lpstrText, bCanUndo);
    return nRet;
}

int CRichEditUI::AppendText(faw::string_t lpstrText, bool bCanUndo)
{
    int nRet = SetSel(-1, -1);
    ReplaceSel(lpstrText, bCanUndo);
    return nRet;
}

DWORD CRichEditUI::GetDefaultCharFormat(CHARFORMAT2 &cf) const
{
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETCHARFORMAT, 0, (LPARAM) &cf, &lResult);
    return (DWORD) lResult;
}

bool CRichEditUI::SetDefaultCharFormat(CHARFORMAT2 &cf)
{
    if(!m_pTwh) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, 0, (LPARAM) &cf, &lResult);
    if((BOOL) lResult == TRUE)
    {
        CHARFORMAT2W cfw;
        cfw.cbSize = sizeof(CHARFORMAT2W);
        TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM) &cfw, 0);
        m_pTwh->SetCharFormat(cfw);
        return true;
    }
    return false;
}

DWORD CRichEditUI::GetSelectionCharFormat(CHARFORMAT2 &cf) const
{
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM) &cf, &lResult);
    return (DWORD) lResult;
}

bool CRichEditUI::SetSelectionCharFormat(CHARFORMAT2 &cf)
{
    if(!m_pTwh) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::SetWordCharFormat(CHARFORMAT2 &cf)
{
    if(!m_pTwh) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf, &lResult);
    return (BOOL) lResult == TRUE;
}

DWORD CRichEditUI::GetParaFormat(PARAFORMAT2 &pf) const
{
    pf.cbSize = sizeof(PARAFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM) &pf, &lResult);
    return (DWORD) lResult;
}

bool CRichEditUI::SetParaFormat(PARAFORMAT2 &pf)
{
    if(!m_pTwh) return false;
    pf.cbSize = sizeof(PARAFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM) &pf, &lResult);
    if((BOOL) lResult == TRUE)
    {
        m_pTwh->SetParaFormat(pf);
        return true;
    }
    return false;
}

bool CRichEditUI::CanUndo()
{
    if(!m_pTwh) return false;
    LRESULT lResult;
    TxSendMessage(EM_CANUNDO, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::CanRedo()
{
    if(!m_pTwh) return false;
    LRESULT lResult;
    TxSendMessage(EM_CANREDO, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::CanPaste()
{
    if(!m_pTwh) return false;
    LRESULT lResult;
    TxSendMessage(EM_CANPASTE, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}
bool CRichEditUI::Redo()
{
    if(!m_pTwh) return false;
    LRESULT lResult;
    TxSendMessage(EM_REDO, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

bool CRichEditUI::Undo()
{
    if(!m_pTwh) return false;
    LRESULT lResult;
    TxSendMessage(EM_UNDO, 0, 0, &lResult);
    return (BOOL) lResult == TRUE;
}

void CRichEditUI::Clear()
{
    TxSendMessage(WM_CLEAR, 0, 0, 0);
}

void CRichEditUI::Copy()
{
    TxSendMessage(WM_COPY, 0, 0, 0);
}

void CRichEditUI::Cut()
{
    TxSendMessage(WM_CUT, 0, 0, 0);
}

void CRichEditUI::Paste()
{
    TxSendMessage(WM_PASTE, 0, 0, 0);
}

int CRichEditUI::GetLineCount() const
{
    if(!m_pTwh) return 0;
    LRESULT lResult;
    TxSendMessage(EM_GETLINECOUNT, 0, 0, &lResult);
    return (int) lResult;
}

faw::string_t CRichEditUI::GetLine(int nIndex, int nMaxLength) const
{
    LPWSTR lpText = nullptr;
    lpText = new WCHAR[nMaxLength + 1];
    ::ZeroMemory(lpText, (nMaxLength + 1) * sizeof(WCHAR));
    *(LPWORD) lpText = (WORD) nMaxLength;
    TxSendMessage(EM_GETLINE, nIndex, (LPARAM) lpText, 0);
    faw::string_t sText = FawTools::utf16_to_T(lpText);
    delete[] lpText;
    return sText;
}

int CRichEditUI::LineIndex(int nLine) const
{
    LRESULT lResult;
    TxSendMessage(EM_LINEINDEX, nLine, 0, &lResult);
    return (int) lResult;
}

int CRichEditUI::LineLength(int nLine) const
{
    LRESULT lResult;
    TxSendMessage(EM_LINELENGTH, nLine, 0, &lResult);
    return (int) lResult;
}

bool CRichEditUI::LineScroll(int nLines, int nChars)
{
    LRESULT lResult;
    TxSendMessage(EM_LINESCROLL, nChars, nLines, &lResult);
    return (BOOL) lResult == TRUE;
}

POINT CRichEditUI::GetCharPos(long lChar) const
{
    POINT pt = { 0 };
    TxSendMessage(EM_POSFROMCHAR, (WPARAM) &pt, (LPARAM) lChar, 0);
    return pt;
}

long CRichEditUI::LineFromChar(long nIndex) const
{
    if(!m_pTwh) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_EXLINEFROMCHAR, 0, nIndex, &lResult);
    return (long) lResult;
}

POINT CRichEditUI::PosFromChar(UINT nChar) const
{
    POINT pt = { 0 };
    TxSendMessage(EM_POSFROMCHAR, (WPARAM) &pt, nChar, 0);
    return { pt.x, pt.y };
}

int CRichEditUI::CharFromPos(POINT pt) const
{
    if(!m_pTwh) return 0;
    LRESULT lResult;
    TxSendMessage(EM_CHARFROMPOS, 0, (LPARAM) &pt, &lResult);
    return (int) lResult;
}

void CRichEditUI::EmptyUndoBuffer()
{
    TxSendMessage(EM_EMPTYUNDOBUFFER, 0, 0, 0);
}

UINT CRichEditUI::SetUndoLimit(UINT nLimit)
{
    if(!m_pTwh) return 0;
    LRESULT lResult;
    TxSendMessage(EM_SETUNDOLIMIT, (WPARAM) nLimit, 0, &lResult);
    return (UINT) lResult;
}

long CRichEditUI::StreamIn(int nFormat, EDITSTREAM &es)
{
    if(!m_pTwh) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_STREAMIN, nFormat, (LPARAM) &es, &lResult);
    return (long) lResult;
}

long CRichEditUI::StreamOut(int nFormat, EDITSTREAM &es)
{
    if(!m_pTwh) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_STREAMOUT, nFormat, (LPARAM) &es, &lResult);
    return (long) lResult;
}

void CRichEditUI::SetAccumulateDBCMode(bool bDBCMode)
{
    m_fAccumulateDBC = bDBCMode;
}

bool CRichEditUI::IsAccumulateDBCMode()
{
    return m_fAccumulateDBC;
}

void CRichEditUI::DoInit()
{
    if(m_bInited)
        return;

    CREATESTRUCT cs;
    cs.style = m_lTwhStyle;
    cs.x = 0;
    cs.y = 0;
    cs.cy = 0;
    cs.cx = 0;
    cs.lpszName = m_sText.c_str();
    auto self = std::dynamic_pointer_cast<CRichEditUI>(shared_from_this());
    CreateHost(self, &cs, &m_pTwh);
    if(m_pTwh)
    {
        if(m_bTransparent) m_pTwh->SetTransparent(TRUE);
        LRESULT lResult;
        m_pTwh->GetTextServices()->TxSendMessage(EM_SETLANGOPTIONS, 0, 0, &lResult);
        m_pTwh->GetTextServices()->TxSendMessage(EM_SETEVENTMASK, 0, ENM_DROPFILES | ENM_LINK | ENM_CHANGE, &lResult);
        m_pTwh->OnTxInPlaceActivate(nullptr);
        m_pManager->AddMessageFilter(this);
        m_pManager->SetTimer(shared_from_this(), RICHEDIT_TIMER_ID, ::GetCaretBlinkTime());
        if(!m_bEnabled)
        {
            m_pTwh->SetColor(m_pManager->GetDefaultDisabledColor());
        }
    }

    IRichEditOleCallback *pCallback = new CRichEditOleCallback(this);
    SetOLECallback(pCallback);

    m_bInited= true;
}

HRESULT CRichEditUI::TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const
{
    if(m_pTwh)
    {
        if(msg == WM_KEYDOWN && TCHAR(wparam) == VK_RETURN)
        {
            if(m_bWantReturn || (::GetKeyState(VK_CONTROL) < 0 && m_bWantCtrlReturn))
            {
                auto self = std::dynamic_pointer_cast<CRichEditUI>(std::const_pointer_cast<CControlUI>(shared_from_this()));
                if(m_pManager) m_pManager->SendNotify(self, DUI_MSGTYPE_RETURN);
                return S_OK;
            }

        }
        return m_pTwh->GetTextServices()->TxSendMessage(msg, wparam, lparam, plresult);
    }
    return S_FALSE;
}

IDropTarget* CRichEditUI::GetTxDropTarget()
{
    IDropTarget *pdt = nullptr;
    if(m_pTwh->GetTextServices()->TxGetDropTarget(&pdt) == NOERROR) return pdt;
    return nullptr;
}

bool CRichEditUI::OnTxViewChanged()
{
    return true;
}

bool CRichEditUI::SetDropAcceptFile(bool bAccept)
{
    LRESULT lResult;
    TxSendMessage(EM_SETEVENTMASK, 0, ENM_DROPFILES | ENM_LINK,  // ENM_CHANGE| ENM_CORRECTTEXT | ENM_DRAGDROPDONE | ENM_DROPFILES | ENM_IMECHANGE | ENM_LINK | ENM_OBJECTPOSITIONS | ENM_PROTECTED | ENM_REQUESTRESIZE | ENM_SCROLL | ENM_SELCHANGE | ENM_UPDATE,
                  &lResult);
    return (BOOL) lResult == FALSE;
}

void CRichEditUI::OnTxNotify(DWORD iNotify, void *pv)
{
    switch(iNotify)
    {
    case EN_CHANGE:
    {
        GetManager()->SendNotify(shared_from_this(), DUI_MSGTYPE_TEXTCHANGED);
        break;
    }
    case EN_DROPFILES:
    case EN_MSGFILTER:
    case EN_OLEOPFAILED:
    case EN_PROTECTED:
    case EN_SAVECLIPBOARD:
    case EN_SELCHANGE:
    case EN_STOPNOUNDO:
    case EN_LINK:
    case EN_OBJECTPOSITIONS:
    case EN_DRAGDROPDONE:
    {
        if(pv)                         // Fill out NMHDR portion of pv
        {
            LONG nId = GetWindowLong(this->GetManager()->GetPaintWindow(), GWL_ID);
            NMHDR  *phdr = (NMHDR *) pv;
            phdr->hwndFrom = this->GetManager()->GetPaintWindow();
            phdr->idFrom = nId;
            phdr->code = iNotify;

            if(SendMessage(this->GetManager()->GetPaintWindow(), WM_NOTIFY, (WPARAM) nId, (LPARAM) pv))
            {
                //hr = S_FALSE;
            }
        }
    }
    break;
    }
}

// 多行非rich格式的richedit有一个滚动条bug，在最后一行是空行时，LineDown和SetScrollPos无法滚动到最后
// 引入iPos就是为了修正这个bug
void CRichEditUI::SetScrollPos(SIZE szPos, bool bMsg)
{
    int cx = 0;
    int cy = 0;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
        m_pVerticalScrollBar->SetScrollPos(szPos.cy);
        cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
        m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
        cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if(cy != 0)
    {
        int iPos = 0;
        if(m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
            iPos = m_pVerticalScrollBar->GetScrollPos();
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pVerticalScrollBar->GetScrollPos());
        TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
        if(m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
        {
            if(cy > 0 && m_pVerticalScrollBar->GetScrollPos() <= iPos)
                m_pVerticalScrollBar->SetScrollPos(iPos);
        }
    }
    if(cx != 0)
    {
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pHorizontalScrollBar->GetScrollPos());
        TxSendMessage(WM_HSCROLL, wParam, 0L, 0);
    }
}

void CRichEditUI::LineUp()
{
    TxSendMessage(WM_VSCROLL, SB_LINEUP, 0L, 0);
}

void CRichEditUI::LineDown()
{
    int iPos = 0;
    if(m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
        iPos = m_pVerticalScrollBar->GetScrollPos();
    TxSendMessage(WM_VSCROLL, SB_LINEDOWN, 0L, 0);
    if(m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        if(m_pVerticalScrollBar->GetScrollPos() <= iPos)
            m_pVerticalScrollBar->SetScrollPos(m_pVerticalScrollBar->GetScrollRange());
    }
}

void CRichEditUI::PageUp()
{
    TxSendMessage(WM_VSCROLL, SB_PAGEUP, 0L, 0);
}

void CRichEditUI::PageDown()
{
    TxSendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L, 0);
}

void CRichEditUI::HomeUp()
{
    TxSendMessage(WM_VSCROLL, SB_TOP, 0L, 0);
}

void CRichEditUI::EndDown()
{
    TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
}

void CRichEditUI::LineLeft()
{
    TxSendMessage(WM_HSCROLL, SB_LINELEFT, 0L, 0);
}

void CRichEditUI::LineRight()
{
    TxSendMessage(WM_HSCROLL, SB_LINERIGHT, 0L, 0);
}

void CRichEditUI::PageLeft()
{
    TxSendMessage(WM_HSCROLL, SB_PAGELEFT, 0L, 0);
}

void CRichEditUI::PageRight()
{
    TxSendMessage(WM_HSCROLL, SB_PAGERIGHT, 0L, 0);
}

void CRichEditUI::HomeLeft()
{
    TxSendMessage(WM_HSCROLL, SB_LEFT, 0L, 0);
}

void CRichEditUI::EndRight()
{
    TxSendMessage(WM_HSCROLL, SB_RIGHT, 0L, 0);
}

void CRichEditUI::DoEvent(TEventUI& event)
{
    if(!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if(m_pParent) m_pParent->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if(event.Type == UIEVENT_SETCURSOR && IsEnabled())
    {
        if(m_pTwh && m_pTwh->DoSetCursor(nullptr, &event.ptMouse))
        {
            return;
        }
    }
    else if(event.Type == UIEVENT_WINDOWSIZE)
    {
        if(m_pTwh) m_pTwh->NeedFreshCaret();
    }
    else if(event.Type == UIEVENT_SETFOCUS)
    {
        if(m_pTwh)
        {
            m_pTwh->OnTxInPlaceActivate(nullptr);
            m_pTwh->GetTextServices()->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
        }
        m_bFocused = true;
        Invalidate();
        return;
    }
    if(event.Type == UIEVENT_KILLFOCUS)
    {
        if(m_pTwh)
        {
            m_pTwh->OnTxInPlaceActivate(nullptr);
            m_pTwh->GetTextServices()->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
        }
        m_bFocused = false;
        Invalidate();
        return;
    }
    else if(event.Type == UIEVENT_TIMER)
    {
        if(event.wParam == RICHEDIT_TIMER_ID)
        {
            if(m_pManager->IsLayered() && IsFocused() && m_pTwh && m_pTwh->IsShowCaret())
            {
                if(::GetFocus() != m_pManager->GetPaintWindow()) return;
                m_bDrawCaret = !m_bDrawCaret;
                POINT ptCaret = { 0 };
                ::GetCaretPos(&ptCaret);
                RECT rcCaret = { ptCaret.x, ptCaret.y, ptCaret.x + m_pTwh->GetCaretWidth(), ptCaret.y + m_pTwh->GetCaretHeight() };
                RECT rcTemp = rcCaret;
                if(!::IntersectRect(&rcCaret, &rcTemp, &m_rcItem)) return;
                std::shared_ptr<CControlUI> pParent = shared_from_this();
                RECT rcParent = { 0 };
                while(!!(pParent = pParent->GetParent()))
                {
                    rcTemp = rcCaret;
                    rcParent = pParent->GetPos();
                    if(!::IntersectRect(&rcCaret, &rcTemp, &rcParent))
                        return;
                }
                m_pManager->Invalidate(rcCaret);
            }
            else if(IsFocused() && m_pTwh)
            {
                if(::GetFocus() != m_pManager->GetPaintWindow()) return;
                if(m_pTwh->IsShowCaret()) m_pTwh->TxShowCaret(FALSE);
                else m_pTwh->TxShowCaret(TRUE);
            }
            return;
        }
        else if(m_pTwh)
        {
            m_pTwh->GetTextServices()->TxSendMessage(WM_TIMER, event.wParam, event.lParam, 0);
        }
        return;
    }
    if(event.Type == UIEVENT_SCROLLWHEEL)
    {
        if((event.wKeyState & MK_CONTROL) != 0)
        {
            return;
        }
    }
    if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
    {
        return;
    }
    if(event.Type == UIEVENT_MOUSEMOVE)
    {
        return;
    }
    if(event.Type == UIEVENT_BUTTONUP)
    {
        return;
    }
    if(event.Type > UIEVENT__KEYBEGIN && event.Type < UIEVENT__KEYEND)
    {
        return;
    }
    CContainerUI::DoEvent(event);
}

SIZE CRichEditUI::EstimateSize(SIZE szAvailable)
{
    return CContainerUI::EstimateSize(szAvailable);
}

void CRichEditUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    RECT rcScrollView = rc;
    bool bVScrollBarVisiable = false;
    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        bVScrollBarVisiable = true;
        rc.top -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom += m_pVerticalScrollBar->GetScrollRange();
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        rcScrollView.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        rc.left -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right += m_pHorizontalScrollBar->GetScrollRange();
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
        rcScrollView.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }

    if(m_pTwh)
    {
        RECT rcScrollTextView = rcScrollView;
        rcScrollTextView.left += m_rcTextPadding.left;
        rcScrollTextView.right -= m_rcTextPadding.right;
        rcScrollTextView.top += m_rcTextPadding.top;
        rcScrollTextView.bottom -= m_rcTextPadding.bottom;
        RECT rcText = rc;
        rcText.left += m_rcTextPadding.left;
        rcText.right -= m_rcTextPadding.right;
        rcText.top += m_rcTextPadding.top;
        rcText.bottom -= m_rcTextPadding.bottom;
        m_pTwh->SetClientRect(&rcScrollTextView);
        if(bVScrollBarVisiable && (!m_pVerticalScrollBar->IsVisible() || m_bVScrollBarFixing))
        {
            LONG lWidth = rcText.right - rcText.left + m_pVerticalScrollBar->GetFixedWidth();
            LONG lHeight = 0;
            SIZEL szExtent = { -1, -1 };
            m_pTwh->GetTextServices()->TxGetNaturalSize(
                DVASPECT_CONTENT,
                GetManager()->GetPaintDC(),
                nullptr,
                nullptr,
                TXTNS_FITTOCONTENT,
                &szExtent,
                &lWidth,
                &lHeight);
            if(lHeight > rcText.bottom - rcText.top)
            {
                m_pVerticalScrollBar->SetVisible(true);
                m_pVerticalScrollBar->SetScrollPos(0);
                m_bVScrollBarFixing = true;
            }
            else
            {
                if(m_bVScrollBarFixing)
                {
                    m_pVerticalScrollBar->SetVisible(false);
                    m_bVScrollBarFixing = false;
                }
            }
        }
    }

    if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    {
        RECT rcScrollBarPos = { rcScrollView.right, rcScrollView.top,
                                rcScrollView.right + m_pVerticalScrollBar->GetFixedWidth(), rcScrollView.bottom
                              };
        m_pVerticalScrollBar->SetPos(rcScrollBarPos, false);
    }
    if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    {
        RECT rcScrollBarPos = { rcScrollView.left, rcScrollView.bottom, rcScrollView.right,
                                rcScrollView.bottom + m_pHorizontalScrollBar->GetFixedHeight()
                              };
        m_pHorizontalScrollBar->SetPos(rcScrollBarPos, false);
    }
    int Index = -1;
    for(auto& pItem : m_items)
    {
        Index++;
        std::shared_ptr<CControlUI> pControl = pItem;
        if(!pControl->IsVisible()) continue;
        if(pControl->IsFloat())
        {
            SetFloatPos(Index);
        }
        else
        {
            SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
            if(sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if(sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            if(sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if(sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
            RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy };
            pControl->SetPos(rcCtrl, false);
        }
    }
}

void CRichEditUI::Move(SIZE szOffset, bool bNeedInvalidate)
{
    CContainerUI::Move(szOffset, bNeedInvalidate);
    if(m_pTwh)
    {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;

        if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
        m_pTwh->SetClientRect(&rc);
    }
}

// TODO
bool CRichEditUI::DoPaint(HDC hDC, const RECT& rcPaint, std::shared_ptr<CControlUI> pStopControl)
{
    RECT rcTemp = { 0 };
    if(!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) return true;

    CRenderClip clip;
    CRenderClip::GenerateClip(hDC, rcTemp, clip);
    CControlUI::DoPaint(hDC, rcPaint, pStopControl);

    if(m_pTwh)
    {
        RECT rc = { 0 };
        m_pTwh->GetControlRect(&rc);
        // Remember wparam is actually the hdc and lparam is the update
        // rect because this message has been preprocessed by the window.
        m_pTwh->GetTextServices()->TxDraw(
            DVASPECT_CONTENT,  		// Draw Aspect
            /*-1*/0,				// Lindex
            nullptr,				// Info for drawing optimazation
            nullptr,				// target device information
            hDC,			        // Draw device HDC
            nullptr,				// Target device HDC
            (RECTL*) &rc,			// Bounding client rectangle
            nullptr,				// Clipping rectangle for metafiles
            (RECT*) &rcPaint,		// Update rectangle
            nullptr, 	   			// Call back function
            0,						// Call back parameter
            0);				        // What view of the object
        if(m_bVScrollBarFixing)
        {
            LONG lWidth = rc.right - rc.left + m_pVerticalScrollBar->GetFixedWidth();
            LONG lHeight = 0;
            SIZEL szExtent = { -1, -1 };
            m_pTwh->GetTextServices()->TxGetNaturalSize(
                DVASPECT_CONTENT,
                GetManager()->GetPaintDC(),
                nullptr,
                nullptr,
                TXTNS_FITTOCONTENT,
                &szExtent,
                &lWidth,
                &lHeight);
            if(lHeight <= rc.bottom - rc.top)
            {
                NeedUpdate();
            }
        }
    }

    if(m_items.size() > 0)
    {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;
        if(m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if(m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

        if(!::IntersectRect(&rcTemp, &rcPaint, &rc))
        {
            for(auto& pItem : m_items)
            {
                std::shared_ptr<CControlUI> pControl = pItem;
                if(pControl == pStopControl) return false;
                if(!pControl->IsVisible()) continue;
                if(!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) continue;
                if(pControl->IsFloat())
                {
                    if(!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos())) continue;
                    if(!pControl->Paint(hDC, rcPaint, pStopControl)) return false;
                }
            }
        }
        else
        {
            CRenderClip childClip;
            CRenderClip::GenerateClip(hDC, rcTemp, childClip);
            int Index = -1;
            for(auto& pItem : m_items)
            {
                Index++;
                std::shared_ptr<CControlUI> pControl = pItem;
                if(pControl == pStopControl) return false;
                if(!pControl->IsVisible()) continue;
                if(!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) continue;
                if(pControl->IsFloat())
                {
                    if(!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos())) continue;
                    CRenderClip::UseOldClipBegin(hDC, childClip);
                    if(!pControl->Paint(hDC, rcPaint, pStopControl)) return false;
                    CRenderClip::UseOldClipEnd(hDC, childClip);
                }
                else
                {
                    if(!::IntersectRect(&rcTemp, &rc, &pControl->GetPos())) continue;
                    if(!pControl->Paint(hDC, rcPaint, pStopControl)) return false;
                }
            }
        }
    }

    if(m_pManager->IsLayered() && IsFocused() && m_pTwh && m_pTwh->IsShowCaret())
    {
        if(m_bDrawCaret)
        {
            POINT ptCaret;
            ::GetCaretPos(&ptCaret);
            if(::PtInRect(&m_rcItem, ptCaret))
            {
                RECT rcCaret = { ptCaret.x, ptCaret.y, ptCaret.x, ptCaret.y + m_pTwh->GetCaretHeight() };
                DWORD dwTextColor = GetTextColor();
                CRenderEngine::DrawLine(hDC, rcCaret, m_pTwh->GetCaretWidth(), dwTextColor);
            }
        }
    }

    if(m_pVerticalScrollBar)
    {
        if(m_pVerticalScrollBar == pStopControl) return false;
        if(m_pVerticalScrollBar->IsVisible())
        {
            if(::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()))
            {
                if(!m_pVerticalScrollBar->Paint(hDC, rcPaint, pStopControl)) return false;
            }
        }
    }

    if(m_pHorizontalScrollBar)
    {
        if(m_pHorizontalScrollBar == pStopControl) return false;
        if(m_pHorizontalScrollBar->IsVisible())
        {
            if(::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()))
            {
                if(!m_pHorizontalScrollBar->Paint(hDC, rcPaint, pStopControl)) return false;
            }
        }
    }
    // 绘制提示文字
    faw::string_t sDrawText = GetText();
    if(sDrawText.empty() && !m_bFocused)
    {
        DWORD dwTextColor = GetTipValueColor();
        faw::string_t sTipValue = GetTipValue();
        RECT rc = m_rcItem;
        rc.left += m_rcTextPadding.left;
        rc.right -= m_rcTextPadding.right;
        rc.top += m_rcTextPadding.top;
        rc.bottom -= m_rcTextPadding.bottom;
        UINT uTextAlign = GetTipValueAlign();
        if(IsMultiLine()) uTextAlign |= DT_TOP;
        else uTextAlign |= DT_VCENTER;
        CRenderEngine::DrawText(hDC, m_pManager, rc, sTipValue, dwTextColor, m_iFont, uTextAlign);
    }
    return true;
}

faw::string_t CRichEditUI::GetNormalImage()
{
    return m_sNormalImage;
}

void CRichEditUI::SetNormalImage(faw::string_t pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

faw::string_t CRichEditUI::GetHotImage()
{
    return m_sHotImage;
}

void CRichEditUI::SetHotImage(faw::string_t pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

faw::string_t CRichEditUI::GetFocusedImage()
{
    return m_sFocusedImage;
}

void CRichEditUI::SetFocusedImage(faw::string_t pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

faw::string_t CRichEditUI::GetDisabledImage()
{
    return m_sDisabledImage;
}

void CRichEditUI::SetDisabledImage(faw::string_t pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

RECT CRichEditUI::GetTextPadding() const
{
    return m_rcTextPadding;
}

void CRichEditUI::SetTextPadding(RECT rc)
{
    m_rcTextPadding = rc;
    Invalidate();
}

void CRichEditUI::SetTipValue(faw::string_t pStrTipValue)
{
    m_sTipValue = pStrTipValue;
    Invalidate();
}

faw::string_t CRichEditUI::GetTipValue()
{
    return m_sTipValue;
}

void CRichEditUI::SetTipValueColor(faw::string_t pStrColor)
{
    m_dwTipValueColor = (DWORD) FawTools::parse_hex(pStrColor);
    Invalidate();
}

DWORD CRichEditUI::GetTipValueColor()
{
    return m_dwTipValueColor;
}

void CRichEditUI::SetTipValueAlign(UINT uAlign)
{
    m_uTipValueAlign = uAlign;
    if(GetText().empty()) Invalidate();
}

UINT CRichEditUI::GetTipValueAlign()
{
    return m_uTipValueAlign;
}

void CRichEditUI::PaintStatusImage(HDC hDC)
{
    if(IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~UISTATE_FOCUSED;
    if(!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~UISTATE_DISABLED;

    if((m_uButtonState & UISTATE_DISABLED) != 0)
    {
        if(!m_sDisabledImage.empty())
        {
            if(!DrawImage(hDC, m_sDisabledImage))
            {
            }
            else return;
        }
    }
    else if((m_uButtonState & UISTATE_FOCUSED) != 0)
    {
        if(!m_sFocusedImage.empty())
        {
            if(!DrawImage(hDC, m_sFocusedImage))
            {
            }
            else return;
        }
    }
    else if((m_uButtonState & UISTATE_HOT) != 0)
    {
        if(!m_sHotImage.empty())
        {
            if(!DrawImage(hDC, m_sHotImage))
            {
            }
            else return;
        }
    }

    if(!m_sNormalImage.empty())
    {
        if(!DrawImage(hDC, m_sNormalImage))
        {
        }
        else return;
    }
}

void CRichEditUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("vscrollbar"))
    {
        if(FawTools::parse_bool(pstrValue)) m_lTwhStyle |= ES_DISABLENOSCROLL | WS_VSCROLL;
    }
    if(pstrName == _T("autovscroll"))
    {
        if(FawTools::parse_bool(pstrValue)) m_lTwhStyle |= ES_AUTOVSCROLL;
    }
    else if(pstrName == _T("hscrollbar"))
    {
        if(FawTools::parse_bool(pstrValue)) m_lTwhStyle |= ES_DISABLENOSCROLL | WS_HSCROLL;
    }
    if(pstrName == _T("autohscroll"))
    {
        if(FawTools::parse_bool(pstrValue))
            m_lTwhStyle |= ES_AUTOHSCROLL;
        else
            m_lTwhStyle &= ~ES_AUTOHSCROLL;
    }
    else if(pstrName == _T("multiline"))
    {
        SetMultiLine(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("wanttab"))
    {
        SetWantTab(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("wantreturn"))
    {
        SetWantReturn(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("wantctrlreturn"))
    {
        SetWantCtrlReturn(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("transparent"))
    {
        SetTransparent(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("rich"))
    {
        SetRich(FawTools::parse_bool(pstrValue));
    }
    else if(pstrName == _T("readonly"))
    {
        if(FawTools::parse_bool(pstrValue))
        {
            m_lTwhStyle |= ES_READONLY;
            m_bReadOnly = true;
        }
    }
    else if(pstrName == _T("password"))
    {
        if(FawTools::parse_bool(pstrValue)) m_lTwhStyle |= ES_PASSWORD;
    }
    else if(pstrName == _T("align"))
    {
        if(pstrValue.find(_T("left")) != faw::string_t::npos)
        {
            m_lTwhStyle &= ~(ES_CENTER | ES_RIGHT);
            m_lTwhStyle |= ES_LEFT;
        }
        if(pstrValue.find(_T("center")) != faw::string_t::npos)
        {
            m_lTwhStyle &= ~(ES_LEFT | ES_RIGHT);
            m_lTwhStyle |= ES_CENTER;
        }
        if(pstrValue.find(_T("right")) != faw::string_t::npos)
        {
            m_lTwhStyle &= ~(ES_LEFT | ES_CENTER);
            m_lTwhStyle |= ES_RIGHT;
        }
    }
    else if(pstrName == _T("font")) SetFont(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("textcolor")) SetTextColor((DWORD) FawTools::parse_hex(pstrValue));
    else if(pstrName == _T("maxchar")) SetLimitText(FawTools::parse_dec(pstrValue));
    else if(pstrName == _T("normalimage")) SetNormalImage(pstrValue);
    else if(pstrName == _T("hotimage")) SetHotImage(pstrValue);
    else if(pstrName == _T("focusedimage")) SetFocusedImage(pstrValue);
    else if(pstrName == _T("disabledimage")) SetDisabledImage(pstrValue);
    else if(pstrName == _T("textpadding")) SetTextPadding(FawTools::parse_rect(pstrValue));
    else if(pstrName == _T("tipvalue")) SetTipValue(pstrValue);
    else if(pstrName == _T("tipvaluecolor")) SetTipValueColor(pstrValue);
    else if(pstrName == _T("tipvaluealign"))
    {
        if(pstrValue.find(_T("left")) != faw::string_t::npos)
        {
            m_uTipValueAlign = DT_SINGLELINE | DT_LEFT;
        }
        if(pstrValue.find(_T("center")) != faw::string_t::npos)
        {
            m_uTipValueAlign = DT_SINGLELINE | DT_CENTER;
        }
        if(pstrValue.find(_T("right")) != faw::string_t::npos)
        {
            m_uTipValueAlign = DT_SINGLELINE | DT_RIGHT;
        }
    }
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

std::optional<LRESULT> CRichEditUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(!IsVisible() || !IsEnabled()) return std::nullopt;
    if(!IsMouseEnabled() && uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) return std::nullopt;
    if(uMsg == WM_MOUSEWHEEL && (LOWORD(wParam) & MK_CONTROL) == 0) return std::nullopt;

    if(uMsg == WM_IME_COMPOSITION)
    {
        // 解决微软输入法位置异常的问题
        HIMC hIMC = ImmGetContext(GetManager()->GetPaintWindow());
        if(hIMC)
        {
            POINT point = { 0 };
            GetCaretPos(&point);

            COMPOSITIONFORM Composition;
            Composition.dwStyle = CFS_POINT;
            Composition.ptCurrentPos.x = point.x;
            Composition.ptCurrentPos.y = point.y;
            ImmSetCompositionWindow(hIMC, &Composition);

            ImmReleaseContext(GetManager()->GetPaintWindow(), hIMC);
        }

        return std::nullopt;
    }
    std::optional<LRESULT> lRes = 0;
    if((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_SETCURSOR)
    {
        if(!m_pTwh->IsCaptured())
        {
            switch(uMsg)
            {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MOUSEMOVE:
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                std::shared_ptr<CControlUI> pHover = GetManager()->FindControl(pt);
                if(pHover != shared_from_this())
                {
                    return std::nullopt;
                }
            }
            break;
            }
        }
        // Mouse message only go when captured or inside rect
        DWORD dwHitResult = m_pTwh->IsCaptured() ? HITRESULT_HIT : HITRESULT_OUTSIDE;
        if(dwHitResult == HITRESULT_OUTSIDE)
        {
            RECT rc = { 0 };
            m_pTwh->GetControlRect(&rc);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if(uMsg == WM_SETCURSOR)
            {
                ::GetCursorPos(&pt);
                ::ScreenToClient(GetManager()->GetPaintWindow(), &pt);
            }
            else if(uMsg == WM_MOUSEWHEEL) ::ScreenToClient(GetManager()->GetPaintWindow(), &pt);
            if(::PtInRect(&rc, pt) && !GetManager()->IsCaptured()) dwHitResult = HITRESULT_HIT;
        }
        if(dwHitResult != HITRESULT_HIT) return std::nullopt;
        if(uMsg == WM_SETCURSOR) lRes = std::nullopt;
        else if(uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK || uMsg == WM_RBUTTONDOWN)
        {
            if(!GetManager()->IsNoActivate()) ::SetFocus(GetManager()->GetPaintWindow());
            SetFocus();
        }
        if(uMsg == WM_RBUTTONUP || uMsg == WM_NCRBUTTONUP)
            return std::nullopt;
    }
#ifdef _UNICODE
    else if(uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
    {
#else
    else if((uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || uMsg == WM_CHAR || uMsg == WM_IME_CHAR)
    {
#endif
        if(!IsFocused()) return std::nullopt;
    }
#ifdef _USEIMM
    else if(uMsg == WM_IME_STARTCOMPOSITION)
    {
        if(IsFocused())
        {
            POINT ptCaret = { 0 };
            ::GetCaretPos(&ptCaret);
            HIMC hMic = ::ImmGetContext(GetManager()->GetPaintWindow());
            COMPOSITIONFORM cpf;
            cpf.dwStyle = CFS_FORCE_POSITION;
            cpf.ptCurrentPos.x = ptCaret.x + m_pTwh->GetCaretWidth();
            cpf.ptCurrentPos.y = ptCaret.y;
            ::ImmSetCompositionWindow(hMic, &cpf);

            HFONT hFont = GetManager()->GetFont(m_iFont);
            LOGFONT lf;
            ::GetObject(hFont, sizeof(LOGFONT), &lf);
            ::ImmSetCompositionFont(hMic, &lf);

            ::ImmReleaseContext(GetManager()->GetPaintWindow(), hMic);
        }
        return std::nullopt;
    }
#endif
    else if(uMsg == WM_CONTEXTMENU)
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ::ScreenToClient(GetManager()->GetPaintWindow(), &pt);
        std::shared_ptr<CControlUI> pHover = GetManager()->FindControl(pt);
        if(pHover != shared_from_this())
        {
            return std::nullopt;
        }
        //创建一个弹出式菜单
        HMENU hPopMenu = CreatePopupMenu();
        AppendMenu(hPopMenu, 0, ID_RICH_UNDO, _T("撤销(&U)"));
        AppendMenu(hPopMenu, 0, ID_RICH_REDO, _T("重做(&R)"));
        AppendMenu(hPopMenu, MF_SEPARATOR, 0, _T(""));
        AppendMenu(hPopMenu, 0, ID_RICH_CUT, _T("剪切(&X)"));
        AppendMenu(hPopMenu, 0, ID_RICH_COPY, _T("复制(&C)"));
        AppendMenu(hPopMenu, 0, ID_RICH_PASTE, _T("粘帖(&V)"));
        AppendMenu(hPopMenu, 0, ID_RICH_CLEAR, _T("清空(&L)"));
        AppendMenu(hPopMenu, MF_SEPARATOR, 0, _T(""));
        AppendMenu(hPopMenu, 0, ID_RICH_SELECTALL, _T("全选(&A)"));

        //初始化菜单项
        UINT uUndo = (CanUndo() ? 0 : MF_GRAYED);
        EnableMenuItem(hPopMenu, ID_RICH_UNDO, MF_BYCOMMAND | uUndo);
        UINT uRedo = (CanRedo() ? 0 : MF_GRAYED);
        EnableMenuItem(hPopMenu, ID_RICH_REDO, MF_BYCOMMAND | uRedo);
        UINT uSel = ((GetSelectionType() != SEL_EMPTY) ? 0 : MF_GRAYED);
        UINT uReadonly = IsReadOnly() ? MF_GRAYED : 0;
        EnableMenuItem(hPopMenu, ID_RICH_CUT, MF_BYCOMMAND | uSel | uReadonly);
        EnableMenuItem(hPopMenu, ID_RICH_COPY, MF_BYCOMMAND | uSel);
        EnableMenuItem(hPopMenu, ID_RICH_CLEAR, MF_BYCOMMAND | uSel | uReadonly);
        EnableMenuItem(hPopMenu, ID_RICH_PASTE, MF_BYCOMMAND | uReadonly);
        ::ClientToScreen(GetManager()->GetPaintWindow(), &pt);
        TrackPopupMenu(hPopMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, GetManager()->GetPaintWindow(), nullptr);
        DestroyMenu(hPopMenu);
    }
    else if(uMsg == WM_COMMAND)
    {
        lRes = std::nullopt;
        if(!IsFocused()) return std::nullopt;
        UINT uCmd = (UINT)wParam;
        switch(uCmd)
        {
        case ID_RICH_UNDO:
            Undo();
            break;
        case ID_RICH_REDO:
            Redo();
            break;
        case ID_RICH_CUT:
            Cut();
            break;
        case ID_RICH_COPY:
            Copy();
            break;
        case ID_RICH_PASTE:
            Paste();
            break;
        case ID_RICH_CLEAR:
            Clear();
            break;
        case ID_RICH_SELECTALL:
            SetSelAll();
            break;
        default:
            break;
        }
    }
    else
    {
        switch(uMsg)
        {
        case WM_HELP:
            lRes = std::nullopt;
            break;
        default:
            return std::nullopt;
        }
    }
    if(WM_CHAR == uMsg)
    {
#ifndef _UNICODE
        // check if we are waiting for 2 consecutive WM_CHAR messages
        if(IsAccumulateDBCMode())
        {
            if((GetKeyState(VK_KANA) & 0x1))
            {
                // turn off accumulate mode
                SetAccumulateDBCMode(false);
                m_chLeadByte = 0;
            }
            else
            {
                if(!m_chLeadByte)
                {
                    // This is the first WM_CHAR message,
                    // accumulate it if this is a LeadByte.  Otherwise, fall thru to
                    // regular WM_CHAR processing.
                    if(IsDBCSLeadByte((WORD)wParam))
                    {
                        // save the Lead Byte and don't process this message
                        m_chLeadByte = (WORD)wParam << 8;

                        //TCHAR a = (WORD)wParam << 8 ;
                        return std::nullopt;
                    }
                }
                else
                {
                    // This is the second WM_CHAR message,
                    // combine the current byte with previous byte.
                    // This DBC will be handled as WM_IME_CHAR.
                    wParam |= m_chLeadByte;
                    uMsg = WM_IME_CHAR;

                    // setup to accumulate more WM_CHAR
                    m_chLeadByte = 0;
                }
            }
        }
#endif
    }
    LRESULT lResult = 0;
    HRESULT Hr = TxSendMessage(uMsg, wParam, lParam, &lResult);
    if(Hr == S_OK)
    {
        return lRes;
    }
    else if((uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || uMsg == WM_CHAR || uMsg == WM_IME_CHAR)
    {
        return lRes;
    }
    else if(uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST && m_pTwh->IsCaptured())
    {
        return lRes;
    }
    else
    {
        return lResult;
    }

}

bool CRichEditUI::SetOLECallback(IRichEditOleCallback* pCallback)
{
    LRESULT lResult;
    TxSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pCallback, &lResult);
    if((BOOL)lResult == TRUE)
    {
        pCallback->AddRef();
        m_pCallback = pCallback;
        return true;
    }
    return false;
}

IRichEditOleCallback *CRichEditUI::GetOLECallback()
{
    return m_pCallback;
}

LPRICHEDITOLE CRichEditUI::GetRichEditOle()
{
    LRESULT lResult;
    TxSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)(LPVOID*)&m_pRichEditOle, &lResult);
    if((BOOL)lResult == TRUE)
    {
        m_pRichEditOle->AddRef();
        return m_pRichEditOle;
    }
    return NULL;
}
LPOLEINPLACEACTIVEOBJECT COleInPlaceFrame::g_pActiveObject = NULL;

} // namespace DuiLib
