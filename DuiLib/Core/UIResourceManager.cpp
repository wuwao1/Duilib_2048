﻿#include "StdAfx.h"
#include "UIResourceManager.h"

namespace DuiLib
{

CResourceManager::CResourceManager(void)
{
    m_pQuerypInterface = nullptr;

}

CResourceManager::~CResourceManager(void)
{
    ResetResourceMap();
}

BOOL CResourceManager::LoadResource(std::variant<UINT, faw::string_t> xml, faw::string_t type)
{
    if(xml.index() == 1)
    {
        if(std::get<1> (xml)[0] == _T('<'))
        {
            if(!m_xml.Load(std::get<1> (xml).c_str())) return FALSE;
        }
        else
        {
            if(!m_xml.LoadFromFile(std::get<1> (xml).c_str())) return FALSE;
        }
    }
    else
    {
        HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), MAKEINTRESOURCE(std::get<0> (xml)), type.data());
        if(hResource == NULL) return FALSE;
        HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
        if(hGlobal == NULL)
        {
            FreeResource(hResource);
            return FALSE;
        }

        if(!m_xml.LoadFromMem((BYTE*) ::LockResource(hGlobal), ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource)))
        {
            ::FreeResource(hResource);
            return FALSE;
        }
        ::FreeResource(hResource);
    }

    return LoadResource(m_xml.GetRoot());
}

BOOL CResourceManager::LoadResource(CMarkupNode Root)
{
    if(!Root.IsValid()) return FALSE;

    int nAttributes = 0;

    //加载图片资源
    faw::string_t pstrId = _T("");
    faw::string_t pstrPath = _T("");
    for(CMarkupNode node = Root.GetChild(); node.IsValid(); node = node.GetSibling())
    {
        faw::string_t pstrClass = node.GetName(), pstrName = _T(""), pstrValue = _T("");
        CMarkupNode ChildNode = node.GetChild();
        if(ChildNode.IsValid()) LoadResource(node);
        else if(pstrClass == _T("Image") && node.HasAttributes())
        {
            //加载图片资源
            nAttributes = node.GetAttributeCount();
            for(int i = 0; i < nAttributes; i++)
            {
                pstrName = node.GetAttributeName(i);
                pstrValue = node.GetAttributeValue(i);

                if(pstrName == _T("id"))
                {
                    pstrId = pstrValue;
                }
                else if(pstrName == _T("path"))
                {
                    pstrPath = pstrValue;
                }
            }
            if(pstrId.empty() || pstrPath.empty()) continue;
            faw::string_t * pstrFind = static_cast<faw::string_t *>(m_mImageHashMap.Find(pstrId));
            if(pstrFind) continue;
            m_mImageHashMap.Insert(pstrId, (LPVOID)new faw::string_t (pstrPath));
        }
        else if(pstrClass == _T("Xml") && node.HasAttributes())
        {
            //加载XML配置文件
            nAttributes = node.GetAttributeCount();
            for(int i = 0; i < nAttributes; i++)
            {
                pstrName = node.GetAttributeName(i);
                pstrValue = node.GetAttributeValue(i);

                if(pstrName == _T("id"))
                {
                    pstrId = pstrValue;
                }
                else if(pstrName == _T("path"))
                {
                    pstrPath = pstrValue;
                }
            }
            if(pstrId.empty() || pstrPath.empty()) continue;
            faw::string_t * pstrFind = static_cast<faw::string_t *>(m_mXmlHashMap.Find(pstrId));
            if(pstrFind) continue;
            m_mXmlHashMap.Insert(pstrId, (LPVOID)new faw::string_t (pstrPath));
        }
        else continue;
    }
    return TRUE;
}

faw::string_t CResourceManager::GetImagePath(faw::string_t lpstrId)
{
    faw::string_t * lpStr = static_cast<faw::string_t *>(m_mImageHashMap.Find(lpstrId));
    return !lpStr ? _T("") : lpStr->c_str();
}

faw::string_t CResourceManager::GetXmlPath(faw::string_t lpstrId)
{
    faw::string_t * lpStr = static_cast<faw::string_t *>(m_mXmlHashMap.Find(lpstrId));
    return !lpStr ? _T("") : lpStr->c_str();
}

