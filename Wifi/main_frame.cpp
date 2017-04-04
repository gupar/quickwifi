#include "stdafx.h"
#if !defined(UNDER_CE)
#endif
#include "resource.h"

#define MAX_KEY_LEN  63
#define WM_TRARMESSAGE WM_USER+100
#define MAX_NOTIFICATION_LENGTH 1024

const TCHAR* const kNetNameControlName = _T("netname");
const TCHAR* const kPasswordControlName = _T("passowrd");
const TCHAR* const kConnectControlName = _T("netinputs");
const TCHAR* const kDeviceListControlName = _T("devicelist");
const TCHAR* const kTitleControlName = _T("WinText");
const TCHAR* const kNotifyMsgControlName = _T("notifyMsg");
const TCHAR* const kSystemMsgControlName = _T("msgbox");
const TCHAR* const kCloseButtonControlName = _T("closebtn");
const TCHAR* const kMinButtonControlName = _T("minbtn");
const TCHAR* const kChangeBkSkinControlName = _T("bkskinbtn");
const TCHAR* const kChangeColorSkinControlName = _T("colorskinbtn");
const TCHAR* const kBackgroundControlName = _T("bg");
const TCHAR* const kFuncBtnControlName = _T("loginBtn");
const TCHAR* const kUpSpeedControlName = _T("uploadSpeed");
const TCHAR* const kDownSpeedControlName = _T("downloadSpeed");
const TCHAR* const kTotalUPControlName = _T("totalUpload");
const TCHAR* const kTotalDownControlName = _T("totalDownLoad");

const int kBackgroundSkinImageCount = 3;


CString GUIDToString(GUID _guid)
{
	CString result;
	result.Format(L"{%X-%X-%X-%.02X%.02X-%.02X%.02X%.02X%.02X%.02X%.02X}",_guid.Data1,_guid.Data2,_guid.Data3,_guid.Data4[0],_guid.Data4[1],_guid.Data4[2],_guid.Data4[3],_guid.Data4[4],_guid.Data4[5],_guid.Data4[6],_guid.Data4[7]);
	return result;
}

