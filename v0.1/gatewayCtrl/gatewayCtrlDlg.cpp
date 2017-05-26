
// gatewayCtrlDlg.cpp : 구현 파일
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


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CgatewayCtrlDlg 대화 상자



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


// CgatewayCtrlDlg 메시지 처리기

BOOL CgatewayCtrlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
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
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CgatewayCtrlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

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
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	int nSelected = selectedCount();	//선택된 아이템들의 갯수
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