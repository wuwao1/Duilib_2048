﻿#include "StdAfx.h"
#include "ControlFactory.h"

namespace DuiLib
{
CControlFactory::CControlFactory()
{
    INNER_REGISTER_DUICONTROL(CControlUI);
    INNER_REGISTER_DUICONTROL(CContainerUI);
    INNER_REGISTER_DUICONTROL(CButtonUI);
    INNER_REGISTER_DUICONTROL(CComboUI);
    INNER_REGISTER_DUICONTROL(CComboBoxUI);
    INNER_REGISTER_DUICONTROL(CDateTimeUI);
    INNER_REGISTER_DUICONTROL(CEditUI);
    INNER_REGISTER_DUICONTROL(CActiveXUI);
    INNER_REGISTER_DUICONTROL(CFlashUI);
    INNER_REGISTER_DUICONTROL(CGifAnimUI);
#ifdef USE_XIMAGE_EFFECT
    INNER_REGISTER_DUICONTROL(CGifAnimExUI);
#endif
    INNER_REGISTER_DUICONTROL(CGroupBoxUI);
    INNER_REGISTER_DUICONTROL(CIPAddressUI);
    INNER_REGISTER_DUICONTROL(CIPAddressExUI);
    INNER_REGISTER_DUICONTROL(CLabelUI);
    INNER_REGISTER_DUICONTROL(CListUI);
    INNER_REGISTER_DUICONTROL(CListHeaderUI);
    INNER_REGISTER_DUICONTROL(CListHeaderItemUI);
    INNER_REGISTER_DUICONTROL(CListLabelElementUI);
    INNER_REGISTER_DUICONTROL(CListTextElementUI);
    INNER_REGISTER_DUICONTROL(CListContainerElementUI);
    INNER_REGISTER_DUICONTROL(CMenuUI);
    INNER_REGISTER_DUICONTROL(CMenuElementUI);
    INNER_REGISTER_DUICONTROL(COptionUI);
    INNER_REGISTER_DUICONTROL(CCheckBoxUI);
    INNER_REGISTER_DUICONTROL(CProgressUI);
    INNER_REGISTER_DUICONTROL(CRichEditUI);
    INNER_REGISTER_DUICONTROL(CScrollBarUI);
    INNER_REGISTER_DUICONTROL(CSliderUI);
    INNER_REGISTER_DUICONTROL(CTextUI);
    INNER_REGISTER_DUICONTROL(CTreeNodeUI);
    INNER_REGISTER_DUICONTROL(CTreeViewUI);
    INNER_REGISTER_DUICONTROL(CWebBrowserUI);
    INNER_REGISTER_DUICONTROL(CAnimationTabLayoutUI);
    INNER_REGISTER_DUICONTROL(CChildLayoutUI);
    INNER_REGISTER_DUICONTROL(CHorizontalLayoutUI);
    INNER_REGISTER_DUICONTROL(CTabLayoutUI);
    INNER_REGISTER_DUICONTROL(CTileLayoutUI);
    INNER_REGISTER_DUICONTROL(CVerticalLayoutUI);
    INNER_REGISTER_DUICONTROL(CRollTextUI);
    INNER_REGISTER_DUICONTROL(CColorPaletteUI);
    INNER_REGISTER_DUICONTROL(CListExUI);
    INNER_REGISTER_DUICONTROL(CListContainerHeaderItemUI);
    INNER_REGISTER_DUICONTROL(CListTextExtElementUI);
    INNER_REGISTER_DUICONTROL(CHotKeyUI);
    INNER_REGISTER_DUICONTROL(CFadeButtonUI);
    INNER_REGISTER_DUICONTROL(CRingUI);
}

CControlFactory::~CControlFactory() {}

std::shared_ptr<CControlUI> CControlFactory::CreateControl(faw::string_t strClassName)
{
    FawTools::lower(strClassName);
    auto iter = m_mapControl.find(strClassName);
    if(iter == m_mapControl.end())
    {
        return nullptr;
    }
    else
    {
        return iter->second();
    }
}

void CControlFactory::RegistControl(faw::string_t strClassName, CreateClass pFunc)
{
    FawTools::lower(strClassName);
    m_mapControl.insert(MAP_DUI_STRING_CTRATECLASS::value_type(strClassName, pFunc));
}

std::shared_ptr<CControlFactory> CControlFactory::GetInstance()
{
    static std::shared_ptr<CControlFactory> instance = std::make_shared<CControlFactory>();
    return instance;
}

}