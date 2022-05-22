
// TaskListControlDemoDlg.h : header file
//

#pragma once
#include "STXListControlEx.h"


// CTaskListControlDemoDlg dialog
class CTaskListControlDemoDlg : public CDialogEx
{
// Construction
public:
	CTaskListControlDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASKLISTCONTROLDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CSTXListControlEx m_list;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnListItemClick(NMHDR *pNotifyStructure, LRESULT *pResult);
	afx_msg void OnListItemDblClick(NMHDR *pNotifyStructure, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
