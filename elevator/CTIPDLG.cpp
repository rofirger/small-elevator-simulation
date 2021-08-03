// CTIPDLG.cpp: 实现文件
//

#include "pch.h"
#include "elevator.h"
#include "CTIPDLG.h"
#include "afxdialogex.h"

// CTIPDLG 对话框

IMPLEMENT_DYNAMIC(CTIPDLG, CDialogEx)

CTIPDLG::CTIPDLG(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TIP_DIALOG, pParent)
{
}

CTIPDLG::~CTIPDLG()
{
}

void CTIPDLG::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, text);
}

BEGIN_MESSAGE_MAP(CTIPDLG, CDialogEx)
	ON_BN_CLICKED(IDOK, &CTIPDLG::OnBnClickedOk)
END_MESSAGE_MAP()

// CTIPDLG 消息处理程序

void CTIPDLG::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

BOOL CTIPDLG::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
