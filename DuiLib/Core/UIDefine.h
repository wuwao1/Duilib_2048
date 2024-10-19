﻿#pragma once

namespace DuiLib
{
#define MAX_FONT_ID		30000

//TIMER ID集中管理,避免雷同
#define CARET_TIMER_ID	            10000
#define FADE_BUTTON_IN_ID           10001
#define FADE_BUTTON_OUT_ID          10002
#define GIF_EX_TIMER_ID	            10003
#define	GIF_TIMER_ID                10004
#define	RICHEDIT_TIMER_ID           10005
#define	RING_TIMER_ID               10006
#define ROLLTEXT_TIMER_ID			10007
#define ROLLTEXT_ROLL_END_ID		10008
#define	SCROLL_TIMER_ID             10009
#define	TAB_ANIMATION_ID            10010
#define	SHOW_CONTANIER_ID           10011
#define	HIDE_CONTANIER_ID           10012

// 列表类型
enum ListType
{
    LT_LIST = 0,
    LT_COMBO,
    LT_TREE,
    LT_MENU,
};

// 鼠标光标定义
#define DUI_ARROW           32512
#define DUI_IBEAM           32513
#define DUI_WAIT            32514
#define DUI_CROSS           32515
#define DUI_UPARROW         32516
#define DUI_SIZE            32640
#define DUI_ICON            32641
#define DUI_SIZENWSE        32642
#define DUI_SIZENESW        32643
#define DUI_SIZEWE          32644
#define DUI_SIZENS          32645
#define DUI_SIZEALL         32646
#define DUI_NO              32648
#define DUI_HAND            32649

enum class DuiEvent
{
    nofound,
    menu,
    link,
    timer,
    click,
    dbclick,
    _return,
    scroll,
    predropdown,
    dropdown,
    setfocus,
    killfocus,
    itemclick,
    itemrclick,
    tabselect,
    itemselect,
    itemexpand,
    windowinit,
    windowsize,
    buttondown,
    mouseenter,
    mouseleave,
    textchanged,
    headerclick,
    itemdbclick,
    showactivex,
    itemcollapse,
    itemactivate,
    valuechanged,
    movevaluechanged,
    selectchanged,
    unselected,
    treeitemdbclick,
    checkclick,
    textrollend,
    colorchanged,
    listitemselect,
    listitemchecked,
    comboitemselect,
    listheaderclick,
    listheaditemchecked,
    listpagechanged
};


// 消息类型
enum DuiSig
{
    DuiSig_end = 0, // [marks end of message map]
    DuiSig_lwl,     // LRESULT (WPARAM, LPARAM)
    DuiSig_vn,      // void (TNotifyUI)
};

// 核心控件
class CControlUI;

// Structure for notifications to the outside world
typedef struct UILIB_API tagTNotifyUI
{
    faw::string_t sType;
    faw::string_t sVirtualWnd;
    std::shared_ptr<CControlUI> pSender;
    DWORD dwTimestamp;
    POINT ptMouse = { 0 };
    WPARAM wParam;
    LPARAM lParam;
    static inline std::map<faw::string_t, DuiEvent > StrEventMap{};
    static DuiEvent GetEventType(faw::string_t sType);
} TNotifyUI;

class CNotifyPump;
typedef void (CNotifyPump::*DUI_PMSG)(TNotifyUI& msg);  //指针类型

union DuiMessageMapFunctions
{
    DUI_PMSG pfn;   // generic member function pointer
    LRESULT(CNotifyPump::*pfn_Notify_lwl)(WPARAM, LPARAM);
    void (CNotifyPump::*pfn_Notify_vn)(TNotifyUI&);
};

//定义所有消息类型
//////////////////////////////////////////////////////////////////////////

#define DUI_MSGTYPE_MENU                   (_T("menu"))
#define DUI_MSGTYPE_LINK                   (_T("link"))

#define DUI_MSGTYPE_TIMER                  (_T("timer"))
#define DUI_MSGTYPE_CLICK                  (_T("click"))
#define DUI_MSGTYPE_DBCLICK                (_T("dbclick"))

#define DUI_MSGTYPE_RETURN                 (_T("return"))
#define DUI_MSGTYPE_SCROLL                 (_T("scroll"))

#define DUI_MSGTYPE_PREDROPDOWN            (_T("predropdown"))
#define DUI_MSGTYPE_DROPDOWN               (_T("dropdown"))
#define DUI_MSGTYPE_SETFOCUS               (_T("setfocus"))

#define DUI_MSGTYPE_KILLFOCUS              (_T("killfocus"))
#define DUI_MSGTYPE_ITEMCLICK 		   	   (_T("itemclick"))
#define DUI_MSGTYPE_ITEMRCLICK 			   (_T("itemrclick"))
#define DUI_MSGTYPE_TABSELECT              (_T("tabselect"))

#define DUI_MSGTYPE_ITEMSELECT 		   	   (_T("itemselect"))
#define DUI_MSGTYPE_ITEMEXPAND             (_T("itemexpand"))
#define DUI_MSGTYPE_WINDOWINIT             (_T("windowinit"))
#define DUI_MSGTYPE_WINDOWSIZE             (_T("windowsize"))
#define DUI_MSGTYPE_BUTTONDOWN 		   	   (_T("buttondown"))
#define DUI_MSGTYPE_MOUSEENTER			   (_T("mouseenter"))
#define DUI_MSGTYPE_MOUSELEAVE			   (_T("mouseleave"))

#define DUI_MSGTYPE_TEXTCHANGED            (_T("textchanged"))
#define DUI_MSGTYPE_HEADERCLICK            (_T("headerclick"))
#define DUI_MSGTYPE_ITEMDBCLICK            (_T("itemdbclick"))
#define DUI_MSGTYPE_SHOWACTIVEX            (_T("showactivex"))

#define DUI_MSGTYPE_ITEMCOLLAPSE           (_T("itemcollapse"))
#define DUI_MSGTYPE_ITEMACTIVATE           (_T("itemactivate"))
#define DUI_MSGTYPE_VALUECHANGED           (_T("valuechanged"))
#define DUI_MSGTYPE_VALUECHANGED_MOVE      (_T("movevaluechanged"))

#define DUI_MSGTYPE_SELECTCHANGED 		   (_T("selectchanged"))
#define DUI_MSGTYPE_UNSELECTED	 		   (_T("unselected"))

#define DUI_MSGTYPE_TREEITEMDBCLICK 		(_T("treeitemdbclick"))
#define DUI_MSGTYPE_CHECKCLICK				(_T("checkclick"))
#define DUI_MSGTYPE_TEXTROLLEND 			(_T("textrollend"))
#define DUI_MSGTYPE_COLORCHANGED		    (_T("colorchanged"))

#define DUI_MSGTYPE_LISTITEMSELECT 		   	(_T("listitemselect"))
#define DUI_MSGTYPE_LISTITEMCHECKED 		(_T("listitemchecked"))
#define DUI_MSGTYPE_COMBOITEMSELECT 		(_T("comboitemselect"))
#define DUI_MSGTYPE_LISTHEADERCLICK			(_T("listheaderclick"))
#define DUI_MSGTYPE_LISTHEADITEMCHECKED		(_T("listheaditemchecked"))
#define DUI_MSGTYPE_LISTPAGECHANGED			(_T("listpagechanged"))


//////////////////////////////////////////////////////////////////////////

struct DUI_MSGMAP_ENTRY;
struct DUI_MSGMAP
{
#ifndef UILIB_STATIC
    const DUI_MSGMAP* (PASCAL* pfnGetBaseMap)();
#else
    const DUI_MSGMAP* pBaseMap;
#endif
    const DUI_MSGMAP_ENTRY* lpEntries;
};

//结构定义
struct DUI_MSGMAP_ENTRY //定义一个结构体，来存放消息信息
{
    faw::string_t sMsgType;          // DUI消息类型
    faw::string_t sCtrlName;         // 控件名称
    UINT       nSig;              // 标记函数指针类型
    DUI_PMSG   pfn;               // 指向函数的指针
};

//定义
#ifndef UILIB_STATIC
#define DUI_DECLARE_MESSAGE_MAP()                                         \
private:                                                                  \
	static const DUI_MSGMAP_ENTRY _messageEntries[];                      \
protected:                                                                \
	static const DUI_MSGMAP messageMap;                                   \
	static const DUI_MSGMAP* PASCAL _GetBaseMessageMap();                 \
	virtual const DUI_MSGMAP* GetMessageMap() const;                      \

#else
#define DUI_DECLARE_MESSAGE_MAP()                                         \
private:                                                                  \
	static const DUI_MSGMAP_ENTRY _messageEntries[];                      \
protected:                                                                \
	static  const DUI_MSGMAP messageMap;				                  \
	virtual const DUI_MSGMAP* GetMessageMap() const;                      \

#endif


//基类声明开始
#ifndef UILIB_STATIC
#define DUI_BASE_BEGIN_MESSAGE_MAP(theClass)                              \
	const DUI_MSGMAP* PASCAL theClass::_GetBaseMessageMap()               \
	{ return nullptr; }                                                  \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{  &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] };\
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#else
#define DUI_BASE_BEGIN_MESSAGE_MAP(theClass)                              \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{  nullptr, &theClass::_messageEntries[0] };                         \
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#endif


