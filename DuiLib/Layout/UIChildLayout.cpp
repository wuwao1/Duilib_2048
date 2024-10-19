#include "StdAfx.h"
#include "UIChildLayout.h"

namespace DuiLib
{
IMPLEMENT_DUICONTROL(CChildLayoutUI)

CChildLayoutUI::CChildLayoutUI() {}

void CChildLayoutUI::Init()
{
    if(!m_pstrXMLFile.empty())
    {
        CDialogBuilder builder;
        auto pChildWindow = std::dynamic_pointer_cast<CContainerUI>(builder.Create(m_pstrXMLFile, _T(""), nullptr, m_pManager));
        if(pChildWindow)
        {
            this->Add(pChildWindow);
        }
        else
        {
            this->RemoveAll();
        }
    }
}

void CChildLayoutUI::SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)
{
    if(pstrName == _T("xmlfile"))
        SetChildLayoutXML(pstrValue);
    else
        CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CChildLayoutUI::SetChildLayoutXML(faw::string_t pXML)
{
    m_pstrXMLFile = pXML;
}

faw::string_t CChildLayoutUI::GetChildLayoutXML()
{
    return m_pstrXMLFile;
}

std::shared_ptr<CControlUI> CChildLayoutUI::GetInterface(faw::string_t pstrName)
{
    if(pstrName == DUI_CTRL_CHILDLAYOUT) return std::dynamic_pointer_cast<CChildLayoutUI>(shared_from_this());
    return CControlUI::GetInterface(pstrName);
}

faw::string_t CChildLayoutUI::GetClass() const
{
    return _T("ChildLayoutUI");
}
} // namespace DuiLib
