/*
 * @Author: 宗杰、罗飞杰
 * @File: CPassengerFlow.cpp
 * @Date: 2021-7-16
 * @LastEditTime: 2021-7-26
 * @LastEditors: 罗飞杰
 * @brief: 电梯客流设置窗体
 */

#include "pch.h"
#include "elevator.h"
#include "CPassagerFlow.h"
#include "afxdialogex.h"
#include"elevatorDlg.h"
#include<string>
#include"CTIPDLG.h"
#include "CelevatorSHOW.h"
#include "CelevatorInfo.h"

 //#define LISTBOX_FORMAT(a,b,c,d,e,f,g) "%-"#a"d%-"#b"d%-"#c"d%-"#d"g%-"#e"g%-"#f"g%-"#g"g"
 // CPassagerFlow 对话框
using namespace std;
IMPLEMENT_DYNAMIC(CPassagerFlow, CDialogEx)
extern bool IS_STOP_ALL;

CPassagerFlow::CPassagerFlow(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FLOW, pParent)
	, m_ArrivalRate(0.5)
	, m_PercentageUp(0.1)
	, m_PercentageDown(0.15)
	, m_PercentageMedium(0.75)
	, m_NumOfPeople(40)
	, m_Period(180)
	, m_NumOfPeriod(6)
{
}

CPassagerFlow::~CPassagerFlow()
{
}

void CPassagerFlow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT7, m_ArrivalRate);
	DDX_Text(pDX, IDC_EDIT8, m_PercentageUp);
	DDX_Text(pDX, IDC_EDIT9, m_PercentageDown);
	DDX_Text(pDX, IDC_EDIT10, m_PercentageMedium);
	DDX_Control(pDX, IDC_COMBO3, m_Comb);
	DDX_Text(pDX, IDC_EDIT5, m_NumOfPeople);
	//DDX_Control(pDX, IDC_EDIT1, display1);
	DDX_Text(pDX, IDC_EDIT2, m_Period);
	DDX_Text(pDX, IDC_EDIT3, m_NumOfPeriod);
	DDX_Control(pDX, IDC_COMBO2, PeriodComb);
	DDX_Control(pDX, IDC_LIST5, m_ListBox);
}
BEGIN_MESSAGE_MAP(CPassagerFlow, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CPassagerFlow::OnCbnSelchangeCombo3)
	ON_BN_CLICKED(IDC_BUTTON1, &CPassagerFlow::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT10, &CPassagerFlow::OnEnChangeEdit10)
	ON_BN_CLICKED(IDC_BUTTON2, &CPassagerFlow::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CPassagerFlow::OnBnClickedOk)
	//ON_BN_CLICKED(IDC_BUTTON3, &CPassagerFlow::OnBnClickedButton3)
	ON_EN_CHANGE(IDC_EDIT3, &CPassagerFlow::OnEnChangeEdit3)
	ON_BN_CLICKED(IDCANCEL, &CPassagerFlow::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT8, &CPassagerFlow::OnEnChangeEdit8)
	ON_EN_CHANGE(IDC_EDIT9, &CPassagerFlow::OnEnChangeEdit9)
END_MESSAGE_MAP()

// CPassagerFlow 消息处理程序

