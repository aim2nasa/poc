
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
	DECLARE_MESSAGE_MAP()

	int reqStatus();
	afx_msg void OnBnClickedReadStatusButton();

public:
	UINT m_uPort;
	CIPAddressCtrl m_ctrlIpAddress;
	CListBox m_ctrlLog;

	ACE_SOCK_Stream m_stream;
	ACE_SOCK_Connector m_connector;
};
