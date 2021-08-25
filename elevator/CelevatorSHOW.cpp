/*
 * @Author: 朱文裕、郑飞、罗飞杰
 * @File: CelevatorsSHOW.cpp
 * @Date: 2021-7-17
 * @LastEditTime: 2021-8-25
 * @LastEditors: 罗飞杰
 * @brief: 电梯动态运行画面绘制及控制
 */

 // Warning: 下面这段程序存在尚未修复的BUG！

#include <random>
#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <thread>
#include <chrono>
#include <assert.h>
#include "pch.h"
#include "elevator.h"
#include "CelevatorSHOW.h"
#include "afxdialogex.h"
#include "CPassagerFlow.h"
#include "g_a.h"
#include "timer.h"

 // CelevatorSHOW 对话框

 // 全局数据
extern GlobalElevator globalData;
extern Assignment** assignment;
extern bool IS_STOP_ALL;
vector<Person>response; // 用于回应后台数据，外部新信号，未安排
vector<Person>totalForResponse; // 外部人员等待

volatile bool g_stop_elevators = false;
extern volatile bool gaIsFinish;
// 待后台算法分配相应电梯的信号
Signal _signal;
// 未在上梯时消除的信号需要重新分配
Signal leftSigal;
// 防止数据读写发生冲突
ElevatorCopy _g_a_elevators;
mutex g_cplock;

// 电梯位置
vector<float>elePos;
vector<int>lastPos;
vector<EleRunParam>eleRunParam;

// 灯
vector<int>UpGreenLight;
vector<int>DownGreenLight;

IMPLEMENT_DYNAMIC(CelevatorSHOW, CDialogEx)

CelevatorSHOW::CelevatorSHOW(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ELEVATOR_SHOW, pParent)
{
}

CelevatorSHOW::~CelevatorSHOW()
{
}

void CelevatorSHOW::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CelevatorSHOW, CDialogEx)
	ON_WM_PAINT()
	//	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// 灯光生成
void LightDataUpdate()
{
	UpGreenLight.clear();
	DownGreenLight.clear();
	for (int i = 0; i < totalForResponse.size(); ++i)
	{
		if (totalForResponse[i].distFloor > totalForResponse[i].initialFloor)
		{
			UpGreenLight.push_back(totalForResponse[i].initialFloor);
		}
		else
		{
			DownGreenLight.push_back(totalForResponse[i].initialFloor);
		}
	}

	sort(UpGreenLight.begin(), UpGreenLight.end());
	sort(DownGreenLight.begin(), DownGreenLight.end());
	auto end_unique = unique(UpGreenLight.begin(), UpGreenLight.end());
	UpGreenLight.erase(end_unique, UpGreenLight.end());
	end_unique = unique(DownGreenLight.begin(), DownGreenLight.end());
	DownGreenLight.erase(end_unique, DownGreenLight.end());
}

