/*
 * @Author: 罗飞杰
 * @File: g_a.cpp
 * @Date: 2021-7-10
 * @LastEditTime: 2021-7-30
 * @LastEditors: 罗飞杰
 * @brief: 电梯群控调度核心算法——遗传算法
 */
#include <random>
#include <vector>
#include <iostream>
#include <cmath>
#include <windows.h>
#include <tchar.h>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <atlstr.h>
#include <assert.h>
#include <chrono>
#include "transit.h"
#include "pattern.h"
extern bool IS_STOP_ALL;
using namespace std;
volatile bool gaIsFinish = false;

// 交叉概率
const double CROSSOVER_PROBABILITY = 0.75;
// 变异概率
const double MUTATION_PROBABILITY = 0.1;
// 随机种子
const unsigned int SEED = 20;
// 种群规模，可变
int POPULATION_SIZE = 100;
// 进化代数，可变
int NUM_EVOLUTION = 80;

// 个体
class Individual
{
private:
	// 序列，电梯在Elevators->elevators数组中的索引
	int* sequence;
	// 序列中元素的个数
	int num;
	// 适应度
	double fitness;
	// 适应度概率
	double fitnessPro;
	// 累加适应度概率
	double fitnessSumPro;
public:
	Individual(int* paramSequence, Signal* signal);
	// 获取参数
	int* GetSequence()
	{
		return sequence;
	}
	double GetFitness()
	{
		return fitness;
	}
	double GetFitnessPro()
	{
		return fitnessPro;
	}
	double GetFitnessSumPro()
	{
		return fitnessSumPro;
	}
	// 设置参数
	double SetFitness(const double param)
	{
		fitness = param;
		return fitness;
	}
	double SetFitnessPro(const double param)
	{
		fitnessPro = param;
		return fitnessPro;
	}
	double SetFitnessSumPro(const double param)
	{
		fitnessSumPro = param;
		return fitnessSumPro;
	}
	Individual(const Individual& user)
	{
		this->num = user.num;
		this->sequence = (int*)malloc(num * sizeof(int));
		for (int i = 0; i < num; ++i)
		{
			this->sequence[i] = user.sequence[i];
		}
		this->fitness = user.fitness;
		this->fitnessPro = user.fitnessPro;
		this->fitnessSumPro = user.fitnessSumPro;
	}
	~Individual() { free(sequence); };
};
class Const
{
public:
	const int constNum;
	Const(int num) :constNum(num) {};
};

Individual::Individual(int* paramSequence, Signal* pSignal)
{
	sequence = (int*)malloc(pSignal->signalNum * sizeof(int));
	num = pSignal->signalNum;
	for (int i = 0; i < num; ++i)
	{
		sequence[i] = paramSequence[i];
	}
	fitness = 0;
	fitnessPro = 0;
	fitnessSumPro = 0;
}

// 存放当代个体
vector<Individual>nowPopulation;
// 存放选择后的中间个体
vector<Individual>midPopulation;
// 存放下一代个体
vector<Individual>nextPopulation;

double random()
{
	int n = rand() % 999;
	// 随机产生0到1的小数
	return double(n) / 1000.0;
}

// 生成初始种群
void Initialize(Elevators* pElevators, Signal* pSignal)
{
	int** temp = new int* [POPULATION_SIZE];
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		// 储存的是 Elevators 中个电梯在elevators数组中的索引
		temp[i] = new int[pSignal->signalNum];
	}
	// 随机数引擎
	default_random_engine e(static_cast<unsigned>(20));
	// 动态生成 常量
	Const num(pElevators->numElevators);
	// 生成[0,  pElevators->numElevators - 1] 的数
	uniform_int_distribution<int>u(0, num.constNum - 1);
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		for (int n = 0; n < pSignal->signalNum; ++n)
		{
			// 不消重，一部电梯可能需要响应多个信号
			temp[i][n] = u(e);
		}
	}
	// 加入初始种群
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		int* tempTemp = new int[pSignal->signalNum];
		for (int n = 0; n < pSignal->signalNum; ++n)
		{
			tempTemp[n] = temp[i][n];
		}
		Individual individual(tempTemp, pSignal);
		nowPopulation.push_back(individual);
		delete[]tempTemp;
	}
	// 动态内存释放
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		delete[]temp[i];
	}
	delete[]temp;
}

