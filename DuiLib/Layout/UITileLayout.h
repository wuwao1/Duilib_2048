#ifndef __UITILELAYOUT_H__
#define __UITILELAYOUT_H__

#pragma once

namespace DuiLib
{
class UILIB_API CTileLayoutUI: public CContainerUI
{
    DECLARE_DUICONTROL(CTileLayoutUI)
public:
    using ptr = std::shared_ptr<CTileLayoutUI>;
    CTileLayoutUI();

    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);

    void SetPos(RECT rc, bool bNeedInvalidate = true);

    SIZE GetItemSize() const;
    void SetItemSize(SIZE szItem);
    int GetColumns() const;
    void SetColumns(int nCols);

    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

protected:
    SIZE	    m_szItem	= { 0 };
    size_t		m_nColumns	= 1;
};
}
#endif // __UITILELAYOUT_H__
