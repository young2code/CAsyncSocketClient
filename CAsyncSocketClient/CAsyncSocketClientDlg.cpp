
// CAsyncSocketClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CAsyncSocketClient.h"
#include "CAsyncSocketClientDlg.h"

#include "LogMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCAsyncSocketClientDlg dialog




CCAsyncSocketClientDlg::CCAsyncSocketClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCAsyncSocketClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCAsyncSocketClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCAsyncSocketClientDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCAsyncSocketClientDlg message handlers

BOOL CCAsyncSocketClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	mSocket.Create();

	mSocket.ResolveLocalAddress();
	LOG("CCAsyncSocketClientDlg::OnInitDialog() - socket created. [%s]", mSocket.GetLocalAddress());

	mSocket.Connect("127.0.0.1", 54347);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCAsyncSocketClientDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCAsyncSocketClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCAsyncSocketClientDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	mSocket.ShutDown();
}
