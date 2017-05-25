#pragma once


// CGroupName 대화 상자입니다.

class CGroupName : public CDialogEx
{
	DECLARE_DYNAMIC(CGroupName)

public:
	CGroupName(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CGroupName();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_GROUP_NAME_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CString m_strGroupName;
};