// 电梯位置计算
float Location(ElevatorParam& E_Param, int pfloor, int dfloor, float h, float t)
{
	float s1 = 2.0 * pow(E_Param.ratedAcceleration, 3) / (E_Param.ratedJerk * E_Param.ratedJerk);
	float s2 = (E_Param.ratedAcceleration * E_Param.ratedSpeed) / E_Param.ratedJerk + pow(E_Param.ratedSpeed, 2) / E_Param.ratedAcceleration;
	float s = abs(pfloor - dfloor) * h;
	if (s < s1)
	{
		float t0 = pow(s / (2.0 * E_Param.ratedJerk), 1 / 3);
		if (t <= t0) return 1 / 6.0 * E_Param.ratedJerk * t * t * t;
		else if (t > t0 && t <= 2.0 * t0)
			return E_Param.ratedJerk * t0 * t0 * t + 1 / 6.0 * E_Param.ratedJerk * pow((2.0 * t0 - t), 3) - E_Param.ratedJerk * pow(t0, 3);
		else if (t > 2.0 * t0 && t <= 3.0 * t0)
			return E_Param.ratedJerk * t0 * t0 * t + 1 / 6.0 * E_Param.ratedJerk * pow((2.0 * t0 - t), 3) - E_Param.ratedJerk * pow(t0, 3);
		else return -1 / 6.0 * E_Param.ratedJerk * pow((4.0 * t0 - t), 3) + 2.0 * E_Param.ratedJerk * pow(t0, 3);
	}
	if (s >= s1 && s < s2)
	{
		float t1 = E_Param.ratedAcceleration / E_Param.ratedJerk + sqrt(pow(E_Param.ratedAcceleration, 2) / pow(E_Param.ratedJerk, 2) + 4.0 * s / E_Param.ratedAcceleration);
		float t2 = 1 / 2.0 * t1 - E_Param.ratedAcceleration / E_Param.ratedJerk;
		float t3 = t2 + E_Param.ratedAcceleration / E_Param.ratedJerk;
		float t4 = t2 + 2.0 * E_Param.ratedAcceleration / E_Param.ratedJerk;
		float t5 = 2.0 * t2 + E_Param.ratedAcceleration / E_Param.ratedJerk;
		if (t <= E_Param.ratedAcceleration / E_Param.ratedJerk)
			return 1 / 6.0 * E_Param.ratedJerk * t * t * t;
		if (t > E_Param.ratedAcceleration / E_Param.ratedJerk && t <= t2)
			return 1 / 2.0 * E_Param.ratedAcceleration * t * t - (pow(E_Param.ratedAcceleration, 2)) / (2.0 * E_Param.ratedJerk) * t + pow(E_Param.ratedAcceleration, 3) / (2.0 * pow(E_Param.ratedJerk, 2));
		if (t > t2 && t <= t3)
			return E_Param.ratedAcceleration * t2 * t + 1 / 6.0 * E_Param.ratedJerk * pow((t3 - t), 3) - 1 / 2.0 * E_Param.ratedAcceleration * t2 * t2 - pow(E_Param.ratedAcceleration, 2) / (2.0 * E_Param.ratedJerk) * t2;
		if (t > t3 && t <= t4)
			return E_Param.ratedAcceleration * t2 * t + 1 / 6.0 * E_Param.ratedJerk * pow((t3 - t), 3) - 1 / 2.0 * E_Param.ratedAcceleration * t2 * t2 - pow(E_Param.ratedAcceleration, 2) / (2.0 * E_Param.ratedJerk) * t2;
		if (t > t4 && t <= t5)
			return (2.0 * E_Param.ratedAcceleration * t2 + (3.0 * pow(E_Param.ratedAcceleration, 2)) / (2.0 * E_Param.ratedJerk)) * t - 1 / 2.0 * E_Param.ratedAcceleration * t * t - E_Param.ratedAcceleration * t2 * t2 - (2.0 * pow(E_Param.ratedAcceleration, 2)) / E_Param.ratedJerk * t2 - 7.0 / 6.0 * pow(E_Param.ratedAcceleration, 3) / pow(E_Param.ratedJerk, 2);
		else
			return E_Param.ratedAcceleration * t2 * t2 + (pow(E_Param.ratedAcceleration, 2)) / (E_Param.ratedJerk) - 1 / 6.0 * E_Param.ratedJerk * pow((t1 - t), 3);
	}
	if (s >= s2)
	{
		float a1 = E_Param.ratedAcceleration / E_Param.ratedJerk;
		float a2 = E_Param.ratedSpeed / E_Param.ratedAcceleration;
		float a3 = s / E_Param.ratedSpeed;
		if (t <= a1)
			return 1 / 6.0 * E_Param.ratedJerk * t * t * t;
		if (t > a1 && t <= a2)
			return 1 / 2.0 * E_Param.ratedAcceleration * t * t - (pow(E_Param.ratedAcceleration, 2)) / (2.0 * E_Param.ratedJerk) * t + pow(E_Param.ratedAcceleration, 3) / (6.0 * pow(E_Param.ratedJerk, 2));
		if (t > a2 && t <= a1 + a2)
			return E_Param.ratedSpeed * t + 1 / 6.0 * E_Param.ratedJerk * pow((a1 + a2 - t), 3) - a2 * E_Param.ratedSpeed / 2.0 - a1 * E_Param.ratedSpeed / 2;
		if (t > a1 + a2 && t <= a3)
			return E_Param.ratedSpeed * t - a1 * E_Param.ratedSpeed / 2.0 - a2 * E_Param.ratedSpeed / 2.0;
		if (t > a3 && t <= a3 + a1)
			return E_Param.ratedSpeed * t + 1 / 6.0 * E_Param.ratedJerk * pow((a3 - t), 3) - a2 * E_Param.ratedSpeed / 2.0 - a1 * E_Param.ratedSpeed / 2.0;
		if (t > a3 + a1 && t < a3 + a2)
			return (E_Param.ratedSpeed + s / a2 + a1 * E_Param.ratedAcceleration / 2) * t - 1 / 2.0 * E_Param.ratedAcceleration * t * t - 1 / 6 * a1 * a1 * E_Param.ratedAcceleration - a1 * E_Param.ratedSpeed / 2.0 - a2 * E_Param.ratedSpeed / 2.0 - a3 * a3 * E_Param.ratedAcceleration / 2.0 - a1 * a3 * E_Param.ratedAcceleration / 2.0;
		else
			return s - 1 / 6.0 * E_Param.ratedJerk * pow((a1 + a2 + a3 - t), 3);
	}
}