DWORD WINAPI checkVision(LPVOID lpParameter)
{
	int num = -2;
	MainFrame *pDlg = (MainFrame *)lpParameter;
	CString str;
	CString address = L"http://gupartool.duapp.com/devicecheck.php?mac="+pDlg->strMac+L"&toolname=wifi";
	char buffer[MAX_PATH];
	DWORD dwBytesRead = 0;
	HINTERNET hNet = ::InternetOpen(L"GUPAR",PRE_CONFIG_INTERNET_ACCESS,NULL,INTERNET_INVALID_PORT_NUMBER,0) ;
	HINTERNET hUrlFile = ::InternetOpenUrl(hNet,address,NULL,0,INTERNET_FLAG_RELOAD,0) ;	
	BOOL bRead = ::InternetReadFile(hUrlFile,buffer,sizeof(buffer),	&dwBytesRead);
	::InternetCloseHandle(hUrlFile) ;
	::InternetCloseHandle(hNet) ;
	if (bRead)
	{
		if (dwBytesRead)
		{
			char *tmp = new char[MAX_PATH];
			memcpy(tmp,buffer,dwBytesRead);
			int n = MultiByteToWideChar( CP_UTF8, 0,(LPCSTR)(LPCTSTR)tmp, -1, NULL, 0 );
			wstring result;
			result.resize(n);
			::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)(LPCTSTR)tmp, -1, (LPWSTR)result.c_str(), n);
			num = _ttoi(result.c_str());
		}
	}
	if (num>=0 && num<10)
	{
		pDlg->SetWindowText(L"QuickWifi—试用版");
	}
	else if (num >=10)
	{
		BOOL isReg = FALSE;
		HKEY Key;
		CString sKeyPath = L"Software\\Gupar\\WiFi";
		if(RegOpenKey(HKEY_LOCAL_MACHINE,sKeyPath,&Key)!=2)//已经存在注册信息
		{
			LPBYTE Data=new BYTE[80];
			DWORD TYPE=REG_SZ;
			DWORD cbData=80;
			//取出已记载的数量
			::RegQueryValueEx(Key,L"RegCode",0,&TYPE,Data,&cbData);
			CString str;
			str.Format(L"%s",Data);
			isReg = IsCompare(pDlg->strMac,str);
			::RegCloseKey(Key);	
		}	
		if (!isReg)
		{
			pDlg->DisableAllControls();
			pDlg->SetWindowText(L"QuickWifi—未注册");
			
			CControlUI* background = pDlg->paint_manager_.FindControl(kBackgroundControlName);
			TCHAR szBuf[MAX_PATH] = {0};
			_stprintf_s(szBuf, MAX_PATH - 1, _T("bg%d.png"), pDlg->bk_image_index_);
			pDlg->m_regDlg= new CRegDlg(szBuf, background->GetBkColor());
			if( pDlg->m_regDlg) 
			{
				pDlg->m_regDlg->Create(NULL, _T("注册"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
				pDlg->skin_changed_observer_.AddReceiver(pDlg->m_regDlg);
				pDlg->m_regDlg->CenterWindow();
				pDlg->m_regDlg->SetParent(pDlg->GetHWND());
				pDlg->m_regDlg->SetStr(pDlg->strMac);
				pDlg->m_regDlg->ShowModal();
			}
			
		}		
	}
	return 0;
}

MainFrame::MainFrame()
: bk_image_index_(0),
m_NotificationSink(NULL),
m_fProcessNotifications(false),
m_IcsNeeded(false),
m_IcsEnabled(false),
m_HostedNetworkStarted(false),
m_IcsMgr(NULL)
{
	dwIfBufSize = 0;
	m_oldDownloadOctets=0;
	m_oldUploadOctets=0;
	m_curUploadOctets=0;
	m_curDownloadOctets=0;
	totalUp = 0.0;
	totalDown = 0.0;
	m_pMIT = NULL;
	m_regDlg = NULL;
}

MainFrame::~MainFrame()
{
	if(m_NotificationSink != NULL)
	{
		m_WlanMgr.UnadviseHostedNetworkNotification();
		delete m_NotificationSink;
		m_NotificationSink = NULL;
	}
	DeinitIcs();
	free(m_pMIT);
	_ASSERT(m_IcsMgr == NULL);
	PostQuitMessage(0);
}

LPCTSTR MainFrame::GetWindowClassName() const
{
	return _T("TXGuiFoundation");
}

CControlUI* MainFrame::CreateControl(LPCTSTR pstrClass)
{
	return NULL;
}

void MainFrame::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

tString MainFrame::GetSkinFile()
{
	return _T("main_frame.xml");
}

tString MainFrame::GetSkinFolder()
{
	return tString(CPaintManagerUI::GetInstancePath());
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#if defined(WIN32) && !defined(UNDER_CE)
	BOOL bZoomed = ::IsZoomed(m_hWnd);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
// 	if (::IsZoomed(m_hWnd) != bZoomed)
// 	{
// 		if (!bZoomed)
// 		{
// 			CControlUI* pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kMaxButtonControlName));
// 			if( pControl ) pControl->SetVisible(false);
// 			pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kRestoreButtonControlName));
// 			if( pControl ) pControl->SetVisible(true);
// 		}
// 		else 
// 		{
// 			CControlUI* pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kMaxButtonControlName));
// 			if( pControl ) pControl->SetVisible(true);
// 			pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kRestoreButtonControlName));
// 			if( pControl ) pControl->SetVisible(false);
// 		}
// 	}
#else
	return __super::OnSysCommand(uMsg, wParam, lParam, bHandled);
#endif

	return 0;
}

LRESULT MainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NEW_HN_NOTIFICATION:
		if (m_fProcessNotifications)
		{
			ProcessNotifications();
		}
		break;
	case WM_HASREG:
		{
			SetWindowText(L"QuickWifi");
			EnableAllControls();
		}
		break;
	case CM_TRAYMESSAGE:
		OnTrayMessage(wParam, lParam);
		break;
 	case WM_TIMER:
 		OnTimer((INT_PTR)wParam);
 		break;
	case WM_COMMAND:
		if (wParam == ID_TRAYCLOSE)
		{
			SendMessage(WM_CLOSE,NULL,NULL);
		}
		else if (wParam == ID_OPENWEB)
		{
			ShellExecute(GetHWND(),L"open",L"http://www.gupar.com",NULL,NULL,SW_SHOW);
		}
		else if (wParam == ID_REGSOFT)
		{
			CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
			TCHAR szBuf[MAX_PATH] = {0};
			_stprintf_s(szBuf, MAX_PATH - 1, _T("bg%d.png"), bk_image_index_);
			if( !m_regDlg) 
			{
				m_regDlg = new CRegDlg(szBuf, background->GetBkColor());
				m_regDlg->Create(NULL, _T("注册"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
				skin_changed_observer_.AddReceiver(m_regDlg);
				m_regDlg->CenterWindow();
				m_regDlg->SetParent(GetHWND());
				m_regDlg->SetStr(strMac);
				m_regDlg->ShowModal();
			}
		}
		break;
	case WM_REGCLOSE:
		{
			if (m_regDlg)
			{
				delete m_regDlg;
			}
			m_regDlg = NULL;
		}
		

		break;
	case WM_CLOSE:
		{
			Shell_NotifyIcon(NIM_DELETE, &m_IconData);
			if (m_regDlg)
			{
				m_regDlg->Close();
			}
			// TODO: 在此添加专用代码和/或调用基类
			m_fProcessNotifications = false;
			if (m_HostedNetworkStarted)
			{
				StopHostedNetwork();//**//
			}
			//写入INI配置文件
			CString str;
			str.Format(L"%0.2f",totalUp);
			WritePrivateProfileString(L"congfig",L"upload",str,strPath+L"\\Gupar\\Wifi\\config.ini");
			str.Format(L"%0.2f",totalDown);
			WritePrivateProfileString(L"congfig",L"download",str,strPath+L"\\Gupar\\Wifi\\config.ini");
			str.Format(L"%d",bk_image_index_);
			WritePrivateProfileString(L"congfig",L"skin",str,strPath+L"\\Gupar\\Wifi\\config.ini");
			//********************************************************
		}
		break;
	default:
		break;
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT MainFrame::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return FALSE;
}

LRESULT MainFrame::OnTimer(INT_PTR nID)
{
	switch (nID)
	{
	case 1:
		{
			GetSpeed();
		}
	
	}
	return LRESULT();
}

void MainFrame::OnTimer(TNotifyUI& msg)
{

}

void MainFrame::OnExit(TNotifyUI& msg)
{
	Close();
}

void MainFrame::Init()
{
	m_NameEdit = static_cast<CEditUI*>(paint_manager_.FindControl(kNetNameControlName));
	m_KeyEdit = static_cast<CEditUI*>(paint_manager_.FindControl(kPasswordControlName));
	m_HostedNetworkButton = static_cast<CButtonUI*>(paint_manager_.FindControl(kFuncBtnControlName));
	m_speed_up = static_cast<CLabelUI*>(paint_manager_.FindControl(kUpSpeedControlName));
	m_speed_down = static_cast<CLabelUI*>(paint_manager_.FindControl(kDownSpeedControlName));
	m_TotalDown = static_cast<CLabelUI*>(paint_manager_.FindControl(kTotalDownControlName));
	m_TotalUp = static_cast<CLabelUI*>(paint_manager_.FindControl(kTotalUPControlName));
	m_Notify = static_cast<CLabelUI*>(paint_manager_.FindControl(kNotifyMsgControlName));
	m_ConnectionComboBox =  static_cast<CComboBoxUI*>(paint_manager_.FindControl(kConnectControlName));
	m_DeviceListCtrl = static_cast<CListUI*>(paint_manager_.FindControl(kDeviceListControlName));
	m_DeviceListCtrl->SetTextCallback(this);
	m_HostedNetworkStatusTxt = static_cast<CLabelUI*>(paint_manager_.FindControl(kSystemMsgControlName));
	::SetWindowLongA(*this,GWL_EXSTYLE,WS_EX_TOOLWINDOW);
//****************************************************************
//**************************系统托盘******************************
	m_IconData.cbSize = sizeof(NOTIFYICONDATA);
	m_IconData.hWnd = m_hWnd;
	char szTip[] = "QuickWiFi";
	int nLen = strlen(szTip)+1; 
	int nwLen = MultiByteToWideChar(CP_ACP, 0, szTip,nLen, NULL, 0);
	MultiByteToWideChar(CP_ACP,0,szTip,nLen,m_IconData.szTip,nwLen); 
	m_IconData.uCallbackMessage = CM_TRAYMESSAGE;
	m_IconData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_IconData.hIcon = LoadIcon(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDI_SESSION));
	Shell_NotifyIcon(NIM_ADD, &m_IconData);
	SetIcon(IDI_SESSION);

	//读取INI配置文件
//*************************获取我的文档路径****************************
	LPITEMIDLIST pidl;
	TCHAR DocumentPath[MAX_PATH];
	SHGetSpecialFolderLocation(0,CSIDL_PERSONAL,&pidl); 
	SHGetSpecialFolderLocation(GetHWND(),CSIDL_PERSONAL,&pidl);
	SHGetPathFromIDList(pidl,DocumentPath); 
	CString strs = DocumentPath;
	strs.Replace(L"\\",L"\\\\");
	strPath = strs;
	CreateDirectory(strs+L"\\Gupar",NULL);CreateDirectory(strs+L"\\Gupar\\Wifi",NULL);
	TCHAR ss[260];
	if (PathFileExists(strs+L"\\Gupar\\Wifi\\config.ini"))
	{
		GetPrivateProfileString(L"congfig",L"upload",L"",ss,MAX_PATH,strs+L"\\Gupar\\Wifi\\config.ini");
		totalUp = _ttof(ss);
		GetPrivateProfileString(L"congfig",L"download",L"",ss,MAX_PATH,strs+L"\\Gupar\\Wifi\\config.ini");
		totalDown = _ttof(ss);
		GetPrivateProfileString(L"congfig",L"skin",L"",ss,MAX_PATH,strs+L"\\Gupar\\Wifi\\config.ini");
		bk_image_index_ = _ttoi(ss);
	}
	else
	{
		WritePrivateProfileString(L"congfig",L"upload",L"0.0",strs+L"\\Gupar\\Wifi\\config.ini");
		WritePrivateProfileString(L"congfig",L"download",L"0.0",strs+L"\\Gupar\\Wifi\\config.ini");
	}
//********************************************************
	if (ShellExecute(GetHWND(),_T("open"),_T("cmd.exe"),L"/c netsh wlan set hostednetwork mode=allow",NULL,NULL))
	{
		PostNotification(_T("启动网络服务成功！"));
	}
	else
	{
		PostNotification(_T("启动网络服务失败！"));
	}
//	CreateThread(NULL,0,checkVision,(LPVOID*)this,0,NULL);
	m_pMIT = (MIB_IFTABLE *)malloc(sizeof (MIB_IFTABLE));
	dwIfBufSize = sizeof (MIB_IFTABLE);
	if(::GetIfTable(m_pMIT,&dwIfBufSize,FALSE) == ERROR_INSUFFICIENT_BUFFER)//如果内存不足，则重新分配内存大小 
	{ 
		if(dwIfBufSize != NULL) 
			free(m_pMIT);
		m_pMIT = (MIB_IFTABLE *)malloc(dwIfBufSize);
	}
	// set the length limit for SSID/key input
	m_NameEdit->SetMaxChar(DOT11_SSID_MAX_LENGTH);
	m_KeyEdit->SetMaxChar(MAX_KEY_LEN);
	m_HostedNetworkStatusTxt->SetText(L"网络空闲");
	// Load the messages for start/stop hosted network
	LoadString (GetModuleHandle(NULL),IDS_START_HOSTED_NETWORK,m_strStartHostedNetwork,256);
	LoadString (GetModuleHandle(NULL),IDS_STOP_HOSTED_NETWORK,m_strStopHostedNetwork,256);
	// Following code is to initialize ICS manager
	// Step 1: check if the app is running under admin privilege
	// if not, use has no access to ICS settings,
	// disable the ICS settings and post message
	m_bIsAdmin = IsUserAdmin();//**//
	m_bIsICSAllowed = IsICSAllowed();//**//

	m_IcsNeeded = true;
	m_ConnectionComboBox->SetEnabled(true);
	if (!m_bIsAdmin || !m_bIsICSAllowed)
	{    
		m_IcsNeeded = FALSE;

		if (!m_bIsAdmin)
		{
			PostErrorMessage(L"没有获取系统管理员权限,无法取得ICS的控制权");
		}
		else
		{
			PostErrorMessage(L"ICS被管理员禁止,无法取得ICS的控制权.");
		}
		m_NameEdit->SetFocus();
	}
	// Step 2: if user has admin privilege and ICS is allowed in the domain,
	// initialize ICS manager
	if (m_bIsAdmin && m_bIsICSAllowed)
	{		
		if (FAILED(InitIcs()))// Initialize ICS
		{
			PostNotification(L"初始化ICS管理失败."); // Post a notificatoin   
			fRetCode = FALSE;
			BAIL();
		}		
		PostNotification(L"初始化ICS管理成功.");		
		m_IcsMgr->GetIcsConnections(m_ConnectionList);// Get connection info
	}
	// Following code is to Initialize WLAN
	if (FAILED(InitWlan()))//**//
	{
		PostNotification(L"初始化WiFi管理器失败.");
		fRetCode = FALSE;
		BAIL();
	}
	PostNotification(L"初始化WiFi管理器成功.");
	GetHostedNetworkInfo();// Get hosted network info	
	if (m_bIsAdmin && m_bIsICSAllowed)// Get ICS info. Must be called after GetHostedNetworkInfo().
	{
		ASSERT(m_IcsMgr);
		GetIcsInfo();//**//
	}
	m_HostedNetworkButton->SetText(m_strStartHostedNetwork);
	// Name
	m_NameEdit->SetEnabled(TRUE);
	// Key
	m_KeyEdit->SetEnabled(TRUE);
	// Set state
	m_HostedNetworkStarted = false;
	m_fProcessNotifications = true;
	PostNotification(GUIDToString(m_HostedNetworkGuid));
	::SetTimer(GetHWND(),1,1000,NULL);
error:
	return;
}

DWORD MainFrame::GetBkColor()
{
	CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
	if (background != NULL)
		return background->GetBkColor();

	return 0;
}

void MainFrame::SetBkColor(DWORD dwBackColor)
{
	CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
	if (background != NULL)
	{
		background->SetBkImage(_T(""));
		background->SetBkColor(dwBackColor);
		background->NeedUpdate();

		SkinChangedParam param;
		param.bkcolor = background->GetBkColor();
		param.bgimage = background->GetBkImage();
		skin_changed_observer_.Broadcast(param);
	}
}

void MainFrame::OnPrepare(TNotifyUI& msg)
{

	CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
	if (background != NULL)
	{
		TCHAR szBuf[MAX_PATH] = {0};

		_stprintf_s(szBuf, MAX_PATH - 1, _T("file='bg%d.png' corner='600,200,1,1'"), bk_image_index_);
		background->SetBkImage(szBuf);
	}
}

void MainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, _T("windowinit")) == 0)
	{
		OnPrepare(msg);
	}
	else if (_tcsicmp(msg.sType, _T("click")) == 0)
	{
		if (_tcsicmp(msg.pSender->GetName(), kCloseButtonControlName) == 0)
		{
			if (MessageBox(NULL,L"退出程序将会断开Wifi，您要继续吗?",L"提示",MB_OKCANCEL) == IDOK)
			{
				OnExit(msg);
			}			
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"myUrl") == 0)
		{
			ShellExecute(GetHWND(),L"open",L"http://www.gupar.com",NULL,NULL,SW_SHOW);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kMinButtonControlName) == 0)
		{
			::ShowWindow(m_hWnd, SW_HIDE);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kFuncBtnControlName) == 0)
		{
			OnBnClickedButtonHostednetwork();
		}	
		else if (_tcsicmp(msg.pSender->GetName(), kChangeBkSkinControlName) == 0)
		{
			CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
			if (background != NULL)
			{
				TCHAR szBuf[MAX_PATH] = {0};
				++bk_image_index_;
				if (kBackgroundSkinImageCount < bk_image_index_)
					bk_image_index_ = 0;
				_stprintf_s(szBuf, MAX_PATH - 1, _T("file='bg%d.png' corner='600,200,1,1'"), bk_image_index_);
				background->SetBkImage(szBuf);
				SkinChangedParam param;
				CControlUI* background = paint_manager_.FindControl(kBackgroundControlName);
				if (background != NULL)
				{
					param.bkcolor = background->GetBkColor();
					if (_tcslen(background->GetBkImage()) > 0)
					{
						_stprintf_s(szBuf, MAX_PATH - 1, _T("bg%d.png"), bk_image_index_);
					}

					param.bgimage = szBuf;
				}
				skin_changed_observer_.Broadcast(param);
			}
		}
	}
	else if (_tcsicmp(msg.sType, _T("timer")) == 0)
	{
		return OnTimer(msg);
	}
}

