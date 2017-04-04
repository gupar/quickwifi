#include "StdAfx.h"
#include "RegDlg.h"

const TCHAR* const kBackgroundControlName = _T("bg");
CRegDlg::CRegDlg(const tString& bgimage, DWORD bkcolor)
	: bgimage_(bgimage)
	, bkcolor_(bkcolor)
{}

CRegDlg::CRegDlg()
{

}
CRegDlg::~CRegDlg(void)
{
	return;
}

LRESULT CRegDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

void CRegDlg::OnExit(TNotifyUI& msg)
{
	Close();
}

tString CRegDlg::GetSkinFile()
{
	return _T("reg.xml");
}

tString CRegDlg::GetSkinFolder()
{
	return tString(CPaintManagerUI::GetInstancePath());
}

void CRegDlg::SetStr(CString str)
{
	this->m_machine = str;
	CEditUI *code = static_cast<CEditUI*>(paint_manager_.FindControl(L"code"));
	if (code)
	{
		code->SetText(str);
	}
}

void CRegDlg::Init()
{
	CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
	if (background != NULL)
	{
		background->SetBkImage(bgimage_.c_str());
		background->SetBkColor(bkcolor_);
	}
}

void CRegDlg::Notify(TNotifyUI& msg)
{
	if( msg.sType == _T("click") ) 
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("closebtn")) == 0)
		{
			Close();
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("submit")) == 0)
		{
			CEditUI *regcode = static_cast<CEditUI*>(paint_manager_.FindControl(L"regcode"));
			if (regcode)
			{
				CString str =regcode->GetText();
				if (IsCompare(m_machine,str))
				{
					TCHAR value[MAX_PATH];
					wcscpy_s(value,str);
					HKEY Key;
					CString sKeyPath = L"Software\\Gupar\\WiFi";
					::RegCreateKey(HKEY_LOCAL_MACHINE,sKeyPath,&Key);
					::RegSetValueEx(Key,L"RegCode",0,REG_SZ,(BYTE*)value,(str.GetLength()+1)*2);
					::SendMessage(m_parent,WM_HASREG,NULL,NULL);
					Close();
				}
				else
				{
					MessageBeep(MB_ICONERROR);
					MessageBox(GetHWND(),L"¼¤»îÂë´íÎó",L"´íÎó",NULL);
				}
			}
			
		}
		if (_tcsicmp(msg.pSender->GetName(), _T("getcode")) == 0)
		{
			CString str = m_machine.MakeLower();
			ShellExecute(NULL,L"open",L"http://gupartool.duapp.com/getcode.php?mac="+str+L"&toolname=wifi",NULL,NULL,SW_SHOW);
		}
	}
}

LRESULT CRegDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CRegDlg::OnFinalMessage(HWND hWnd) 
{
	RemoveObserver();
	WindowImplBase::OnFinalMessage(hWnd);
	::SendMessage(m_parent,WM_REGCLOSE,NULL,NULL);
//	delete this;
};

void CRegDlg::SetParent(HWND hwnd)
{
	m_parent = hwnd;
}

BOOL CRegDlg::Receive(SkinChangedParam param)
{
	bgimage_ = param.bgimage;
	bkcolor_ = param.bkcolor;
	CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
	if (background != NULL)
	{
		if (!param.bgimage.empty())
		{
			TCHAR szBuf[MAX_PATH] = {0};
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("file='%s' corner='600,200,1,1'"), param.bgimage.c_str());
#else
			_stprintf_s(szBuf, MAX_PATH - 1, _T("file='%s' corner='600,200,1,1'"), param.bgimage.c_str());
#endif
			background->SetBkImage(szBuf);
		}
		else
			background->SetBkImage(_T(""));

		background->SetBkColor(param.bkcolor);
	}

	return TRUE;
}