int GetDownFirstPartIndex(const ElevatorStatus& param_)
{
	int presentFloor = param_.presentFloor;
	// 寻找分割点, 第一部分分割点
	int intialFirstPoint = 0;
	for (int i = 0; i < param_.distFloors.size(); ++i)
	{
		if (param_.distFloors[i].type == INTERN ||
			(param_.distFloors[i].dist < presentFloor && param_.distFloors[i].type == EXTERN_DOWN))
		{
			intialFirstPoint = i;
		}
	}
	return intialFirstPoint;
}
mutex mtxP;
// 电梯id
void ProcessPerson(int id)
{
	mtxP.lock();

	Direction dir = globalData.GetElevators().elevators[id].elevatorStatus.dir;
	int sfloor = globalData.GetElevators().elevators[id].elevatorStatus.presentFloor;

	// 以下i的初始条件不可改为0
	// 内部乘客下梯
	vector<Person>::iterator pIter;
	for (int i = globalData.GetElevators().elevators[id].elevatorStatus.person.size() - 1; i >= 0; --i)
	{
		if (globalData.GetElevators().elevators[id].elevatorStatus.person[i].distFloor == sfloor)
		{
			pIter = globalData.GetElevators().elevators[id].elevatorStatus.person.begin() + i;
			globalData.GetElevators().elevators[id].elevatorStatus.person.erase(pIter);
		}
	}
	// 抹除内部信号
	vector<DistFloor>::iterator fIter;
	for (int i = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() - 1; i >= 0; --i)
	{
		if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[i].type == INTERN
			&& globalData.GetElevators().elevators[id].elevatorStatus.distFloors[i].dist == sfloor)
		{
			fIter = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin() + i;
			globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(fIter);
		}
	}

	// 外部乘客上梯
	if (globalData.GetElevators().elevators[id].elevatorStatus.person.size() < globalData.GetElevatorParam()[id].ratedCapacity)
	{
		switch (dir)
		{
		case UP:
		{
			for (int i = totalForResponse.size() - 1; i >= 0; --i)
			{
				if (globalData.GetElevators().elevators[id].elevatorStatus.person.size() < globalData.GetElevatorParam()[id].ratedCapacity)
				{
					if (totalForResponse[i].initialFloor == sfloor && (totalForResponse[i].initialFloor < totalForResponse[i].distFloor) &&
						totalForResponse[i].distFloor > globalData.GetElevators().elevators[id].elevatorStatus.presentFloor)
					{
						globalData.GetElevators().elevators[id].elevatorStatus.person.push_back(totalForResponse[i]);
						// 插入内部信号
						int* tempS = new int;
						tempS[0] = id;
						ExternSignal tempEx;
						tempEx.dir = UP;
						tempEx.floors = totalForResponse[i].distFloor;
						tempEx.id = 0;
						Signal tempSinal;
						tempSinal.signalNum = 1;
						tempSinal.externSignal.push_back(tempEx);
						InsertExternSignal(&globalData.GetElevators(), tempS, 0, &tempSinal, true);

						// 抹除该外部信号
						vector<Person>::iterator pIter;
						pIter = totalForResponse.begin() + i;
						totalForResponse.erase(pIter);
					}
				}
			} // for (int i = totalForResponse.size() - 1; i >= 0; --i)
			LightDataUpdate();
			break;
		}
		case DOWN:
		{
			for (int i = totalForResponse.size() - 1; i >= 0; --i)
			{
				if (globalData.GetElevators().elevators[id].elevatorStatus.person.size() < globalData.GetElevatorParam()[id].ratedCapacity)
				{
					if (totalForResponse[i].initialFloor == sfloor && (totalForResponse[i].initialFloor > totalForResponse[i].distFloor) &&
						totalForResponse[i].distFloor < globalData.GetElevators().elevators[id].elevatorStatus.presentFloor)
					{
						globalData.GetElevators().elevators[id].elevatorStatus.person.push_back(totalForResponse[i]);
						// 插入内部信号
						int* tempS = new int;
						tempS[0] = id;
						ExternSignal tempEx;
						tempEx.dir = DOWN;
						tempEx.floors = totalForResponse[i].distFloor;
						tempEx.id = 0;
						Signal tempSinal;
						tempSinal.signalNum = 1;
						tempSinal.externSignal.push_back(tempEx);
						InsertExternSignal(&globalData.GetElevators(), tempS, 0, &tempSinal, true);

						// 抹除该外部信号
						vector<Person>::iterator pIter;
						pIter = totalForResponse.begin() + i;
						totalForResponse.erase(pIter);
					}
				}
			} // for (int i = 0; i < totalForResponse.size(); ++i)
			LightDataUpdate();
			break;
		}
		case STATIC:
		{
			// 全部下梯
			vector<Person>::iterator pIter;
			for (int i = globalData.GetElevators().elevators[id].elevatorStatus.person.size() - 1; i >= 0; --i)
			{
				pIter = globalData.GetElevators().elevators[id].elevatorStatus.person.begin() + i;
				globalData.GetElevators().elevators[id].elevatorStatus.person.erase(pIter);
			}
			break;
		}
		default:
			break;
		}
	}
	mtxP.unlock();
}

