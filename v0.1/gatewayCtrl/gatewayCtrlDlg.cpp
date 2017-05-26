
// gatewayCtrlDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "gatewayCtrl.h"
#include "gatewayCtrlDlg.h"
#include "afxdialogex.h"
#include "ace\Init_ACE.h"
#include "ace\Thread_Manager.h"
#include "ace\OS_NS_string.h"
#include "protocol.h"
#include "GroupName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CgatewayCtrlDlg ��ȭ ����



CgatewayCtrlDlg::CgatewayCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CgatewayCtrlDlg::IDD, pParent)
	, m_uPort(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgatewayCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PORT_EDIT, m_uPort);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctrlIpAddress);
	DDX_Control(pDX, IDC_LOG_LIST, m_ctrlLog);
	DDX_Control(pDX, IDC_LIST, m_ctrlList);
}

BEGIN_MESSAGE_MAP(CgatewayCtrlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CgatewayCtrlDlg::OnBnClickedConnectButton)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_STATUS_BUTTON, &CgatewayCtrlDlg::OnBnClickedReadStatusButton)
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, &CgatewayCtrlDlg::OnBnClickedDisconnectButton)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CgatewayCtrlDlg::OnNMRClickList)
	ON_COMMAND(ID_GENERATE_KEY, &CgatewayCtrlDlg::OnGenerateKey)
END_MESSAGE_MAP()


// CgatewayCtrlDlg �޽��� ó����

BOOL CgatewayCtrlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	ACE::init();

	m_ctrlIpAddress.SetAddress(127, 0, 0, 1);
	m_uPort = CONTRL_PORT;

	UpdateData(FALSE);
	m_ctrlList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ctrlList.InsertColumn(0, _T("CID"), LVCFMT_LEFT, 50, -1);
	m_ctrlList.InsertColumn(1, _T("SerialNo"), LVCFMT_LEFT, 470, -1);
	m_bConnect = FALSE;

	GetDlgItem(IDC_CONNECT_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDC_DISCONNECT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATUS_BUTTON)->EnableWindow(FALSE);
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CgatewayCtrlDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CgatewayCtrlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CgatewayCtrlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CgatewayCtrlDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	ACE::fini();
}

void CgatewayCtrlDlg::OnBnClickedConnectButton()
{
	BYTE f1, f2, f3, f4;
	m_ctrlIpAddress.GetAddress(f1, f2, f3, f4);
	CString strIp,strPort;
	strIp.Format(_T("%d.%d.%d.%d"), f1, f2, f3, f4);
	strPort.Format(_T("%u"), m_uPort);
	log(_T("IP address:") + strIp + _T(",Port:")+strPort);

	ACE_INET_Addr remote_addr(m_uPort, "127.0.0.1");

	if (m_connector.connect(m_stream, remote_addr) == -1) {
		m_bConnect = FALSE;
		GetDlgItem(IDC_CONNECT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISCONNECT_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATUS_BUTTON)->EnableWindow(FALSE);
		log(_T("Connection failed"));
	} else {
		m_bConnect = TRUE;
		ACE_Thread_Manager::instance()->spawn(recvThread, this);
		GetDlgItem(IDC_CONNECT_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_DISCONNECT_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATUS_BUTTON)->EnableWindow(TRUE);
		log(_T("Connected"));
	}
}

int CgatewayCtrlDlg::reqStatus()
{
	int nRtn = 0;
	CString str;

	//prefix
	if ((nRtn = m_stream.send_n(PRF_REQ_STAT, PREFIX_SIZE)) == -1) {
		str.Format(_T("reqStatus, prefix send error:%d"), nRtn);
		log(str);
		return -1;
	}

	//dataSize
	ACE_UINT32 dataSize = 0;
	if ((nRtn = m_stream.send_n(&dataSize, sizeof(ACE_UINT32)) == -1)) {
		str.Format(_T("reqStatus, dataSize send error:%d"), nRtn);
		log(str);
		return -1;
	}
	return 0;
}

