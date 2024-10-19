#ifndef UITreeView_h__
#define UITreeView_h__

#pragma once

#include <vector>

namespace DuiLib
{
class CTreeViewUI;
class CCheckBoxUI;
class CLabelUI;
class COptionUI;

class UILIB_API CTreeNodeUI: public CListContainerElementUI
{
    DECLARE_DUICONTROL(CTreeNodeUI)
public:
    using ptr = std::shared_ptr<CTreeNodeUI>;
    CTreeNodeUI(std::shared_ptr<CTreeNodeUI> _ParentNode = nullptr);
    virtual ~CTreeNodeUI(void);

public:
    faw::string_t GetClass() const;
    std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    void DoEvent(TEventUI& event);
    void Invalidate();
    bool Select(bool bSelect = true);
    bool SelectMulti(bool bSelect = true);

    bool Add(std::shared_ptr<CControlUI> _pTreeNodeUI);
    bool AddAt(std::shared_ptr<CControlUI> pControl, int iIndex);
    bool Remove(std::shared_ptr<CControlUI> pControl);

    void SetVisibleTag(bool _IsVisible);
    bool GetVisibleTag();
    void SetItemText(faw::string_t pstrValue);
    faw::string_t GetItemText();
    void CheckBoxSelected(bool _Selected);
    bool IsCheckBoxSelected() const;
    bool IsHasChild() const;
    long GetTreeLevel();
    bool AddChildNode(std::shared_ptr<CTreeNodeUI> _pTreeNodeUI);
    bool RemoveAt(std::shared_ptr<CTreeNodeUI> _pTreeNodeUI);
    bool RemoveAllNodes();
    bool RemoveAt(int index)
    {
        return __super::RemoveAt(index);
    };
    void SetParentNode(std::shared_ptr<CTreeNodeUI> _pParentTreeNode);
    std::shared_ptr<CTreeNodeUI> GetParentNode();
    long GetCountChild();
    void SetTreeView(std::shared_ptr<CTreeViewUI> _CTreeViewUI);
    std::shared_ptr<CTreeViewUI> GetTreeView();
    std::shared_ptr<CTreeNodeUI> GetChildNode(int _nIndex);
    void SetVisibleFolderBtn(bool _IsVisibled);
    bool GetVisibleFolderBtn();
    void SetVisibleCheckBtn(bool _IsVisibled);
    bool GetVisibleCheckBtn();
    void SetItemTextColor(DWORD _dwItemTextColor);
    DWORD GetItemTextColor() const;
    void SetItemHotTextColor(DWORD _dwItemHotTextColor);
    DWORD GetItemHotTextColor() const;
    void SetSelItemTextColor(DWORD _dwSelItemTextColor);
    DWORD GetSelItemTextColor() const;
    void SetSelItemHotTextColor(DWORD _dwSelHotItemTextColor);
    DWORD GetSelItemHotTextColor() const;
    void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    std::vector<std::shared_ptr<CControlUI>>& GetTreeNodes();
    int			 GetTreeIndex();
    int			 GetNodeIndex();

public:
    std::shared_ptr<CHorizontalLayoutUI> GetTreeNodeHoriznotal() const
    {
        return pHoriz;
    };
    std::shared_ptr<CCheckBoxUI>  GetFolderButton() const
    {
        return pFolderButton;
    };
    std::shared_ptr<CLabelUI> GetDottedLine() const
    {
        return pDottedLine;
    };
    std::shared_ptr<CCheckBoxUI> GetCheckBox() const
    {
        return pCheckBox;
    };
    std::shared_ptr<COptionUI> GetItemButton() const
    {
        return pItemButton;
    };

private:
    std::shared_ptr<CTreeNodeUI> GetLastNode();
    std::shared_ptr<CTreeNodeUI> CalLocation(std::shared_ptr<CTreeNodeUI> _pTreeNodeUI);

private:
    long					m_iTreeLavel			= 0;
    bool					m_bIsVisable			= true;
    bool					m_bIsCheckBox			= false;
    DWORD					m_dwItemTextColor		= 0x00000000;
    DWORD					m_dwItemHotTextColor	= 0;
    DWORD					m_dwSelItemTextColor	= 0;
    DWORD					m_dwSelItemHotTextColor	= 0;

    std::shared_ptr<CTreeViewUI>                pTreeView	        = nullptr;
    std::shared_ptr<CHorizontalLayoutUI>		pHoriz              = nullptr;
    std::shared_ptr<CCheckBoxUI>				pFolderButton       = nullptr;
    std::shared_ptr<CLabelUI>				    pDottedLine         = nullptr;
    std::shared_ptr<CCheckBoxUI>				pCheckBox           = nullptr;
    std::shared_ptr<COptionUI>				    pItemButton         = nullptr;
    std::shared_ptr<CTreeNodeUI>				pParentTreeNode	    = nullptr;
    std::vector<std::shared_ptr<CControlUI>>	mTreeNodes;
};

class UILIB_API CTreeViewUI: public CListUI, public INotifyUI
{
    DECLARE_DUICONTROL(CTreeViewUI)
public:
    using ptr = std::shared_ptr<CTreeViewUI>;
    CTreeViewUI(void);
    virtual ~CTreeViewUI(void);

public:
    virtual faw::string_t GetClass() const;
    virtual std::shared_ptr<CControlUI> 	GetInterface(faw::string_t pstrName);

    virtual UINT GetListType();
    virtual bool Add(std::shared_ptr<CTreeNodeUI> pControl);
    virtual bool Add(std::shared_ptr<CControlUI> pControl)
    {
        return __super::Add(pControl);
    };
    virtual long AddAt(std::shared_ptr<CTreeNodeUI> pControl, int iIndex);
    virtual bool AddAt(std::shared_ptr<CControlUI> pControl, int iIndex)
    {
        return __super::AddAt(pControl, iIndex);
    };
    virtual bool AddAt(std::shared_ptr<CTreeNodeUI> pControl, std::shared_ptr<CTreeNodeUI> _IndexNode);
    virtual bool Remove(std::shared_ptr<CTreeNodeUI> pControl);
    virtual bool Remove(std::shared_ptr<CControlUI> pControl)
    {
        return __super::Remove(pControl);
    };
    virtual bool RemoveAt(int iIndex);
    virtual void RemoveAll();
    virtual bool OnCheckBoxChanged(void* param);
    virtual bool OnFolderChanged(void* param);
    virtual bool OnDBClickItem(void* param);
    virtual bool SetItemCheckBox(bool _Selected, std::shared_ptr<CTreeNodeUI> _TreeNode = nullptr);
    virtual void SetItemExpand(bool _Expanded, std::shared_ptr<CTreeNodeUI> _TreeNode = nullptr);
    virtual void Notify(TNotifyUI& msg);
    virtual void SetVisibleFolderBtn(bool _IsVisibled);
    virtual bool GetVisibleFolderBtn();
    virtual void SetVisibleCheckBtn(bool _IsVisibled);
    virtual bool GetVisibleCheckBtn();
    virtual void SetItemMinWidth(UINT _ItemMinWidth);
    virtual UINT GetItemMinWidth();
    virtual void SetItemTextColor(DWORD _dwItemTextColor);
    virtual void SetItemHotTextColor(DWORD _dwItemHotTextColor);
    virtual void SetSelItemTextColor(DWORD _dwSelItemTextColor);
    virtual void SetSelItemHotTextColor(DWORD _dwSelHotItemTextColor);

    virtual void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);
private:
    UINT m_uItemMinWidth;
    bool m_bVisibleFolderBtn;
    bool m_bVisibleCheckBtn;
};
}


#endif // UITreeView_h__
