#pragma once

// CPassagerFlow 对话框

class CPassagerFlow : public CDialogEx
{
	DECLARE_DYNAMIC(CPassagerFlow)

public:
	CPassagerFlow(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPassagerFlow();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FLOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox IDC_COMNOX;
	virtual BOOL OnInitDialog();
	float m_ArrivalRate;
	float m_PercentageUp;
	float m_PercentageDown;
	float m_PercentageMedium;
	CComboBox m_Comb;
	int m_NumOfPeople;
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnBnClickedButton1();
	CEdit display1;
	afx_msg void OnEnChangeEdit10();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk();
	int m_Period;
	int m_NumOfPeriod;
	CComboBox PeriodComb;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedCancel();
	CListBox m_ListBox;
	afx_msg void OnEnChangeEdit8();
	afx_msg void OnEnChangeEdit9();
};
typedef struct Assignment {
	int m_NumOfPeople;
	float m_ArrivalRate;
	float m_PercentageUp;
	float m_PercentageDown;
	float m_PercentageMedium;
}Assignment;