//子类声明开始
#ifndef UILIB_STATIC
#define DUI_BEGIN_MESSAGE_MAP(theClass, baseClass)                        \
	const DUI_MSGMAP* PASCAL theClass::_GetBaseMessageMap()               \
	{ return &baseClass::messageMap; }                                \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{ &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] }; \
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#else
#define DUI_BEGIN_MESSAGE_MAP(theClass, baseClass)                        \
	const DUI_MSGMAP* theClass::GetMessageMap() const                     \
	{ return &theClass::messageMap; }                                 \
	UILIB_COMDAT const DUI_MSGMAP theClass::messageMap =                  \
	{ &baseClass::messageMap, &theClass::_messageEntries[0] };        \
	UILIB_COMDAT const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =     \
	{                                                                     \

#endif


//声明结束
#define DUI_END_MESSAGE_MAP()                                             \
	{ _T(""), _T(""), DuiSig_end, (DUI_PMSG)0 }                           \
	};                                                                    \


//定义消息类型--执行函数宏
#define DUI_ON_MSGTYPE(msgtype, memberFxn)                                \
	{ msgtype, _T(""), DuiSig_vn, (DUI_PMSG)&memberFxn},                  \


//定义消息类型--控件名称--执行函数宏
#define DUI_ON_MSGTYPE_CTRNAME(msgtype,ctrname,memberFxn)                 \
	{ msgtype, ctrname, DuiSig_vn, (DUI_PMSG)&memberFxn },                \


