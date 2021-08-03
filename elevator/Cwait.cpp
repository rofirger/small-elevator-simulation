// Cwait.cpp: 实现文件
//

#include "pch.h"
#include "elevator.h"
#include "Cwait.h"
#include "afxdialogex.h"
#include "CPassagerFlow.h"
#include "CelevatorSHOW.h"

// Cwait 对话框
// 全局数据
extern GlobalElevator globalData;
extern Assignment** assignment;

IMPLEMENT_DYNAMIC(Cwait, CDialogEx)

Cwait::Cwait(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WAIT, pParent)
{
}

Cwait::~Cwait()
{
}

void Cwait::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cwait, CDialogEx)
END_MESSAGE_MAP()

// Cwait 消息处理程序

BOOL Cwait::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 客流数据生成器
	Passengers passenger(assignment, 0, globalData.GetBuildingParam().floors);
	globalData.SetPassengers(passenger);

	// 关闭此对话框
	EndDialog(0);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
