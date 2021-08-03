/*
 * @Author: 罗飞杰
 * @File: gm.cpp
 * @Date: 2021-7-8
 * @LastEditTime: 2021-7-15
 * @LastEditors: 罗飞杰
 * @brief: 灰度预测模型GM(1,1),该模型用于电梯乘客交通流预测
*/
#include <stdio.h>
#include <malloc.h>
#include <cmath>
#include <cstdlib>
#include "gm.h"
#include"transit.h"
/* 累加生成 */
float* GM_AGO(float* xArr, int arrSize)
{
	float* ret;
	if (arrSize > 0)
	{
		do
		{
			ret = (float*)malloc(arrSize * sizeof(float));
		} while (ret == NULL);
		ret[0] = xArr[0];
		for (int i = 1; i < arrSize; ++i)
		{
			ret[i] = ret[i - 1] + xArr[i];
		}
		return ret;
	}
	else
	{
		printf("Invalid size!");
		exit(0);
	}
}
/* 累减生成 */
float* GM_IAGO(float* xArr, int arrSize)
{
	float* ret;
	if (arrSize > 0)
	{
		do
		{
			ret = (float*)malloc(arrSize * sizeof(float));
		} while (ret == NULL);
		ret[0] = xArr[0];
		for (int i = 1; i < arrSize; ++i)
		{
			ret[i] = xArr[i] - xArr[i - 1];
		}
		return ret;
	}
	else
	{
		printf("Invalid size!");
		exit(0);
	}
}
/*			加权邻值生成
* 由于由一阶线性微分逼近,该算法存在较大的误差
* generateCoefficent: 生成系数(0.5为均值生成系数)
* note: 返回数组大小比传入数组大小要小 1
*/
float* GM_WAV_Weight(float* xArr, int arrSize, float generateCoefficent)
{
	float* ret;
	if (arrSize > 0)
	{
		do
		{
			ret = (float*)malloc((arrSize - 1) * sizeof(float));
		} while (ret == NULL);
		for (int i = 1; i < arrSize; ++i)
		{
			ret[i - 1] = generateCoefficent * xArr[i] + (1 - generateCoefficent) * xArr[i - 1];
		}
		return ret;
	}
	else
	{
		printf("Invalid size!");
		exit(0);
	}
}
/*			背景值Z(1)(k)优化
* note: 返回数组大小比传入数组大小要小 1
*/
float* GM_WAV_Optimize(float* xArr, int arrSize)
{
	float* ret;
	if (arrSize > 0)
	{
		do
		{
			ret = (float*)malloc((arrSize - 1) * sizeof(float));
		} while (ret == NULL);
		for (int i = 1; i < arrSize; ++i)
		{
			ret[i - 1] = (float)((double)(xArr[i] - xArr[i - 1]) / (log(xArr[i]) - log(xArr[i - 1])));
		}
		return ret;
	}
	else
	{
		printf("Invalid size!");
		exit(0);
	}
}

/* 数据检验处理 */
float* GM_DataProcess(float* xArr, int arrSize, double* c)
{
	*c = 0;
	double e = 2.7182;
	float r;
	double temp = (double)2 / (arrSize + 1);
	float lowerLimit = pow(e, -temp);
	float upperLimit = pow(e, temp);

	char isEnd = 1;
	while (isEnd)
	{
		isEnd = 0;
		for (int i = 1; i < arrSize; ++i)
		{
			// 数据消重
			if (xArr[i] == xArr[i - 1])
			{
				xArr[i] += 0.1; // 消重统一后者加0.1
			}
			r = (float)(xArr[i - 1] + (*c)) / (xArr[i] + (*c));
			if (r < lowerLimit || r > upperLimit)
			{
				isEnd = 1;
				*c = (xArr[i - 1] - xArr[i] > 0) ? ((xArr[i - 1] - xArr[i]) / (upperLimit - 1) - xArr[i]) : (((xArr[i] - xArr[i - 1]) / (upperLimit - 1) - xArr[i - 1]));
				break;
			}
		}
	}
	return xArr;
}

/*										预测
* 参数2:参数1的数组大小,参数3: 返回预测数组的大小(自定义),参数3: 灰色微分方程的初始条件(基准点)
*/
float* GM_Predict(float* xArr, int arrSize, int retArrSize, int datumPoint)
{
	const double e = 2.7182;
	float* ret;
	if (arrSize > 0)
	{
		double r = 0;
		GM_DataProcess(xArr, arrSize, &r);
		float* rXArr = (float*)malloc(arrSize * sizeof(float)); // 用于修正xArr
		for (int i = 0; i < arrSize; ++i)
		{
			rXArr[i] = xArr[i]; // 修正xArr;
		}
		float* temp = GM_AGO(rXArr, arrSize); // x(1)
		float* wav = GM_WAV_Optimize(temp, arrSize); // 均值生成
		float* b = (float*)malloc(((arrSize << 1) - 2) * sizeof(float));
		for (int i = 0, n = 0; i < arrSize - 1; i++, n += 2)
		{
			b[n] = -wav[i];
			b[n + 1] = 1;
		}
		Mat matOfb;
		MatCreate(&matOfb, arrSize - 1, 2);
		MatSetVal(&matOfb, b);
		Mat matOfy;
		MatCreate(&matOfy, arrSize - 1, 1);
		float* tempxArr = rXArr + 1;
		MatSetVal(&matOfy, tempxArr);
		Mat tempTrans;
		MatCreate(&tempTrans, 2, arrSize - 1);
		tempTrans = *MatTrans(&matOfb, &tempTrans);
		Mat tempMul;
		MatCreate(&tempMul, 2, 2);
		tempMul = *MatMul(&tempTrans, &matOfb, &tempMul);
		Mat tempInv;
		MatCreate(&tempInv, 2, 2);
		tempInv = *MatInv(&tempMul, &tempInv);
		Mat tempMul1;
		MatCreate(&tempMul1, 2, arrSize - 1);
		tempMul1 = *MatMul(&tempInv, &tempTrans, &tempMul1);
		Mat resultOfu;
		MatCreate(&resultOfu, 2, 1);
		resultOfu = *MatMul(&tempMul1, &matOfy, &resultOfu);
		float resultOfa = resultOfu.element[0][0];
		float resultOfb = resultOfu.element[1][0];

		float* tempRet = (float*)malloc(retArrSize * sizeof(float));
		tempRet[0] = rXArr[0];
		for (int i = 1; i < retArrSize; ++i)
		{
			tempRet[i] = (rXArr[datumPoint] - resultOfb / resultOfa) * pow(e, (double)-resultOfa * i) + resultOfb / resultOfa;
		}
		ret = GM_IAGO(tempRet, retArrSize);
		MatDelete(&matOfb);
		MatDelete(&matOfy);
		MatDelete(&tempInv);
		MatDelete(&tempMul);
		MatDelete(&tempMul1);
		MatDelete(&tempTrans);
		MatDelete(&resultOfu);
		free(rXArr);
		free(b);
		free(temp);
		free(tempRet);
		free(wav);
		return ret;
	}
	else
	{
		printf("Invalid size!");
		exit(0);
	}
}
