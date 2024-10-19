#ifndef __UIDLGBUILDER_H__
#define __UIDLGBUILDER_H__

#pragma once

namespace DuiLib
{

class IDialogBuilderCallback
{
public:
    virtual std::shared_ptr<CControlUI> CreateControl(faw::string_t pstrClass) = 0;
};


class UILIB_API CDialogBuilder
{
public:
    CDialogBuilder();
    std::shared_ptr<CControlUI> Create(std::variant<UINT, faw::string_t> xml, faw::string_t type = _T(""), IDialogBuilderCallback* pCallback = nullptr, CPaintManagerUI* pManager = nullptr, std::shared_ptr<CControlUI> pParent = nullptr);
    std::shared_ptr<CControlUI> Create(IDialogBuilderCallback* pCallback = nullptr, CPaintManagerUI* pManager = nullptr, std::shared_ptr<CControlUI> pParent = nullptr);

    CMarkup* GetMarkup();

    faw::string_t GetLastErrorMessage() const;
    faw::string_t GetLastErrorLocation() const;
    void SetInstance(HINSTANCE instance)
    {
        m_instance = instance;
    };
private:
    std::shared_ptr<CControlUI> _Parse(CMarkupNode* parent, std::shared_ptr<CControlUI> pParent = nullptr, CPaintManagerUI* pManager = nullptr);

    CMarkup					m_xml;
    IDialogBuilderCallback	*m_pCallback	= nullptr;
    faw::string_t			m_pstrtype		= _T("");
    HINSTANCE				m_instance		= NULL;
};

} // namespace DuiLib

#endif // __UIDLGBUILDER_H__