// 电梯线程
void ELevatorControl(int id)
{
	if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() == 0)
	{
		globalData.GetElevators().elevators[id].elevatorStatus.dir == STATIC;
		return;
	}
	// 信号处理
	switch (globalData.GetElevators().elevators[id].elevatorStatus.dir)
	{
	case UP:
	{
		if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].type != INTERN)
		{
			int f = globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].dist;
			bool t = true;
			for (int i = 0; i < totalForResponse.size(); ++i)
			{
				if (totalForResponse[i].initialFloor == f)
				{
					t = false;
					break;
				}
			}
			if (t)
			{
				globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin());
				if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() != 0)
				{
					if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].dist < globalData.GetElevators().elevators[id].elevatorStatus.presentFloor)
					{
						globalData.GetElevators().elevators[id].elevatorStatus.dir = DOWN;
					}
				}
				else
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = STATIC;
					break;
				}
			}
		}

		break;
	}
	case DOWN:
	{
		int firstIndex = GetDownFirstPartIndex(globalData.GetElevators().elevators[id].elevatorStatus);
		if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[firstIndex].type != INTERN)
		{
			int f = globalData.GetElevators().elevators[id].elevatorStatus.distFloors[firstIndex].dist;
			bool t = true;
			for (int i = 0; i < totalForResponse.size(); ++i)
			{
				if (totalForResponse[i].initialFloor == f)
				{
					t = false;
					break;
				}
			}
			if (t)
			{
				globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin() + firstIndex);
				firstIndex = GetDownFirstPartIndex(globalData.GetElevators().elevators[id].elevatorStatus);
				if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() != 0)
				{
					if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[firstIndex].dist > globalData.GetElevators().elevators[id].elevatorStatus.presentFloor)
					{
						globalData.GetElevators().elevators[id].elevatorStatus.dir = UP;
					}
				}
				else
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = STATIC;
					break;
				}
			}
		}

		break;
	}
	default:
		break;
	}

	if (!eleRunParam[id].isRestart)
	{
		eleRunParam[id].t += 0.5;
		switch (globalData.GetElevators().elevators[id].elevatorStatus.dir)
		{
		case UP:
		{
			elePos[id] = lastPos[id] +
				Location(*globalData.GetElevatorParam(), lastPos[id],
					globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].dist, 1, eleRunParam[id].t);
			globalData.GetElevators().elevators[id].elevatorStatus.presentFloor = round(elePos[id]);

			// 运行到或略超目标站点
			int dist = globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].dist;
			if (elePos[id] >= dist)
			{
				// 判断是否需要在接下来改变电梯方向
				DistFloor& tempDistfloor = globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0];
				if (tempDistfloor.type == EXTERN_DOWN ||
					(tempDistfloor.type == EXTERN_UP && tempDistfloor.dist < globalData.GetElevators().elevators[id].elevatorStatus.presentFloor))
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = DOWN;
					elePos[id] = globalData.GetElevators().elevators[id].elevatorStatus.presentFloor;
					lastPos[id] = globalData.GetElevators().elevators[id].elevatorStatus.presentFloor;
					ELevatorChange(&globalData.GetElevators().elevators[id]);
					// 内外部乘客更新
					ProcessPerson(id);

					// 抹除一个distFloor
					vector<DistFloor>::iterator pIter;
					int num = 0;
					for (int i = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() - 1; i >= 0; --i)
					{
						if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[i].dist == dist)
						{
							pIter = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin() + i;
							globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(pIter);
							num++;
						}
					}
					globalData.GetElevators().elevators[id].elevatorStatus.numOfFloors -= num;
					eleRunParam[id].t = 0;
					eleRunParam[id].isRestart = true;
					break;
				}

				elePos[id] = dist;
				assert(ABS(globalData.GetElevators().elevators[id].elevatorStatus.presentFloor - dist) < 2);
				lastPos[id] = dist;
				// 内外部乘客更新
				ProcessPerson(id);
				// 抹除一个distFloor
				vector<DistFloor>::iterator pIter;
				int num = 0;
				for (int i = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() - 1; i >= 0; --i)
				{
					if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[i].dist == dist)
					{
						pIter = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin() + i;
						globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(pIter);
						num++;
					}
				}
				globalData.GetElevators().elevators[id].elevatorStatus.numOfFloors -= num;

				// 检测0号元素是否为EXTERN_DOWN
				if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() == 0)
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = STATIC;
				}

				eleRunParam[id].t = 0;
				eleRunParam[id].isRestart = true;
			}
			break;
		}
		case DOWN:
		{
			int index = GetDownFirstPartIndex(globalData.GetElevators().elevators[id].elevatorStatus);
			elePos[id] = lastPos[id] -
				Location(*globalData.GetElevatorParam(), lastPos[id],
					globalData.GetElevators().elevators[id].elevatorStatus.distFloors[index].dist, 1, eleRunParam[id].t);
			globalData.GetElevators().elevators[id].elevatorStatus.presentFloor = round(elePos[id]);

			int dist = globalData.GetElevators().elevators[id].elevatorStatus.distFloors[index].dist;
			if (elePos[id] <= dist)
			{
				// 判断是否需要改变电梯运行方向
				DistFloor& tempDistfloor = globalData.GetElevators().elevators[id].elevatorStatus.distFloors[index];
				if (tempDistfloor.type == EXTERN_UP ||
					(tempDistfloor.type == EXTERN_DOWN && tempDistfloor.dist > globalData.GetElevators().elevators[id].elevatorStatus.presentFloor))
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = UP;
					elePos[id] = globalData.GetElevators().elevators[id].elevatorStatus.presentFloor;
					lastPos[id] = globalData.GetElevators().elevators[id].elevatorStatus.presentFloor;
					// 内外部乘客更新
					ProcessPerson(id);
					// 抹除一个distFloor
					vector<DistFloor>::iterator pIter;
					int num = 0;
					for (int i = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() - 1; i >= 0; --i)
					{
						if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[i].dist == dist)
						{
							pIter = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin() + i;
							globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(pIter);
							num++;
						}
					}
					globalData.GetElevators().elevators[id].elevatorStatus.numOfFloors -= num;
					eleRunParam[id].t = 0;
					eleRunParam[id].isRestart = true;
					break;
				}
				elePos[id] = dist;
				lastPos[id] = dist;
				assert(ABS(globalData.GetElevators().elevators[id].elevatorStatus.presentFloor - dist) < 2);
				// 内外部乘客更新
				ProcessPerson(id);

				// 抹除一个distFloor
				vector<DistFloor>::iterator pIter;
				int num = 0;
				for (int i = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() - 1; i >= 0; --i)
				{
					if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors[i].dist == dist)
					{
						pIter = globalData.GetElevators().elevators[id].elevatorStatus.distFloors.begin() + i;
						globalData.GetElevators().elevators[id].elevatorStatus.distFloors.erase(pIter);
						num++;
					}
				}
				globalData.GetElevators().elevators[id].elevatorStatus.numOfFloors -= num;

				// 检测是否需要静止
				if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() == 0)
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = STATIC;
				}

				eleRunParam[id].t = 0;
				eleRunParam[id].isRestart = true;
			}
			break;
		}
		case STATIC:
		{
			if (globalData.GetElevators().elevators[id].elevatorStatus.distFloors.size() != 0)
			{
				if (globalData.GetElevators().elevators[id].elevatorStatus.presentFloor < globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].dist)
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = UP;
				}
				else if (globalData.GetElevators().elevators[id].elevatorStatus.presentFloor > globalData.GetElevators().elevators[id].elevatorStatus.distFloors[0].dist)
				{
					globalData.GetElevators().elevators[id].elevatorStatus.dir = DOWN;
				}
			}
			break;
		}
		default:
			break;
		}
	}
	// 电梯处于暂停状态
	else
	{
		eleRunParam[id].restartTime += 0.5;
		if (eleRunParam[id].restartTime == eleRunParam[id].stopTime)
		{
			eleRunParam[id].isRestart = false;
			eleRunParam[id].restartTime = 0;
			eleRunParam[id].t = 0;
		}
	}
	// 更新
	//_g_a_elevators.ChangeOrGet(globalData.GetElevators(), true);
}