extern GlobalElevator globalData;
Assignment** assignment = NULL;
int sizeOfAssignment;
int oldNumOfPeriod;
BOOL CPassagerFlow::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateData(TRUE);
	for (int k = 0; k < m_NumOfPeriod; k++)
	{
		PeriodComb.AddString(CString(std::to_string(k + 1).c_str()));
	}
	for (int i = 0; i < globalData.GetBuildingParam().floors; ++i)
	{
		m_Comb.AddString(CString(std::to_string(i + 1).c_str()));
	}
	sizeOfAssignment = globalData.GetBuildingParam().floors;
	assignment = new Assignment * [m_NumOfPeriod];
	oldNumOfPeriod = m_NumOfPeriod;
	int count = m_ListBox.GetCount();
	for (int i = count; i >= 0; i--)
		m_ListBox.DeleteString(i);
	for (int m = 0; m < m_NumOfPeriod; ++m) {
		assignment[m] = new Assignment[sizeOfAssignment];
		for (int j = 0; j < sizeOfAssignment; j++) {
			assignment[m][j].m_NumOfPeople = m_NumOfPeople;
			assignment[m][j].m_ArrivalRate = m_ArrivalRate;
			assignment[m][j].m_PercentageUp = m_PercentageUp;
			assignment[m][j].m_PercentageDown = m_PercentageDown;
			assignment[m][j].m_PercentageMedium = m_PercentageMedium;
			int a = 23, b = 13, c = 13, d = 16, e = 16, f = 16, g = 20;
			a -= (int)log10(m + 1);
			b -= (int)log10(j + 1);
			c -= (int)log10(assignment[m][j].m_NumOfPeople);
			CString str;
			string temp = "\t%-" + to_string(a) + "d%-" + to_string(b) + "d%-" + to_string(c) + "d%-" + to_string(d) + ".4f%-" + to_string(e) + ".4f%-" + to_string(f) + ".4f%-" + to_string(g) + ".4f";
			CString tempTemp;
			tempTemp = temp.c_str();
			str.Format(tempTemp, m + 1, j + 1, assignment[m][j].m_NumOfPeople, assignment[m][j].m_ArrivalRate,
				assignment[m][j].m_PercentageUp, assignment[m][j].m_PercentageDown, assignment[m][j].m_PercentageMedium);
			m_ListBox.AddString(str);
		}
	}
	/*for (int i = 0; i < m_NumOfPeriod; ++i)
	{
		for (int j = 0; j < sizeOfAssignment; ++j)
		{
			CString str;
			str.Format(_T(""));
		}
	}*/
	m_Comb.SetCurSel(0);
	PeriodComb.SetCurSel(0);
	globalData.SetNumOfPeriod(m_NumOfPeriod);
	UpdateData(FALSE);
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

int combValue, combvalue;
void CPassagerFlow::OnCbnSelchangeCombo3()
{
	// TODO: 在此添加控件通知处理程序代码
	combvalue = PeriodComb.GetCurSel();
	combValue = m_Comb.GetCurSel();
	UpdateData(true);
	m_NumOfPeople = assignment[combvalue][combValue].m_NumOfPeople;
	m_ArrivalRate = assignment[combvalue][combValue].m_ArrivalRate;
	m_PercentageDown = assignment[combvalue][combValue].m_PercentageDown;
	m_PercentageMedium = assignment[combvalue][combValue].m_PercentageMedium;
	m_PercentageUp = assignment[combvalue][combValue].m_PercentageUp;
	UpdateData(false);
}

void CPassagerFlow::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_PercentageUp + m_PercentageDown + m_PercentageMedium > 1)
	{
		MessageBox(_T("占比之和不能大于1"), _T("提示"), 0);
		m_PercentageUp = 0.1;
		m_PercentageDown = 0.15;
		m_PercentageMedium = 0.75;
		UpdateData(FALSE);
		return;
	}
	m_ListBox.DeleteString(combvalue * globalData.GetBuildingParam().floors + combValue);
	assignment[combvalue][combValue].m_NumOfPeople = m_NumOfPeople;
	assignment[combvalue][combValue].m_ArrivalRate = m_ArrivalRate;
	assignment[combvalue][combValue].m_PercentageDown = m_PercentageDown;
	assignment[combvalue][combValue].m_PercentageMedium = m_PercentageMedium;
	assignment[combvalue][combValue].m_PercentageUp = m_PercentageUp;
	int m = combvalue, j = combValue;
	int a = 23, b = 13, c = 13, d = 16, e = 16, f = 16, g = 20;
	a -= (int)log10(m + 1);
	b -= (int)log10(j + 1);
	c -= (int)log10(assignment[m][j].m_NumOfPeople);
	CString str;
	string temp = "\t%-" + to_string(a) + "d%-" + to_string(b) + "d%-" + to_string(c) + "d%-" + to_string(d) + ".4f%-" + to_string(e) + ".4f%-" + to_string(f) + ".4f%-" + to_string(g) + ".4f";
	CString tempTemp;
	tempTemp = temp.c_str();
	str.Format(tempTemp, m + 1, j + 1, assignment[m][j].m_NumOfPeople, assignment[m][j].m_ArrivalRate,
		assignment[m][j].m_PercentageUp, assignment[m][j].m_PercentageDown, assignment[m][j].m_PercentageMedium);
	m_ListBox.InsertString(combvalue * globalData.GetBuildingParam().floors + combValue, str);
	UpdateData(false);
	MessageBox(_T("设置成功"), _T("提示"), 0);
}

