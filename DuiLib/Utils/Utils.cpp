﻿#include "StdAfx.h"
#include "Utils.h"

namespace DuiLib
{
/*void RECT::Join (const RECT& rc) {
	left = min (left, rc.left);
	top = min (top, rc.top);
	right = max (right, rc.right);
	bottom = max (bottom, rc.bottom);
}*/

//void RECT::ResetOffset () {
//	::OffsetRect (this, -left, -top);
//}

//void RECT::Normalize () {
//	if (left > right) {
//		int iTemp = left; left = right; right = iTemp;
//	}
//	if (top > bottom) {
//		int iTemp = top; top = bottom; bottom = iTemp;
//	}
//}

//void RECT::Offset (int cx, int cy) {
//	::OffsetRect (this, cx, cy);
//}

//void RECT::Inflate (int cx, int cy) {
//	::InflateRect (this, cx, cy);
//}

//void RECT::Deflate (int cx, int cy) {
//	::InflateRect (this, -cx, -cy);
//}

//void RECT::Union (RECT& rc) {
//	::UnionRect (this, this, &rc);
//}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CStdPtrArray::CStdPtrArray(int iPreallocSize) :
    m_ppVoid(nullptr), m_nCount(0), m_nAllocated(iPreallocSize)
{
    ASSERT(iPreallocSize >= 0);
    if(iPreallocSize > 0) m_ppVoid = static_cast<LPVOID*>(malloc(iPreallocSize * sizeof(LPVOID)));
}

CStdPtrArray::CStdPtrArray(const CStdPtrArray& src) :
    m_ppVoid(nullptr), m_nCount(0), m_nAllocated(0)
{
    for(int i = 0; i < src.GetSize(); i++)
        Add(src.GetAt(i));
}

CStdPtrArray::~CStdPtrArray()
{
    if(m_ppVoid) free(m_ppVoid);
}

void CStdPtrArray::Clear()
{
    if(m_ppVoid) free(m_ppVoid);
    m_ppVoid = nullptr;
    m_nCount = m_nAllocated = 0;
}

void CStdPtrArray::Resize(int iSize)
{
    Clear();
    m_ppVoid = static_cast<LPVOID*>(malloc(iSize * sizeof(LPVOID)));
    ::ZeroMemory(m_ppVoid, iSize * sizeof(LPVOID));
    m_nAllocated = iSize;
    m_nCount = iSize;
}

bool CStdPtrArray::empty() const
{
    return m_nCount == 0;
}

bool CStdPtrArray::Add(LPVOID pData)
{
    if(++m_nCount >= m_nAllocated)
    {
        int nAllocated = m_nAllocated * 2;
        if(nAllocated == 0) nAllocated = 11;
        LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
        if(ppVoid)
        {
            m_nAllocated = nAllocated;
            m_ppVoid = ppVoid;
        }
        else
        {
            --m_nCount;
            return false;
        }
    }
    m_ppVoid[m_nCount - 1] = pData;
    return true;
}

bool CStdPtrArray::InsertAt(int iIndex, LPVOID pData)
{
    if(iIndex == m_nCount) return Add(pData);
    if(iIndex < 0 || iIndex > m_nCount) return false;
    if(++m_nCount >= m_nAllocated)
    {
        int nAllocated = m_nAllocated * 2;
        if(nAllocated == 0) nAllocated = 11;
        LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
        if(ppVoid)
        {
            m_nAllocated = nAllocated;
            m_ppVoid = ppVoid;
        }
        else
        {
            --m_nCount;
            return false;
        }
    }
    memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(LPVOID));
    m_ppVoid[iIndex] = pData;
    return true;
}

bool CStdPtrArray::SetAt(int iIndex, LPVOID pData)
{
    if(iIndex < 0 || iIndex >= m_nCount) return false;
    m_ppVoid[iIndex] = pData;
    return true;
}