// 乘客等待时间增长
void PassengerTimeCal()
{
	// 外部乘客时间增长
	int size_ = totalForResponse.size();
	for (int i = 0; i < size_; ++i)
	{
		totalForResponse[i].time += 2;
	}
	// 电梯内乘客时间增长
	for (int i = 0; i < globalData.GetBuildingParam().numOfElevator; ++i)
	{
		int tempSize = globalData.GetElevators().elevators[i].elevatorStatus.person.size();
		for (int j = 0; j < tempSize; ++j)
		{
			globalData.GetElevators().elevators[i].elevatorStatus.person[j].time += 2;
		}
		int tempSize2 = globalData.GetElevators().elevators[i].elevatorStatus.distFloors.size();
		for (int n = 0; n < tempSize2; ++n)
		{
			if (globalData.GetElevators().elevators[i].elevatorStatus.distFloors[n].type == EXTERN_DOWN)
			{
				globalData.GetElevators().elevators[i].elevatorStatus.distFloors[n].waitTimeOfDOWN += 2;
			}
			else if (globalData.GetElevators().elevators[i].elevatorStatus.distFloors[n].type == EXTERN_UP)
			{
				globalData.GetElevators().elevators[i].elevatorStatus.distFloors[n].waitTimeOfUP += 2;
			}
		}
	}
}