// 中间函数，其他函数勿调用
double CalDistanceTime(double s, double totalTime, double tempTime, double distNodeConstant, double distNodeMaxAcce, Elevators* pElevators, unsigned int elevatorIndex)
{
	if (s < distNodeMaxAcce)
	{
		totalTime = tempTime + pow(32 * s / pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, (double)1 / 3);
	}
	else if (s >= distNodeMaxAcce && s < distNodeConstant)
	{
		totalTime = tempTime + (pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration *
			(1 + pow(1 + 4 * pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, 2) * s / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 3), (double)1 / 2))) /
			pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk;
	}
	else if (s >= distNodeConstant)
	{
		totalTime = tempTime + (pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed +
			pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk +
			(double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk * s) /
			((double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk);
	}
	return totalTime;
}

// 计算电梯内人乘梯时间
double CaculateTakeTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam)
{
	// 假设乘梯时间为 0s 时评价值为1， 乘梯时间为90秒时评价函数值为0.001
	double totalTime = 0;
	int lastDistFloorIndex = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size() - 1;
	// 该时间的意义是电梯在每一目的层所停留的时间
	double tempTime = (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfWait + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfOpenDoor
		+ (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfCloseDoor + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfBrake;
	// 电梯能达到额定速度的距离节点
	double distNodeConstant = (pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed + pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk) /
		((double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk);
	// 电梯能达到最大运行加速度的距离节点
	double distNodeMaxAcce = pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 3) * 2 / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, 2);

	typedef struct Distance
	{
		int whatFloor;
		double time;
	}Distance;
	vector<Distance> temp;

	switch (pElevators->elevators[elevatorIndex].elevatorStatus.dir)
	{
	case UP:
	{
		double s = 0;
		// 初始部分
		s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist - pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor));
		// 第一部分前所用的时间
		double initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);

		for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN)
			{
				s = 0;
				// 计算各内部信号层到电梯目前所在层的距离
				Distance tempDistance;
				tempDistance.whatFloor = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist;
				tempDistance.time = 0;
				int j = i;

				while (j > 0)
				{
					s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist));
					tempDistance.time += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					j--;
				}
				tempDistance.time += initialPartTime;
				temp.push_back(tempDistance);
			}
		}
		break;
	}
	case DOWN:
	{
		// 寻找分割点, 第一部分分割点
		int presentFloor = pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor;
		int intialFirstPoint = 0;
		for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN ||
				(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist < presentFloor && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN))
			{
				intialFirstPoint = i;
			}
		}

		double s = 0;
		// 初始部分
		s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[intialFirstPoint].dist - pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor));
		// 第一部分前所用的时间
		double initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);

		for (int i = intialFirstPoint; i > 0; --i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN)
			{
				s = 0;
				// 计算各内部信号层到电梯目前所在层的距离
				Distance tempDistance;
				tempDistance.whatFloor = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist;
				tempDistance.time = 0;
				int j = i;
				double s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[intialFirstPoint].dist);
				while (j <= intialFirstPoint)
				{
					s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist));
					tempDistance.time += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					j++;
				}
				tempDistance.time += initialPartTime;
				temp.push_back(tempDistance);
			}
		}
		break;
	}
	default:
		break;
	}

	// 查找
	for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.person.size(); ++i)
	{
		for (int j = 0; j < temp.size(); ++j)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.person[i].distFloor == temp[j].whatFloor)
			{
				totalTime += (double)pElevators->elevators[elevatorIndex].elevatorStatus.person[i].time + temp[j].time;
			}
		}
	}
	double x = 3 * log(10) / pow(90, 2);
	double ret = exp(-x * totalTime);
	return ret;
}

