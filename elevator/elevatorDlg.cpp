/*
 * @Author: 宗杰
 * @File: elevatorDlg.cpp
 * @Date: 2021-7-10
 * @LastEditTime: 2021-7-16
 * @LastEditors: 宗杰
 * @brief: 电梯参数及建筑物参数设置界面
 */
#include "pch.h"
#include "framework.h"
#include "elevator.h"
#include "elevatorDlg.h"
#include "afxdialogex.h"
#include <string>
#include"CTIPDLG.h"
#include"CPassagerFlow.h"
#include<iostream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 // CelevatorDlg 对话框
extern GlobalElevator globalData;
CelevatorDlg::CelevatorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ELEVATOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ELEVATOR);
}

void CelevatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STOREY_HEIGHT, storeyHeight);
	DDX_Text(pDX, IDC_FLOOR_NUM, floors);
	DDX_Text(pDX, IDC_ELEVATOR_NUM, numOfElevator);
	//DDX_Text(pDX, IDC_ELEVATOR_ID, id);
	DDX_Text(pDX, IDC_RATED_PEOPLE, ratedCapacity);
	DDX_Text(pDX, IDC_CLOSE_TIME, timeOfCloseDoor);
	DDX_Text(pDX, IDC_OPEN_TIME, timeOfOpenDoor);
	DDX_Text(pDX, IDC_WAIT_TIME, timeOfWait);
	DDX_Text(pDX, IDC_BRAKE_TIME, timeOfBrake);
	DDX_Text(pDX, IDC_RATED_SPEED, ratedSpeed);
	DDX_Text(pDX, IDC_RATED_ACCELERATE, ratedAcceleration);
	DDX_Text(pDX, IDC_RATED_JERK, ratedJerk);
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
}

BEGIN_MESSAGE_MAP(CelevatorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_ELEVATOR_NUM, &CelevatorDlg::OnEnChangeElevatorNum)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CelevatorDlg::OnCbnSelchangeCombo1)
	ON_EN_CHANGE(IDC_ELEVATOR_ID, &CelevatorDlg::OnEnChangeElevatorId)
	//	ON_BN_CLICKED(IDC_BUTTON1, &CelevatorDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CONFIG, &CelevatorDlg::OnClickedSet)
	ON_BN_CLICKED(IDC_BUTTON1, &CelevatorDlg::OnAllSet)
	ON_BN_CLICKED(IDC_UNIFORM_CONFIG, &CelevatorDlg::OnBnClickedUniformConfig)
END_MESSAGE_MAP()

// CelevatorDlg 消息处理程序
extern GlobalElevator globalData;
BOOL CelevatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CFont* f;
	f = new CFont;
	f->CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	GetDlgItem(IDC_BUTTON1)->SetFont(f);
	// 编辑框默认值设置
	UpdateData(TRUE);
	floors = 30;
	numOfElevator = 7;
	storeyHeight = 3;
	id = 1;
	ratedCapacity = 16;
	timeOfCloseDoor = 2;
	timeOfOpenDoor = 2;
	timeOfWait = 5;
	timeOfBrake = 1;
	ratedSpeed = 2;
	ratedAcceleration = 1;
	ratedJerk = 1.5;
	UpdateData(FALSE);
	for (int i = 0; i < numOfElevator; ++i)
	{
		m_Combo.AddString(CString(std::to_string(i + 1).c_str()));
	}
	if (globalData.GetElevatorParam() != NULL)
	{
		delete[]globalData.GetElevatorParam();
	}
	ElevatorParam* temp = new ElevatorParam[numOfElevator];

	for (int i = 0; i < numOfElevator; ++i)
	{
		temp[i].id = i;
		temp[i].ratedAcceleration = ratedAcceleration;
		temp[i].ratedCapacity = ratedCapacity;
		temp[i].ratedJerk = ratedJerk;
		temp[i].ratedSpeed = ratedSpeed;
		temp[i].timeOfBrake = timeOfBrake;
		temp[i].timeOfCloseDoor = timeOfCloseDoor;
		temp[i].timeOfOpenDoor = timeOfOpenDoor;
		temp[i].timeOfWait = timeOfWait;
	}
	globalData.SetElevatorParam(temp);
	m_Combo.SetCurSel(0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CelevatorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CelevatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CelevatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CelevatorDlg::OnEnChangeElevatorNum()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	// 更新combo内容
	CString str;
	GetDlgItemText(IDC_ELEVATOR_NUM, str);
	int num = _ttoi(str);
	m_Combo.ResetContent();
	for (int i = 0; i < num; ++i)
	{
		m_Combo.AddString(CString(std::to_string(i + 1).c_str()));
	}
	if (globalData.GetElevatorParam() != NULL)
	{
		delete[]globalData.GetElevatorParam();
	}
	ElevatorParam* temp = new ElevatorParam[num];
	globalData.SetElevatorParam(temp);
	for (int i = 0; i < num; ++i)
	{
		temp[i].id = i;
		temp[i].ratedAcceleration = ratedAcceleration;
		temp[i].ratedCapacity = ratedCapacity;
		temp[i].ratedJerk = ratedJerk;
		temp[i].ratedSpeed = ratedSpeed;
		temp[i].timeOfBrake = timeOfBrake;
		temp[i].timeOfCloseDoor = timeOfCloseDoor;
		temp[i].timeOfOpenDoor = timeOfOpenDoor;
		temp[i].timeOfWait = timeOfWait;
	}
	m_Combo.SetCurSel(0);
}

