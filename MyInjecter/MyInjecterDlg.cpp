
// MyInjecterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MyInjecter.h"
#include "MyInjecterDlg.h"
#include "afxdialogex.h"
#include "InjectUtil.h"
#include "LogFile.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyInjecterDlg 对话框




CMyInjecterDlg::CMyInjecterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMyInjecterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMyInjecterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMyInjecterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_INJECT, &CMyInjecterDlg::OnBnClickedInject)
    ON_BN_CLICKED(IDC_BROWSER_DLL, &CMyInjecterDlg::OnBnClickedBrowser)
    ON_BN_CLICKED(IDC_LANCH, &CMyInjecterDlg::OnBnClickedLanch)
    ON_BN_CLICKED(IDC_BROWSER_EXE, &CMyInjecterDlg::OnBnClickedBrowserExe)
    ON_BN_CLICKED(IDC_UNINJECT, &CMyInjecterDlg::OnBnClickedUninject)
END_MESSAGE_MAP()


// CMyInjecterDlg 消息处理程序

BOOL CMyInjecterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    CHAR szPath[MAX_PATH] = {0};
    ::GetModuleFileName(NULL, (LPTSTR)szPath, MAX_PATH);
    ::PathRemoveFileSpec((LPTSTR)szPath);
    ::PathRemoveBackslash((LPTSTR)szPath);
    ::PathAppend((LPTSTR)szPath, _T("set.ini"));
    CString strIniPath = (LPCTSTR)szPath;

    GetPrivateProfileString(
        _T("Work Parameter"),       // section name
        _T("Last Exe Path"),	// key name
        _T(""),
        (LPTSTR)szPath,
        255,
        (LPCTSTR)strIniPath		    // initialization file name
        ); 
    this->SetDlgItemText(IDC_EDIT_EXE_PATH, (LPCTSTR)szPath);

    GetPrivateProfileString(
        _T("Work Parameter"),       // section name
        _T("Last Dll Path"),	// key name
        _T(""),
        (LPTSTR)szPath,
        255,
        (LPCTSTR)strIniPath		    // initialization file name
        ); 
    this->SetDlgItemText(IDC_EDIT_DLL_PATH, (LPCTSTR)szPath);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMyInjecterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMyInjecterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMyInjecterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 注入进程
void CMyInjecterDlg::OnBnClickedInject()
{
	CString strDllPath;
	this->GetDlgItemText(IDC_EDIT_DLL_PATH, strDllPath);
	DWORD dwPid = this->GetDlgItemInt(IDC_EDIT_PID);
    if(strDllPath.IsEmpty())
    {
        this->MessageBox(_T("The DLL name cannot be empty."), _T("Warning"), MB_OK | MB_ICONWARNING);
        this->GetDlgItem(IDC_EDIT_DLL_PATH)->SetFocus();
        return;
    }

    if(dwPid == 0)
    {
        this->MessageBox(_T("The target process id cannot be empty."), _T("Warning"), MB_OK | MB_ICONWARNING);
        this->GetDlgItem(IDC_EDIT_PID)->SetFocus();
        return;
    }

    // 保存程序路径
    CHAR szPath[MAX_PATH] = {0};
    ::GetModuleFileName(NULL, (LPTSTR)szPath, MAX_PATH);
    ::PathRemoveFileSpec((LPTSTR)szPath);
    ::PathRemoveBackslash((LPTSTR)szPath);
    ::PathAppend((LPTSTR)szPath, _T("set.ini"));
    WritePrivateProfileString(
        _T("Work Parameter"),       // section name
        _T("Last Dll Path"),	    // key name
        (LPCTSTR)strDllPath,	    // string to add
        (LPCTSTR)szPath             // initialization file name
        ); 

    CString strError;
	OSType osType = CheckOS();
	switch(osType)
	{
	case WindowsXP:
	case WindowsVista:
	case Windows7:
	case Windows10:
        {
            char* pDllPath = NULL;
#ifdef UNICODE
            pDllPath = WideStringToAnsiString((LPCTSTR)strDllPath);
#else
            pDllPath = (LPTSTR)(LPCTSTR)strDllPath;
#endif
            if(InjectByPid(dwPid, pDllPath))
            {
                this->SetDlgItemText(IDC_EDIT_LOG, _T("注入成功"));
            }
            else
            {
                this->SetDlgItemText(IDC_EDIT_LOG, _T("注入失败"));
            }
#ifdef UNICODE
            free(pDllPath); pDllPath = NULL;
#endif

        }
		break;
    default:
        break;
	}
}

