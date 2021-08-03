
// elevator.h: PROJECT_NAME 应用程序的主头文件
//

// Warning: 全局数据类GlobalElevator未加锁！

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号
#include "g_a.h"
#include "timer.h"
#include "CPassagerFlow.h"
#include "CelevatorInfo.h"
#include "CeleMoreInfo.h"
// CelevatorApp:
// 有关此类的实现，请参阅 elevator.cpp
//
using namespace std;
class CelevatorApp : public CWinApp
{
public:
	CelevatorApp();

	// 重写
public:
	virtual BOOL InitInstance();

	// 实现

	DECLARE_MESSAGE_MAP()
};
typedef struct PeriodPassengerFlow
{
	Person person;
	int floorIndex; // 层数
	unsigned int num;  // 人数
}PeriodPassengerFlow;
typedef struct PeriodPerson
{
	vector<PeriodPassengerFlow>upFlow; //上行客流
	vector<PeriodPassengerFlow>downFlow; //下行客流
	vector<Person> mid; // 层间客流
	unsigned int downFlowPersonNum;
	unsigned int upFlowPersonNum;
}PeriodPerson;
class Passengers
{
protected:
	PeriodPerson periodPerson;
	int num; // 一个周期客流总数
public:
	// 上行、下行、层间客流匹配
	Passengers(Assignment** assignment, int numOfPeriod, int floors);
	Passengers() {};
	PeriodPerson& GetPeriodPerson()
	{
		return periodPerson;
	}
	int GetPassengersNum()
	{
		return num;
	}
};
class GlobalElevator
{
protected:
	Elevators elevator;
	ElevatorParam* pElevatorParam;
	BuildingParam buildingParam;
	PassengerFLow passagerFlow;
	PassengerFLow periodPassengerFlow;
	Passengers passengers;
	Timer timer;
	Timer* pElevatorTimer;
	Timer consumeTimer;
	Timer infoTimer;
	Timer moreInfoTimer;
	CelevatorInfo* eleInfoDlg;
	CeleMoreInfo* eleMoreInfoDlg;
	int DATA_INTERVAL;
	int numOfPeriod;
public:
	GlobalElevator()
	{
		eleInfoDlg = NULL;
		eleMoreInfoDlg = NULL;
		pElevatorParam = NULL;
	}
	int SetDataInterval(int param)
	{
		DATA_INTERVAL = param;
		return DATA_INTERVAL;
	}
	int GetDataInterval()
	{
		return DATA_INTERVAL;
	}
	PassengerFLow& InitPeriodPassengerFlow()
	{
		for (int i = 0; i < 6; ++i)
		{
			PassengerFlowVolume* temp1 = (PassengerFlowVolume*)P_GET_PASSENGER_FLOW_ELEM(&periodPassengerFlow, i);
			PassengerFlowVolume* temp2 = (PassengerFlowVolume*)P_GET_PASSENGER_FLOW_ELEM(&passagerFlow, i);
			temp1->passengerFlow = new int[FLOW_VOLUME_NUM];
			temp1->nowTimeIndex = FLOW_VOLUME_NUM - 1;
			for (int j = 0; j < FLOW_VOLUME_NUM; ++j)
			{
				temp1->passengerFlow[j] = temp2->passengerFlow[0]; // 以第一个元素
			}
		}
		return periodPassengerFlow;
	}
	// 更新6个周期客流，用于预测
	PassengerFLow& UpdatePeriodPassengerFlow(int periodIndexParam)
	{
		for (int i = 0; i < 6; ++i)
		{
			PassengerFlowVolume* temp1 = (PassengerFlowVolume*)P_GET_PASSENGER_FLOW_ELEM(&periodPassengerFlow, i);
			PassengerFlowVolume* temp2 = (PassengerFlowVolume*)P_GET_PASSENGER_FLOW_ELEM(&passagerFlow, i);
			temp1->nowTimeIndex = FLOW_VOLUME_NUM;
			int period_ = periodIndexParam;
			for (int j = FLOW_VOLUME_NUM - 1; j >= 0; --j)
			{
				temp1->passengerFlow[j] = temp2->passengerFlow[(~(period_ >> 31) & period_)];
				period_--;
			}
		}
		return periodPassengerFlow;
	}
	PassengerFLow& SetPeriodPassengerFlow(PassengerFLow& param)
	{
		periodPassengerFlow = param;
		return periodPassengerFlow;
	}
	PassengerFLow& GetPeriodPassengerFlow()
	{
		return periodPassengerFlow;
	}
	void InitElevators()
	{
		elevator.elevators = new ElevatorRealTimeParam[buildingParam.numOfElevator];
		for (int i = 0; i < buildingParam.numOfElevator; ++i)
		{
			elevator.numElevators = buildingParam.numOfElevator;
			elevator.elevators[i].elevatorParam = pElevatorParam[i];
			elevator.elevators[i].elevatorStatus.dir = STATIC;
			elevator.elevators[i].elevatorStatus.numOfFloors = 0;
			if (i < buildingParam.numOfElevator / 3)
			{
				elevator.elevators[i].elevatorStatus.presentFloor = buildingParam.floors - 1;
			}
			else if (i >= buildingParam.numOfElevator / 3 && i < buildingParam.numOfElevator * 2 / 3)
			{
				elevator.elevators[i].elevatorStatus.presentFloor = buildingParam.floors / 2;
			}
			else
			{
				elevator.elevators[i].elevatorStatus.presentFloor = 0;
			}
		}
	}
	void SetBuildingParam(BuildingParam& param)
	{
		buildingParam = param;
	}
	void SetElevatorParam(ElevatorParam* param)
	{
		pElevatorParam = param;
		elevator.numElevators = buildingParam.numOfElevator;
		elevator.elevators = new ElevatorRealTimeParam[buildingParam.numOfElevator];
		for (int i = 0; i < buildingParam.numOfElevator; ++i)
		{
			elevator.elevators[i].elevatorParam = *param;
		}
	}
	Elevators& GetElevators()
	{
		return elevator;
	}
	ElevatorParam* GetElevatorParam()
	{
		return pElevatorParam;
	}
	BuildingParam& GetBuildingParam()
	{
		return buildingParam;
	}
	void SetPassagerFlow(PassengerFLow& flow)
	{
		passagerFlow = flow;
	}
	PassengerFLow& GetPassagerFlow()
	{
		return passagerFlow;
	}
	Passengers& GetPassengers()
	{
		return passengers;
	}
	Passengers& SetPassengers(Passengers& param)
	{
		passengers = param;
		return passengers;
	}
	int SetNumOfPeriod(int param)
	{
		numOfPeriod = param;
		return numOfPeriod;
	}
	int GetNumOfPeriod()
	{
		return numOfPeriod;
	}
	Timer& GetTimer()
	{
		return timer;
	}
	Timer& GetInfoTimer()
	{
		return infoTimer;
	}
	Timer& GetMoreInfoTimer()
	{
		return moreInfoTimer;
	}
	void InitEleTimer()
	{
		pElevatorTimer = new Timer[buildingParam.numOfElevator];
	}
	Timer& GetElevatorTimer(int index)
	{
		return pElevatorTimer[index];
	}
	Timer& GetConsumeTimer()
	{
		return consumeTimer;
	}
	CelevatorInfo* GetCeleInfoDlg()
	{
		return eleInfoDlg;
	}
	CelevatorInfo* SetCeleInfoDlg(CelevatorInfo* param)
	{
		eleInfoDlg = param;
		return eleInfoDlg;
	}
	CeleMoreInfo* GetCeleMoreInfoDlg()
	{
		return eleMoreInfoDlg;
	}
	CeleMoreInfo* SetCeleMoreInfoDlg(CeleMoreInfo* param)
	{
		eleMoreInfoDlg = param;
		return eleMoreInfoDlg;
	}
	void Reset()
	{
		for (int i = 0; i < buildingParam.numOfElevator; ++i)
		{
			pElevatorTimer[i].Expire();
		}
		timer.Expire();
		consumeTimer.Expire();
		//std::this_thread::sleep_for(std::chrono::seconds(2));
		for (int i = 0; i < buildingParam.numOfElevator; ++i)
		{
			elevator.elevators[i].elevatorStatus.distFloors.clear();
			elevator.elevators[i].elevatorStatus.person.clear();
		}
		delete[]elevator.elevators;
		elevator.elevators = NULL;
		delete eleInfoDlg;
		eleInfoDlg = NULL;
	}
	~GlobalElevator()
	{
		delete[] pElevatorTimer;
		delete pElevatorParam;
		delete[] elevator.elevators;
		delete eleInfoDlg;
	}
};//全局变量
// 客流生成类

extern CelevatorApp theApp;