LRESULT MainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
//系统托盘函数
LRESULT MainFrame::OnTrayMessage(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDBLCLK )		//双击事件
	{
		SetWindowPos(GetHWND(),HWND_TOP,0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);		
	}
	else if (lParam == WM_RBUTTONDOWN)	//右键单击事件
	{
		HMENU hMenu = LoadMenu(paint_manager_.GetInstance(), MAKEINTRESOURCE(IDR_TRAYMENU));
		hMenu = GetSubMenu (hMenu, 0) ;
		CPoint Curpt;
		::GetCursorPos(&Curpt);			//获取鼠标指针位置
		SetForegroundWindow(this->GetHWND()); // 修正当用户按下ESCAPE 键或者在菜单之外单击鼠标时菜单不会消失的情况
		TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, Curpt.x, Curpt.y, 0, this->GetHWND(), NULL);
	}
	return 0;
}

void MainFrame::OnBnClickedButtonHostednetwork()
{
	HRESULT     hr            = S_OK;
	HRESULT     hrPrintError  = S_OK;
	TCHAR       sErrorMsg[256];
	size_t      errorSize     = 256;
	LPCTSTR     sErrorFormat  = TEXT("%s. 错误 %x.");
	LPCTSTR     sOperation;
	bool        bValidSSIDKey = false;
	// Check if the lengths of SSID and key are valid
	bValidSSIDKey = CheckValidSSIDKeyLen();//**//
	if (!bValidSSIDKey)
	{
		return;
	}
	if (m_HostedNetworkStarted)
	{
		hr = StopHostedNetwork();//**//
		sOperation = TEXT("停止网络服务失败");
	}
	else
	{
		hr = StartHostedNetwork();
		sOperation = TEXT("启动网络服务失败");
	}
	if (FAILED(hr))
	{
		// Post an error message
		hrPrintError = StringCchPrintf(sErrorMsg, errorSize, sErrorFormat, sOperation, hr);
		if (hrPrintError != S_OK)
		{
			PostErrorMessage(sOperation);
		}
		else
		{
			PostErrorMessage(sErrorMsg);
		}
	}
}
//**//
HRESULT MainFrame::InitWlan()
{
	HRESULT hr                      = S_OK;
	bool    bIsHostedNetworkStarted = FALSE;
	// Create notification sink
	_ASSERT(m_NotificationSink == NULL);
	m_NotificationSink = new(std::nothrow) CNotificationSink(m_hWnd);
	if (NULL == m_NotificationSink)
	{
		hr = E_OUTOFMEMORY;
		BAIL();
	}
	// Initialize WLAN manager
	hr = m_WlanMgr.Init();
	BAIL_ON_FAILURE(hr);
	// Set notification sink
	hr = m_WlanMgr.AdviseHostedNetworkNotification(m_NotificationSink); 
	BAIL_ON_FAILURE(hr);

	hr = m_WlanMgr.IsHostedNetworkStarted(bIsHostedNetworkStarted); 
	BAIL_ON_FAILURE(hr);

	if (bIsHostedNetworkStarted)
	{
		m_HostedNetworkStatusTxt->SetText(L"网络可用");
	}
error:
	if (S_OK != hr && m_NotificationSink != NULL)
	{
		delete m_NotificationSink;
		m_NotificationSink = NULL;
	}
	return hr;
}

