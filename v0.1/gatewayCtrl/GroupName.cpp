// GroupName.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "gatewayCtrl.h"
#include "GroupName.h"
#include "afxdialogex.h"


// CGroupName ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CGroupName, CDialogEx)

CGroupName::CGroupName(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGroupName::IDD, pParent)
	, m_strGroupName(_T(""))
{

}

CGroupName::~CGroupName()
{
}

void CGroupName::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME_EDIT, m_strGroupName);
	DDV_MaxChars(pDX, m_strGroupName, 16);
}


BEGIN_MESSAGE_MAP(CGroupName, CDialogEx)
END_MESSAGE_MAP()


// CGroupName �޽��� ó�����Դϴ�.
