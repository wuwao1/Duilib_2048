#ifndef __BIND_BASE_H__
#define __BIND_BASE_H__

#pragma once

namespace DuiLib
{
class BindCtrlBase
{
public:
    BindCtrlBase(std::shared_ptr<CControlUI> _ctrl) :
        m_ctrl(_ctrl) {}
    BindCtrlBase(faw::string_t ctrl_name, CPaintManagerUI* pm)
    {
        m_ctrl_name = ctrl_name;
        m_pm = pm;
    }
    virtual ~BindCtrlBase() {}
    operator bool()
    {
        if(!m_pm)
            m_pm = CPaintManagerUI::GetPaintManager(L"");
        if(!m_pm)
            return false;
        if(!m_RootCtrl)
            m_RootCtrl = m_pm->GetRoot();
        if(m_RootCtrl != m_pm->GetRoot())
        {
            m_RootCtrl = m_pm->GetRoot();
            m_ctrl = m_pm->FindControl(m_ctrl_name);
        }
        if(!m_ctrl)
            m_ctrl = m_pm->FindControl(m_ctrl_name);
        return !!m_ctrl;
    }
protected:
    virtual faw::string_t           GetClassType() const = 0;
    virtual void                    binded() {}
    std::shared_ptr<CControlUI>     m_ctrl = nullptr;
    std::shared_ptr<CControlUI>     m_RootCtrl = nullptr;
    faw::string_t                   m_ctrl_name;
    CPaintManagerUI*                m_pm = nullptr;
};
}

#endif //__BIND_BASE_H__