// 计算等待时间评价函数值
double CalculateWaitTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam)
{
	// 假设等待时间为 0s 时评价值为1， 等待时间为60秒时评价函数值为0.001
	// 全部外部信号的等待时间之和
	double totalTime = 0;
	int lastDistFloorIndex = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size() - 1;
	double tempTime = (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfWait + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfOpenDoor
		+ (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfCloseDoor + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfBrake;
	// 电梯能达到额定速度的距离节点
	double distNodeConstant = (pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed + pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk) /
		((double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk);
	// 电梯能达到最大运行加速度的距离节点
	double distNodeMaxAcce = pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 3) * 2 / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, 2);
	switch (pElevators->elevators[elevatorIndex].elevatorStatus.dir)
	{
	case UP:
	{
		double s = 0;
		// 第一部分前所用的时间
		double initialPartTime = 0;
		s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist - pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor));
		// 无论第一部分有没有都有这一部分的时间
		initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
		if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_DOWN)
		{
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfDOWN;
		}
		else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_UP)
		{
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfUP - tempTime;
		}

		for (int i = 1; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN || pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
			{
				s = 0;
				int j = i;
				while (j > 0)
				{
					s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist));
					// s = 0 时不必加上tempTime
					if (s != 0)
					{
						totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					}
					j--;
				}

				totalTime += initialPartTime;
				if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfDOWN;
				}
				else
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfUP;
				}
			}
		}
		break;
	}
	case DOWN:
	{
		int presentFloor = pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor;
		bool isHaveFirstPart = false;
		// 寻找分割点, 第一部分分割点
		int intialFirstPoint = 0;
		for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN ||
				(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist < presentFloor && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN))
			{
				isHaveFirstPart = true;
				intialFirstPoint = i;
			}
		}

		double s = 0;
		// 第一部分所用的时间
		double firstPartTime = 0;
		// 第一部分前所用的时间
		double initialPartTime = 0;
		s = static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist)) * static_cast<double>(pBuildingParam->storeyHeight);
		// 无论有没有第一部分都有这段时间
		initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
		if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_DOWN && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist < presentFloor)
		{
			// 此情况下默认存在第一部分
			totalTime -= tempTime;
		}
		else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_DOWN && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist >= presentFloor)
		{
			// 不存在第一部分
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfDOWN - tempTime;
		}
		else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_UP)
		{
			// 不存在第一部分
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfUP - tempTime;
		}

		if (isHaveFirstPart)
		{
			// 计算第一部分外部信号乘客等待时间
			for (int i = intialFirstPoint; i >= 0; --i)
			{
				if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN || pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
				{
					s = 0;
					int j = i;
					while (j < intialFirstPoint)
					{
						s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j + 1].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist));
						// s = 0 时不必加上tempTime
						if (s != 0)
						{
							totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
						}
						++j;
					}

					totalTime += initialPartTime;
					if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
					{
						totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfDOWN;
					}
					else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
					{
						totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfUP;
					}
				}
			}
			s = static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[intialFirstPoint].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist)) * static_cast<double>(pBuildingParam->storeyHeight);
			firstPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
		}

		// 计算第二部分、第三部分外部信号乘客等待时间
		for (int i = intialFirstPoint + 1; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN || pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
			{
				s = 0;
				int j = i;
				while (j > intialFirstPoint + 1)
				{
					s = static_cast<double>(pBuildingParam->storeyHeight) * static_cast<double>(ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist));
					// s = 0 时不必加上tempTime
					if (s != 0)
					{
						totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					}
					j--;
				}

				totalTime += firstPartTime + initialPartTime;
				if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfDOWN;
				}
				else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfUP;
				}
			}
		}

		break;
	}
	default:
		break;
	}
	double x = 3 * log(10) / pow(60, 2);
	double ret = exp(-x * totalTime);
	return ret;
}

// 计算拥挤评价函数值
double CaculateComfort(Elevators* pElevators, unsigned int elevatorIndex)
{
	// 假设电梯内的人数为 0 时舒适度为1，人数为额定人数时舒适度为0.001，满足 e 指数级变化
	double x = 3 * log(10) / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedCapacity, 2);
	double ret = exp(-x * pElevators->elevators[elevatorIndex].elevatorStatus.person.size());
	return ret;
}

