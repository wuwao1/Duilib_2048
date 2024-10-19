#include "Main2048.h"


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE h, LPTSTR L, int nCmdShow) {
	try {

		CPaintManagerUI::SetInstance(hInstance);
		CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
		CPaintManagerUI::SetResourceZip(faw::string_t(_T("2048.zip")));

		std::unique_ptr<CWnd2048>  pFrame = MAKE_UNIQUE(CWnd2048);
		pFrame->Create(NULL, _T("DictionaryWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES);
		pFrame->CenterWindow();
		pFrame->ShowWindow();

		CPaintManagerUI::MessageLoop();
		CPaintManagerUI::Term();
		return 0;
	}
	catch (std::exception& e)
	{
		return -1;
	}

}