void MainFrame::ProcessNotifications()
{
	CHostedNetworkNotification * pNotification = NULL;
	CWlanDevice * pDevice = NULL;
	// Get the first notification
	pNotification = m_NotificationSink->GetNextNotification();
	// pNotification could be NULL
	while (pNotification != NULL)
	{
		pDevice = (CWlanDevice *)pNotification->GetNotificationData();
		switch(pNotification->GetNotificationType())
		{
		case CHostedNetworkNotification::HostedNetworkNotAvailable:
			_ASSERT(NULL == pDevice);
			OnHostedNetworkNotAvailable();
			break;

		case CHostedNetworkNotification::HostedNetworkAvailable:
			_ASSERT(NULL == pDevice);
			OnHostedNetworkAvailable();
			break;

		case CHostedNetworkNotification::HostedNetworkStarted:
			_ASSERT(NULL == pDevice);
			PostNotification(L"网络服务器已启动.");
			m_HostedNetworkStatusTxt->SetText(L"网络服务可用");
			break;

		case CHostedNetworkNotification::HostedNetworkStopped:
			_ASSERT(NULL == pDevice);
			PostNotification(L"网络停止.");
			m_HostedNetworkStatusTxt->SetText(L"该网络是空闲的");
			// Remove all devices from the device list
			if (m_DeviceListCtrl->GetCount())
			{
				m_DeviceListCtrl->RemoveAll();
			}		
			break;
		case CHostedNetworkNotification::DeviceAdd:
			_ASSERT(pDevice != NULL);
			if (pDevice != NULL)
			{
				OnDeviceAdd(pDevice);
			}
			break;

		case CHostedNetworkNotification::DeviceRemove:
			_ASSERT(pDevice != NULL);
			if (pDevice != NULL)
			{
				OnDeviceRemove(pDevice);
			}
			break;

		case CHostedNetworkNotification::DeviceUpdate:
			_ASSERT(pDevice != NULL);
			if (pDevice != NULL)
			{
				OnDeviceUpdate(pDevice);
			}
			break;
		}
		pNotification->Release();
		pNotification = NULL;
		// Get the next notification
		pNotification = m_NotificationSink->GetNextNotification();
	}
}
//**//
void MainFrame::GetHostedNetworkInfo()// Get hosted network info and set controls accordingly
{
	HRESULT hr = S_OK;
	bool fStarted = false;
	// Get hosted network name
	hr = m_WlanMgr.GetHostedNetworkName(m_CurrentName);
	BAIL_ON_FAILURE(hr);
	// Get hosted network key
	hr = m_WlanMgr.GetHostedNetworkKey(m_CurrentKey);
	BAIL_ON_FAILURE(hr);
	// Get adapter GUID
	m_WlanMgr.GetHostedNetworkInterfaceGuid(m_HostedNetworkGuid);
	// Check whether the hosted network is started or not
	hr = m_WlanMgr.IsHostedNetworkStarted(fStarted);
	_ASSERT(S_OK == hr);
	BAIL_ON_FAILURE(hr);
	// Update controls
	// Set network name
	m_NameEdit->SetText(m_CurrentName);
	// Set network key
	m_KeyEdit->SetText(m_CurrentKey);
	// Set the button text
	m_HostedNetworkStarted = fStarted;
	m_HostedNetworkButton->SetText(fStarted ? m_strStopHostedNetwork : m_strStartHostedNetwork);
	// Disable controls if needed
	if (fStarted)
	{
		m_NameEdit->SetEnabled(FALSE);// Name		
		m_KeyEdit->SetText(FALSE);// Key
		m_ConnectionComboBox->SetEnabled(FALSE);
		PostNotification(L"网络已经启动.");
	}
error:
	return;
}