// 计算能耗评价函数值
double CaculateEnergyConsume(Elevators* pElevators, unsigned int elevatorIndex)
{
	int num = 0;
	// 根据停层次数来判定, 假设停层次数为5次时评价函数值为0.01
	for (int i = 1; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
	{
		// 避开重复
		if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist != pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i - 1].dist)
		{
			num++;
		}
	}
	double x = 2 * log(10) / pow(5, 2);
	return exp(-x * num);
}
mutex mtx;
Elevators* InsertExternSignal(Elevators* pTempElevator, int* sequence, int sequenceIndex, Signal* pSignal, bool isIntern)
{
	mtx.lock();
	int index = sequence[sequenceIndex];
	// 新外部信号初始化
	DistFloor tempDist;
	// 外部信号层
	tempDist.dist = pSignal->externSignal[sequenceIndex].floors;
	if (!isIntern)
	{
		if (pSignal->externSignal[sequenceIndex].dir == UP)
		{
			tempDist.type = EXTERN_UP;
		}
		else if (pSignal->externSignal[sequenceIndex].dir == DOWN)
		{
			tempDist.type = EXTERN_DOWN;
		}
	}
	else
	{
		tempDist.type = INTERN;
	}
	// 新外部信号等待时间初始化
	tempDist.waitTimeOfDOWN = 0;
	tempDist.waitTimeOfUP = 0;

	// 新外部信号插入
	switch (pTempElevator->elevators[index].elevatorStatus.dir)
	{
		// 此情况下电梯目的层集必定存在第一部分
	case UP:
	{
		switch (tempDist.type)
		{
		case EXTERN_UP:
		{
			// 规则：电梯响应信号集分为三个部分
			// 外部信号层在电梯当前所处层之上，插入第一部分
			if (tempDist.dist > pTempElevator->elevators[index].elevatorStatus.presentFloor)
			{
				// 第一部分
				// 迭代器初始化
				vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
				int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
				int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
				int ii = 0;
				while (tempDist.dist > pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor &&
					(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP))
				{
					ii++;
					if (ii == numOfFloor)
					{
						break;
					}
				}
				distIter += ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

				break;
			}
			// 外部信号层在电梯当前所在层之下，插入到第三部分
			else
			{
				/*
				* 规则：电梯响应信号集分为三个部分 当tempElevator.elevators[index].elevatorStatus.dir == UP 且 tempDist.type == EXTERN_UP，该外部信号将被引入
				* 第三部分； 当tempElevator.elevators[index].elevatorStatus.dir == UP 且 tempDist.type == EXTERN_DOWN, 该外部信号将被引入第二部分
				*/
				// 第三部分
				// 迭代器初始化
				vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
				int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
				int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();

				int ii = 0;
				while ((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor) ||
					pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN ||
					(tempDist.dist > pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP))
				{
					ii++;
					if (ii == numOfFloor)
					{
						break;
					}
				}
				distIter += ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);
			}
			break;
		}
		case EXTERN_DOWN:
		{
			// 插入到第二部分
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int ii = 0;
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor) ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > tempDist.dist))
			{
				ii++;
				// 不存在第二部分与第三部分
				if (ii == numOfFloor)
				{
					break;
				}
			}
			distIter += ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		case INTERN:
		{
			// 插入到第一部分
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int ii = 0;
			int dist = tempDist.dist;
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			while ((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP) &&
				pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist <= dist && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist >= presentFloor)
			{
				ii++;
			}
			distIter = distIter + ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		default:
			break;
		}
		break;
	}
	case DOWN:
	{
		int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
		bool isHaveFirstPart = false;
		// 寻找分割点, 第一部分分割点
		int intialFirstPoint = 0;
		for (int i = 0; i < pTempElevator->elevators[index].elevatorStatus.distFloors.size(); ++i)
		{
			if (pTempElevator->elevators[index].elevatorStatus.distFloors[i].type == INTERN ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[i].dist < presentFloor && pTempElevator->elevators[index].elevatorStatus.distFloors[i].type == EXTERN_DOWN))
			{
				isHaveFirstPart = true;
				intialFirstPoint = i;
			}
		}

		switch (tempDist.type)
		{
		case EXTERN_DOWN:
		{
			// 迭代器初始化
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			// 插入到第一部分
			if (tempDist.dist < presentFloor)
			{
				int ii = 0;
				while ((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN)
					&& pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < tempDist.dist)
				{
					ii++;
					if (ii == intialFirstPoint + 1)
					{
						break;
					}
				}
				distIter += ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

				break;
			}
			// 插入到第三部分
			else
			{
				vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
				int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
				int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();

				int ii = intialFirstPoint;
				while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP ||
					((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN) &&
						pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < presentFloor) ||
					(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > tempDist.dist))
				{
					ii++;
					if (ii == numOfFloor)
					{
						break;
					}
				}
				distIter = distIter + ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);
			}
			break;
		}
		case EXTERN_UP:
		{
			// 第二部分
			// 迭代器初始化
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			int ii = intialFirstPoint;
			while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < presentFloor) ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < tempDist.dist))
			{
				ii++;
				if (ii == numOfFloor)
				{
					break;
				}
			}
			distIter = distIter + ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		case INTERN:
		{
			// 插入到第一部分
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int ii = 0;
			int dist = tempDist.dist;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist <= presentFloor &&
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN) &&
				pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist <= dist)
			{
				ii++;
			}
			distIter = distIter + ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		default:
			break;
		}
		break;
	}
	case STATIC:
	{
		pTempElevator->elevators[index].elevatorStatus.distFloors.push_back(tempDist);
		// 电梯运行方向更改
		if (pTempElevator->elevators[index].elevatorStatus.presentFloor < tempDist.dist)
		{
			pTempElevator->elevators[index].elevatorStatus.dir = UP;
		}
		else if (pTempElevator->elevators[index].elevatorStatus.presentFloor > tempDist.dist)
		{
			pTempElevator->elevators[index].elevatorStatus.dir = DOWN;
		}
		break;
	}
	default:
		break;
	}
	mtx.unlock();
	return pTempElevator;
}

