// GroupName.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "gatewayCtrl.h"
#include "GroupName.h"
#include "afxdialogex.h"


// CGroupName 대화 상자입니다.

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


// CGroupName 메시지 처리기입니다.
