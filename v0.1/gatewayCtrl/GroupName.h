#pragma once


// CGroupName ��ȭ �����Դϴ�.

class CGroupName : public CDialogEx
{
	DECLARE_DYNAMIC(CGroupName)

public:
	CGroupName(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CGroupName();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_GROUP_NAME_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CString m_strGroupName;
};
