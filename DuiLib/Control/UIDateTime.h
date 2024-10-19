#ifndef __UIDATETIME_H__
#define __UIDATETIME_H__

#pragma once

namespace DuiLib
{
class CDateTimeWnd;

/// 时间选择控件
class UILIB_API CDateTimeUI: public CLabelUI
{
    DECLARE_DUICONTROL(CDateTimeUI)
    friend class CDateTimeWnd;
public:
    using ptr = std::shared_ptr<CDateTimeUI>;
    CDateTimeUI();
    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    SYSTEMTIME& GetTime();
    void SetTime(SYSTEMTIME* pst);

    void SetReadOnly(bool bReadOnly);
    bool IsReadOnly() const;

    void UpdateText();

    void DoEvent(TEventUI& event);

protected:
    SYSTEMTIME m_sysTime;
    int        m_nDTUpdateFlag;
    bool       m_bReadOnly;

    CDateTimeWnd* m_pWindow;
};
}
#endif // __UIDATETIME_H__