void CgatewayCtrlDlg::OnBnClickedReadStatusButton()
{
	GetDlgItem(IDC_CONNECT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_DISCONNECT_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATUS_BUTTON)->EnableWindow(TRUE);
	if (reqStatus() == 0) log(_T("status request sent"));
}

ACE_THR_FUNC_RETURN CgatewayCtrlDlg::recvThread(void *arg)
{
	CgatewayCtrlDlg *pDlg = static_cast<CgatewayCtrlDlg*>(arg);
	ACE_ASSERT(pDlg);

	pDlg->log(_T("recvThread started"));

	size_t recv_cnt = 0;
	char buffer[1024];
	while (pDlg->m_bConnect)
	{
		//prefix
		if ((recv_cnt = pDlg->m_stream.recv_n(buffer, PREFIX_SIZE)) == -1) {
			pDlg->log(_T("prefix receive error"));
			break;
		}
		ACE_ASSERT(PREFIX_SIZE == recv_cnt);

		buffer[PREFIX_SIZE] = 0;
		std::string prefix = buffer;

		//dataSize
		ACE_INT32 dataSize;
		if ((recv_cnt = pDlg->m_stream.recv_n(&dataSize, sizeof(ACE_INT32))) <= 0) {
			pDlg->log(_T("dataSize receive error"));
			break;
		}
		ACE_ASSERT(sizeof(ACE_INT32) == recv_cnt);

		//data
		if ((recv_cnt = pDlg->m_stream.recv_n(buffer, dataSize)) <= 0) {
			pDlg->log(_T("data receive error"));
			break;
		}
		ACE_ASSERT(dataSize == recv_cnt);

		if (prefix == PRF_ACK_STAT) 
			pDlg->onAckStat(buffer,dataSize);
		else if (prefix == PRF_ACK_KEYG)
			pDlg->onAckKeyG(buffer, dataSize);
	}

	pDlg->log(_T("recvThread terminated"));
	return 0;
}

int CgatewayCtrlDlg::onAckStat(const char *buffer,unsigned int len)
{
	m_ctrlList.DeleteAllItems();

	int row = 0;
	unsigned int count = len / (sizeof(ACE_UINT32)+SERIAL_NO_SIZE);
	for (unsigned int i = 0; i < count; i++) {
		const char *pOffset = &buffer[i*(sizeof(ACE_UINT32)+SERIAL_NO_SIZE)];

		ACE_UINT32 cid;
		memcpy(&cid, pOffset, sizeof(ACE_UINT32));

		CString strCid, strSerialNo;
		strCid.Format(_T("%x"), cid);
		m_ctrlList.InsertItem(count, strCid);

		for (int i = 0; i < SERIAL_NO_SIZE; i++) {
			CString strTmp;
			strTmp.Format(_T("%x "), (unsigned char)pOffset[sizeof(ACE_UINT32)+i]);
			strSerialNo += strTmp;
		}
		m_ctrlList.SetItem(row, 1, LVIF_TEXT, strSerialNo,0,0,0,NULL);
		row++;
	}
	return 0;
}

int CgatewayCtrlDlg::onAckKeyG(const char *buffer, unsigned int len)
{
	char groupName[GROUP_NAME_SIZE + 1];
	ACE_OS::memset(groupName, 0, sizeof(groupName));
	ACE_OS::memcpy(groupName, buffer, GROUP_NAME_SIZE);

	CString str(_T("Key generated:"));
	str += (CString(groupName) + CString(_T(" {")));
	buffer += GROUP_NAME_SIZE;
	unsigned int count = (len-GROUP_NAME_SIZE) / sizeof(ACE_UINT32);
	for (unsigned int i = 0; i < count; i++) {
		const char *pOffset = &buffer[i*sizeof(ACE_UINT32)];

		ACE_UINT32 cid;
		memcpy(&cid, pOffset, sizeof(ACE_UINT32));
		CString strTmp;
		strTmp.Format(_T("%u "), cid);
		str += strTmp;
	}
	log(str+_T("}"));
	return 0;
}