int periodIndex = 0;
// 周期客流生成线程
void  GeneratePerson()
{
	// 信号初始化
	_signal.signalNum = 0;
	// 获取所处周期的上行、下行、层间乘客分配客流
	PeriodPerson& periodPassengerFlow = globalData.GetPassengers().GetPeriodPerson();

	// 一个周期客流
	int upFlowPersonNum = periodPassengerFlow.upFlowPersonNum;
	int downFlowPersonNum = periodPassengerFlow.downFlowPersonNum;
	int midFlowPersonNum = periodPassengerFlow.mid.size();
	int sum = downFlowPersonNum + upFlowPersonNum + midFlowPersonNum;

	// 解开上行、下行、层间乘客分配包装
	vector<Person>person;

	int tempOne = periodPassengerFlow.upFlow.size();
	for (int j = 0; j < tempOne; ++j)
	{
		int tempNum = periodPassengerFlow.upFlow[j].num;
		for (int i = 0; i < tempNum; ++i)
		{
			person.push_back(periodPassengerFlow.upFlow[j].person);
		}
	}

	tempOne = periodPassengerFlow.downFlow.size();
	for (int j = 0; j < tempOne; ++j)
	{
		int tempNum = periodPassengerFlow.downFlow[j].num;
		for (int i = 0; i < tempNum; ++i)
		{
			person.push_back(periodPassengerFlow.downFlow[j].person);
		}
	}

	tempOne = periodPassengerFlow.mid.size();
	for (int j = 0; j < tempOne; ++j)
	{
		person.push_back(periodPassengerFlow.mid[j]);
	}

	const int AVERAGER_ONE = 6;
	int tempSize1 = globalData.GetPassengers().GetPassengersNum();
	// 一个周期划分若干个小周期，用于产生外部信号响应
	tempSize1 = (float)tempSize1 / (globalData.GetDataInterval() * 60 / AVERAGER_ONE); // 一个小周期生成的人数

	default_random_engine e(time(0));
	Const constN(sum);

	// 生成，用于打乱Person索引以达到随机在某个小周期分配客流的目的
	int* randOne = new int[constN.constNum];
	for (int i = 0; i < constN.constNum; ++i)
	{
		randOne[i] = i;
	}
	// 左区间边界闭合，右开
	shuffle(randOne, randOne + constN.constNum, default_random_engine(random_device()())); // 打乱数组

	// 左右区间边界均闭合
	std::uniform_int_distribution<int> dis2(AVERAGER_ONE - 3, AVERAGER_ONE + 3); // 平均为 AVERAGER_ONE,单位为s
	unsigned long t1 = ::GetTickCount64();
	// 保证数组RandOne不越界
	int indexOfRandone = 0;

	// 后台算法：交通模式的识别，一个周期内交通模式不变
	PassengerFlowPredict temp;
	temp = *FlowPredict(&globalData.GetPeriodPassengerFlow(), &temp);
	PatternJudge(&globalData.GetPeriodPassengerFlow(), &temp, globalData.GetElevatorParam(), &globalData.GetBuildingParam());

	while (indexOfRandone != constN.constNum)
	{
		_signal.externSignal.clear();
		_signal.signalNum = 0;
		response.clear();
		unsigned int t2 = ::GetTickCount64();
		if ((t2 - t1) >= MINUTE_TO_MS(globalData.GetDataInterval()))
		{
			periodIndex++;
			person.~vector();
			// 结束该线程
			_endthreadex(0);
		}

		for (int i = 0; i < tempSize1; ++i)
		{
			if (indexOfRandone != constN.constNum)
			{
				ExternSignal tempSignal;
				tempSignal.id = _signal.externSignal.size();
				tempSignal.floors = person[randOne[indexOfRandone]].initialFloor;
				if (person[randOne[indexOfRandone]].initialFloor > person[randOne[indexOfRandone]].distFloor)
				{
					tempSignal.dir = DOWN;
				}
				else
				{
					tempSignal.dir = UP;
				}

				int m = 0;
				for (int j = 0; j < _signal.externSignal.size(); ++j)
				{
					if (tempSignal.floors == _signal.externSignal[j].floors && tempSignal.dir == _signal.externSignal[j].dir)
					{
						m++;
					}
				}
				if (m == 0)
				{
					_signal.externSignal.push_back(tempSignal);
					_signal.signalNum++;
				}
				response.push_back(person[randOne[indexOfRandone]]);
				totalForResponse.push_back(person[randOne[indexOfRandone++]]);
			}
			else
			{
				break;
			}
		}
		// 外部信号显示，根据response信息加入，response信息每个小周期更新一次(先抹去先前的数据)
		LightDataUpdate();

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		// 加入剩下的信号
		for (int i = 0; i < leftSigal.externSignal.size(); ++i)
		{
			_signal.externSignal.push_back(leftSigal.externSignal[i]);
			_signal.signalNum++;
		}

		// 人员加入电梯响应信号集
		Timer t;
		if (_signal.signalNum != 0)
		{
			gaIsFinish = false;
			t.SyncWait(100, std::bind(GeneticAlgorithm, &globalData.GetElevators(), &globalData.GetPeriodPassengerFlow(), &globalData.GetBuildingParam(), &_signal));
		}
		Const constNumber(dis2(e));
		std::this_thread::sleep_for(std::chrono::seconds(constNumber.constNum));
		// 等待后台遗传算法计算完毕进行下一个小区间计算
		while (!gaIsFinish)
		{
		}
	}
	periodIndex++;
}

// 周期客流生成，一个周期执行一次
void GeneratePasengerPeriodFlow()
{
	// 所处周期的客流 打包上行、下行、层间乘客分配
	Passengers passenger(assignment, periodIndex, globalData.GetBuildingParam().floors);
	globalData.SetPassengers(passenger);

	// 6周期客流数据生成，用于位于GneratePerson函数中的PatternJudge预测、判定交通模式
	globalData.UpdatePeriodPassengerFlow(periodIndex);
	// 同步，解包上行、下行、层间乘客分配，运行时间为一个周期
	GeneratePerson();
}
void PasengerPeriodFlowThread()
{
	// 开启乘客等待时间增长,2秒级增长
	globalData.GetConsumeTimer().StartTimer(2000, std::bind(PassengerTimeCal));
	Timer timer;
	GeneratePasengerPeriodFlow();
	// 周期执行
	timer.StartTimer(MINUTE_TO_MS(globalData.GetDataInterval()), std::bind(GeneratePasengerPeriodFlow));
	std::this_thread::sleep_for(std::chrono::seconds(globalData.GetDataInterval() * (globalData.GetNumOfPeriod() - 1) * 60));
	// 终结生命期
	timer.Expire();
	globalData.GetTimer().Expire();
}

// CelevatorSHOW 消息处理程序