void CPassagerFlow::OnEnChangeEdit10()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	UpdateData(TRUE);
	if (m_PercentageMedium > 1)
	{
		MessageBox(_T("占比不能大于1"), _T("提示"), 0);
		m_PercentageMedium = 0.75;
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void CPassagerFlow::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_PercentageUp + m_PercentageDown + m_PercentageMedium > 1)
	{
		MessageBox(_T("占比之和不能大于1"), _T("提示"), 0);
		m_PercentageUp = 0.1;
		m_PercentageDown = 0.15;
		m_PercentageMedium = 0.75;
		UpdateData(FALSE);
		return;
	}
	CTIPDLG tipDlg;
	INT_PTR nRes;
	nRes = tipDlg.DoModal();
	if (IDCANCEL == nRes)
		return;
	UpdateData(true);
	int count = m_ListBox.GetCount();
	for (int i = count; i >= 0; i--)
		m_ListBox.DeleteString(i);
	for (int m = 0; m < m_NumOfPeriod; ++m) {
		for (int j = 0; j < sizeOfAssignment; ++j)
		{
			assignment[m][j].m_NumOfPeople = m_NumOfPeople;
			assignment[m][j].m_ArrivalRate = m_ArrivalRate;
			assignment[m][j].m_PercentageDown = m_PercentageDown;
			assignment[m][j].m_PercentageMedium = m_PercentageMedium;
			assignment[m][j].m_PercentageUp = m_PercentageUp;
			int a = 23, b = 13, c = 13, d = 16, e = 16, f = 16, g = 20;
			a -= (int)log10(m + 1);
			b -= (int)log10(j + 1);
			c -= (int)log10(assignment[m][j].m_NumOfPeople);
			CString str;
			string temp = "\t%-" + to_string(a) + "d%-" + to_string(b) + "d%-" + to_string(c) + "d%-" + to_string(d) + ".4f%-" + to_string(e) + ".4f%-" + to_string(f) + ".4f%-" + to_string(g) + ".4f";
			CString tempTemp;
			tempTemp = temp.c_str();
			str.Format(tempTemp, m + 1, j + 1, assignment[m][j].m_NumOfPeople, assignment[m][j].m_ArrivalRate,
				assignment[m][j].m_PercentageUp, assignment[m][j].m_PercentageDown, assignment[m][j].m_PercentageMedium);
			m_ListBox.AddString(str);
		}
	}
	UpdateData(false);
}

void CPassagerFlow::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (IS_STOP_ALL)
	{
		::MessageBox(NULL, _T("所有设置均已取消，如需再次仿真，请重新打开软件！"), _T("提示"), 0);
		return;
	}
	PassengerFLow passengerFlow;
	passengerFlow.totalPassengerNum.passengerFlow = new int[m_NumOfPeriod];
	passengerFlow.enterPassengerNum.passengerFlow = new int[m_NumOfPeriod];
	passengerFlow.outPassengerNum.passengerFlow = new int[m_NumOfPeriod];
	passengerFlow.totalLayerPassengerNum.passengerFlow = new int[m_NumOfPeriod];
	passengerFlow.maxLayerPassengerNum.passengerFlow = new int[m_NumOfPeriod];
	passengerFlow.otherLayerPassengerNum.passengerFlow = new int[m_NumOfPeriod];
	for (int i = 0; i < m_NumOfPeriod; ++i) {
		passengerFlow.totalPassengerNum.passengerFlow[i] = 0;
		passengerFlow.enterPassengerNum.passengerFlow[i] = 0;
		passengerFlow.outPassengerNum.passengerFlow[i] = 0;
		passengerFlow.totalLayerPassengerNum.passengerFlow[i] = 0;
		passengerFlow.maxLayerPassengerNum.passengerFlow[i] = 0;
		passengerFlow.otherLayerPassengerNum.passengerFlow[i] = 0;
	}
	int temp = assignment[0][0].m_NumOfPeople * assignment[0][0].m_PercentageMedium;
	for (int i = 0; i < m_NumOfPeriod; ++i) {
		for (int j = 0; j < sizeOfAssignment; ++j) {
			passengerFlow.totalPassengerNum.passengerFlow[i] += assignment[i][j].m_NumOfPeople;
			passengerFlow.enterPassengerNum.passengerFlow[i] += assignment[i][j].m_NumOfPeople * assignment[i][j].m_PercentageUp;
			passengerFlow.outPassengerNum.passengerFlow[i] += assignment[i][j].m_NumOfPeople * assignment[i][j].m_PercentageDown;
			passengerFlow.totalLayerPassengerNum.passengerFlow[i] += assignment[i][j].m_NumOfPeople * assignment[i][j].m_PercentageMedium;
		}
	}
	for (int i = 0; i < m_NumOfPeriod; ++i) {
		for (int j = 0; j < sizeOfAssignment; ++j) {
			if (temp < assignment[i][j].m_NumOfPeople * assignment[i][j].m_PercentageMedium) {
				temp = assignment[i][j].m_NumOfPeople * assignment[i][j].m_PercentageMedium;
			}
		}
		passengerFlow.maxLayerPassengerNum.passengerFlow[i] = temp;
		passengerFlow.otherLayerPassengerNum.passengerFlow[i] = passengerFlow.totalLayerPassengerNum.passengerFlow[i] - temp;
	}
	UpdateData(TRUE);
	// 分钟级别
	globalData.SetDataInterval(m_Period / 60);
	globalData.SetNumOfPeriod(m_NumOfPeriod);
	UpdateData(FALSE);
	globalData.SetPassagerFlow(passengerFlow);

	CelevatorSHOW elevatorShow;
	elevatorShow.DoModal();
}