void CMyInjecterDlg::OnBnClickedUninject()
{
    CString strDllPath;
    this->GetDlgItemText(IDC_EDIT_DLL_PATH, strDllPath);
    DWORD dwPid = this->GetDlgItemInt(IDC_EDIT_PID);
    if(strDllPath.IsEmpty())
    {
        this->MessageBox(_T("The DLL name cannot be empty."), _T("Warning"), MB_OK | MB_ICONWARNING);
        this->GetDlgItem(IDC_EDIT_DLL_PATH)->SetFocus();
        return;
    }

    if(dwPid == 0)
    {
        this->MessageBox(_T("The target process id cannot be empty."), _T("Warning"), MB_OK | MB_ICONWARNING);
        this->GetDlgItem(IDC_EDIT_PID)->SetFocus();
        return;
    }

    // 保存程序路径
    CHAR szPath[MAX_PATH] = {0};
    ::GetModuleFileName(NULL, (LPTSTR)szPath, MAX_PATH);
    ::PathRemoveFileSpec((LPTSTR)szPath);
    ::PathRemoveBackslash((LPTSTR)szPath);
    ::PathAppend((LPTSTR)szPath, _T("set.ini"));
    WritePrivateProfileString(
        _T("Work Parameter"),       // section name
        _T("Last Dll Path"),	    // key name
        (LPCTSTR)strDllPath,	    // string to add
        (LPCTSTR)szPath             // initialization file name
        ); 

    CString strError;
    OSType osType = CheckOS();
    switch(osType)
    {
    case WindowsXP:
    case WindowsVista:
    case Windows7:
    case Windows10:
        {
            char* pDllPath = NULL;
#ifdef UNICODE
            pDllPath = WideStringToAnsiString((LPCTSTR)strDllPath);
#else
            pDllPath = (LPTSTR)(LPCTSTR)strDllPath;
#endif
            if(UnInjectByPid(dwPid, pDllPath))
            {
                this->SetDlgItemText(IDC_EDIT_LOG, _T("撤销注入成功"));
            }
            else
            {
                this->SetDlgItemText(IDC_EDIT_LOG, _T("撤销注入失败"));
            }
#ifdef UNICODE
            free(pDllPath); pDllPath = NULL;
#endif

        }
        break;
    default:
        break;
    }
}

void CMyInjecterDlg::OnBnClickedBrowser()
{
    const TCHAR szFilter[] = _T("File list (*.dll)|*.dll|All (*.*)|*.*||");
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter);
    if(dlg.DoModal() == IDOK)   
    { 
        CString strFilePath = dlg.GetPathName();
        this->SetDlgItemText(IDC_EDIT_DLL_PATH, strFilePath);
    }	
}

void CMyInjecterDlg::OnBnClickedLanch()
{
    CString strExePath;
    this->GetDlgItemText(IDC_EDIT_EXE_PATH, strExePath);
    if(strExePath.IsEmpty())
    {
        this->MessageBox(_T("The exe name cannot be empty."), _T("Warning"), MB_OK | MB_ICONWARNING);
        this->GetDlgItem(IDC_EDIT_EXE_PATH)->SetFocus();
        return;
    }

    CString strDllPath;
    this->GetDlgItemText(IDC_EDIT_DLL_PATH, strDllPath);
    DWORD dwPid = this->GetDlgItemInt(IDC_EDIT_PID);
    if(strDllPath.IsEmpty())
    {
        this->MessageBox(_T("The DLL name cannot be empty."), _T("Warning"), MB_OK | MB_ICONWARNING);
        this->GetDlgItem(IDC_EDIT_DLL_PATH)->SetFocus();
        return;
    }

    // 保存程序路径
    CHAR szPath[MAX_PATH] = {0};
    ::GetModuleFileName(NULL, (LPTSTR)szPath, MAX_PATH);
    ::PathRemoveFileSpec((LPTSTR)szPath);
    ::PathRemoveBackslash((LPTSTR)szPath);
    ::PathAppend((LPTSTR)szPath, _T("set.ini"));
    WritePrivateProfileString(
        _T("Work Parameter"),       // section name
        _T("Last Exe Path"),	    // key name
        (LPCTSTR)strExePath,	    // string to add
        (LPCTSTR)szPath             // initialization file name
        ); 

    WritePrivateProfileString(
        _T("Work Parameter"),       // section name
        _T("Last Dll Path"),	    // key name
        (LPCTSTR)strDllPath,	    // string to add
        (LPCTSTR)szPath             // initialization file name
        ); 

    CString strError;
    OSType osType = CheckOS();
    switch(osType)
    {
    case WindowsXP:
    case WindowsVista:
    case Windows7:
    case Windows10:
        {
            char* pDllPath = NULL;
#ifdef UNICODE
            pDllPath = WideStringToAnsiString((LPCTSTR)strDllPath);
#else
            pDllPath = (LPTSTR)(LPCTSTR)strDllPath;
#endif
            DWORD dwPid = 0;
            if(LanchProcess((LPCTSTR)strExePath, NULL, NULL, 0, pDllPath, dwPid, strError))
            {
                this->SetDlgItemInt(IDC_EDIT_PID, dwPid);
                this->SetDlgItemText(IDC_EDIT_LOG, _T("注入成功"));
            }
            else
            {
                this->SetDlgItemText(IDC_EDIT_LOG, _T("注入失败：") + strError);
            }
#ifdef UNICODE
            free(pDllPath); pDllPath = NULL;
#endif
        }
        break;
    default:
        break;
    }
}

void CMyInjecterDlg::OnBnClickedBrowserExe()
{
    const TCHAR szFilter[] = _T("File list (*.exe)|*.exe|All (*.*)|*.*||");
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter);
    if(dlg.DoModal() == IDOK)   
    { 
        CString strFilePath = dlg.GetPathName();
        this->SetDlgItemText(IDC_EDIT_EXE_PATH, strFilePath);
    }	
}

