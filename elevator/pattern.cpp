/*
 * @Author: 罗飞杰
 * @File: pattern.cpp
 * @Date: 2021-7-7
 * @LastEditTime: 2021-7-15
 * @LastEditors: 罗飞杰
 */
 // 对不同的交通模式选择合适的群控算法
 // 交通模式的分类与 预测的客流量、当前时刻的客流量和实际运行电梯台数有关
 // 交通模式的识别分类主要依靠大楼内各楼层客流百分比例进行划分
 // 假设各个时间段(已有研究表明如此), 各个时间段的客流变化率 r 在某个固定的区间变化
 /*
 * 交通模式: 空闲交通模式；繁忙交通模式->(上高峰交通模式；层间交通模式->(两路、四路交通模式；平衡层间交通模式)；下高峰交通模式)
 */
 /*
 * 电梯客流分类: 总客流、进门厅客流、出门厅客流、层间客流
 */
#include "gm.h"
#include <stdio.h>
#include <cstdlib>
#include"transit.h"
#include "pattern.h"
 ////PassengerFlowVolume passengerFlowVolume;
 //// 参数为passenger_flow_volume_data文件第一个数据所在的时间
 //void PatternInit(PassengerFlowVolume* pPassengerFlowVolume)
 //{
 //	pPassengerFlowVolume->fileCursorPoint = 0;
 //	FILE* file;
 //	errno_t err = fopen_s(&file, "passenger_flow_volume_data", "rb");
 //	if (err != 0)
 //	{
 //		perror("Open failed!");
 //		exit(1);
 //	}
 //	fseek(file, pPassengerFlowVolume->fileCursorPoint, SEEK_SET);
 //	fread(pPassengerFlowVolume->passengerFlow, sizeof(int), MAX_VOLUME_ARR_SIZE, file);
 //	pPassengerFlowVolume->fileCursorPoint = ftell(file);
 //	fclose(file);
 //	// arr 初始化
 //	for (int i = 0; i < MAX_VOLUME_ARR_SIZE; ++i)
 //	{
 //	}
 //	// 初始化arr完毕后, 开启时间线程, 三分钟更新一次pPassengerFlowVolume
 //	void* timeHandle = StartupTimeThread(pPassengerFlowVolume); // handle 统一交由transit处理
 //	// 变化率初始化，
 //	/*pGradient->maxGradient = (arr[DATA_INTERVAL] - arr[0]) / DATA_INTERVAL;
 //	pGradient->minGradient = pGradient->maxGradient;
 //	float slope;
 //	for (int i = DATA_INTERVAL; i < MAX_VOLUME_ARR_SIZE; ++i)
 //	{
 //		slope = (arr[i] - arr[i - DATA_INTERVAL]) / DATA_INTERVAL;
 //		if (slope < pGradient->minGradient)
 //		{
 //			pGradient->minGradient = slope;
 //		}
 //		else
 //		{
 //			pGradient->maxGradient = slope;
 //		}
 //	}*/
 //}
 //// 客流预测修正, 第三个参数为数组尾部属于预测值的数量
 //float* PredictCorrection(float* arr, int size, int predictNum)
 //{
 //	// k = (arr[i] - arr[i-1]) / DATA_INTERVAL;
 //	return NULL;
 //}