bool CStdPtrArray::Remove(int iIndex)
{
    if(iIndex < 0 || iIndex >= m_nCount) return false;
    if(iIndex < --m_nCount) ::CopyMemory(m_ppVoid + iIndex, m_ppVoid + iIndex + 1, (m_nCount - iIndex) * sizeof(LPVOID));
    return true;
}

int CStdPtrArray::Find(LPVOID pData) const
{
    for(int i = 0; i < m_nCount; i++) if(m_ppVoid[i] == pData) return i;
    return -1;
}

int CStdPtrArray::GetSize() const
{
    return m_nCount;
}

LPVOID* CStdPtrArray::GetData()
{
    return m_ppVoid;
}

LPVOID CStdPtrArray::GetAt(int iIndex) const
{
    if(iIndex < 0 || iIndex >= m_nCount) return nullptr;
    return m_ppVoid[iIndex];
}

LPVOID CStdPtrArray::operator[](int iIndex) const
{
    ASSERT(iIndex >= 0 && iIndex < m_nCount);
    return m_ppVoid[iIndex];
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CStdValArray::CStdValArray(int iElementSize, int iPreallocSize /*= 0*/) :
    m_pVoid(nullptr),
    m_nCount(0),
    m_iElementSize(iElementSize),
    m_nAllocated(iPreallocSize)
{
    ASSERT(iElementSize > 0);
    ASSERT(iPreallocSize >= 0);
    if(iPreallocSize > 0) m_pVoid = static_cast<LPBYTE>(malloc(iPreallocSize * m_iElementSize));
}

CStdValArray::~CStdValArray()
{
    if(m_pVoid) free(m_pVoid);
}

void CStdValArray::Clear()
{
    m_nCount = 0;  // NOTE: We keep the memory in place
}

bool CStdValArray::empty() const
{
    return m_nCount == 0;
}

bool CStdValArray::Add(LPCVOID pData)
{
    if(++m_nCount >= m_nAllocated)
    {
        int nAllocated = m_nAllocated * 2;
        if(nAllocated == 0) nAllocated = 11;
        LPBYTE pVoid = static_cast<LPBYTE>(realloc(m_pVoid, nAllocated * m_iElementSize));
        if(pVoid)
        {
            m_nAllocated = nAllocated;
            m_pVoid = pVoid;
        }
        else
        {
            --m_nCount;
            return false;
        }
    }
    ::CopyMemory(m_pVoid + ((m_nCount - 1) * m_iElementSize), pData, m_iElementSize);
    return true;
}

bool CStdValArray::Remove(int iIndex)
{
    if(iIndex < 0 || iIndex >= m_nCount) return false;
    if(iIndex < --m_nCount) ::CopyMemory(m_pVoid + (iIndex * m_iElementSize), m_pVoid + ((iIndex + 1) * m_iElementSize), (m_nCount - iIndex) * m_iElementSize);
    return true;
}

int CStdValArray::GetSize() const
{
    return m_nCount;
}

LPVOID CStdValArray::GetData()
{
    return static_cast<LPVOID>(m_pVoid);
}

LPVOID CStdValArray::GetAt(int iIndex) const
{
    if(iIndex < 0 || iIndex >= m_nCount) return nullptr;
    return m_pVoid + (iIndex * m_iElementSize);
}

LPVOID CStdValArray::operator[](int iIndex) const
{
    ASSERT(iIndex >= 0 && iIndex < m_nCount);
    return m_pVoid + (iIndex * m_iElementSize);
}



static UINT HashKey(faw::string_t Key)
{
    UINT i = 0;
    SIZE_T len = Key.length();
    while(len-- > 0) i = (i << 5) + i + Key[len];
    return i;
}

CStdStringPtrMap::CStdStringPtrMap(int nSize) :
    m_nCount(0)
{
    if(nSize < 16) nSize = 16;
    m_nBuckets = nSize;
    m_aT = new TITEM*[nSize];
    memset(m_aT, 0, nSize * sizeof(TITEM*));
}

CStdStringPtrMap::~CStdStringPtrMap()
{
    if(m_aT)
    {
        int len = m_nBuckets;
        while(len--)
        {
            TITEM* pItem = m_aT[len];
            while(pItem)
            {
                TITEM* pKill = pItem;
                pItem = pItem->pNext;
                delete pKill;
            }
        }
        delete[] m_aT;
        m_aT = nullptr;
    }
}

void CStdStringPtrMap::RemoveAll()
{
    this->Resize(m_nBuckets);
}

void CStdStringPtrMap::Resize(int nSize)
{
    if(m_aT)
    {
        int len = m_nBuckets;
        while(len--)
        {
            TITEM* pItem = m_aT[len];
            while(pItem)
            {
                TITEM* pKill = pItem;
                pItem = pItem->pNext;
                delete pKill;
            }
        }
        delete[] m_aT;
        m_aT = nullptr;
    }

    if(nSize < 0) nSize = 0;
    if(nSize > 0)
    {
        m_aT = new TITEM*[nSize];
        memset(m_aT, 0, nSize * sizeof(TITEM*));
    }
    m_nBuckets = nSize;
    m_nCount = 0;
}

LPVOID CStdStringPtrMap::Find(faw::string_t key, bool optimize) const
{
    if(m_nBuckets == 0 || GetSize() == 0) return nullptr;

    UINT slot = HashKey(key) % m_nBuckets;
    for(TITEM* pItem = m_aT[slot]; pItem; pItem = pItem->pNext)
    {
        if(pItem->Key == key)
        {
            if(optimize && pItem != m_aT[slot])
            {
                if(pItem->pNext)
                {
                    pItem->pNext->pPrev = pItem->pPrev;
                }
                pItem->pPrev->pNext = pItem->pNext;
                pItem->pPrev = nullptr;
                pItem->pNext = m_aT[slot];
                pItem->pNext->pPrev = pItem;
                //将item移动至链条头部
                m_aT[slot] = pItem;
            }
            return pItem->Data;
        }
    }

    return nullptr;
}

bool CStdStringPtrMap::Insert(faw::string_t key, LPVOID pData)
{
    if(m_nBuckets == 0) return false;
    if(Find(key)) return false;

    // Add first in bucket
    UINT slot = HashKey(key) % m_nBuckets;
    TITEM* pItem = new TITEM;
    pItem->Key = key;
    pItem->Data = pData;
    pItem->pPrev = nullptr;
    pItem->pNext = m_aT[slot];
    if(pItem->pNext)
        pItem->pNext->pPrev = pItem;
    m_aT[slot] = pItem;
    m_nCount++;
    return true;
}

LPVOID CStdStringPtrMap::Set(faw::string_t key, LPVOID pData)
{
    if(m_nBuckets == 0) return pData;

    if(GetSize() > 0)
    {
        UINT slot = HashKey(key) % m_nBuckets;
        // Modify existing item
        for(TITEM* pItem = m_aT[slot]; pItem; pItem = pItem->pNext)
        {
            if(pItem->Key == key)
            {
                LPVOID pOldData = pItem->Data;
                pItem->Data = pData;
                return pOldData;
            }
        }
    }

    Insert(key, pData);
    return nullptr;
}

bool CStdStringPtrMap::Remove(faw::string_t key)
{
    if(m_nBuckets == 0 || GetSize() == 0) return false;

    UINT slot = HashKey(key) % m_nBuckets;
    TITEM** ppItem = &m_aT[slot];
    while(*ppItem)
    {
        if((*ppItem)->Key == key)
        {
            TITEM* pKill = *ppItem;
            *ppItem = (*ppItem)->pNext;
            if(*ppItem)
                (*ppItem)->pPrev = pKill->pPrev;
            delete pKill;
            m_nCount--;
            return true;
        }
        ppItem = &((*ppItem)->pNext);
    }

    return false;
}

int CStdStringPtrMap::GetSize() const
{
#if 0//def _DEBUG
    int nCount = 0;
    int len = m_nBuckets;
    while(len--)
    {
        for(const TITEM* pItem = m_aT[len]; pItem; pItem = pItem->pNext) nCount++;
    }
    ASSERT(m_nCount == nCount);
#endif
    return m_nCount;
}

TITEM *CStdStringPtrMap::GetAt(int iIndex) const
{
    if(m_nBuckets == 0 || GetSize() == 0) return nullptr;

    int pos = 0;
    int len = m_nBuckets;
    while(len--)
    {
        for(TITEM* pItem = m_aT[len]; pItem; pItem = pItem->pNext)
        {
            if(pos++ == iIndex)
            {
                return pItem;
            }
        }
    }

    return nullptr;
}

TITEM * CStdStringPtrMap::operator[](int nIndex) const
{
    return GetAt(nIndex);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CWaitCursor::CWaitCursor()
{
    m_hOrigCursor = ::SetCursor(::LoadCursor(nullptr, IDC_WAIT));
}

CWaitCursor::~CWaitCursor()
{
    ::SetCursor(m_hOrigCursor);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
//CImageString::CImageString () {
//	Clear ();
//}

//CImageString::CImageString (const CImageString& image) {
//	Clone (image);
//}

//const CImageString& CImageString::operator= (const CImageString& image) {
//	Clone (image);
//	return *this;
//}

//void CImageString::Clone (const CImageString& image) {
//	m_sImageAttribute = image.m_sImageAttribute;

//	m_sImage = image.m_sImage;
//	m_sResType = image.m_sResType;
//	m_imageInfo = image.m_imageInfo;
//	m_bLoadSuccess = image.m_bLoadSuccess;

//	m_rcDest = image.m_rcDest;
//	m_rcSource = image.m_rcSource;
//	m_rcCorner = image.m_rcCorner;
//	m_bFade = image.m_bFade;
//	m_dwMask = image.m_dwMask;
//	m_bHole = image.m_bHole;
//	m_bTiledX = image.m_bTiledX;
//	m_bTiledY = image.m_bTiledY;
//}

//CImageString::~CImageString () {

//}

//const faw::string_t& CImageString::GetAttributeString() const {
//	return m_sImageAttribute;
//}

//void CImageString::SetAttributeString(faw::string_t pStrImageAttri) {
//	if (m_sImageAttribute == pStrImageAttri) return;
//	Clear ();
//	m_sImageAttribute = pStrImageAttri;
//	m_sImage = m_sImageAttribute;
//}

//bool CImageString::LoadImage(CPaintManagerUI* pManager) {
//	m_imageInfo = nullptr;
//	m_bLoadSuccess = true;
//	ZeroMemory(&m_rcDest, sizeof(RECT));
//	ZeroMemory(&m_rcSource, sizeof(RECT));
//	ZeroMemory(&m_rcCorner, sizeof(RECT));
//	m_bFade = 0xFF;
//	m_dwMask = 0;
//	m_bHole = false;
//	m_bTiledX = false;
//	m_bTiledY = false;
//	ParseAttribute(m_sImageAttribute,*pManager->GetDPIObj());
//	if (!m_bLoadSuccess) return false;

//	const std::shared_ptr<TImageInfo> data = nullptr;
//	if (m_sResType.empty()) {
//		data = pManager->GetImageEx(m_sImage.c_str (), nullptr, m_dwMask);
//	} else {
//		data = pManager->GetImageEx(m_sImage.c_str (), m_sResType.c_str (), m_dwMask);
//	}
//	if (!data) {
//		m_bLoadSuccess = false;
//		return false;
//	} else {
//		m_bLoadSuccess = true;
//	}

//	if (m_rcSource.left == 0 && m_rcSource.right == 0 && m_rcSource.top == 0 && m_rcSource.bottom == 0) {
//		m_rcSource.right = data->nX;
//		m_rcSource.bottom = data->nY;
//	}
//	if (m_rcSource.right > data->nX) m_rcSource.right = data->nX;
//	if (m_rcSource.bottom > data->nY) m_rcSource.bottom = data->nY;
//	m_imageInfo = const_cast<std::shared_ptr<TImageInfo>>(data);

//	return true;
//}

//bool CImageString::IsLoadSuccess () {
//	return !m_sImageAttribute.empty () && m_bLoadSuccess;
//}

//void CImageString::ModifyAttribute (faw::string_t pStrModify) {
//	//ParseAttribute (pStrModify);
//}

//void CImageString::Clear () {
//	m_sImageAttribute.Clear ();
//	m_sImage.Clear();
//	m_sResType.Clear();
//	m_imageInfo = nullptr;
//	m_bLoadSuccess = true;
//	ZeroMemory (&m_rcDest, sizeof (RECT));
//	ZeroMemory (&m_rcSource, sizeof (RECT));
//	ZeroMemory (&m_rcCorner, sizeof (RECT));
//	m_bFade = 0xFF;
//	m_dwMask = 0;
//	m_bHole = false;
//	m_bTiledX = false;
//	m_bTiledY = false;
//}

//void CImageString::ParseAttribute(faw::string_t pStrImage) {
//	if (!pStrImage.empty ())
//		return;

//	// 1、aaa.jpg
//	// 2、file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0'
//	// mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false'
//	faw::string_t sItem;
//	faw::string_t sValue;

//	while (*pStrImage != _T('\0')) {
//		sItem.Clear();
//		sValue.Clear();
//		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
//		while (*pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ')) {
//			LPTSTR pstrTemp = ::CharNext(pStrImage);
//			while (pStrImage < pstrTemp) {
//				sItem += *pStrImage++;
//			}
//		}
//		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
//		if (*pStrImage++ != _T('=')) break;
//		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
//		if (*pStrImage++ != _T('\'')) break;
//		while (*pStrImage != _T('\0') && *pStrImage != _T('\'')) {
//			LPTSTR pstrTemp = ::CharNext(pStrImage);
//			while (pStrImage < pstrTemp) {
//				sValue += *pStrImage++;
//			}
//		}
//		if (*pStrImage++ != _T('\'')) break;
//		if (!sValue.empty()) {
//			if (sItem == _T("file") || sItem == _T("res")) {
//				m_sImage = sValue;
//				//if (g_Dpi.GetScale() != 100) {
//				//	std::wstringstream wss;
//				//	wss << L"@" << g_Dpi.GetScale() << L".";
//				//	std::wstring suffix = wss.str();
//				//	m_sImage.replace_self(L".", suffix.c_str());
//				//}
//			} else if (sItem == _T("restype")) {
//				m_sResType = sValue;
//			} else if (sItem == _T("dest")) {
//				m_rcDest = FawTools::parse_rect (sValue);
//				//g_Dpi.ScaleRect(&m_rcDest);
//			} else if (sItem == _T("source")) {
//				m_rcSource = FawTools::parse_rect (sValue);
//				//g_Dpi.ScaleRect(&m_rcSource);
//			} else if (sItem == _T("corner")) {
//				m_rcCorner = FawTools::parse_rect (sValue);
//				//g_Dpi.ScaleRect(&m_rcCorner);
//			} else if (sItem == _T("mask")) {
//				m_dwMask = (DWORD) FawTools::parse_hex (sValue);
//			} else if (sItem == _T("fade")) {
//				m_bFade = (BYTE) FawTools::parse_dec (sValue);
//			} else if (sItem == _T("hole")) {
//				m_bHole = FawTools::parse_bool (sValue);
//			} else if (sItem == _T("xtiled")) {
//				m_bTiledX = FawTools::parse_bool (sValue);
//			} else if (sItem == _T("ytiled")) {
//				m_bTiledY = FawTools::parse_bool (sValue);
//			}
//		}
//		if (*pStrImage++ != _T(' ')) break;
//	}
//}

//void CImageString::SetDest(const RECT &rcDest)
//{
//	m_rcDest = rcDest;
//}

//RECT CImageString::GetDest() const
//{
//	return m_rcDest;
//}

DuiEvent tagTNotifyUI::GetEventType(faw::string_t strEvent)
{
    if(StrEventMap.empty())
    {
        StrEventMap[(_T("menu"))] = DuiEvent::menu;
        StrEventMap[(_T("link"))] = DuiEvent::link;
        StrEventMap[(_T("timer"))] = DuiEvent::timer;
        StrEventMap[(_T("click"))] = DuiEvent::click;
        StrEventMap[(_T("dbclick"))] = DuiEvent::dbclick;
        StrEventMap[(_T("return"))] = DuiEvent::_return;
        StrEventMap[(_T("scroll"))] = DuiEvent::scroll;
        StrEventMap[(_T("predropdown"))] = DuiEvent::predropdown;
        StrEventMap[(_T("dropdown"))] = DuiEvent::dropdown;
        StrEventMap[(_T("setfocus"))] = DuiEvent::setfocus;
        StrEventMap[(_T("killfocus"))] = DuiEvent::killfocus;
        StrEventMap[(_T("itemclick"))] = DuiEvent::itemclick;
        StrEventMap[(_T("itemrclick"))] = DuiEvent::itemrclick;
        StrEventMap[(_T("tabselect"))] = DuiEvent::tabselect;
        StrEventMap[(_T("itemselect"))] = DuiEvent::itemselect;
        StrEventMap[(_T("itemexpand"))] = DuiEvent::itemexpand;
        StrEventMap[(_T("windowinit"))] = DuiEvent::windowinit;
        StrEventMap[(_T("windowsize"))] = DuiEvent::windowsize;
        StrEventMap[(_T("buttondown"))] = DuiEvent::buttondown;
        StrEventMap[(_T("mouseenter"))] = DuiEvent::mouseenter;
        StrEventMap[(_T("mouseleave"))] = DuiEvent::mouseleave;
        StrEventMap[(_T("textchanged"))] = DuiEvent::textchanged;
        StrEventMap[(_T("headerclick"))] = DuiEvent::headerclick;
        StrEventMap[(_T("itemdbclick"))] = DuiEvent::itemdbclick;
        StrEventMap[(_T("showactivex"))] = DuiEvent::showactivex;
        StrEventMap[(_T("itemcollapse"))] = DuiEvent::itemcollapse;
        StrEventMap[(_T("itemactivate"))] = DuiEvent::itemactivate;
        StrEventMap[(_T("valuechanged"))] = DuiEvent::valuechanged;
        StrEventMap[(_T("movevaluechanged"))] = DuiEvent::movevaluechanged;
        StrEventMap[(_T("selectchanged"))] = DuiEvent::selectchanged;
        StrEventMap[(_T("unselected"))] = DuiEvent::unselected;
        StrEventMap[(_T("treeitemdbclick"))] = DuiEvent::treeitemdbclick;
        StrEventMap[(_T("checkclick"))] = DuiEvent::checkclick;
        StrEventMap[(_T("textrollend"))] = DuiEvent::textrollend;
        StrEventMap[(_T("colorchanged"))] = DuiEvent::colorchanged;
        StrEventMap[(_T("listitemselect"))] = DuiEvent::listitemselect;
        StrEventMap[(_T("listitemchecked"))] = DuiEvent::listitemchecked;
        StrEventMap[(_T("comboitemselect"))] = DuiEvent::comboitemselect;
        StrEventMap[(_T("listheaderclick"))] = DuiEvent::listheaderclick;
        StrEventMap[(_T("listheaditemchecked"))] = DuiEvent::listheaditemchecked;
        StrEventMap[(_T("listpagechanged"))] = DuiEvent::listpagechanged;
    }
    auto CurEevet = StrEventMap.find(strEvent);
    if(CurEevet != StrEventMap.end())
        return CurEevet->second;
    return DuiEvent::nofound;
}

//const std::shared_ptr<TImageInfo> CImageString::GetImageInfo() const
//{
//	return m_imageInfo;
//}
} // namespace DuiLib