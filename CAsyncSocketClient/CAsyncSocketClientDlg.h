
// CAsyncSocketClientDlg.h : header file
//

#pragma once

#include "ClientSocket.h"


// CCAsyncSocketClientDlg dialog
class CCAsyncSocketClientDlg : public CDialog
{
// Construction
public:
	CCAsyncSocketClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CASYNCSOCKETCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	ClientSocket mSocket;
public:
	afx_msg void OnDestroy();
};
