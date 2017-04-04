#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include "shellapi.h"


class MainFrame : public WindowImplBase,public IListCallbackUI
{
public:
	CRegDlg *m_regDlg;
	CString strPath,strMac;
	int bk_image_index_;
	MainFrame();
	~MainFrame();

public:
	LPCTSTR GetWindowClassName() const;	
	virtual void OnFinalMessage(HWND hWnd);
	virtual void Init();
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual tString GetSkinFile();
	virtual tString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    DWORD GetBkColor();
    void SetBkColor(DWORD dwBackColor);
	LRESULT OnTrayMessage(WPARAM wParam, LPARAM lParam);
protected:	
	
	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	void OnExit(TNotifyUI& msg);
	void OnTimer(TNotifyUI& msg);
	
private:
	BOOL fRetCode;
	CEditUI *m_NameEdit,*m_KeyEdit;
	CButtonUI *m_HostedNetworkButton;
	CComboBoxUI *m_ConnectionComboBox;
	CLabelUI *m_HostedNetworkStatusTxt,*m_Notify,*m_speed_up,*m_TotalDown,*m_TotalUp,*m_speed_down;
	COptionUI *m_auto;
	
	CListUI *m_DeviceListCtrl;

	NOTIFYICONDATA m_IconData;
private:
	CIcsManager * m_IcsMgr;// ICS manager	
	CWlanManager m_WlanMgr;// Wlan manager	
	CNotificationSink * m_NotificationSink;// Hosted network notification sink	
	CRefObjList<CWlanDevice *> m_WlanDeviceList;// List of WLAN devices	
	CRefObjList<CIcsConnectionInfo *> m_ConnectionList;// List of connections (adapters)
	vector<CIcsConnectionInfo *> m_SuportList;
	vector<CString> domain,MacStr;
	BOOL m_bIsAdmin;
	bool m_bIsICSAllowed;
	bool m_fProcessNotifications;	
	bool m_IcsNeeded;// whether softAP will be started with full ICS	
	bool m_IcsEnabled;// ICS enabled	
	bool m_HostedNetworkStarted;// Hosted network started	
	GUID m_HostedNetworkGuid;// GUID of hosted network adapter
	WCHAR m_strStartHostedNetwork[256];// messages for start/stop hosted network
	WCHAR m_strStopHostedNetwork[256];
	
	CAtlString m_CurrentName,m_CurrentKey;// Current name and key for the hosted network
	DWORD dwIfBufSize;
	MIB_IFTABLE *m_pMIT;
	long long m_oldDownloadOctets,m_oldUploadOctets,m_curUploadOctets,m_curDownloadOctets;
	double totalUp,totalDown;
public:
	void DisableAllControls();
private:
	void OnReg();
	void OnBnClickedAuto();
	bool IsICSAllowed();
	BOOL IsUserAdmin();
	void PostNotification(  LPCWSTR msg   ) ;
	void PostDeviceNotification( CWlanDevice * pDevice,int Event);
	bool CheckValidSSIDKeyLen();
	void OnEnKillfocusEditKey();
	void OnEnKillfocusEditName();
	void EnableAllControls();
	bool StopIcsIfNeeded();
	HRESULT StartIcsIfNeeded();
	void DeinitIcs();
	HRESULT InitIcs();
	void UpdateIcsConnectionList();
	void GetIcsInfo();
	HRESULT StopHostedNetwork();
	HRESULT StartHostedNetwork();
	void OnDeviceUpdate( CWlanDevice * pDevice);
	void OnDeviceRemove( CWlanDevice * pDevice);
	void OnDeviceAdd(CWlanDevice * pDevice );
	void OnHostedNetworkAvailable();
	void OnHostedNetworkNotAvailable();
	void OnHostedNetworkStopped();
	void OnHostedNetworkStarted();
	void GetWlanDeviceInfo();
	void GetHostedNetworkInfo();
	void ProcessNotifications();
	HRESULT InitWlan();
	void OnBnClickedButtonHostednetwork();
	void PostErrorMessage(LPCTSTR str);
	LRESULT OnTimer(INT_PTR nID);
	void GetSpeed();
	LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);
public:
	void SetWindowText(CString str);
	SkinChangedObserver skin_changed_observer_;
};

#endif // MAINFRAME_HPP