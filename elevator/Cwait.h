#pragma once

// Cwait 对话框

class Cwait : public CDialogEx
{
	DECLARE_DYNAMIC(Cwait)

public:
	Cwait(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Cwait();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WAIT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
