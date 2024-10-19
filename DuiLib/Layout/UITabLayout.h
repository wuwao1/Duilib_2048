#ifndef __UITABLAYOUT_H__
#define __UITABLAYOUT_H__

#pragma once

namespace DuiLib
{
class UILIB_API CTabLayoutUI: public CContainerUI
{
    DECLARE_DUICONTROL(CTabLayoutUI)
public:
    using ptr = std::shared_ptr<CTabLayoutUI>;
    CTabLayoutUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    bool Add(std::shared_ptr<CControlUI> pControl);
    bool AddAt(std::shared_ptr<CControlUI> pControl, int iIndex);
    bool Remove(std::shared_ptr<CControlUI> pControl);
    void RemoveAll();
    int GetCurSel() const;
    virtual bool SelectItem(int iIndex);
    virtual bool SelectItem(std::shared_ptr<CControlUI> pControl);

    void SetPos(RECT rc, bool bNeedInvalidate = true);

    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

protected:
    int m_iCurSel = -1;
};
}
#endif // __UITABLAYOUT_H__