void MainFrame::GetWlanDeviceInfo()
{
	CRefObjList<CWlanStation *> stationList;

	if (SUCCEEDED(m_WlanMgr.GetStaionList(stationList)))
	{
		// add stations that are already connected
		while ( 0 != stationList.GetCount() )
		{
			CWlanStation* pStation = stationList.RemoveHead();

			// OnDeviceAdd(pStation);

			pStation->Release();

			pStation = NULL;
		}
	}
}
//**//
void MainFrame::OnHostedNetworkStarted()
{
	// Post a notificatoin
	PostNotification(L"开始使用网络.");
	// Set state
	m_HostedNetworkStarted = true;
	// Get adapter GUID
	m_WlanMgr.GetHostedNetworkInterfaceGuid(m_HostedNetworkGuid);
	// Set button text and enable it
	m_HostedNetworkButton->SetText(m_strStopHostedNetwork);
	m_HostedNetworkButton->SetEnabled();
	// SSID and key setting should be disabled since we cannot modify them now
	m_NameEdit->SetEnabled(FALSE);
	m_KeyEdit->SetEnabled(FALSE);
}
//**//
void MainFrame::OnHostedNetworkStopped()
{
	if (m_HostedNetworkStarted)
	{
		// Post a notificatoin
		PostNotification(L"停止使用网络.");
		// Stop ICS if needed
		StopIcsIfNeeded();
		// Set state
		m_HostedNetworkStarted = false;
		// Set button text
		m_HostedNetworkButton->SetText(m_strStartHostedNetwork);
		EnableAllControls();
	}
}

void MainFrame::OnHostedNetworkNotAvailable()
{
	PostNotification(L"网络不可用当前网卡.");
	m_HostedNetworkStatusTxt->SetText(L"托管网络不可用");
	// Set state
	m_HostedNetworkStarted = false;
	// Stop ICS if needed
	StopIcsIfNeeded();
	// Set button text and disable it -- we cannot enable hosted network now.
	m_HostedNetworkButton->SetText(m_strStartHostedNetwork);
	m_HostedNetworkButton->SetEnabled(FALSE);
	// Enable ICS: We cannot do this since hosted network cannot be started
	//    m_EnableIcsCheck.EnableWindow(FALSE);    
	// Connection List
	m_ConnectionComboBox->SetEnabled(FALSE);
}

void MainFrame::OnHostedNetworkAvailable()
{
	PostNotification(L"网络可在当前网卡使用.");
	m_HostedNetworkStatusTxt->SetText(L"该网络是空闲的");
	// Set state
	m_HostedNetworkStarted = false;
	// Get hosted network info
	GetHostedNetworkInfo();
	if (m_bIsAdmin && m_bIsICSAllowed)
	{
		ASSERT(m_IcsMgr);
		// reset ICS to update ICS list
		m_IcsMgr->ResetIcsManager();
		// ICS list updated
		GetIcsInfo();
	}
	// Set button text
	m_HostedNetworkButton->SetText(m_strStartHostedNetwork);
	EnableAllControls();
}

void MainFrame::OnDeviceAdd(CWlanDevice * pDevice )
{
	_ASSERT(pDevice != NULL);
	if (pDevice != NULL)
	{
		// The device should not be in the list
		_ASSERT(!m_WlanDeviceList.IsInArray(pDevice));
		pDevice->AddRef();
		// Add the device to the list
		m_WlanDeviceList.AddTail(pDevice);
		// Update the device list view
		int nItem = m_DeviceListCtrl->GetCount();
		CListTextElementUI* pListElement = new CListTextElementUI;
		pListElement->SetTag(nItem);
		if (pListElement != NULL)
		{
			m_DeviceListCtrl->Add(pListElement);
		}
		// Post a notification
		PostDeviceNotification(pDevice, CHostedNetworkNotification::DeviceAdd);
	}
}

void MainFrame::OnDeviceRemove( CWlanDevice * pDevice)
{
	_ASSERT(pDevice != NULL);
	CWlanDevice * pRemovedDevice = NULL;
	// Find the device and remove it from the device list
	for (size_t i = 0; i < m_WlanDeviceList.GetCount(); i++)
	{
		POSITION pos = m_WlanDeviceList.FindIndex(i);
		CWlanDevice * pTmpDevice = m_WlanDeviceList.GetAt(pos);
		if (*pTmpDevice == *pDevice)
		{
			// Found the device, remove it from the list
			m_WlanDeviceList.RemoveAt(pos);
			m_DeviceListCtrl->RemoveAt(i);
			pRemovedDevice = pTmpDevice;
			break;
		}
	}
	if (pRemovedDevice != NULL)
	{
		// Update the device list view
	//	RemoveWlanDevice(pRemovedDevice);
		// Post a notification
		PostDeviceNotification(pDevice, CHostedNetworkNotification::DeviceRemove);
		pRemovedDevice->Release();
		pRemovedDevice = NULL;
	}
}