/*
* note:::::: 电梯在读取信号集时注意处理从何处开始读信号集！！！！！！！！！！
* 规则：外部信号目的层一定大于(EXTERN_UP)或小于(EXTERN_DOWN)当前信号层
*
*/
// 计算适应度
void CalculateFitness(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal)
{
	float w1, w2, w3, w4;
	double fit = 0;
	switch (pPassengerFLow->patterns)
	{
	case FREE:
	{
		w1 = 0.4;
		w2 = 0.1;
		w3 = 0.2;
		w4 = 0.3;
		break;
	}
	case BUSY_UPRUSH:
	{
		w1 = 0.7;
		w2 = 0.2;
		w3 = 0.0;
		w4 = 0.1;
		break;
	}
	case BUSY_DOWNRUSH:
	{
		w1 = 0.7;
		w2 = 0.2;
		w3 = 0.0;
		w4 = 0.1;
		break;
	}
	case BUSY_LAYER_BALANCE:
	case BUSY_LAYER_DUPLEX:
	{
		w1 = 0.3;
		w2 = 0.0;
		w3 = 0.3;
		w4 = 0.4;
		break;
	}
	default:
		break;
	}

	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		// 拷贝一份elevators
		Elevators tempElevator;
		tempElevator.elevators = new ElevatorRealTimeParam[pElevators->numElevators];
		tempElevator.numElevators = pElevators->numElevators;
		for (int n = 0; n < pElevators->numElevators; ++n)
		{
			tempElevator.elevators[n].elevatorParam = pElevators->elevators[n].elevatorParam;
			tempElevator.elevators[n].elevatorStatus = pElevators->elevators[n].elevatorStatus;
		} // 拷贝完毕
		Elevators* pTempElevator = &tempElevator;

		// 将目标层加入distFloors并计算适应度
		for (int j = 0; j < pSignal->signalNum; ++j)
		{
			int index = nowPopulation.at(i).GetSequence()[j];
			assert(IS_VALUE_IN_SECTION(index, 0, pSignal->externSignal.size() - 1));
			pTempElevator = InsertExternSignal(pTempElevator, nowPopulation.at(i).GetSequence(), j, pSignal, false);
			fit = fit + CalculateWaitTime(pTempElevator, index, pBuildingParam) * w1 +
				CaculateComfort(pTempElevator, index) * w2 + CaculateEnergyConsume(pTempElevator, index) * w3 + CaculateTakeTime(pTempElevator, index, pBuildingParam) * w4;
		}
		for (int k = 0; k < pTempElevator->numElevators; ++k)
		{
			vector<DistFloor>().swap(pTempElevator->elevators[k].elevatorStatus.distFloors);
		}
		delete[] tempElevator.elevators;
		//printf("%lf\n", fit); // 用于查看适应度变化情况
		nowPopulation.at(i).SetFitness(fit);
		fit = 0;
	}
}

// 计算适应值概率
void CaculatePFitness()
{
	double sumFitness = 0; // 适应度累计值
	double temp = 0;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		sumFitness += nowPopulation.at(i).GetFitness();
	}
	for (int n = 0; n < POPULATION_SIZE; ++n)
	{
		temp = nowPopulation.at(n).GetFitness() / sumFitness;
		nowPopulation.at(n).SetFitnessPro(temp);
	}
}

// 计算适应值累计值
void CaculateSumFitness()
{
	double summation = 0; // 累计值
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		summation += nowPopulation.at(i).GetFitnessPro();
		nowPopulation.at(i).SetFitnessSumPro(summation);
	}
}

