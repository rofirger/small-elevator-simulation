#pragma once

// CelevatorInfo 对话框

class CelevatorInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CelevatorInfo)

public:
	CelevatorInfo(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CelevatorInfo();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INFORM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CListBox m_list;
	//	afx_msg void OnPaint();
	CComboBox m_combo;
	afx_msg void OnBnClickedButton1();
};