//定义click消息的控件名称--执行函数宏
#define DUI_ON_CLICK_CTRNAME(ctrname,memberFxn)                           \
	{ DUI_MSGTYPE_CLICK, ctrname, DuiSig_vn, (DUI_PMSG)&memberFxn },      \


//定义selectchanged消息的控件名称--执行函数宏
#define DUI_ON_SELECTCHANGED_CTRNAME(ctrname,memberFxn)                   \
	{ DUI_MSGTYPE_SELECTCHANGED,ctrname,DuiSig_vn,(DUI_PMSG)&memberFxn }, \


//定义killfocus消息的控件名称--执行函数宏
#define DUI_ON_KILLFOCUS_CTRNAME(ctrname,memberFxn)                       \
	{ DUI_MSGTYPE_KILLFOCUS,ctrname,DuiSig_vn,(DUI_PMSG)&memberFxn },     \


//定义menu消息的控件名称--执行函数宏
#define DUI_ON_MENU_CTRNAME(ctrname,memberFxn)                            \
	{ DUI_MSGTYPE_MENU,ctrname,DuiSig_vn,(DUI_PMSG)&memberFxn },          \


//定义与控件名称无关的消息宏

//定义timer消息--执行函数宏
#define DUI_ON_TIMER()                                                    \
	{ DUI_MSGTYPE_TIMER, _T(""), DuiSig_vn,(DUI_PMSG)&OnTimer },          \


