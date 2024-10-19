#ifndef __BIND_CTRLS_HPP__
#define __BIND_CTRLS_HPP__

#pragma once

namespace DuiLib
{
#define DEF_BINDCTRL(CTRL_TYPE) class Bind##CTRL_TYPE##UI: public BindCtrlBase {\
	public: \
		Bind##CTRL_TYPE##UI (std::shared_ptr<CControlUI> _ctrl): BindCtrlBase (_ctrl) {}\
		Bind##CTRL_TYPE##UI (faw::string_t ctrl_name, CPaintManagerUI *pm = nullptr): BindCtrlBase (ctrl_name,pm) {}	\
		std::shared_ptr<C##CTRL_TYPE##UI> operator* () {\
			if (!*this)\
				throw std::exception ("BindControlUI bind failed");\
			return  std::dynamic_pointer_cast<C##CTRL_TYPE##UI> (m_ctrl);\
		}\
		std::shared_ptr<C##CTRL_TYPE##UI> operator-> () { return operator* (); }\
	protected:\
		faw::string_t GetClassType () const override { return _T (#CTRL_TYPE##"UI"); }\
	}



// Core
DEF_BINDCTRL(Control);
//class BindControlUI : public BindCtrlBase {
//public:
//	BindControlUI(std::shared_ptr<CControlUI> _ctrl) : BindCtrlBase(_ctrl) {}
//	BindControlUI(faw::string_t ctrl_name, CPaintManagerUI* pm = nullptr) : BindCtrlBase(ctrl_name, pm) {}
//	std::shared_ptr<CControlUI> operator* () {
//		if (!*this) throw std::exception("BindControlUI bind failed"); return static_cast<std::shared_ptr<CControlUI>> (m_ctrl);
//	}
//	std::shared_ptr<CControlUI> operator-> () {
//		return operator* ();
//	}
//protected:
//	faw::string_t GetClassType() const override {
//	return L"Control""UI";
//	}
//}
DEF_BINDCTRL(Container);

// Control
DEF_BINDCTRL(ActiveX);
//DEF_BINDCTRL (Animation);
DEF_BINDCTRL(Button);
DEF_BINDCTRL(ColorPalette);
DEF_BINDCTRL(Combo);
DEF_BINDCTRL(ComboBox);
DEF_BINDCTRL(DateTime);
DEF_BINDCTRL(Edit);
DEF_BINDCTRL(FadeButton);
DEF_BINDCTRL(Flash);
DEF_BINDCTRL(GifAnim);
#ifdef USE_XIMAGE_EFFECT
DEF_BINDCTRL(GifAnimEx);
#endif
DEF_BINDCTRL(GroupBox);
DEF_BINDCTRL(HotKey);
DEF_BINDCTRL(IPAddress);
DEF_BINDCTRL(IPAddressEx);
DEF_BINDCTRL(Label);
DEF_BINDCTRL(List);
DEF_BINDCTRL(ListEx);
DEF_BINDCTRL(Menu);
DEF_BINDCTRL(Option);
DEF_BINDCTRL(CheckBox);
DEF_BINDCTRL(Progress);
DEF_BINDCTRL(RichEdit);
DEF_BINDCTRL(Ring);
DEF_BINDCTRL(RollText);
DEF_BINDCTRL(ScrollBar);
DEF_BINDCTRL(Slider);
DEF_BINDCTRL(Text);
DEF_BINDCTRL(TreeView);
DEF_BINDCTRL(WebBrowser);

// Layout
DEF_BINDCTRL(AnimationTabLayout);
DEF_BINDCTRL(ChildLayout);
DEF_BINDCTRL(HorizontalLayout);
DEF_BINDCTRL(TabLayout);
DEF_BINDCTRL(TileLayout);
DEF_BINDCTRL(VerticalLayout);
}

#endif //__BIND_CTRLS_HPP__