void MainFrame::OnDeviceUpdate( CWlanDevice * pDevice    )
{
	_ASSERT(pDevice != NULL);
	CWlanDevice * pRemovedDevice = NULL;
	POSITION pos;
	// Find the device and remove it from the device list
	for (size_t i = 0; i < m_WlanDeviceList.GetCount(); i++)
	{
		pos = m_WlanDeviceList.FindIndex(i);
		CWlanDevice * pTmpDevice = m_WlanDeviceList.GetAt(pos);
		if (*pTmpDevice == *pDevice)
		{
			// Found the device, update it with the new object
			pDevice->AddRef();
			m_WlanDeviceList.SetAt(pos, pDevice);
			pRemovedDevice = pTmpDevice;
			break;
		}
	}
	if (pRemovedDevice != NULL)
	{
		// Update the device list view
//		UpdateWlanDevice(pDevice);
		// Post a notification
		PostDeviceNotification(pDevice, CHostedNetworkNotification::DeviceUpdate);
		pRemovedDevice->Release();
		pRemovedDevice = NULL;
	}
}
//**//
HRESULT MainFrame::StartHostedNetwork()//**//
{
	HRESULT     hr                    = S_OK;
	CString     strName;
	CAtlString  name;
	CString     strKey;
	CAtlString  key;
	bool        bHostedNetworkStarted = false;
	bool        bICSStarted           = false;
	DisableAllControls();
	// Set name
	strName = m_NameEdit->GetText();
	name = strName;
	if (m_CurrentName != name)
	{
		// Set name only when needed
		hr = m_WlanMgr.SetHostedNetworkName(name);		
		BAIL_ON_FAILURE(hr);
		m_CurrentName = name;
	}
	// Set key
	strKey = m_KeyEdit->GetText();
	key = strKey;
	if (m_CurrentKey != key)
	{
		// Set key only when needed
		hr = m_WlanMgr.SetHostedNetworkKey(key);
		BAIL_ON_FAILURE(hr);
		m_CurrentKey = key;
	}
	// Cache the current ICS-enabled interfaces
	if (m_IcsNeeded)
	{
		m_IcsMgr->CacheICSIntfIndex();
		// Start ICS if:
		// 1. User has selected that softAP starts with full ICS, and
		// 2. the current ICS setting is different from the new setting
		hr = StartIcsIfNeeded();
		bICSStarted = true;
		// Force stop the currently running hosted network,
		// if there is any
		// this step is taken no matter  whether the start ICS 
		// succeeds or fails
		m_WlanMgr.IsHostedNetworkStarted(bHostedNetworkStarted);
		if (bHostedNetworkStarted)
		{
			m_WlanMgr.ForceStopHostedNetwork();
		}
		// if start ICS fails, bail out
		BAIL_ON_FAILURE(hr);
	}

	// Start hosted network
	hr = m_WlanMgr.StartHostedNetwork();
	// if start hosted network fails, bail out
	BAIL_ON_FAILURE(hr);
	m_WlanMgr.IsHostedNetworkStarted(bHostedNetworkStarted);
	OnHostedNetworkStarted();//**//
error:
	if (hr != S_OK)
	{
		if (m_IcsNeeded && bICSStarted)
		{
			// restore the previous ICS settings
			m_IcsMgr->EnableICSonCache();
		}
		EnableAllControls();
	}
	return hr;
}
//**//
HRESULT MainFrame::StopHostedNetwork()//**//
{
	HRESULT hr = S_OK;
	bool bIcsStopped = false;
	// Disable the button of Start/Stop hosted network
	m_HostedNetworkButton->SetEnabled(FALSE);
	// Stop full ICS if ICS is needed
	bIcsStopped = StopIcsIfNeeded();///**//
	// If a previous running ICS is stopped
	// force stop the hosted network
	// otherwise, stop using the hosted network
	if (bIcsStopped)
	{
		hr = m_WlanMgr.ForceStopHostedNetwork();
	}
	else
	{
		hr = m_WlanMgr.StopHostedNetwork();
	}

	BAIL_ON_FAILURE(hr);

	OnHostedNetworkStopped();//**//
error:
	return hr;
}
//**//
void MainFrame::GetIcsInfo()
{
	// Get the list of ICS connections
	m_IcsMgr->GetIcsConnections(m_ConnectionList);

	// If the hosted network is started, check whether ICS is enabled for hosted network
	if (m_HostedNetworkStarted)
	{
		for (size_t i = 0; i < m_ConnectionList.GetCount(); i++)
		{
			CIcsConnectionInfo * pConn = m_ConnectionList.GetAt(m_ConnectionList.FindIndex(i));
			if (*pConn == m_HostedNetworkGuid)
			{
				m_IcsEnabled = (pConn->m_Supported && pConn->m_SharingEnabled && pConn->m_Private);
				break;
			}
		}
	}

	// Update control
	if (m_IcsEnabled)
	{
		m_IcsNeeded = true;
	}
	//    m_EnableIcsCheck.SetCheck(m_IcsNeeded ? BST_CHECKED : BST_UNCHECKED);

	// Update connection combobox
	UpdateIcsConnectionList();

	if (!m_IcsNeeded)
	{
		// Disable connection list
		m_ConnectionComboBox->SetEnabled(FALSE);
	}
	else
	{
		if (m_IcsEnabled)
		{
		}
	}
}
//**//
void MainFrame::UpdateIcsConnectionList()
{
	int nConnections = 0;
	// Empty the ICS connection combo box.
	m_ConnectionComboBox->RemoveAll();
	m_SuportList.empty();
	for (size_t i = 0; i < m_ConnectionList.GetCount(); i++)
	{
		CListLabelElementUI *item = new CListLabelElementUI;
		CIcsConnectionInfo * pConn = m_ConnectionList.GetAt(m_ConnectionList.FindIndex(i));
		if (pConn->m_Supported && !(*pConn == m_HostedNetworkGuid))
		{			
			// Don't add a connection if it doesn't support ICS or it is the hosted network connection
			nConnections++;
			int index = 0;
			item->SetText(pConn->m_Name);
			char *buffer = new char[sizeof(CIcsConnectionInfo)];
			memcpy(buffer,pConn,sizeof(CIcsConnectionInfo));
			item->SetUserData((CString)buffer);// Set data pointer for the item.
			m_ConnectionComboBox->Add(item);
			m_SuportList.push_back(pConn);
			if (0 == index)
			{
				m_ConnectionComboBox->SelectItem(index,false);
			}

			if (m_HostedNetworkStarted && m_IcsNeeded && pConn->m_SharingEnabled && pConn->m_Public)
			{
				m_ConnectionComboBox->SelectItem(index,false);
			}
		}
	}
}
//**//
HRESULT MainFrame::InitIcs()
{
	HRESULT hr = S_OK;
	CIcsManager * pIcsMgr = NULL;
	bool fDeinitCom = false;
	bool fDeinitNSMod = false;

	_ASSERT(NULL == m_IcsMgr);

	// initialize NS mod
	hr = NSModInit();//**//
	BAIL_ON_FAILURE(hr);

	fDeinitNSMod = true;
	// initialize COM
// 	hr = ::CoInitializeEx(
// 		NULL,
// 		COINIT_MULTITHREADED
// 		);
// 	BAIL_ON_FAILURE(hr);

	fDeinitCom = true;

	// create ICS manager
	pIcsMgr = new(std::nothrow) CIcsManager();
	if (NULL == pIcsMgr)
	{
		hr = E_OUTOFMEMORY;
		BAIL();
	}
	// initialize ICS manager
	hr = pIcsMgr->InitIcsManager();
	BAIL_ON_FAILURE(hr);

	// Everything is fine
	// COM and NSMod are deinitialized later
	fDeinitCom = false;
	fDeinitNSMod = false;

	m_IcsMgr = pIcsMgr;
	pIcsMgr = NULL;

error:
	if (pIcsMgr != NULL)
	{
		delete pIcsMgr;
		pIcsMgr = NULL;
	}

	if (fDeinitCom)
	{
//		::CoUninitialize();
	}

	if (fDeinitNSMod)
	{
		NSModDeinit();//**//
	}

	return hr;
}

