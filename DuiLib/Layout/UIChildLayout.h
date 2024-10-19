#ifndef __UICHILDLAYOUT_H__
#define __UICHILDLAYOUT_H__

#pragma once

namespace DuiLib
{
class UILIB_API CChildLayoutUI: public CContainerUI
{
    DECLARE_DUICONTROL(CChildLayoutUI)
public:
    using ptr = std::shared_ptr<CChildLayoutUI>;
    CChildLayoutUI();

    void Init();
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);
    void SetChildLayoutXML(faw::string_t pXML);
    faw::string_t GetChildLayoutXML();
    virtual std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    virtual faw::string_t GetClass() const;

private:
    faw::string_t m_pstrXMLFile;
};
} // namespace DuiLib
#endif // __UICHILDLAYOUT_H__
