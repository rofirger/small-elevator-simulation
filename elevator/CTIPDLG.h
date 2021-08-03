#pragma once


// CTIPDLG 对话框

class CTIPDLG : public CDialogEx
{
	DECLARE_DYNAMIC(CTIPDLG)

public:
	CTIPDLG(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CTIPDLG();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TIP_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CButton text;
};
