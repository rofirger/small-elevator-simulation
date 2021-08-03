#pragma once

// CeleMoreInfo 对话框

class CeleMoreInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CeleMoreInfo)

public:
	CeleMoreInfo(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CeleMoreInfo();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListBox m_list1;
	CListBox m_list2;
	CButton button1;
	afx_msg void OnBnClickedButton1();
};
