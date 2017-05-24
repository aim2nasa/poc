
// gatewayCtrlDlg.h : ��� ����
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "ace/SOCK_Connector.h" 

#define CONTRL_PORT 9875

// CgatewayCtrlDlg ��ȭ ����
class CgatewayCtrlDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CgatewayCtrlDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_GATEWAYCTRL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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
