#include "Main2048.h"

void CWnd2048::InitWindow()
{
	GetWindowRect(this->m_hWnd, &m_rcOriginalWnd);
	HandleChangedDPI();

	m_pMove = std::make_unique<CMoveControl>();	
	m_pMove->AddButton(*bt1);
	m_pMove->AddButton(*bt2);
	m_pMove->AddButton(*bt3);
	m_pMove->AddButton(*bt4);
	m_pMove->AddButton(*bt5);
	m_pMove->AddButton(*bt6);
	m_pMove->AddButton(*bt7);
	m_pMove->AddButton(*bt8);
	m_pMove->AddButton(*bt9);
	m_pMove->AddButton(*bt10);
	m_pMove->AddButton(*bt11);
	m_pMove->AddButton(*bt12);
	m_pMove->AddButton(*bt13);
	m_pMove->AddButton(*bt14);
	m_pMove->AddButton(*bt15);
	m_pMove->AddButton(*bt16);
	m_pMove->AddTextScore(*scores);
	m_pMove->Init(); 
	m_pMove->Print();
}

void CWnd2048::HandleChangedDPI()
{
	GetWindowRect(this->GetHWND(), &m_rcScaleWnd);
	POINT WndLeftTopPt = { m_rcScaleWnd.left,m_rcScaleWnd.top };
	POINT WndCenterPt = {
		m_rcScaleWnd.left + (m_rcScaleWnd.right - m_rcScaleWnd.left) / 2 ,
		m_rcScaleWnd.top + (m_rcScaleWnd.bottom - m_rcScaleWnd.top) / 2
	};
	auto CurDPI = CDPI::GetDPIOfMonitorNearestToPoint(WndCenterPt);
	if (CurDPI != m_nOldDPI)
	{
		m_nOldDPI = CurDPI;
		m_pm.GetDPIObj()->SetScale(CurDPI);
		m_rcScaleWnd = m_pm.GetDPIObj()->Scale(m_rcOriginalWnd);
		SetWindowPos(this->GetHWND(), NULL, 0, 0, m_rcScaleWnd.right - m_rcScaleWnd.left, m_rcScaleWnd.bottom - m_rcScaleWnd.top, SWP_NOZORDER | SWP_NOMOVE);
		m_pm.SetDPI(CurDPI);
	}
}


std::optional<LRESULT> CWnd2048::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_UP)
	{
		m_pMove->KeyUp();
		m_pMove->Print();
	}
	else if (wParam == VK_RIGHT)
	{
		m_pMove->KeyRight();
		m_pMove->Print();
	}
	else if (wParam == VK_DOWN)
	{
		m_pMove->KeyDown();
		m_pMove->Print();
	}
	else if (wParam == VK_LEFT)
	{
		m_pMove->KeyLeft();
		m_pMove->Print();
	}
	else if (wParam == VK_SPACE)
	{
		m_pMove->Init();
		m_pMove->Print();
	}
	else if (wParam == VK_ESCAPE)
	{
		PostQuitMessage(0);
	}
	return std::nullopt;
}

std::optional<LRESULT> CWnd2048::OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::optional<LRESULT> lRes = 0;
	if (!m_pm.GetRoot())	
		return std::nullopt;
	m_nOldDPI = HIWORD(wParam);
	m_pm.GetDPIObj()->SetScale(m_nOldDPI);
	m_pm.SetDPI(m_nOldDPI);
	std::cout << "DPI: " << m_nOldDPI << std::endl;
	RECT* const prcNewWindow = (RECT*)lParam;
	SetWindowPos(m_hWnd,
		NULL,
		prcNewWindow->left,
		prcNewWindow->top,
		prcNewWindow->right - prcNewWindow->left,
		prcNewWindow->bottom - prcNewWindow->top,
		SWP_NOZORDER | SWP_NOACTIVATE);
	return lRes;
}

std::optional<LRESULT> CWnd2048::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
	RECT rcWork = oMonitor.rcWork;
	OffsetRect(&rcWork, -oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
	RECT rcWnd;
	GetWindowRect(this->GetHWND(), &rcWnd);
	
	lpMMI->ptMaxPosition.x = rcWork.left;
	lpMMI->ptMaxPosition.y = rcWork.top;
	lpMMI->ptMaxSize.x = rcWnd.right- rcWnd.left;
	lpMMI->ptMaxSize.y = rcWnd.bottom - rcWnd.top;
	lpMMI->ptMaxTrackSize.x = rcWnd.right - rcWnd.left;
	lpMMI->ptMaxTrackSize.y = rcWnd.bottom - rcWnd.top;

	if (m_rcOriginalWnd.left > 0 && m_rcScaleWnd.bottom > 0) {
		m_rcScaleWnd = m_pm.GetDPIObj()->Scale(m_rcOriginalWnd);
		lpMMI->ptMaxSize.x = m_rcScaleWnd.right - m_rcScaleWnd.left;
		lpMMI->ptMaxSize.y = m_rcScaleWnd.bottom - m_rcScaleWnd.top;
		lpMMI->ptMaxTrackSize.x = m_rcScaleWnd.right - m_rcScaleWnd.left;
		lpMMI->ptMaxTrackSize.y = m_rcScaleWnd.bottom - m_rcScaleWnd.top;
	}
	return 0;
}
