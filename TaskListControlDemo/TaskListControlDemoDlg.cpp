
// TaskListControlDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TaskListControlDemo.h"
#include "TaskListControlDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////

IStream* LoadImageFromResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
	HRSRC hRC = FindResource(hModule, lpName, lpType);
	if (hRC == NULL)
		return NULL;

	HGLOBAL hPkg = LoadResource(NULL, hRC);
	if (hPkg == NULL)
		return NULL;

	DWORD dwSize = SizeofResource(NULL, hRC);
	LPVOID pData = LockResource(hPkg);

	HGLOBAL hImage = GlobalAlloc(GMEM_FIXED, dwSize);
	LPVOID pImageBuf = GlobalLock(hImage);
	memcpy(pImageBuf, pData, dwSize);
	GlobalUnlock(hImage);

	UnlockResource(hPkg);

	IStream *pStream = NULL;
	CreateStreamOnHGlobal(hImage, TRUE, &pStream);

	return pStream;
}

HRESULT LoadImageFromResource(CImage &img, HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
	HRSRC hRC = FindResource(hModule, lpName, lpType);
	if (hRC == NULL)
		return E_FAIL;

	HGLOBAL hPkg = LoadResource(NULL, hRC);
	if (hPkg == NULL)
		return E_FAIL;

	DWORD dwSize = SizeofResource(NULL, hRC);
	LPVOID pData = LockResource(hPkg);

	HGLOBAL hImage = GlobalAlloc(GMEM_FIXED, dwSize);
	LPVOID pImageBuf = GlobalLock(hImage);
	memcpy(pImageBuf, pData, dwSize);
	GlobalUnlock(hImage);

	UnlockResource(hPkg);

	IStream *pStream = NULL;
	CreateStreamOnHGlobal(hImage, TRUE, &pStream);

	img.Destroy();
	HRESULT hRes = img.Load(pStream);

	pStream->Release();
	return hRes;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTaskListControlDemoDlg dialog



CTaskListControlDemoDlg::CTaskListControlDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TASKLISTCONTROLDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTaskListControlDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTaskListControlDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(STXALN_ITEMCLICK, 100, &CTaskListControlDemoDlg::OnListItemClick)
	ON_NOTIFY(STXALN_ITEMDBLCLICK, 100, &CTaskListControlDemoDlg::OnListItemDblClick)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CTaskListControlDemoDlg message handlers

BOOL CTaskListControlDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	srand((unsigned int)time(NULL));

	m_list.Create(_T("ABCDEF"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 240, 360), this, 100);

	CComPtr<IStream> spImage1 = LoadImageFromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PNG1), _T("PNG"));
	CComPtr<IStream> spImage2 = LoadImageFromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PNG2), _T("PNG"));
	CComPtr<IStream> spImage3 = LoadImageFromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PNG3), _T("PNG"));

	CString str;
	for (int i = 0; i < 5; i++)
	{
		str.Format(_T("%d"), rand());
		INT_PTR iItem = m_list.InsertNewItem(0, _T("Group_") + str);
		m_list.SetItemImage(iItem, spImage2, TRUE);
		m_list.SetItemImage(iItem, spImage1, FALSE);
		for (int k = 0; k < rand() % 5; k++)
		{
			str.Format(_T("%d"), rand());
			INT_PTR iSubItem = m_list.InsertNewSubItem(iItem, 0, _T("Item_") + str);
			m_list.SetSubItemImage(iItem, iSubItem, spImage3);
		}
	}

	CComPtr<IStream> spImg = LoadImageFromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PNG5), _T("PNG"));
	m_list.SetBackgroundImage(spImg);

	SetTimer(1, 3000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTaskListControlDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTaskListControlDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTaskListControlDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTaskListControlDemoDlg::OnListItemDblClick(NMHDR *pNotifyStructure, LRESULT *pResult)
{

}

void CTaskListControlDemoDlg::OnListItemClick(NMHDR *pNotifyStructure, LRESULT *pResult)
{
	LPNMSTXALITEMCLICK pNM = (LPNMSTXALITEMCLICK)pNotifyStructure;
	if (pNM->iSubIndex == -1)
	{
		if (pNM->bInHeader)
		{
			if (m_list.IsItemExpanded(pNM->iIndex))
				m_list.ExpandItem(pNM->iIndex, FALSE);
			else
				m_list.ExpandItem(pNM->iIndex, TRUE);
		}
	}
	else
	{
		CString str;
		str.Format(_T("Item_%d"), rand());

		m_list.SetSubItemCaption(pNM->iIndex, pNM->iSubIndex, str);
	}
}

void CTaskListControlDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		CString str;
		str.Format(_T("NewItem_%d"), rand());

		int random = rand();
		int itemIndex = random % m_list.GetItemCount();
		INT_PTR subItemIndex = m_list.InsertNewSubItem(itemIndex, 0, str);

		if (random % 3 == 0)
		{
			CComPtr<IStream> spImg = LoadImageFromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PNG6), _T("PNG"));
			m_list.AddSubItemButton(itemIndex, subItemIndex, spImg, _T("More..."));
		}

		CComPtr<IStream> spImage3 = LoadImageFromResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PNG3), _T("PNG"));
		m_list.SetSubItemImage(itemIndex, subItemIndex, spImage3);
	}

	CDialogEx::OnTimer(nIDEvent);
}
