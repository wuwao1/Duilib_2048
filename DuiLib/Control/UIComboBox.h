#ifndef __UICOMBOBOX_H__
#define __UICOMBOBOX_H__

#pragma once

namespace DuiLib
{
/// 扩展下拉列表框
/// 增加arrowimage属性,一张图片平均分成5份,Normal/Hot/Pushed/Focused/Disabled(必须有source属性)
/// <Default name="ComboBox" value="arrowimage=&quot;file='sys_combo_btn.png' source='0,0,16,16'&quot; "/>
class UILIB_API CComboBoxUI: public CComboUI
{
    DECLARE_DUICONTROL(CComboBoxUI)
public:
    using ptr = std::shared_ptr<CComboBoxUI>;
    CComboBoxUI();
    faw::string_t GetClass() const;

    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue)override;

    void PaintText(HDC hDC)override;
    void PaintStatusImage(HDC hDC) override;

protected:
    faw::string_t m_sArrowImage;
    int        m_nArrowWidth	= 0;
};
}

#endif // __UICOMBOBOX_H__
