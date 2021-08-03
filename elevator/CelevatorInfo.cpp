/*
 * @Author: 罗飞杰
 * @File: CelevatorInfo.cpp
 * @Date: 2021-7-29
 * @LastEditTime: 2021-7-31
 * @LastEditors: 罗飞杰
 */

#include "pch.h"
#include "elevator.h"
#include "CelevatorInfo.h"
#include "afxdialogex.h"
#include "timer.h"
using namespace std;
// CelevatorInfo 对话框
CeleMoreInfo* dlg;
IMPLEMENT_DYNAMIC(CelevatorInfo, CDialogEx)

CelevatorInfo::CelevatorInfo(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INFORM, pParent)
{
}

CelevatorInfo::~CelevatorInfo()
{
}

void CelevatorInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST3, m_list);
	DDX_Control(pDX, IDC_COMBO4, m_combo);
}

BEGIN_MESSAGE_MAP(CelevatorInfo, CDialogEx)
	ON_WM_CLOSE()
	//	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON1, &CelevatorInfo::OnBnClickedButton1)
END_MESSAGE_MAP()

// CelevatorInfo 消息处理程序
extern GlobalElevator globalData;
extern volatile bool isStop;
volatile bool IS_STOP_INFO = false;
void CelevatorInfo::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (globalData.GetCeleMoreInfoDlg() != NULL)
	{
		isStop = true;
		globalData.GetMoreInfoTimer().Expire();
		delete globalData.GetCeleMoreInfoDlg();
		globalData.SetCeleMoreInfoDlg(NULL);
	}
	CDialogEx::OnClose();
}
void Info(CelevatorInfo* pDlg)
{
	if (!IS_STOP_INFO)
	{
		CString cStr;
		int count = pDlg->m_list.GetCount();
		for (int i = count; i >= 0; --i)
		{
			pDlg->m_list.DeleteString(i);
		}
		for (int i = 0; i < globalData.GetBuildingParam().numOfElevator; ++i)
		{
			int pfr = globalData.GetElevators().elevators[i].elevatorStatus.presentFloor + 1;
			int s = globalData.GetElevators().elevators[i].elevatorStatus.person.size();
			int s1 = globalData.GetElevators().elevators[i].elevatorStatus.distFloors.size();
			if (globalData.GetElevators().elevators[i].elevatorStatus.dir == UP)
			{
				cStr.Format(_T("%d          UP%17d%13d%12d"), i, pfr, s, s1);
				pDlg->m_list.AddString(cStr);
			}
			else if (globalData.GetElevators().elevators[i].elevatorStatus.dir == DOWN)
			{
				cStr.Format(_T("%d          DOWN%15d%13d%12d"), i, pfr, s, s1);
				pDlg->m_list.AddString(cStr);
			}
			else
			{
				cStr.Format(_T("%d         STATIC%14d%13d%12d"), i, pfr, s, s1);
				pDlg->m_list.AddString(cStr);
			}
		}
	}
}
BOOL CelevatorInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//
	globalData.GetInfoTimer().StartTimer(1000, std::bind(Info, this));
	for (int i = 0; i < globalData.GetBuildingParam().numOfElevator; ++i)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_combo.AddString(str);
	}
	m_combo.SetCurSel(0);
	return TRUE;
}
extern GlobalElevator globalData;
int elevatorId;
void CelevatorInfo::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	isStop = false;
	if (globalData.GetCeleMoreInfoDlg() != NULL)
	{
		delete globalData.GetCeleMoreInfoDlg();
		globalData.SetCeleMoreInfoDlg(NULL);
	}
	elevatorId = m_combo.GetCurSel();
	globalData.GetMoreInfoTimer().Expire();
	dlg = new CeleMoreInfo;
	globalData.SetCeleMoreInfoDlg(dlg);
	dlg->Create(IDD_DIALOG2);
	dlg->ShowWindow(SW_SHOWNORMAL);
}
