#pragma once
#include "skin_change_event.h"
class WindowImplBase;
class CRegDlg : public WindowImplBase, public SkinChangedReceiver
{
private:
	HWND m_parent;
	CEditUI m_RegCode;
	CString m_machine;
public:
	CRegDlg();
	CRegDlg(const tString& bgimage, DWORD bkcolor);
	~CRegDlg(void);
	LPCTSTR GetWindowClassName() const { return _T("RegDialog"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	virtual tString GetSkinFile();
	void OnExit(TNotifyUI& msg);
	virtual tString GetSkinFolder();
	void SetStr(CString str);
	void OnFinalMessage(HWND /*hWnd*/);
	void Init();
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void Notify(TNotifyUI& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	CPaintManagerUI m_pm;
	void SetParent(HWND hwnd);
	virtual BOOL Receive(SkinChangedParam param);
	tString bgimage_;
	DWORD bkcolor_;
};