void CPassagerFlow::OnEnChangeEdit3()
{
	for (int i = 0; i < oldNumOfPeriod; ++i)
	{
		delete[]assignment[i];
	}
	delete[]assignment;
	UpdateData(true);
	PeriodComb.ResetContent();
	for (int k = 0; k < m_NumOfPeriod; k++)
	{
		PeriodComb.AddString(CString(std::to_string(k + 1).c_str()));
	}
	sizeOfAssignment = globalData.GetBuildingParam().floors;
	assignment = new Assignment * [m_NumOfPeriod];
	oldNumOfPeriod = m_NumOfPeriod;
	int count = m_ListBox.GetCount();
	for (int i = count; i >= 0; i--)
		m_ListBox.DeleteString(i);
	for (int m = 0; m < m_NumOfPeriod; ++m) {
		assignment[m] = new Assignment[sizeOfAssignment];
		for (int j = 0; j < sizeOfAssignment; j++) {
			assignment[m][j].m_ArrivalRate = m_ArrivalRate;
			assignment[m][j].m_NumOfPeople = m_NumOfPeople;
			assignment[m][j].m_PercentageDown = m_PercentageDown;
			assignment[m][j].m_PercentageMedium = m_PercentageMedium;
			assignment[m][j].m_PercentageUp = m_PercentageUp;
			int a = 23, b = 13, c = 13, d = 16, e = 16, f = 16, g = 20;
			a -= (int)log10(m + 1);
			b -= (int)log10(j + 1);
			c -= (int)log10(assignment[m][j].m_NumOfPeople);
			CString str;
			string temp = "\t%-" + to_string(a) + "d%-" + to_string(b) + "d%-" + to_string(c) + "d%-" + to_string(d) + ".4f%-" + to_string(e) + ".4f%-" + to_string(f) + ".4f%-" + to_string(g) + ".4f";
			CString tempTemp;
			tempTemp = temp.c_str();
			str.Format(tempTemp, m + 1, j + 1, assignment[m][j].m_NumOfPeople, assignment[m][j].m_ArrivalRate,
				assignment[m][j].m_PercentageUp, assignment[m][j].m_PercentageDown, assignment[m][j].m_PercentageMedium);

			m_ListBox.AddString(str);
		}
	}
	PeriodComb.SetCurSel(0);
	globalData.SetNumOfPeriod(m_NumOfPeriod);
	UpdateData(false);
}

void CPassagerFlow::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

void CPassagerFlow::OnEnChangeEdit8()
{
	UpdateData(TRUE);
	if (m_PercentageUp > 1)
	{
		MessageBox(_T("占比不能大于1"), _T("提示"), 0);
		m_PercentageUp = 0.1;
	}
	UpdateData(FALSE);
}

void CPassagerFlow::OnEnChangeEdit9()
{
	UpdateData(TRUE);
	if (m_PercentageDown > 1)
	{
		MessageBox(_T("占比不能大于1"), _T("提示"), 0);
		m_PercentageDown = 0.15;
	}
	UpdateData(FALSE);
}