// 选择
void Select()
{
	double maxFitness = nowPopulation.at(0).GetFitness();
	int maxId = 0;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		if (maxFitness < nowPopulation.at(i).GetFitness())
		{
			maxFitness = nowPopulation.at(i).GetFitness();
			maxId = i;
		}
	}
	for (int i = 0; i < POPULATION_SIZE / 4; ++i)
	{
		midPopulation.push_back(nowPopulation.at(maxId));
	}
	int NEWPOPULATIONSIZE = POPULATION_SIZE - POPULATION_SIZE / 4;
	// 用来保存随机数
	double* arr = new double[NEWPOPULATIONSIZE];
	//引擎，生成随机序列
	default_random_engine e(time(0));
	uniform_real_distribution<double> u(0.0, 1.0);
	for (int i = 0; i < NEWPOPULATIONSIZE; ++i)
	{
		arr[i] = u(e);
	}
	// 轮盘选择
	for (int i = 0; i < NEWPOPULATIONSIZE; ++i)
	{
		if (arr[i] < nowPopulation.at(0).GetFitnessSumPro())
		{
			midPopulation.push_back(nowPopulation.at(0));
		}
		else
		{
			for (int n = 1; n < POPULATION_SIZE; ++n)
			{
				if (arr[i] >= nowPopulation.at(n - 1).GetFitnessSumPro() && arr[i] <= nowPopulation.at(n).GetFitnessSumPro())
				{
					midPopulation.push_back(nowPopulation.at(n));
					break;
				}
			}
		}
	}
	nowPopulation.clear();
	delete[]arr;
}

// 交叉互换,采用一点交叉
void Crossover(Signal* pSignal)
{
	int num = 0; // 记录次数
	int crossPos; // 记录交叉位置
	// 临时储存父亲的访问序列
	int* arr1 = new int[pSignal->signalNum];
	// 临时储存母亲的访问序列
	int* arr2 = new int[pSignal->signalNum];
	// 用于储存交叉产生的个体
	int* newArr1 = new int[pSignal->signalNum];
	int* newArr2 = new int[pSignal->signalNum];
	Const constNum(pSignal->signalNum - 1);
	default_random_engine e(time(0));
	std::uniform_int_distribution<int> r(1, constNum.constNum);
	// POPULATION_SIZE必须为偶数
	while (num < POPULATION_SIZE - 1)
	{
		double randNum = random();
		if (randNum <= CROSSOVER_PROBABILITY)
		{
			for (int i = 0; i < pSignal->signalNum; ++i)
			{
				// 得到父亲的访问序列
				arr1[i] = midPopulation.at(num).GetSequence()[i];
				// 得到母亲的访问序列
				arr2[i] = midPopulation.at(num + 1).GetSequence()[i];
			}
			// 产生 [1, pSigal->signalNum-1] 随机数
			crossPos = r(e);
			// 提取交叉序列片段
			for (int n = 0; n < crossPos; ++n)
			{
				newArr1[n] = arr1[n];
				newArr2[n] = arr2[n];
			}
			for (int i = crossPos; i < pSignal->signalNum; ++i)
			{
				newArr1[i] = arr2[i];
				newArr2[i] = arr1[i];
			}
			Individual newChild1(newArr1, pSignal);
			Individual newChild2(newArr2, pSignal);
			nextPopulation.push_back(newChild1);
			nextPopulation.push_back(newChild2);
		}
		else
		{
			nextPopulation.push_back(midPopulation.at(num));
			nextPopulation.push_back(midPopulation.at(num + 1));
		}
		num += 2;
	}
	midPopulation.clear();
	delete[]arr1;
	delete[]arr2;
	delete[]newArr1;
	delete[]newArr2;
}

// 变异
void Mutation(Signal* pSignal)
{
	// 存放两个交换点
	int mutationPoint[2];
	int temp;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		int* tempSequence = new int[pSignal->signalNum];
		double randNum = random();
		for (int n = 0; n < pSignal->signalNum; ++n)
		{
			tempSequence[n] = nextPopulation.at(i).GetSequence()[n];
		}
		if (randNum <= MUTATION_PROBABILITY)
		{
			for (int m = 0; m < 2; ++m)
			{
				mutationPoint[m] = rand() % pSignal->signalNum;
			}
			temp = tempSequence[mutationPoint[0]];
			tempSequence[mutationPoint[0]] = tempSequence[mutationPoint[1]];
			tempSequence[mutationPoint[1]] = temp;

			Individual mutationChild(tempSequence, pSignal);
			nowPopulation.push_back(mutationChild);
		}
		else
		{
			nowPopulation.push_back(nextPopulation.at(i));
		}
		delete[]tempSequence;
	}
	nextPopulation.clear();
}