int comBoValue;
void CelevatorDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_Combo.GetCurSel();
	comBoValue = index;
	UpdateData(TRUE);
	ratedAcceleration = globalData.GetElevatorParam()[index].ratedAcceleration;
	ratedCapacity = globalData.GetElevatorParam()[index].ratedCapacity;
	ratedJerk = globalData.GetElevatorParam()[index].ratedJerk;
	ratedSpeed = globalData.GetElevatorParam()[index].ratedSpeed;
	timeOfBrake = globalData.GetElevatorParam()[index].timeOfBrake;
	timeOfCloseDoor = globalData.GetElevatorParam()[index].timeOfCloseDoor;
	timeOfOpenDoor = globalData.GetElevatorParam()[index].timeOfOpenDoor;
	timeOfWait = globalData.GetElevatorParam()[index].timeOfWait;

	UpdateData(FALSE);
}

void CelevatorDlg::OnEnChangeElevatorId()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CelevatorDlg::OnClickedSet()//设置不同电梯好参数
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	ElevatorParam paramElevator;
	paramElevator.id = comBoValue;
	paramElevator.ratedAcceleration = ratedAcceleration;
	paramElevator.ratedCapacity = ratedCapacity;
	paramElevator.ratedJerk = ratedJerk;
	paramElevator.ratedSpeed = ratedSpeed;
	paramElevator.timeOfBrake = timeOfBrake;
	paramElevator.timeOfCloseDoor = timeOfCloseDoor;
	paramElevator.timeOfOpenDoor = timeOfOpenDoor;
	paramElevator.timeOfWait = timeOfWait;
	globalData.GetElevatorParam()[comBoValue] = paramElevator;
	UpdateData(false);
	MessageBox(_T("设置成功"), _T("提示"), 0);
}

void CelevatorDlg::OnAllSet()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (floors > 162) {
		MessageBox(_T("楼层数不得超过162"), _T("提示"), 0);
		return;
	}
	if (numOfElevator > 30) {
		MessageBox(_T("电梯数不得超过30"), _T("提示"), 0);
		return;
	}
	BuildingParam buildingParam;
	buildingParam.floors = floors;
	buildingParam.numOfElevator = numOfElevator;
	buildingParam.storeyHeight = storeyHeight;
	globalData.SetBuildingParam(buildingParam);
	UpdateData(FALSE);
	CPassagerFlow Dlg;
	Dlg.DoModal();
}

void CelevatorDlg::OnBnClickedUniformConfig()
{
	// TODO: 在此添加控件通知处理程序代码
	CTIPDLG tipDlg;
	INT_PTR nRes;
	nRes = tipDlg.DoModal();
	if (IDCANCEL == nRes)
		return;
	UpdateData(TRUE);
	ElevatorParam paramElevator;
	paramElevator.ratedAcceleration = ratedAcceleration;
	paramElevator.ratedCapacity = ratedCapacity;
	paramElevator.ratedJerk = ratedJerk;
	paramElevator.ratedSpeed = ratedSpeed;
	paramElevator.timeOfBrake = timeOfBrake;
	paramElevator.timeOfCloseDoor = timeOfCloseDoor;
	paramElevator.timeOfOpenDoor = timeOfOpenDoor;
	paramElevator.timeOfWait = timeOfWait;
	for (int i = 0; i < numOfElevator; i++)
	{
		paramElevator.id = i;
		globalData.GetElevatorParam()[i] = paramElevator;
	}
	UpdateData(FALSE);
}