void CgatewayCtrlDlg::OnBnClickedDisconnectButton()
{
	m_bConnect = FALSE;
	GetDlgItem(IDC_CONNECT_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDC_DISCONNECT_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATUS_BUTTON)->EnableWindow(FALSE);
	if (m_stream.close() == -1) {
		log(_T("connection close error"));
		return;
	}

	m_ctrlList.DeleteAllItems();
	log(_T("connection closed"));
}

int CgatewayCtrlDlg::log(LPCTSTR lpszItem)
{
	m_ctrlLog.AddString(lpszItem);
	return m_ctrlLog.SetTopIndex(m_ctrlLog.GetCount() - 1);
}

void CgatewayCtrlDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	CPoint CurrentPosition;
	::GetCursorPos(&CurrentPosition);
	CMenu MenuTemp;
	CMenu *pContextMenu = NULL;
	MenuTemp.LoadMenu(IDR_COMMAND_MENU);
	pContextMenu = MenuTemp.GetSubMenu(0);

	SetForegroundWindow();
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, CurrentPosition.x, CurrentPosition.y, this);

	*pResult = 0;
}

void CgatewayCtrlDlg::OnGenerateKey()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	int nSelected = selectedCount();	//���õ� �����۵��� ����
	if (nSelected == 0) {
		log(_T("select CID to generate key"));
		return;
	}

	CString strGroupName;
	CGroupName gn;
	if (gn.DoModal() == IDOK) {
		strGroupName = gn.m_strGroupName;
	}else
		return;

	int nRtn = 0;
	CString strErr;
	//prefix
	if ((nRtn = m_stream.send_n(PRF_REQ_KEYG, PREFIX_SIZE)) == -1) {
		strErr.Format(_T("reqKeyGen, prefix send error:%d"), nRtn);
		log(strErr);
		return;
	}

	//dataSize
	ACE_UINT32 dataSize = GROUP_NAME_SIZE + sizeof(ACE_UINT32)*nSelected;
	if ((nRtn = m_stream.send_n(&dataSize, sizeof(ACE_UINT32)) == -1)) {
		strErr.Format(_T("reqKeyGen, dataSize send error:%d"), nRtn);
		log(strErr);
		return;
	}

	//data (GroupName+CIDs)
	//GroupName
	char groupName[GROUP_NAME_SIZE];
	ACE_OS::memset(groupName, 0, GROUP_NAME_SIZE);
	ACE_OS::memcpy(groupName, CT2A(strGroupName), strGroupName.GetLength()>GROUP_NAME_SIZE ? GROUP_NAME_SIZE : strGroupName.GetLength());
	if ((nRtn = m_stream.send_n(groupName, GROUP_NAME_SIZE) == -1)) {
		strErr.Format(_T("reqKeyGen, data(GroupName) send error:%d"), nRtn);
		log(strErr);
		return;
	}

	//CIDs
	CString strSelected,strGroupMsg(_T("GroupName:")+strGroupName+_T(" { "));
	strSelected.Format(_T("selected:"));
	POSITION pos = m_ctrlList.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nSelected = m_ctrlList.GetNextSelectedItem(pos);

		CString str,strCID;
		strCID = m_ctrlList.GetItemText(nSelected, 0);
		str.Format(_T("(%d:%s) "), nSelected, strCID);
		strSelected += str;
		strGroupMsg += (strCID + _T(" "));

		ACE_UINT32 cid = ACE_OS::atoi(strCID);
		if ((nRtn = m_stream.send_n(&cid, sizeof(ACE_UINT32)) == -1)) {
			strErr.Format(_T("reqKeyGen, data send error:%d"), nRtn);
			log(strErr);
			return;
		}
	}
	strGroupMsg += _T("}");
	TRACE(_T("%s\n"), strSelected.GetBuffer(strSelected.GetLength()));
	log(strGroupMsg);
}

int CgatewayCtrlDlg::selectedCount()
{
	int nCount = 0;
	POSITION pos = m_ctrlList.GetFirstSelectedItemPosition();
	while (pos)
	{
		m_ctrlList.GetNextSelectedItem(pos);
		nCount++;
	}
	return nCount;
}