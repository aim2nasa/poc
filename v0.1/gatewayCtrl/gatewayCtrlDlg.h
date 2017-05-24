
// gatewayCtrlDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "ace/SOCK_Connector.h" 

#define CONTRL_PORT 9875

// CgatewayCtrlDlg 대화 상자
class CgatewayCtrlDlg : public CDialogEx
{
// 생성입니다.
public:
	CgatewayCtrlDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_GATEWAYCTRL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedConnectButton();
	afx_msg void OnBnClickedReadStatusButton();
	afx_msg void OnBnClickedDisconnectButton();
	afx_msg void OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGenerateKey();
	DECLARE_MESSAGE_MAP()

	int reqStatus();
	static ACE_THR_FUNC_RETURN recvThread(void *arg);
	int onAckStat(const char *buffer, unsigned int len);
	int log(LPCTSTR lpszItem);
public:
	UINT m_uPort;
	CIPAddressCtrl m_ctrlIpAddress;
	CListBox m_ctrlLog;
	CListCtrl m_ctrlList;
	

	ACE_SOCK_Stream m_stream;
	ACE_SOCK_Connector m_connector;
};
