#pragma once
#ifndef _PATTERN_H_
#define _PATTERN_H_
#include "transit.h"
#define IS_SLOPE_IN_SECTION(slope, min_slope, max_slope) ((slope) >= (min_slope) && (slope) <= (max_slope))
#define ABS(v)  (v >= 0 ? v : -(v))
#define IS_SLOPE_IN_DOUBLE_MIN_SECTION(slope, min_slope, max_slope)  ((slope) < (min_slope) && ABS(slope) <= 2 * ABS(min_slope))
#define IS_SLOPE_IN_DOUBLE_MAX_SECTION(slope, min_slope, max_slope)  ((slope) > (max_slope) && ABS(slope) >= 2 * ABS(max_slope))
#define PASSENGER_FLOW_RATIO(mole, deno)  ((mole) / (deno))
#define IS_VALUE_IN_SECTION(val, val_min, val_max)  ((val) >= (val_min) && (val) <= (val_max))
#define IS_GREATER_THAN_ALL(val, src1, src2)  ((val) > (src1) && (val) > (src2))
#define PREDICT_NUM 2     // 预测流中属于预测数据的个数
#define REAL_FLOW_NUM 4   // 预测流中真实数据的个数
#define FLOW_VOLUME_NUM 6 // 交通流的数量
#define P_GET_PASSENGER_FLOW_PREDICT_ELEM(pf, num)  ((char*)pf + (num) * sizeof(FlowPredictArrInfo))
#define P_GET_PASSENGER_FLOW_ELEM(pf, num)  ((char*)pf + (num) * sizeof(PassengerFlowVolume))
#define PREDICT_ATTRIBUTE 0.7 // 属性重要度
#define REAL_ATTRIBUTE 0.3 // 属性重要度
typedef struct Gradient
{
	float minGradient; // 最小客流量变化率
	float maxGradient; // 最大客流量变化率
}Gradient;
typedef enum Patterns
{
	FREE,                  // 空闲交通模式
	BUSY_UPRUSH,           // 繁忙-上高峰交通模式
	BUSY_DOWNRUSH,         // 繁忙-下高峰交通模式
	BUSY_LAYER_DUPLEX,     // 繁忙-层间-两路交通模式
	BUSY_LAYER_BALANCE,	   // 繁忙-平衡层间
}Patterns;
/*
* PassengerFlowVolume为当下客流量，而当下客流量包含上一时刻剩余客流量和新增客流量
*/
typedef struct PassengerFLow
{
	PassengerFlowVolume totalPassengerNum;	    // 总客流
	PassengerFlowVolume enterPassengerNum;      // 进门厅客流
	PassengerFlowVolume outPassengerNum;        // 出门厅客流
	PassengerFlowVolume totalLayerPassengerNum; // 总层间客流
	PassengerFlowVolume maxLayerPassengerNum;   // 最大层间客流
	PassengerFlowVolume otherLayerPassengerNum; // 其他层间客流
	Patterns patterns;							// 交通模式
}PassengerFLow;
typedef struct FlowPredictArrInfo
{
	float arr[REAL_FLOW_NUM + PREDICT_NUM]; // 前段为真实数据，后段为预测数据
	int predictIndex;                       // 从该索引往后的数据皆为预测数据
}FlowPredictArrInfo;
typedef struct PassengerFlowPredict
{
	FlowPredictArrInfo totalFlowPredictArrInfo;
	FlowPredictArrInfo enterFlowPredictArrInfo;
	FlowPredictArrInfo outFlowPredictArrInfo;
	FlowPredictArrInfo totalLayerFlowPredictArrInfo;
	FlowPredictArrInfo maxLayerFlowPredictArrInfo;
	FlowPredictArrInfo otherLayerFlowPredictArrInfo;
}PassengerFlowPredict;
// 交通模式识别初始化
void PatternInit(PassengerFlowVolume* pPassengerFlowVolume);
// 交通流预测
PassengerFlowPredict* FlowPredict(PassengerFLow* pPassengerFlow, PassengerFlowPredict* pPassengerFlowPredict);
// 交通模式识别
Patterns PatternJudge(PassengerFLow* pPassengerFLow, PassengerFlowPredict* pPassengerFlowPredict, ElevatorParam* pElevatorParam, BuildingParam* pBuildingParam);
#endif