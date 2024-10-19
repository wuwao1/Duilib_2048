﻿#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once
#include "OAIdl.h"
#include <vector>

struct ScopeCostTime
{
    std::chrono::high_resolution_clock::time_point startTime;
    std::string timerTips;
    ScopeCostTime(std::string tips = "") :
        startTime(std::chrono::high_resolution_clock::now()), timerTips(tips) {}

    ~ScopeCostTime()
    {
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration<double, std::milli>(endTime - startTime);
        std::cout << timerTips + " 程序执行时间： " +std::to_string(duration.count())+ " 毫秒" << std::endl;
    }
};

namespace DuiLib
{

class UILIB_API CStdPtrArray
{
public:
    CStdPtrArray(int iPreallocSize = 0);
    CStdPtrArray(const CStdPtrArray& src);
    virtual ~CStdPtrArray();

    void Clear();
    void Resize(int iSize);
    bool empty() const;
    int Find(LPVOID iIndex) const;
    bool Add(LPVOID pData);
    bool SetAt(int iIndex, LPVOID pData);
    bool InsertAt(int iIndex, LPVOID pData);
    bool Remove(int iIndex);
    int GetSize() const;
    LPVOID* GetData();

    LPVOID GetAt(int iIndex) const;
    LPVOID operator[](int nIndex) const;

protected:
    LPVOID* m_ppVoid;
    int m_nCount;
    int m_nAllocated;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CStdValArray
{
public:
    CStdValArray(int iElementSize, int iPreallocSize = 0);
    virtual ~CStdValArray();

    void Clear();
    bool empty() const;
    bool Add(LPCVOID pData);
    bool Remove(int iIndex);
    int GetSize() const;
    LPVOID GetData();

    LPVOID GetAt(int iIndex) const;
    LPVOID operator[](int nIndex) const;

protected:
    LPBYTE m_pVoid;
    int m_iElementSize;
    int m_nCount;
    int m_nAllocated;
};

/////////////////////////////////////////////////////////////////////////////////////
//

struct TITEM
{
    faw::string_t Key;
    LPVOID Data;
    struct TITEM* pPrev;
    struct TITEM* pNext;
};

class UILIB_API CStdStringPtrMap
{
public:
    CStdStringPtrMap(int nSize = 83);
    virtual ~CStdStringPtrMap();

    void Resize(int nSize = 83);
    LPVOID Find(faw::string_t key, bool optimize = true) const;
    bool Insert(faw::string_t key, LPVOID pData);
    LPVOID Set(faw::string_t key, LPVOID pData);
    bool Remove(faw::string_t key);
    void RemoveAll();
    int GetSize() const;
    TITEM *GetAt(int iIndex) const;
    TITEM *operator[](int nIndex) const;

protected:
    TITEM** m_aT;
    int m_nBuckets;
    int m_nCount;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CWaitCursor
{
public:
    CWaitCursor();
    virtual ~CWaitCursor();

protected:
    HCURSOR m_hOrigCursor;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CDuiVariant: public VARIANT
{
public:
    CDuiVariant()
    {
        VariantInit(this);
    }
    CDuiVariant(int i)
    {
        VariantInit(this);
        this->vt = VT_I4;
        this->intVal = i;
    }
    CDuiVariant(float f)
    {
        VariantInit(this);
        this->vt = VT_R4;
        this->fltVal = f;
    }
    CDuiVariant(LPOLESTR s)
    {
        VariantInit(this);
        this->vt = VT_BSTR;
        this->bstrVal = s;
    }
    CDuiVariant(IDispatch *disp)
    {
        VariantInit(this);
        this->vt = VT_DISPATCH;
        this->pdispVal = disp;
    }

    virtual ~CDuiVariant()
    {
        VariantClear(this);
    }
};

///////////////////////////////////////////////////////////////////////////////////////
////
//struct TImageInfo;
//class CPaintManagerUI;
//class UILIB_API CImageString
//{
//public:
//	CImageString();
//	CImageString(const CImageString&);
//	const CImageString& operator=(const CImageString&);
//	virtual ~CImageString();

//	const faw::string_t& GetAttributeString() const;
//	void SetAttributeString(faw::string_t pStrImageAttri);
//	void ModifyAttribute(faw::string_t pStrModify);
//	bool LoadImage(CPaintManagerUI* pManager);
//	bool IsLoadSuccess();

//	RECT GetDest() const;
//	void SetDest(const RECT &rcDest);
//	const std::shared_ptr<TImageInfo> GetImageInfo() const;

//private:
//	void Clone(const CImageString&);
//	void Clear();
//	void ParseAttribute(faw::string_t pStrImageAttri);

//protected:
//	friend class CRenderEngine;
//	faw::string_t	m_sImageAttribute;

//	faw::string_t	m_sImage;
//	faw::string_t	m_sResType;
//	TImageInfo	*m_imageInfo;
//	bool		m_bLoadSuccess;

//	RECT	m_rcDest;
//	RECT	m_rcSource;
//	RECT	m_rcCorner;
//	BYTE	m_bFade;
//	DWORD	m_dwMask;
//	bool	m_bHole;
//	bool	m_bTiledX;
//	bool	m_bTiledY;
//};
}// namespace DuiLib

#endif // __UTILS_H__