void MainFrame::DeinitIcs()
{
	if (m_IcsMgr != NULL)
	{
		// ICS was successfully initialized.
		delete m_IcsMgr;
		m_IcsMgr = NULL;

		::CoUninitialize();
		NSModDeinit();
	}
}
//**//
HRESULT MainFrame::StartIcsIfNeeded()
{
	HRESULT hr            = S_OK;
	HRESULT hrPrintError  = S_OK;
	TCHAR   sErrorMsg[256];
	size_t  errorSize     = 256;
	LPCTSTR sErrorFormat  = TEXT("Failed to enable ICS. Error code %x.");



	_ASSERT(!m_IcsEnabled);
//************************************************************************************************
	if (!m_IcsNeeded)
	{
		// No need to start ICS
		BAIL();
	}
	//***********************************************************************************************
	// Get the public connecton
	
	CIcsConnectionInfo * pConn = m_SuportList.at(m_ConnectionComboBox->GetCurSel());
	if (pConn != NULL)
	{
		// Start ICS
		hr = m_IcsMgr->EnableIcs(pConn->m_Guid, m_HostedNetworkGuid);
		if (hr != S_OK)
		{
			// reset ICS manager
			hr = m_IcsMgr->ResetIcsManager();
			if (S_OK == hr)
			{
				// try it again
				hr = m_IcsMgr->EnableIcs(pConn->m_Guid, m_HostedNetworkGuid);
			}
		}
		if (S_OK == hr)
		{
			PostNotification(L"成功启用ICS.");
			m_IcsEnabled = true;
		}
		else
		{
			hrPrintError = StringCchPrintf(sErrorMsg, errorSize, sErrorFormat, hr);
			if (hrPrintError != S_OK)
			{
				PostNotification(L"未能启用ICS.");
			}
			else
			{
				PostNotification(sErrorMsg);
			}
		}
	}

error:
	return hr;
}
//**//
bool MainFrame::StopIcsIfNeeded()
{
	bool bICSStopped = false;
	if (m_IcsEnabled && m_IcsNeeded)
	{
		_ASSERT(m_IcsMgr != NULL);

		m_IcsMgr->DisableIcsOnAll();

		PostNotification(L"成功禁用ICS.");

		m_IcsEnabled = false;
		bICSStopped = true;
	}
	return bICSStopped;
}

void MainFrame::DisableAllControls()
{
	m_NameEdit->SetEnabled(FALSE);// Name
	m_KeyEdit->SetEnabled(FALSE);// Key
	m_ConnectionComboBox->SetEnabled(FALSE);
	m_HostedNetworkButton->SetEnabled(FALSE);
}

void MainFrame::EnableAllControls()
{
	m_NameEdit->SetEnabled(TRUE);// Name	
	m_KeyEdit->SetEnabled(TRUE);// Key
	if (m_bIsAdmin && m_bIsICSAllowed)
	{
		ASSERT(m_IcsMgr);
		if (m_IcsNeeded)
		{
			m_ConnectionComboBox->SetEnabled(TRUE);
		}
	}
	m_HostedNetworkButton->SetEnabled(TRUE);
}

void MainFrame::OnEnKillfocusEditName()
{
	CString strName;
	strName = m_NameEdit->GetText();
	if (strName.GetLength() > DOT11_SSID_MAX_LENGTH || strName.GetLength() < 1)
	{
		PostErrorMessage(L"网络名称必须包含1—32可区分的大小写字符.");
		m_NameEdit->SetFocus();
	}
}

void MainFrame::OnEnKillfocusEditKey()
{
	CString strKey;
	strKey = m_KeyEdit->GetText();
	if (strKey.GetLength() > 63 || strKey.GetLength() < 8)
	{
		PostErrorMessage(L"网络密码必须包含8-63可区分的大小写字符.");
		m_KeyEdit->SetFocus();
	}
}
//**//
bool MainFrame::CheckValidSSIDKeyLen()//**//
{
	CString strName;
	bool    bSSIDValid   = true;
	bool    bKeyValid    = true;
	bool    bCheckPassed = false;

	strName = m_NameEdit->GetText();
	if (strName.GetLength() > DOT11_SSID_MAX_LENGTH || strName.GetLength() < 1)
	{
		bSSIDValid = false;
	}

	strName = m_KeyEdit->GetText();
	if (strName.GetLength() > 63 || strName.GetLength() < 8)
	{
		bKeyValid = false;
	}

	if (!bSSIDValid && bKeyValid)
	{
		PostErrorMessage(L"网络名字应该包含1-32可区分的大小写字符.");
		m_NameEdit->SetFocus();
	}
	else if (bSSIDValid && !bKeyValid)
	{
		PostErrorMessage(L"网络密码必须包含8-63可区分的大小写字符.");
		m_KeyEdit->SetFocus();
	}
	else if (!bSSIDValid && !bKeyValid)
	{
		PostErrorMessage(L"账号和密码不能为空.");
		m_NameEdit->SetFocus();
	}
	else
	{
		bCheckPassed = true;
	}

	return bCheckPassed;
}

