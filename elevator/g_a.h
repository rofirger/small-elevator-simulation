#pragma once
#ifndef _G_A_H_
#define _G_A_H_
#include "transit.h"
#include "pattern.h"
double random();
// 生成初始种群
void Initialize(Elevators* pElevators, Signal* pSignal);
// 中间函数，其他函数勿调用
double CalDistanceTime(double s, double totalTime, double tempTime, double distNodeConstant, double distNodeMaxAcce, Elevators* pElevators, unsigned int elevatorIndex);
// 计算电梯内人乘梯时间
double CaculateTakeTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam);
// 计算等待时间评价函数值
double CalculateWaitTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam);
// 计算拥挤评价函数值
double CaculateComfort(Elevators* pElevators, unsigned int elevatorIndex);
// 计算能耗评价函数值
double CaculateEnergyConsume(Elevators* pElevators, unsigned int elevatorIndex);
// 计算适应度
void CalculateFitness(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal);
// 计算适应值概率
void CaculatePFitness();
// 计算适应值累计值
void CaculateSumFitness();
// 选择
void Select();
// 交叉互换,采用一点交叉
void Crossover(Signal* pSignal);
// 变异
void Mutation(Signal* pSignal);
Elevators* InsertExternSignal(Elevators* pTempElevator, int* sequence, int sequenceIndex, Signal* pSignal, bool isIntern);
// 初始为向上运行，第二部分为extern_down，到顶层停止时，需要对电梯信号集进行处理
ElevatorRealTimeParam* ELevatorChange(ElevatorRealTimeParam* pElevators);
// 遗传算法总函数
Dispatch GeneticAlgorithm(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal);
#endif