///
//////////////END消息映射宏定义////////////////////////////////////////////////////


//////////////BEGIN控件名称宏定义//////////////////////////////////////////////////
///

#define  DUI_CTRL_EDIT                            (_T("Edit"))
#define  DUI_CTRL_LIST                            (_T("List"))
#define  DUI_CTRL_TEXT                            (_T("Text"))

#define  DUI_CTRL_COMBO                           (_T("Combo"))
#define  DUI_CTRL_LABEL                           (_T("Label"))
#define  DUI_CTRL_FLASH							 (_T("Flash"))

#define  DUI_CTRL_BUTTON                          (_T("Button"))
#define  DUI_CTRL_OPTION                          (_T("Option"))
#define  DUI_CTRL_SLIDER                          (_T("Slider"))

#define  DUI_CTRL_CONTROL                         (_T("Control"))
#define  DUI_CTRL_ACTIVEX                         (_T("ActiveX"))
#define  DUI_CTRL_GIFANIM                         (_T("GifAnim"))

#define  DUI_CTRL_LISTITEM                        (_T("ListItem"))
#define  DUI_CTRL_PROGRESS                        (_T("Progress"))
#define  DUI_CTRL_RICHEDIT                        (_T("RichEdit"))
#define  DUI_CTRL_CHECKBOX                        (_T("CheckBox"))
#define  DUI_CTRL_COMBOBOX                        (_T("ComboBox"))
#define  DUI_CTRL_DATETIME                        (_T("DateTime"))
#define  DUI_CTRL_TREEVIEW                        (_T("TreeView"))
#define  DUI_CTRL_TREENODE                        (_T("TreeNode"))

#define  DUI_CTRL_CONTAINER                       (_T("Container"))
#define  DUI_CTRL_TABLAYOUT                       (_T("TabLayout"))
#define  DUI_CTRL_SCROLLBAR                       (_T("ScrollBar"))
#define  DUI_CTRL_IPADDRESS                       (_T("IPAddress"))

#define  DUI_CTRL_LISTHEADER                      (_T("ListHeader"))
#define  DUI_CTRL_LISTFOOTER                      (_T("ListFooter"))
#define  DUI_CTRL_TILELAYOUT                      (_T("TileLayout"))
#define  DUI_CTRL_WEBBROWSER                      (_T("WebBrowser"))

#define  DUI_CTRL_CHILDLAYOUT                     (_T("ChildLayout"))
#define  DUI_CTRL_LISTELEMENT                     (_T("ListElement"))

#define  DUI_CTRL_VERTICALLAYOUT                  (_T("VerticalLayout"))
#define  DUI_CTRL_LISTHEADERITEM                  (_T("ListHeaderItem"))

#define  DUI_CTRL_LISTTEXTELEMENT                 (_T("ListTextElement"))

#define  DUI_CTRL_HORIZONTALLAYOUT                (_T("HorizontalLayout"))
#define  DUI_CTRL_LISTLABELELEMENT                (_T("ListLabelElement"))

#define  DUI_CTRL_ANIMATIONTABLAYOUT              (_T("AnimationTabLayout"))

#define  DUI_CTRL_LISTCONTAINERELEMENT            (_T("ListContainerElement"))

#define  DUI_CTRL_TEXTSCROLL                      (_T("TextScroll"))

#define DUI_CTRL_COLORPALETTE					  (_T("ColorPalette"))
///
//////////////END控件名称宏定义//////////////////////////////////////////////////

}// namespace DuiLib

