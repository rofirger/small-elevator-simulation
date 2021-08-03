/*
 * @Author:罗飞杰
 * @File: CeleMoreInfo.cpp
 * @Date: 2021-7-29
 * @LastEditTime: 2021-7-31
 * @LastEditors: 罗飞杰
 * @brief: 电梯运行时详细信息
 */

#include "pch.h"
#include "elevator.h"
#include "CeleMoreInfo.h"
#include "afxdialogex.h"

 // CeleMoreInfo 对话框

IMPLEMENT_DYNAMIC(CeleMoreInfo, CDialogEx)

CeleMoreInfo::CeleMoreInfo(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{
}

CeleMoreInfo::~CeleMoreInfo()
{
}

void CeleMoreInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_list1);
	DDX_Control(pDX, IDC_LIST1, m_list2);
	DDX_Control(pDX, IDC_BUTTON1, button1);
}

BEGIN_MESSAGE_MAP(CeleMoreInfo, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CeleMoreInfo::OnBnClickedButton1)
END_MESSAGE_MAP()

// CeleMoreInfo 消息处理程序
extern GlobalElevator globalData;
void MoreInfo(CeleMoreInfo* pDlg);
extern int elevatorId;
volatile bool isStop = false;
BOOL CeleMoreInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString str;
	str.Format(_T("%d 号电梯详细信息"), elevatorId);
	this->SetWindowText(str);
	// TODO:  在此添加额外的初始化
	globalData.GetMoreInfoTimer().StartTimer(1000, std::bind(MoreInfo, this));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void MoreInfo(CeleMoreInfo* pDlg)
{
	if (!isStop)
	{
		CString cStr;
		int count1 = pDlg->m_list1.GetCount();
		for (int i = count1; i >= 0; --i)
		{
			pDlg->m_list1.DeleteString(i);
		}
		int count2 = pDlg->m_list2.GetCount();
		for (int i = count2; i >= 0; --i)
		{
			pDlg->m_list2.DeleteString(i);
		}
		for (int i = 0; i < globalData.GetElevators().elevators[elevatorId].elevatorStatus.person.size(); ++i)
		{
			int pfr = globalData.GetElevators().elevators[elevatorId].elevatorStatus.person[i].initialFloor + 1;
			int s = globalData.GetElevators().elevators[elevatorId].elevatorStatus.person[i].distFloor + 1;
			double s1 = globalData.GetElevators().elevators[elevatorId].elevatorStatus.person[i].time;
			cStr.Format(_T("%d%27d                %.4f"), pfr, s, s1);
			pDlg->m_list1.AddString(cStr);
		}
		CString str;
		for (int i = 0; i < globalData.GetElevators().elevators[elevatorId].elevatorStatus.distFloors.size(); ++i)
		{
			int pfr = globalData.GetElevators().elevators[elevatorId].elevatorStatus.distFloors[i].dist + 1;
			double dt = globalData.GetElevators().elevators[elevatorId].elevatorStatus.distFloors[i].waitTimeOfDOWN;
			double ut = globalData.GetElevators().elevators[elevatorId].elevatorStatus.distFloors[i].waitTimeOfUP;
			if (globalData.GetElevators().elevators[elevatorId].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
			{
				str.Format(_T("%d       EXTERN_DOWN       %.4f          %.4f"), pfr, dt, (double)0);
				pDlg->m_list2.AddString(str);
			}
			else if (globalData.GetElevators().elevators[elevatorId].elevatorStatus.distFloors[i].type == EXTERN_UP)
			{
				str.Format(_T("%d       EXTERN_UP         %.4f          %.4f"), pfr, (double)0, ut);
				pDlg->m_list2.AddString(str);
			}
			else
			{
				str.Format(_T("%d        INTERN            %.4f          %.4f"), pfr, (double)0, (double)0);
				pDlg->m_list2.AddString(str);
			}
		}
	}
}

void CeleMoreInfo::OnBnClickedButton1()
{
	if (isStop)
	{
		isStop = false;
		button1.SetWindowTextW(_T("暂停"));
	}
	else
	{
		isStop = true;
		button1.SetWindowTextW(_T("开启"));
	}
}