void MainFrame::PostDeviceNotification( CWlanDevice * pDevice,int Event)
{
	_ASSERT(pDevice != NULL);
	bool fPostNotification = true;

	CAtlString strFriendlyName;
	pDevice->GetFriendlyName(strFriendlyName);

	CAtlString msg;

	switch(Event)
	{
	case CHostedNetworkNotification::DeviceAdd:
		msg = L"设备 \""+strFriendlyName + L"\" 加入网络";
		break;

	case CHostedNetworkNotification::DeviceRemove:
		msg = L"设备 \""+strFriendlyName + L"\" 离开网络";
		break;

	case CHostedNetworkNotification::DeviceUpdate:
		msg = L"设备 \""+strFriendlyName + L"\" 已经更新";
		break;

	default:
		fPostNotification = false;
	}

	if (fPostNotification)
	{
		PostNotification(msg.GetString());    
	}
}

void MainFrame::PostNotification(  LPCWSTR msg   ) 
{
	if (m_Notify)
	{
		m_Notify->SetText(msg);
	}
};
//**//
BOOL MainFrame::IsUserAdmin()
{
	BOOL                     bIsAdmin              = FALSE;
	SID_IDENTIFIER_AUTHORITY NtAuthority           = SECURITY_NT_AUTHORITY;
	PSID                     AdministratorsGroup; 

	bIsAdmin = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 
		0, 
		0, 
		0, 
		0, 
		0,
		&AdministratorsGroup
		); 

	if(bIsAdmin) 
	{
		if (!CheckTokenMembership( NULL, AdministratorsGroup, &bIsAdmin)) 
		{
			bIsAdmin = FALSE;
		} 
		FreeSid(AdministratorsGroup); 
	}

	return(bIsAdmin);
}
//**//
bool MainFrame::IsICSAllowed()
{
	bool  bIsIcsAllowed = false;
	HKEY  hAllowIcs     = NULL; 
	DWORD dwError       = ERROR_SUCCESS;
	DWORD   ValLen = sizeof(DWORD);
	DWORD   dwAllowIcs = 1;

	dwError = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Policies\\Microsoft\\Windows\\Network Connections",
		NULL,
		KEY_READ,
		&hAllowIcs
		);

	if (ERROR_SUCCESS != dwError || !hAllowIcs)
	{
		BAIL();
	}

	dwError = RegGetValue(
		hAllowIcs,
		NULL,
		L"NC_ShowSharedAccessUI",
		RRF_RT_DWORD,
		NULL,
		&dwAllowIcs,
		&ValLen
		);
	if( dwError == ERROR_SUCCESS && dwAllowIcs == 0)
	{
		bIsIcsAllowed = false;
	}
	else
	{
		bIsIcsAllowed = true;
	}

error:
	if (hAllowIcs)
	{
		RegCloseKey(hAllowIcs);
	}
	return bIsIcsAllowed;
}

void MainFrame::OnBnClickedAuto()
{
	HKEY sub;
	TCHAR szPath[MAX_PATH];
	::GetModuleFileName(NULL,szPath,MAX_PATH);
	CString str =szPath;
	CString skey= L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	::RegCreateKey(HKEY_LOCAL_MACHINE,skey,&sub);
	if(m_auto->IsSelected())
	{
		::RegSetValueEx(sub,L"GuparWifi",NULL,REG_SZ,(BYTE*)szPath,str.GetLength());
	}
	else
	{
		::RegDeleteValue(sub,L"GuparWifi");
	}	
}

CString FloatToStr(double num)
{
	CString str;
	str.Format(L"%0.2fK",(num/1024.0));
	num = num/1024.0;
	if (num>=1024.0)
	{
		str.Format(L"%0.2fM",(num/1024.0));
		num = num/1024.0;
	}
	if (num>=1024.0)
	{
		str.Format(L"%0.2fG",(num/1024.0));
	}
	return str;
}

void MainFrame::GetSpeed()
{
	CString tmp,myGuid;
	myGuid = GUIDToString(m_HostedNetworkGuid);

	if (::GetIfTable(m_pMIT,&dwIfBufSize,FALSE) == NO_ERROR) 
	{
		if (m_pMIT->dwNumEntries <= 1) 
		{ 
			//bResult = false; 
		} 
		else 
		{ 
			__int64 i64TotalOutOctets = 0; 
			__int64 i64TotalInOctets = 0; 
			//多网卡
			for(int i=0; i<(m_pMIT->dwNumEntries); i++) 
			{ 
				tmp= m_pMIT->table[i].wszName;
				int pos = tmp.Find(L"_");
				tmp = tmp.Right(tmp.GetLength() - pos -1 );
				if ((m_pMIT->table[i].dwType != IF_TYPE_SOFTWARE_LOOPBACK) && (tmp==myGuid )) 
				{
					//当前上传 
					i64TotalOutOctets += m_pMIT->table[i].dwOutOctets;					
					//当前下载 
					i64TotalInOctets += m_pMIT->table[i].dwInOctets; 
				} 
			} 
			if(m_oldUploadOctets != 0 && (i64TotalOutOctets >= m_oldUploadOctets)) 
				m_curUploadOctets = i64TotalOutOctets - m_oldUploadOctets; 
			m_oldUploadOctets = i64TotalOutOctets; 
			if(m_oldDownloadOctets != 0 && (i64TotalInOctets >= m_oldDownloadOctets)) 
				m_curDownloadOctets = i64TotalInOctets - m_oldDownloadOctets; 
			m_oldDownloadOctets = i64TotalInOctets; 

			m_speed_down->SetText(FloatToStr((double)m_curUploadOctets)+L"/s");
			m_speed_up->SetText(FloatToStr((double)m_curDownloadOctets)+L"/s");

			totalUp += m_curUploadOctets;
			totalDown += m_curDownloadOctets;
			m_TotalDown->SetText(FloatToStr((double)totalUp));
			m_TotalUp->SetText(FloatToStr((double)totalDown));
		} 
	}
}

void MainFrame::PostErrorMessage(LPCTSTR str)
{
	m_HostedNetworkStatusTxt->SetText(str);
}
// 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
LPCTSTR MainFrame::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
{
	POSITION pos = m_WlanDeviceList.FindIndex(iIndex);
	CWlanDevice * pTmpDevice = m_WlanDeviceList.GetAt(pos);
	CString str;
	switch (iSubItem)
	{
	case 0:
		str.Format(L"%d",iIndex+1);
		break;
	case 1:
		pTmpDevice->GetFriendlyName(str);	
		break;
	case 2:
		pTmpDevice->GetDisplayMacAddress(str);	
		break;
	}
	pControl->SetUserData(str);
	return pControl->GetUserData();
}

void MainFrame::SetWindowText(CString str)
{
	CControlUI* wintext = paint_manager_.FindControl(L"WinText");
	if (wintext != NULL)
	{
		wintext->SetText(str);
	}
}
