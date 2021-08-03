/*
 * @Author: 宗杰
 * @File: elevator.cpp
 * @Date: 2021-7-16
 * @LastEditTime: 2021-7-26
 * @LastEditors: 宗杰
 */
#include "pch.h"
#include "framework.h"
#include "elevator.h"
#include "elevatorDlg.h"
#include "CPassagerFlow.h"
#include<ctime>
#include <cstdlib>
#include <random>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;
bool IS_STOP_ALL = false;
// CelevatorApp

BEGIN_MESSAGE_MAP(CelevatorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CelevatorApp 构造

CelevatorApp::CelevatorApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的 CelevatorApp 对象

CelevatorApp theApp;

// 全局数据类
GlobalElevator globalData;

// CelevatorApp 初始化

BOOL CelevatorApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager* pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CelevatorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}
Passengers::Passengers(Assignment** assignment, int periodIndex, int floors)
{
	typedef struct MidPerson
	{
		int floorIndex;
		int num;
	}MidPerson;
	vector<MidPerson>tempMidPerson;
	num = 0;
	//int midSum = 0; // 层间客流总量
	// 最底层设为门厅层
	int upFlowPersonNum = 0;
	int downFlowPersonNum = 0;

	for (int j = 1; j < floors; ++j)
	{
		// 打包上行人数
		PeriodPassengerFlow temp;
		temp.person.initialFloor = 0;
		temp.person.distFloor = j;
		temp.person.time = 0;
		temp.floorIndex = j;
		temp.num = (int)assignment[periodIndex][j].m_NumOfPeople * assignment[periodIndex][j].m_PercentageUp;
		// 排除无上行人数的楼层
		if (temp.num != 0)
		{
			periodPerson.upFlow.push_back(temp);
			// 统计上行的总人数
			upFlowPersonNum += temp.num;
			// 统计上行、下行、层间的总人数
			num += temp.num;
		}

		// 打包下行人数
		temp.person.initialFloor = j;
		temp.person.distFloor = 0;
		temp.person.time = 0;
		temp.floorIndex = j;
		temp.num = (int)assignment[periodIndex][j].m_NumOfPeople * assignment[periodIndex][j].m_PercentageDown;
		if (temp.num != 0)
		{
			periodPerson.downFlow.push_back(temp);
			downFlowPersonNum += temp.num;
			num += temp.num;
		}

		// 统计层间人数
		MidPerson tempMid;
		tempMid.floorIndex = j;
		tempMid.num = (int)assignment[periodIndex][j].m_NumOfPeople * assignment[periodIndex][j].m_PercentageMedium;
		if (tempMid.num != 0)
		{
			// midSum += tempMid.num;
			tempMidPerson.push_back(tempMid);
			num += tempMid.num;
		}
	}
	periodPerson.upFlowPersonNum = upFlowPersonNum;
	periodPerson.downFlowPersonNum = downFlowPersonNum;

	// 随机匹配层间客流
	vector<MidPerson>::iterator midIter = tempMidPerson.begin();
	Person tempPerson;
	srand(time(0));

	default_random_engine e(time(0));
	uniform_real_distribution<double> u(0.0, 1.0);
	// 当tempMidPerson.size() == 1时已无可与之搭配的层间客流，舍弃
	while (tempMidPerson.size() > 1)
	{
		int randNum = rand() % (tempMidPerson.size() - 1) + 1;
		if (tempMidPerson.size() < 3)
		{
			randNum = 1;
		}

		// 从 0 号往后随机匹配
		int indexNum = tempMidPerson[0].num;
		for (int i = 0; i < indexNum; ++i)
		{
			if (u(e) > 0.5)
			{
				tempPerson.initialFloor = tempMidPerson[0].floorIndex;
				tempPerson.distFloor = tempMidPerson[randNum].floorIndex;
			}
			else
			{
				tempPerson.initialFloor = tempMidPerson[randNum].floorIndex;
				tempPerson.distFloor = tempMidPerson[0].floorIndex;
			}
			tempPerson.time = 0;

			tempMidPerson[randNum].num--;
			tempMidPerson[0].num--;

			if (tempMidPerson[randNum].num == 0)
			{
				midIter = tempMidPerson.begin();
				midIter += randNum;
				tempMidPerson.erase(midIter);
			}
			if (tempMidPerson[0].num == 0)
			{
				midIter = tempMidPerson.begin();
				//midIter += randNum;
				tempMidPerson.erase(midIter);
			}
			periodPerson.mid.push_back(tempPerson);
		}
	}
}
