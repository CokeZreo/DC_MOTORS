#ifndef _ADC_Detection_h
#define _ADC_Detection_h

#include "main.h"

uint16_t GET_ADC1_VALUE(void);                                                  //获取ADC1单通道采集值
uint16_t GET_ADC2_VALUE(void);                                                  //获取ADC2单通道采集值

uint16_t GET_ADC2_VALUE_AVG(uint8_t AvgNum);   /* ← 新增 */

float GET_ADC1_VALUE_AVG(uint8_t AVG_NUM);                                      //获取ADC1单通道采集值N次并取平均值


uint16_t GET_ADC1_Polling_VALUE(uint8_t x);                                     //获取ADC1多通道轮询模式采集值
uint16_t GET_ADC2_Polling_VALUE(uint8_t x);                                     //获取ADC2多通道轮询模式采集值

float GET_ADC1_Polling_VALUE_AVG(uint8_t x,uint8_t AVG_NUM);                    //获取ADC1多通道轮询模式采集值N次并取平均值
float GET_ADC2_Polling_VALUE_AVG(uint8_t x,uint8_t AVG_NUM);                    //获取ADC2多通道轮询模式采集值N次并取平均值

void ADC_DMA(void);                                                             //ADC开启DMA传输

void ADC_DMA(void);                                                             //ADC开启DMA

uint16_t GET_ADC_VOL_VALUE_OVERSAMPLE(uint8_t OVERSAMPLE_NUM);                  //获取电压采集过采样值
uint16_t GET_ADC_CUR_VALUE_OVERSAMPLE(uint8_t OVERSAMPLE_NUM);                  //获取电流采集过采样值
uint16_t GET_ADC_TEMP_VALUE_OVERSAMPLE(uint8_t OVERSAMPLE_NUM);                 //获取温度采集过采样值

float ADC_TO_VOL(uint16_t x);                                  //ADC位数转化为电压值

void GET_VOL_CUR_REF(void);                                    //获取电压、电流ADC检测基准值

float GET_ACT_CUR(void);                                       //计算实际采样电流值
float GET_ACT_TEMP(void);                                      //计算NTC实时温度值（℃）

#endif