BOOL CelevatorSHOW::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmpBackground;
	bmpBackground.LoadBitmap(IDC_STATIC);
	BITMAP bitmap;
	bmpBackground.GetBitmap(&bitmap);
	CBitmap* pbmpOld = dcMem.SelectObject(&bmpBackground);
	dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	// 初始化电梯
	globalData.InitElevators();

	// 周期客流初始化
	globalData.InitPeriodPassengerFlow();

	// 客流
	// 异步，立刻执行
	globalData.GetTimer().AsyncWait(100, PasengerPeriodFlowThread);

	// 电梯线程初始化: 电梯的运行
	EleRunParam tempEle;
	tempEle.isRestart = false;
	tempEle.t = 0;
	tempEle.restartTime = 0;
	globalData.InitEleTimer();
	//_g_a_elevators.ChangeOrGet(globalData.GetElevators(), true);
	for (int i = 0; i < globalData.GetBuildingParam().numOfElevator; ++i)
	{
		tempEle.stopTime = globalData.GetElevatorParam()[i].timeOfBrake + globalData.GetElevatorParam()[i].timeOfCloseDoor +
			globalData.GetElevatorParam()[i].timeOfOpenDoor + globalData.GetElevatorParam()[i].timeOfWait;
		elePos.push_back((double)globalData.GetElevators().elevators[i].elevatorStatus.presentFloor);
		lastPos.push_back(globalData.GetElevators().elevators[i].elevatorStatus.presentFloor);
		// 每0.5秒更新一次电梯位置
		globalData.GetElevatorTimer(i).StartTimer(500, std::bind(ELevatorControl, i));
		eleRunParam.push_back(tempEle);
	}
	return TRUE;
}

