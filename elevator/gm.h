#pragma once
#ifndef _GM_H_
#define _GM_H_

#include"matrix.h"
/* 累加生成 */
float* GM_AGO(float* xArr, int arrSize);
/* 累减生成 */
float* GM_IAGO(float* xArr, int arrSize);
/* 加权邻值生成
* generateCoefficent: 生成系数(0.5为均值生成系数)
* note: 返回数组大小比传入数组大小要小 1
*/
float* GM_WAV_Weight(float* xArr, int arrSize, float generateCoefficent);
/*			背景值Z(1)(k)优化
* note: 返回数组大小比传入数组大小要小 1
*/
float* GM_WAV_Optimize(float* xArr, int arrSize);
/* 数据检验处理 */
float* GM_DataProcess(float* xArr, int arrSize, double* c);
/* 预测 */
float* GM_Predict(float* xArr, int arrSize, int retArrSize, int datumPoint);

#endif