void CResourceManager::ResetResourceMap()
{
    for(int i = 0; i < m_mImageHashMap.GetSize(); i++)
    {
        faw::string_t key = m_mImageHashMap.GetAt(i)->Key;
        if(!key.empty())
            delete static_cast<faw::string_t *>(m_mImageHashMap.Find(key));
    }
    for(int i = 0; i < m_mXmlHashMap.GetSize(); i++)
    {
        faw::string_t key = m_mXmlHashMap.GetAt(i)->Key;
        if(!key.empty())
            delete static_cast<faw::string_t *>(m_mXmlHashMap.Find(key));
    }
    for(int i = 0; i < m_mTextResourceHashMap.GetSize(); i++)
    {
        faw::string_t key = m_mTextResourceHashMap.GetAt(i)->Key;
        if(!key.empty())
            delete static_cast<faw::string_t *>(m_mTextResourceHashMap.Find(key));
    }
}

BOOL CResourceManager::LoadLanguage(faw::string_t pstrXml)
{
    CMarkup xml;
    if(pstrXml[0] == _T('<'))
    {
        if(!xml.Load(pstrXml.data())) return FALSE;
    }
    else
    {
        if(!xml.LoadFromFile(pstrXml.data())) return FALSE;
    }
    CMarkupNode Root = xml.GetRoot();
    if(!Root.IsValid()) return FALSE;

    faw::string_t pstrClass = _T("");
    int nAttributes = 0;
    faw::string_t pstrName = _T("");
    faw::string_t pstrValue = _T("");

    //加载Text资源
    faw::string_t pstrId = _T("");
    faw::string_t pstrText = _T("");
    for(CMarkupNode node = Root.GetChild(); node.IsValid(); node = node.GetSibling())
    {
        pstrClass = node.GetName();
        if(pstrClass == _T("Text") && node.HasAttributes())
        {
            //加载Text资源
            nAttributes = node.GetAttributeCount();
            for(int i = 0; i < nAttributes; i++)
            {
                pstrName = node.GetAttributeName(i);
                pstrValue = node.GetAttributeValue(i);

                if(pstrName == _T("id"))
                {
                    pstrId = pstrValue;
                }
                else if(pstrName == _T("value"))
                {
                    pstrText = pstrValue;
                }
            }
            if(pstrId.empty() || pstrText.empty()) continue;

            faw::string_t *lpstrFind = static_cast<faw::string_t *>(m_mTextResourceHashMap.Find(pstrId));
            if(lpstrFind)
            {
                *lpstrFind = pstrText;
            }
            else
            {
                m_mTextResourceHashMap.Insert(pstrId, (LPVOID)new faw::string_t (pstrText));
            }
        }
        else continue;
    }

    return TRUE;
}

faw::string_t CResourceManager::GetText(faw::string_t lpstrId, faw::string_t lpstrType)
{
    if(lpstrId.empty()) return _T("");

    faw::string_t *lpstrFind = static_cast<faw::string_t *>(m_mTextResourceHashMap.Find(lpstrId));
    if(!lpstrFind && m_pQuerypInterface)
    {
        lpstrFind = new faw::string_t (m_pQuerypInterface->QueryControlText(lpstrId, lpstrType));
        m_mTextResourceHashMap.Insert(lpstrId, (LPVOID) lpstrFind);
    }
    return !lpstrFind ? lpstrId : *lpstrFind;
}

void CResourceManager::ReloadText()
{
    if(!m_pQuerypInterface) return;
    //重载文字描述
    for(int i = 0; i < m_mTextResourceHashMap.GetSize(); i++)
    {
        faw::string_t lpstrId = m_mTextResourceHashMap.GetAt(i)->Key;
        if(lpstrId.empty()) continue;
        faw::string_t lpstrText = m_pQuerypInterface->QueryControlText(lpstrId, _T(""));
        if(!lpstrText.empty())
        {
            faw::string_t *lpStr = static_cast<faw::string_t*>(m_mTextResourceHashMap.Find(lpstrId));
            *lpStr = lpstrText;
        }
    }
}
void CResourceManager::ResetTextMap()
{
    faw::string_t * lpStr;
    for(int i = 0; i < m_mTextResourceHashMap.GetSize(); i++)
    {
        faw::string_t key = m_mTextResourceHashMap.GetAt(i)->Key;
        if(!key.empty())
        {
            lpStr = static_cast<faw::string_t *>(m_mTextResourceHashMap.Find(key));
            delete lpStr;
        }
    }
}


} // namespace DuiLib