void CelevatorSHOW::OnPaint()
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
	}
	else
	{
		COLORREF bkg = RGB(240, 240, 240);

		//CDialog::OnPaint();
		//楼最底层
		BuildingParam& buildingParam = globalData.GetBuildingParam();
		CClientDC dc(this);
		CRect rect;
		GetClientRect(&rect);
		buildingParam.floors = globalData.GetBuildingParam().floors;
		buildingParam.numOfElevator = globalData.GetBuildingParam().numOfElevator;
		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP7);
		BITMAP bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld1 = dcMem.SelectObject(&bmpBackground);
		int times = buildingParam.floors / 10 + 1;
		if (buildingParam.floors < 10 && buildingParam.numOfElevator>10)times = 2;
		for (int i = 1; i <= buildingParam.numOfElevator; i++)
		{
			dc.StretchBlt(rect.Width() / (buildingParam.numOfElevator + 1) * i + 50 / times, rect.Height() - 90 / times, 160 / times, 90 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		}
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld1);
		//楼体
		bmpBackground.LoadBitmap(IDB_BITMAP5);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld2 = dcMem.SelectObject(&bmpBackground);
		for (int j = 1; j < buildingParam.numOfElevator + 1; j++)
		{
			for (int m = 2; m <= buildingParam.floors; m++)
			{
				dc.StretchBlt(rect.Width() / (buildingParam.numOfElevator + 1) * j + 50 / times, rect.Height() - 90 / times * m, 100 / times, 90 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
			}
		}
		DeleteObject(pbmpOld2);
		//楼顶
		bmpBackground.LoadBitmap(IDB_BITMAP6);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld3 = dcMem.SelectObject(&bmpBackground);
		for (int k = 1; k <= buildingParam.numOfElevator; k++)
		{
			dc.StretchBlt(rect.Width() / (buildingParam.numOfElevator + 1) * k + 50 / times, rect.Height() - 90 / times * buildingParam.floors - 40 / times, 160 / times, 40 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		}
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld3);

		//电梯安置
		bmpBackground.LoadBitmap(IDB_BITMAP8);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld4 = dcMem.SelectObject(&bmpBackground);
		for (int num = 1; num <= buildingParam.numOfElevator; num++)
		{
			float shishilouceng = elePos[num - 1] + 1;//替换为下一行

			// 将原来的像素覆盖
			CPen rePen(PS_SOLID, 40 / times, RGB(240, 240, 240));
			CPen* rePrt = dc.SelectObject(&rePen);
			dc.MoveTo(rect.Width() / (buildingParam.numOfElevator + 1) * num + 170 / times, rect.Height() - 90 / times * 1 - 6);
			dc.LineTo(rect.Width() / (buildingParam.numOfElevator + 1) * num + 170 / times, rect.Height() - 90 / times * buildingParam.floors);
			dc.SelectObject(rePrt);

			dc.StretchBlt(rect.Width() / (buildingParam.numOfElevator + 1) * num + 150 / times, rect.Height() - 90 / times * shishilouceng, 40 / times, 65 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

			CPen pen(PS_SOLID, 5, RGB(255, 0, 0));
			CPen* ptr = dc.SelectObject(&pen);
			dc.MoveTo(rect.Width() / (buildingParam.numOfElevator + 1) * num + 170 / times, rect.Height() - 90 / times * shishilouceng);
			dc.LineTo(rect.Width() / (buildingParam.numOfElevator + 1) * num + 170 / times, rect.Height() - 90 / times * buildingParam.floors);
			dc.SelectObject(ptr);
		}
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld4);

		CFont font;
		font.CreatePointFont(160, _T("黑体"));
		CFont* def_font = dc.SelectObject(&font);
		dc.SetBkColor(bkg);
		dc.TextOut(50 / times, rect.Height() - 90 / times * (buildingParam.floors + 1) - 60, _T("楼层"));//大字电梯楼层

		//楼层数
		dc.SelectObject(def_font);
		CString str;
		for (int i = 1; i < buildingParam.floors + 1; i++)
		{
			str.Format(_T("%d"), i);
			CFont font;
			font.CreatePointFont(120, _T("黑体"));
			CFont* def_font = dc.SelectObject(&font);
			dc.TextOut(50 / times, rect.Height() - 90 / times * i, str);
			DeleteObject(font);
			DeleteObject(def_font);
		}

		for (int i = 1; i <= buildingParam.numOfElevator; i++)
		{
			int diantiyunxinggaodu = globalData.GetElevators().elevators[i - 1].elevatorStatus.presentFloor + 1;//此变量到时候替换为每一栋大楼目前电梯运行到的楼层数,电梯编号为第i号到buildingParam
			// 将原来的像素覆盖
			CPen rePen(PS_SOLID, 70, RGB(240, 240, 240));
			CPen* rePrt = dc.SelectObject(&rePen);
			dc.MoveTo(rect.Width() / (buildingParam.numOfElevator + 1) * i + 100 / times, rect.Height() - 90 / times * (buildingParam.floors + 1) - 60);
			dc.LineTo(rect.Width() / (buildingParam.numOfElevator + 1) * i + 100 / times + 30, rect.Height() - 90 / times * (buildingParam.floors + 1) - 60);
			dc.SelectObject(rePrt);
			str.Format(_T("%d"), diantiyunxinggaodu);
			CFont font;
			font.CreatePointFont(160, _T("黑体"));
			CFont* def_font = dc.SelectObject(&font);
			dc.TextOut(rect.Width() / (buildingParam.numOfElevator + 1) * i + 100 / times, rect.Height() - 90 / times * (buildingParam.floors + 1) - 60, str);
			DeleteObject(font);
			DeleteObject(def_font);
		}
		bmpBackground.LoadBitmap(IDB_BITMAP9);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld5 = dcMem.SelectObject(&bmpBackground);

		for (int i = 1; i <= buildingParam.floors; i++)
		{
			// 上行白灯
			auto iter = std::find(UpGreenLight.begin(), UpGreenLight.end(), i - 1);
			if (iter != UpGreenLight.end() || i == buildingParam.floors)
			{
				continue;
			}
			else dc.StretchBlt(150 / times, rect.Height() - 90 / times * i, 40 / times, 40 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		}
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld5);

		bmpBackground.LoadBitmap(IDB_BITMAP10);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld6 = dcMem.SelectObject(&bmpBackground);
		for (int i = 1; i <= buildingParam.floors; i++)
		{
			auto iter = std::find(DownGreenLight.begin(), DownGreenLight.end(), i - 1);
			if (iter != DownGreenLight.end() || i == 1)
			{
				continue;
			}//此处下行白灯
			else dc.StretchBlt(200 / times, rect.Height() - 90 / times * i, 40 / times, 40 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		}
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld6);

		bmpBackground.LoadBitmap(IDB_BITMAP12);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld7 = dcMem.SelectObject(&bmpBackground);
		for (int i = 1; i <= buildingParam.floors; i++)
		{
			auto iter = std::find(UpGreenLight.begin(), UpGreenLight.end(), i - 1);
			if (iter == UpGreenLight.end() || i == buildingParam.floors)
			{
				continue;
			}
			else
			{
				dc.StretchBlt(150 / times, rect.Height() - 90 / times * i, 40 / times, 40 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);//此处为亮灯楼层
			};//上行绿灯
		}
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld7);

		bmpBackground.LoadBitmap(IDB_BITMAP11);
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld8 = dcMem.SelectObject(&bmpBackground);
		for (int i = 1; i <= buildingParam.floors; i++)
		{
			auto iter = std::find(DownGreenLight.begin(), DownGreenLight.end(), i - 1);
			if (iter == DownGreenLight.end() || i == 1)
			{
				continue;
			}
			else
				dc.StretchBlt(200 / times, rect.Height() - 90 / times * i, 40 / times, 40 / times, &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);//此处为亮灯楼层;//下行绿灯
		}
		ReleaseDC(&dcMem);
		ReleaseDC(&dc);
		DeleteObject(rect);
		DeleteObject(bmpBackground);
		DeleteObject(pbmpOld8);
	}
}
extern volatile bool IS_STOP_INFO;
void CelevatorSHOW::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		// 电梯信息窗口
	IS_STOP_INFO = false;
	if (globalData.GetCeleInfoDlg() != NULL)
	{
		delete globalData.GetCeleInfoDlg();
		globalData.SetCeleInfoDlg(NULL);
	}
	globalData.GetInfoTimer().Expire();
	CelevatorInfo* dlg = new CelevatorInfo;
	globalData.SetCeleInfoDlg(dlg);
	dlg->Create(IDD_INFORM);
	dlg->ShowWindow(SW_SHOWNORMAL);
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CelevatorSHOW::OnClose()
{
	IS_STOP_INFO = true;
	IS_STOP_ALL = true;
	if (globalData.GetCeleInfoDlg() != NULL)
	{
		globalData.GetInfoTimer().Expire();
		delete globalData.GetCeleInfoDlg();
		globalData.SetCeleInfoDlg(NULL);
	}
	globalData.Reset();
	CDialogEx::OnClose();
}
