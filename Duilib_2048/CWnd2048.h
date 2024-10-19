#pragma once

#define BIND_BUTTON_UI(name) BindButtonUI name{ _T(#name),&m_pm };

class CWnd2048 : public WindowImplBase
{
	std::unique_ptr<CMoveControl> m_pMove;

	BIND_BUTTON_UI(bt1);
	BIND_BUTTON_UI(bt2);
	BIND_BUTTON_UI(bt3);
	BIND_BUTTON_UI(bt4);
	BIND_BUTTON_UI(bt5);
	BIND_BUTTON_UI(bt6);
	BIND_BUTTON_UI(bt7);
	BIND_BUTTON_UI(bt8);
	BIND_BUTTON_UI(bt9);
	BIND_BUTTON_UI(bt10);
	BIND_BUTTON_UI(bt11);
	BIND_BUTTON_UI(bt12);
	BIND_BUTTON_UI(bt13);
	BIND_BUTTON_UI(bt14);
	BIND_BUTTON_UI(bt15);
	BIND_BUTTON_UI(bt16);

	BindTextUI scores{ _T("scores"),&m_pm};

	RECT										m_rcOriginalWnd{0};
	RECT										m_rcScaleWnd{0};
	int											m_nOldDPI = 96;

public:
	virtual faw::string_t				GetSkinFile() override { return L"WndUI2048.xml"; };
	virtual LPCTSTR						GetWindowClassName(void) const override { return L"WndUI2048"; };

	virtual void						InitWindow() override;
	void								HandleChangedDPI();
	virtual std::optional<LRESULT>      OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam)override;
	virtual std::optional<LRESULT>		OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)override;
	std::optional<LRESULT>              OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam)override;
};