// 初始为向上运行，第二部分为extern_down，到顶层停止时，需要对电梯信号集进行处理
ElevatorRealTimeParam* ELevatorChange(ElevatorRealTimeParam* pElevators)
{
	int j = 0;
	vector<DistFloor>temp;
	while (j < pElevators->elevatorStatus.distFloors.size() && pElevators->elevatorStatus.distFloors[j].type == EXTERN_DOWN)
	{
		temp.push_back(pElevators->elevatorStatus.distFloors[j]);
		j++;
	}
	vector<DistFloor>::iterator tempTemp;
	tempTemp = temp.begin();
	for (int n = j - 1; n >= 0; --n)
	{
		pElevators->elevatorStatus.distFloors[n] = *tempTemp;
		tempTemp += 1;
	}
	return pElevators;
}

// 遗传算法总函数
Dispatch GeneticAlgorithm(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal)
{
	//#define TRT

#ifdef TRT
	// 测试时间
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
#endif // DEBUG
	bool isOnlyOne = false;
	// 只有一个信号时，需要增加一个相同的信号
	if (pSignal->signalNum == 1)
	{
		pSignal->signalNum++;
		isOnlyOne = true;
		ExternSignal tempEx;
		tempEx.id = pSignal->externSignal[0].id + 1;
		tempEx.floors = pSignal->externSignal[0].floors;
		tempEx.dir = pSignal->externSignal[0].dir;
		pSignal->externSignal.push_back(tempEx);
	}
	NUM_EVOLUTION = 150;
	// 临时最大适应值
	double maxFit = 0;
	volatile unsigned int index = 0;
	// 初始化，生成初始种群
	Initialize(pElevators, pSignal);
	for (int i = 0; i < NUM_EVOLUTION; ++i)
	{
		if (IS_STOP_ALL)
		{
			_endthreadex(0);
		}
		assert(nowPopulation.size() == 100);
		CalculateFitness(pElevators, pPassengerFLow, pBuildingParam, pSignal);
		CaculatePFitness();
		CaculateSumFitness();
		Select();
		Crossover(pSignal);
		Mutation(pSignal);
#ifdef DEBUG
		for (int m = 0; m < POPULATION_SIZE; ++m)
		{
			if (maxFit < nowPopulation.at(m).GetFitness())
			{
				maxFit = nowPopulation.at(m).GetFitness();
				index = m;
			}
		}

		CString str;
		str.Format(_T("%lf\n"), maxFit);
		OutputDebugString(str);
		//printf("%lf\n", maxFit);
#endif // DEBUG
	}

	for (int m = 0; m < POPULATION_SIZE; ++m)
	{
		//printf("%lf\n", nowPopulation.at(m).GetFitness());
		if (maxFit < nowPopulation.at(m).GetFitness())
		{
			maxFit = nowPopulation.at(m).GetFitness();
			index = m;
		}
	}
	SingleDispatch* temp = new SingleDispatch[pSignal->signalNum];
	for (int j = 0; j < pSignal->signalNum; ++j)
	{
		temp[j].elevatorId = pElevators->elevators[nowPopulation.at(index).GetSequence()[j]].elevatorParam.id;
		temp[j].signalId = pSignal->externSignal[j].id;
		temp[j].dir = pSignal->externSignal[j].dir;
		temp[j].floors = pSignal->externSignal[j].floors;
	}
	if (!isOnlyOne)
	{
		for (int i = 0; i < pSignal->signalNum; ++i)
		{
			pElevators = InsertExternSignal(pElevators, nowPopulation.at(index).GetSequence(), i, pSignal, false);
		}
	}
	else
	{
		pSignal->signalNum--;
		pSignal->externSignal.pop_back();
		pElevators = InsertExternSignal(pElevators, nowPopulation.at(index).GetSequence(), 0, pSignal, false);
	}
	nowPopulation.clear();
	midPopulation.clear();
	nextPopulation.clear();
	Dispatch ret;
	ret.singleDispatch = temp;
	ret.dispatchNum = pSignal->signalNum;
	gaIsFinish = true;
#ifdef TRT
	// 测试时间
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
#endif // DEBUG
	return ret;
}