PassengerFlowPredict* FlowPredict(PassengerFLow* pPassengerFlow, PassengerFlowPredict* pPassengerFlowPredict)
{
	for (int i = 0; i < FLOW_VOLUME_NUM; ++i)
	{
		PassengerFlowVolume* tempVol = (PassengerFlowVolume*)P_GET_PASSENGER_FLOW_ELEM(pPassengerFlow, i);
		FlowPredictArrInfo* tempInfo = (FlowPredictArrInfo*)P_GET_PASSENGER_FLOW_PREDICT_ELEM(pPassengerFlowPredict, i);
		for (int n = REAL_FLOW_NUM - 1, m = 0; n >= 0; --n, ++m)
		{
			int index = tempVol->nowTimeIndex - 1;
			tempInfo->arr[n] = tempVol->passengerFlow[index - m];
		}
		float* temp = GM_Predict(tempInfo->arr, REAL_FLOW_NUM, REAL_FLOW_NUM + PREDICT_NUM, REAL_FLOW_NUM / 2);
		tempInfo->predictIndex = REAL_FLOW_NUM;
		for (int k = 0; k < PREDICT_NUM; ++k)
		{
			tempInfo->arr[REAL_FLOW_NUM + k] = temp[REAL_FLOW_NUM + k];
		}
	}
	return pPassengerFlowPredict;
}
Patterns PatternJudge(PassengerFLow* pPassengerFLow, PassengerFlowPredict* pPassengerFlowPredict, ElevatorParam* pElevatorParam, BuildingParam* pBuildingParam)
{
	// 实际客流量: 上一阶段剩余(未改程序) + 本阶段新增
	unsigned int maxPassengerNum = pBuildingParam->numOfElevator * pElevatorParam->ratedCapacity;
	unsigned int totalPassengerNum = pPassengerFLow->totalPassengerNum.passengerFlow[pPassengerFLow->totalPassengerNum.nowTimeIndex];
	unsigned int enterPassengerNum = pPassengerFLow->enterPassengerNum.passengerFlow[pPassengerFLow->enterPassengerNum.nowTimeIndex];
	unsigned int outPassengerNum = pPassengerFLow->outPassengerNum.passengerFlow[pPassengerFLow->outPassengerNum.nowTimeIndex];
	unsigned int totalLayerPassengerNum = pPassengerFLow->totalLayerPassengerNum.passengerFlow[pPassengerFLow->totalLayerPassengerNum.nowTimeIndex];
	unsigned int maxLayerPassengerNum = pPassengerFLow->maxLayerPassengerNum.passengerFlow[pPassengerFLow->maxLayerPassengerNum.nowTimeIndex];
	unsigned int otherLayerPassengerNum = pPassengerFLow->otherLayerPassengerNum.passengerFlow[pPassengerFLow->otherLayerPassengerNum.nowTimeIndex];
	float enterRatio = PASSENGER_FLOW_RATIO(enterPassengerNum, totalPassengerNum); // X11
	float outRatio = PASSENGER_FLOW_RATIO(outPassengerNum, totalPassengerNum); // X22
	unsigned int xl = totalPassengerNum - enterPassengerNum - outPassengerNum;
	float layerRaio = PASSENGER_FLOW_RATIO(xl, totalPassengerNum); // X33
	float maxLayerRatio = PASSENGER_FLOW_RATIO(maxLayerPassengerNum, xl); // X44
	float otherLayerRatio = PASSENGER_FLOW_RATIO(otherLayerPassengerNum, xl); // X55
	float totalRatio = PASSENGER_FLOW_RATIO(totalPassengerNum, maxPassengerNum); // X0
	const float ZERO_CONST = 0;
	const float MIN_CONST = 0.25;
	const float MID_CONST = 0.45;
	const float MAX_CONST = 0.55;
	if (totalRatio < MIN_CONST)
	{
		// 规则1
		pPassengerFLow->patterns = FREE;
	}
	else
	{
		// 预测(该预测值是根据本阶段新增量预测)
		int enterPredictIndex = pPassengerFlowPredict->enterFlowPredictArrInfo.predictIndex;
		int outPredictIndex = pPassengerFlowPredict->outFlowPredictArrInfo.predictIndex;
		int layerPredictIndex = pPassengerFlowPredict->totalLayerFlowPredictArrInfo.predictIndex;
		int maxLayerPredictIndex = pPassengerFlowPredict->maxLayerFlowPredictArrInfo.predictIndex;
		int otherLayerPredictIndex = pPassengerFlowPredict->otherLayerFlowPredictArrInfo.predictIndex;
		int totalPredictIndex = pPassengerFlowPredict->totalFlowPredictArrInfo.predictIndex;
		float enterPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->enterFlowPredictArrInfo.arr[enterPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->enterFlowPredictArrInfo.arr[enterPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X11_
		float outPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->outFlowPredictArrInfo.arr[outPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->outFlowPredictArrInfo.arr[outPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X22_
		float totalLayerPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->totalLayerFlowPredictArrInfo.arr[layerPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->totalLayerFlowPredictArrInfo.arr[layerPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X33_
		float maxLayerPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->maxLayerFlowPredictArrInfo.arr[maxLayerPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->maxLayerFlowPredictArrInfo.arr[maxLayerPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X44_
		float otherLayerPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->otherLayerFlowPredictArrInfo.arr[otherLayerPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->otherLayerFlowPredictArrInfo.arr[otherLayerPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X55_

		// 上高峰模式: 规则2，9
		if (enterRatio >= MAX_CONST ||
			(
				IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) &&
				IS_GREATER_THAN_ALL(enterPassengerNumRatio, outPassengerNumRatio, totalLayerPassengerNumRatio)
				)
			)
		{
			pPassengerFLow->patterns = BUSY_UPRUSH;
		}
		// 下高峰模式: 规则3，10
		else if (outRatio >= MAX_CONST ||
			(
				IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) &&
				IS_GREATER_THAN_ALL(outPassengerNumRatio, enterPassengerNumRatio, totalLayerPassengerNumRatio)
				)
			)
		{
			pPassengerFLow->patterns = BUSY_DOWNRUSH;
		}
		// 两路交通模式: 规则4，6，11，13，16，18
		else if ((layerRaio >= MAX_CONST && maxLayerRatio >= MAX_CONST) ||
			(layerRaio >= MAX_CONST && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio > otherLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && maxLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio > otherLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_GREATER_THAN_ALL(layerRaio, enterRatio, outRatio) && maxLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio > otherLayerPassengerNumRatio)
			)
		{
			pPassengerFLow->patterns = BUSY_LAYER_DUPLEX;
		}
		// 平衡层间交通模式: 规则5，7，8，12，14，15，17，19
		else if ((layerRaio >= MAX_CONST && otherLayerRatio >= MAX_CONST) ||
			(layerRaio >= MAX_CONST && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio <= otherLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && (enterPassengerNumRatio == outPassengerNumRatio || enterPassengerNumRatio == totalLayerPassengerNumRatio || outPassengerNumRatio == totalLayerPassengerNumRatio)) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && otherLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && otherLayerPassengerNumRatio >= maxLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && (enterRatio == outRatio || enterRatio == layerRaio || outRatio == layerRaio)) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_GREATER_THAN_ALL(layerRaio, enterRatio, outRatio) && otherLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio <= otherLayerPassengerNumRatio)
			)
		{
			pPassengerFLow->patterns = BUSY_LAYER_BALANCE;
		}
	}
	return pPassengerFLow->patterns;
}
