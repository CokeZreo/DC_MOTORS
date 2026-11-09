/* 使用此外设驱动函数方法 */

/**
  1，在main.c中找到下面第5和第7行的代码，在他们中间复制第6行代码，调用头文件
          USER CODE BEGIN Includes
            #include "ADC_Detection.h"
          USER CODE END Includes
  
  2，在main.c文件中，int.main函数下写ADC校准函数
      a）没有ADC校准类型选择的单片机，用下面ADC校准函数
          HAL_ADCEx_Calibration_Start(&hadc1);                    //ADC1校准函数
          HAL_Delay(1);
      b）有ADC校准类型选择的单片机，用下面ADC校准函数
          HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);   //ADC1校准函数
          HAL_Delay(1);
  
  3，ADC采集，需要经过ADC开启、ADC采集等待、ADC采集获取、ADC关闭四个步骤
      其中ADC采集函数已经写好可以在main.c中直接调用，函数在本文件下有写，主要有三大类
              ADC单通道采集函数、ADC轮询模式采集函数、ADC采集值计算函数
          如果只用到ADC单通道采集函数，需要把ADC轮询模式采集函数注释掉，不然编译会报错
          如果只用到ADC1的采集函数，需要把ADC2的采集函数注释掉，不然编译也会报错
      本文件下已经把ADC采集函数打包整理成一个函数，如果需要进行ADC采集，只需要调取对应函数
          eg.GET_ADC1_VALUE();                   //获取ADC1单通道采集值
             GET_ADC1_VALUE_AVG(10);             //获取ADC1单通道采集值10次,并且取平均值
             GET_ADC1_Polling_VALUE(0);          //获取ADC1多通道采集的第1个轮询采集值
             GET_ADC1_Polling_VALUE_AVG(0,10);   //获取ADC1多通道采集的第1个轮询采集值10次,并且取平均值
             GET_VOL_CUR_REF();                  //获取电流、电压基准值，放大电路加了偏置电压后进行ADC采集的初始值
             GET_ACT_CUR();                      //计算电路实际电流值，计算方法要根据自己的电路进行修改
             ADC_TO_VOL(GET_ADC1_VALUE();        //获取到的ADC1位数值转化为电压值
  
  4，ADC采集到的值是二进制比较值要经过计算才能变成电压值，如果想知道是多少，需要通过屏幕进行显示
          uint16_t X = GET_ADC1_VALUE();          //把获取到的ADC1值赋值给变量
          OLED_ShowNum(96,0,X,4,8,1);             //用显示屏显示获取到的ADC1值
  
  注意要点
    1，在ADC采集之前，先获取ADC采集基准值，用来消除ADC采集的系统误差
        在main.c文件中，int main函数下，while循环语句前写    GET_VOL_CUR_REF();//获取基准值
    2，如果用到轮询模式，需要要改轮询采集次数，在对应的函数上有标注
        eg.找到ADC1多通道采集函数，修改获取采集ADC采集值存放数组的长度与通道数一致
    3，不同的电路，电流计算方法不同，注意在GET_ACT_CUR()函数下改公式，最后几个函数为计算函数
    4，本文件下用到的变量均已在main.c文件中定义，所以在第47行以下定义的变量会用extern进行申明
        申明过的变量，需要复制到main.c文件下定义，要把extern删掉
  */

#include "ADC_Detection.h"
#include "main.h"
#include "adc.h"
#include "math.h"

extern uint16_t ADC_VOL;        //电压检测ADC采集值
extern uint16_t ADC_CUR;        //电流检测ADC采集值

extern uint16_t ADC_VOL_REF;      //电压ADC检测基准值，ADC位数
extern uint16_t ADC_CUR_REF;    //电流ADC检测基准值，ADC位数

extern uint16_t ADC_CUR_DMA;           //开启DMA搬运,电压检测ADC采集值
extern uint16_t ADC_VOL_TEMP_DMA[2];   //开启DMA搬运,电流、温度检测ADC采集值，ADC_CUR_TEMP_DMA[0]是电流检测，ADC_CUR_TEMP_DMA[1]是温度检测

/*            下面的在本文件中没有用到，这里提供了一些变量的命名方法            */

//extern uint16_t ADC_TEMP;     //温度检测ADC采集值

//extern float ACT_VOL;         //经计算后的实时电压，单位：mV
//extern float ACT_CUR;         //经计算后的实时电流，单位：mA
//extern float ACT_TEMP;        //NTC实时温度值

/*                            DAC输出相关变量命名                              */

//extern uint16_t TIM_PWM_CVM;         //恒压模式PWM占空比输出值
//extern uint16_t TIM_PWM_CCM;         //恒流模式PWM占空比输出值
//extern uint16_t DAC_CVM;             //恒压模式DAC输出值
//extern uint16_t DAC_CCM;             //恒流模式DAC输出值

//extern float TIM_PWM_CVM_THE_VOL_OUT;     //恒压模式理论输出电压值
//extern float TIM_PWM_CCM_THE_CUR_OUT;     //恒流模式理论输出电流值
//extern float DAC_CVM_THE_VOL_OUT;         //恒压模式理论输出电压值
//extern float DAC_CCM_THE_CUR_OUT;         //恒流模式理论输出电流值

//extern float TIM_PWM_CVM_ACT_VOL_OUT;     //恒压模式实际输出电压值
//extern float TIM_PWM_CCM_ACT_VOL_OUT;     //恒压模式实际输出电流值
//extern float DAC_CVM_ACT_VOL_OUT;         //恒压模式实际输出电压值
//extern float DAC_CCM_ACT_VOL_OUT;         //恒压模式实际输出电流值

//extern uint16_t DAC_CVM_PID;              //恒压模式PID校准后DAC输出值
//extern uint16_t DAC_CCM_PID;              //恒流模式PID校准后DAC输出值

/*    extern的意思是：如果有变量已经在main.c文件中定义了，同时这个文件下也需要进行调用，用extern在这里申明此变量已经定义过了    */

/**
  * @简要     获取ADC1单通道采集值
  * @参数     无
  * @返回值   ADC1检测的ADC位数
  */
uint16_t GET_ADC1_VALUE(void)
{
  uint16_t temp;

  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1,100);      //等待规则通道转换完成,最大等待时间100 ms，这个可以填数字
  temp = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);
  
  return temp;
}

/**
  * @简要     获取ADC2单通道采集值
  * @参数     无
  * @返回值   ADC2检测的ADC位数
  */
uint16_t GET_ADC2_VALUE(void)
{
  uint16_t temp;

  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2,100);      //等待规则通道转换完成,最大等待时间100 ms，这个可以填数字
  temp = HAL_ADC_GetValue(&hadc2);
  HAL_ADC_Stop(&hadc2);
  
  return temp;
}

/**
  * @简要     获取ADC1单通道采集值N次并取平均值
  * @参数     采集次数，N
  *             比如uint16_t temp = GET_ADC1_VALUE_AVG(10);
  *               获取10次ADC采集值并取平均值
  * @返回值   ADC1检测的ADC位数
  */
float GET_ADC1_VALUE_AVG(uint8_t AVG_NUM)
{
  float Average_SUM=0,Average;
  uint8_t  j;
  for(j=0;j<AVG_NUM;j++)
  {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1,0xffff);   //等待规则通道转换完成,最大等待时间0xffff ms，这个可以填数字
    Average_SUM = HAL_ADC_GetValue(&hadc1)+Average_SUM;//多次取值累加
    HAL_ADC_Stop(&hadc1);
  }
  
  Average = Average_SUM/AVG_NUM;
  
  return Average;
}

/**
  * @简要     获取ADC2单通道采集值N次并取平均值
  * @参数     采集次数，N
  *             比如uint16_t temp = GET_ADC2_VALUE_AVG(10);
  *               获取10次ADC采集值并取平均值
  * @返回值   ADC2检测的ADC位数
  */
/* 获取 ADC2 多次采样的平均值 */
uint16_t GET_ADC2_VALUE_AVG(uint8_t AvgNum)
{
    uint32_t sum = 0;                 /* 注意用 uint32_t！ */
    uint8_t  i;

    if (AvgNum == 0) return 0;        /* 防止除以 0 */

    for (i = 0; i < AvgNum; i++)
    {
        sum += GET_ADC2_VALUE();      /* 累加 20 次 */
    }

    return (uint16_t)(sum / AvgNum);  /* 求平均 */
}

/**
  * @简要     获取ADC1多通道轮询模式采集值
  *   @注意   假设轮询了两次，电压检测ADC1通道1和电流检测ADC1通道2
  *             打开ADC1采集后，不是先检测通道1，再检测通道2，先检测通道几是可以设置的
  *               所以要先实验判断ADC检测第几个值是电压检测，第几个值是电流检测
  *                 eg.adcBuf[0]温度检测
  *                    adcBuf[1]电流检测
  * @参数     获取ADC1轮询的第N个值
  *             比如uint16_t temp = GET_ADC1_Polling_VALUE(1);
  *               获取ADC1轮询的第2个值
  * @返回值   返回ADC1轮询第N个的ADC位数
  */
uint16_t GET_ADC1_Polling_VALUE(uint8_t x)
{
  uint8_t  i;
  static uint16_t  adcBuf[2]={0};
  
  for(i=0;i<2;i++)//          如果用到轮询模式，一定要改轮询模式获取ADC轮询值
    {
      HAL_ADC_Start(&hadc1);
      HAL_ADC_PollForConversion(&hadc1,0xffff);    //等待规则通道转换完成,最大等待时间0xffff ms，这个可以填数字
      adcBuf[i]=HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    
    return adcBuf[x];
}

/**
  * @简要     获取ADC2多通道轮询模式采集值
  *   @注意   假设轮询了两次，电压检测ADC2通道1和电流检测ADC2通道2
  *             打开ADC2采集后，不是先检测通道1，再检测通道2，先检测通道几是可以设置的
  *               所以要先实验判断ADC检测第几个值是电压检测，第几个值是电流检测
  *                 eg.adcBuf[0]温度检测
  *                    adcBuf[1]电流检测
  * @参数     获取ADC2轮询的第N个值
  *             比如uint16_t temp = GET_ADC2_Polling_VALUE(1);
  *               获取ADC2轮询的第2个值
  * @返回值   返回ADC2轮询第N个的ADC位数
  */
//uint16_t GET_ADC2_Polling_VALUE(uint8_t x)//x是调用ADC第几个值
//{
//  uint8_t  i;
//  static uint16_t  adcBuf[2]={0};
//  
//  for(i=0;i<2;i++)//          如果用到轮询模式，一定要改轮询模式获取ADC轮询值
//    {
//      HAL_ADC_Start(&hadc2);
//      HAL_ADC_PollForConversion(&hadc2,0xffff);    //等待规则通道转换完成,最大等待时间0xffff ms，这个可以填数字
//      adcBuf[i]=HAL_ADC_GetValue(&hadc2);
//    }
//    HAL_ADC_Stop(&hadc2);
//    
//    return adcBuf[x];
//}

/**
  * @简要     获取ADC1多通道轮询模式采集值N次并取平均值
  *   @注意   假设轮询了两次，电压检测ADC1通道1和电流检测ADC1通道2
  *             打开ADC1采集后，不是先检测通道1，再检测通道2，先检测通道几是可以设置的
  *               所以要先实验判断ADC检测第几个值是电压检测，第几个值是电流检测
  *                 eg.adcBuf[0]温度检测
  *                    adcBuf[1]电流检测
  * @参数     获取ADC1轮询的第N个值
  * @参数     采集次数，N
  *           比如uint16_t temp = GET_ADC1_Polling_VALUE_AVG(1,10);
  *               会获取ADC1轮询的第2个值，获取10次并取平均值
  * @返回值   返回ADC1轮询第N个的ADC位数
  */
float GET_ADC1_Polling_VALUE_AVG(uint8_t x,uint8_t AVG_NUM)//x是调用ADC第几个值，AVG_NUM是多次取平均值次数
{
  float Average_SUM=0,Average;
  uint8_t  i,j;
  static uint16_t  adcBuf[2]={0};
  for(j=0;j<AVG_NUM;j++)
  {
    for(i=0;i<2;i++)//          如果用到轮询模式，一定要改轮询模式获取ADC轮询值
    {
      HAL_ADC_Start(&hadc1);
      HAL_ADC_PollForConversion(&hadc1,0xffff);    //等待规则通道转换完成,最大等待时间0xffff ms，这个可以填数字
      adcBuf[i]=HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    Average_SUM=adcBuf[x]+Average_SUM;
  }
    Average = Average_SUM/AVG_NUM;
  
    return Average;
}

/**
  * @简要     获取ADC2多通道轮询模式采集值N次并取平均值
  *   @注意   假设轮询了两次，电压检测ADC2通道1和电流检测ADC2通道2
  *             打开ADC2采集后，不是先检测通道1，再检测通道2，先检测通道几是可以设置的
  *               所以要先实验判断ADC检测第几个值是电压检测，第几个值是电流检测
  *                 eg.adcBuf[0]温度检测
  *                    adcBuf[1]电流检测
  * @参数     获取ADC2轮询的第N个值
  * @参数     采集次数，N
  *           比如uint16_t temp = GET_ADC2_Polling_VALUE_AVG(1,10);
  *               会获取ADC2轮询的第2个值，获取10次并取平均值
  * @返回值   返回ADC2轮询第N个的ADC位数
  */
//float GET_ADC2_Polling_VALUE_AVG(uint8_t x,uint8_t AVG_NUM)//x是调用ADC第几个值，AVG_NUM是多次取平均值次数
//{
//  float Average_SUM=0,Average;
//  uint8_t  i,j;
//  static uint16_t  adcBuf[2]={0};
//  for(j=0;j<AVG_NUM;j++)
//  {
//    for(i=0;i<2;i++)//          如果用到轮询模式，一定要改轮询模式获取ADC轮询值
//    {
//      HAL_ADC_Start(&hadc2);
//      HAL_ADC_PollForConversion(&hadc2,0xffff);    //等待规则通道转换完成,最大等待时间0xffff ms，这个可以填数字
//      adcBuf[i]=HAL_ADC_GetValue(&hadc2);
//    }
//    HAL_ADC_Stop(&hadc2);
//    Average_SUM=adcBuf[x]+Average_SUM;
//  }
//    Average = Average_SUM/AVG_NUM;
//  
//    return Average;
//}

/**
  * @简要     ADC开启DMA传输
  * @参数     无
  * @返回值   无
  */
//void ADC_DMA(void)
//{
//  HAL_ADC_Start_DMA(&hadc2,(uint32_t *)&ADC_CUR_DMA,1);           //ADC2开启了一个通道，ADC_CUR_DMA定义的是一个变量
//  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_VOL_TEMP_DMA,2);       //ADC2开启了一个通道，ADC_VOL_TEMP_DMA定义的是一个数组
//}

/**
  * @简要     电压采集过采样设置
  *   @注意   需用DMA进行ADC采集
  * @参数     过采样N位
  *             比如uint16_t ADC_VOL = GET_ADC_VOL_VALUE_OVERSAMPLE(4);
  *                获取电压采集的过采样值，ADC过采样位数比以前大4位
  * @返回值   返回过采样之后的ADC位数
  */
//uint16_t GET_ADC_VOL_VALUE_OVERSAMPLE(uint8_t OVERSAMPLE_NUM)
//{
//  uint32_t ADC_OVERSAMPLE_TIMES = pow(4 , OVERSAMPLE_NUM);    //ADC过采样次数, 相当于是4的n次方
//  uint32_t ADC_DMA_BUF_SIZE = ADC_OVERSAMPLE_TIMES * 10;      //ADC单次采集次数， 是过采样次数的10倍
//  
//  uint32_t OVERSAMPLE_SUM=0;
//  uint32_t OVERSAMPLE;
//  
//  uint16_t  j;
//  for(j=0;j<ADC_DMA_BUF_SIZE;j++)
//  {

//    OVERSAMPLE_SUM += ADC_VOL_TEMP_DMA[0];                    //多次取值累加
////    HAL_Delay(1);                                             //延时一段时间是为了给DMA搬运留够时间，DMA搬运速度没那么快
//  }
//  
//  OVERSAMPLE = OVERSAMPLE_SUM / (ADC_DMA_BUF_SIZE / ADC_OVERSAMPLE_TIMES);
//  OVERSAMPLE >>= OVERSAMPLE_NUM;                  /* 除以2^OVERSAMPLE_NUM倍, 得到12+OVERSAMPLE_NUM位 ADC精度值
//                                                     注意: 提高 N bit精度, 需要 >> N */
//  
//  return OVERSAMPLE;
//}

/**
  * @简要     电流采集过采样设置
  *   @注意   需用DMA进行ADC采集
  * @参数     过采样N位
  *             比如uint16_t ADC_CUR = GET_ADC_CUR_VALUE_OVERSAMPLE(4);
  *                获取电流采集的过采样值，ADC过采样位数比以前大4位
  * @返回值   返回过采样之后的ADC位数
  */
//uint16_t GET_ADC_CUR_VALUE_OVERSAMPLE(uint8_t OVERSAMPLE_NUM)
//{
//  uint32_t ADC_OVERSAMPLE_TIMES = pow(4 , OVERSAMPLE_NUM);    //ADC过采样次数, 相当于是4的n次方
//  uint32_t ADC_DMA_BUF_SIZE = ADC_OVERSAMPLE_TIMES * 10;      //ADC单次采集次数， 是过采样次数的10倍
//  
//  uint32_t OVERSAMPLE_SUM=0;
//  uint32_t OVERSAMPLE;
//  
//  uint16_t  j;
//  for(j=0;j<ADC_DMA_BUF_SIZE;j++)
//  {
//    OVERSAMPLE_SUM += ADC_CUR_DMA;                            //多次取值累加
////    HAL_Delay(1);                                             //延时一段时间是为了给DMA搬运留够时间，DMA搬运速度没那么快
//  }
//  
//  OVERSAMPLE = OVERSAMPLE_SUM / (ADC_DMA_BUF_SIZE / ADC_OVERSAMPLE_TIMES);
//  OVERSAMPLE >>= OVERSAMPLE_NUM;                  /* 除以2^OVERSAMPLE_NUM倍, 得到12+OVERSAMPLE_NUM位 ADC精度值
//                                                     注意: 提高 N bit精度, 需要 >> N */
//  
//  return OVERSAMPLE;
//}

/**
  * @简要     温度采集过采样设置
  *   @注意   需用DMA进行ADC采集
  * @参数     过采样N位
  *             比如uint16_t ADC_TEMP = GET_ADC_TEMP_VALUE_OVERSAMPLE(4);
  *                获取温度采集的过采样值，ADC过采样位数比以前大4位
  * @返回值   返回过采样之后的ADC位数
  */
//uint16_t GET_ADC_TEMP_VALUE_OVERSAMPLE(uint8_t OVERSAMPLE_NUM)
//{
//  uint32_t ADC_OVERSAMPLE_TIMES = pow(4 , OVERSAMPLE_NUM);    //ADC过采样次数, 相当于是4的n次方
//  uint32_t ADC_DMA_BUF_SIZE = ADC_OVERSAMPLE_TIMES * 10;      //ADC单次采集次数， 是过采样次数的10倍
//  
//  uint32_t OVERSAMPLE_SUM=0;
//  uint32_t OVERSAMPLE;
//  
//  uint16_t  j;
//  for(j=0;j<ADC_DMA_BUF_SIZE;j++)
//  {
//    OVERSAMPLE_SUM += ADC_VOL_TEMP_DMA[1];                    //多次取值累加
////    HAL_Delay(1);                                             //延时一段时间是为了给DMA搬运留够时间，DMA搬运速度没那么快
//  }
//  
//  OVERSAMPLE = OVERSAMPLE_SUM / (ADC_DMA_BUF_SIZE / ADC_OVERSAMPLE_TIMES);
//  OVERSAMPLE >>= OVERSAMPLE_NUM;                  /* 除以2^OVERSAMPLE_NUM倍, 得到12+OVERSAMPLE_NUM位 ADC精度值
//                                                     注意: 提高 N bit精度, 需要 >> N */
//  
//  return OVERSAMPLE;
//}

/**
  * @简要     ADC位数转化为电压值
  * @参数     无
  * @返回值   电压值，单位：mV
  */ 
float ADC_TO_VOL(uint16_t x)
{
  float VOL;
  
  VOL = (x + 1) / pow(2 , 12) * 3.3 * 1000;    //12为ADC位数，1<<12：2的12次方，3.3V为ADC供电峰峰值
  
  return VOL;
}

/**
  * @简要     获取电压、电流ADC检测基准值
  *   @注意   绘制原理图时加了基准电压，应该先获取基准电压的值，作为回路电压、电流计算的参考值
  *             即使没有基准电压，也会有各种不理想参数导致的ADC采集值不稳，比如：运放的Vos
  *               需要在main.c文件中，int main主函数下，while语句前调用，上电先获取基准值，作为电流等计算的参考值
  * @参数     无
  * @返回值   无
  */
//void GET_VOL_CUR_REF(void)
//{
//  ADC_CUR_REF = GET_ADC2_Polling_VALUE(1);   //定义电流检测基准电压的ADC位数，ADC2轮询模式的第2个值为电流检测
//}

/**
  * @简要     计算实际采样电流值
  *   @注意   获取到的ADC位数经过计算才能得到实际电流值，下面是计算方法
  * @参数     无
  * @返回值   电流值，单位：mA
  */ 
//float GET_ACT_CUR(void)
//{
//  uint16_t CUR_DATA;
//  float CUR_ACT,CUR_ACT_V,temp;
//  
//  CUR_DATA = GET_ADC2_Polling_VALUE(1);             //获取实际电流检测值的ADC位数，ADC2轮询模式的第2个值为电流检测
//  temp = CUR_DATA-ADC_CUR_REF;                      //检测点的ADC位数=实时电流检测ADC位数-电流检测基准ADC位数
//  CUR_ACT_V = ADC_TO_VOL(temp);                     //检测点的ADC位数转换为电压值
//  CUR_ACT = ((CUR_ACT_V - 1200) / 10) / 10 * 1000;  //实际电流值计算公式，可以改
//  
//  return CUR_ACT;
//}

/**
  * @简要     计算NTC实时温度值（℃）
  *   @注意   此函数使用B值法计算NTC实时温度
  *             本公式基于：NTC位于电阻分压的电源端
  * @参数     无
  * @返回值   温度值，单位：℃
  */ 
//float GET_ACT_TEMP(void)
//{
//  float ACT_TEMP,NTC_VOL,NTC_Res;
//  
//  ACT_TEMP = GET_ADC1_VALUE_AVG(1,20);                        //获取实际温度检测值的ADC位数，ADC1轮询模式的第2个值为温度检测，并且采20次取平均值
//  NTC_VOL = ADC_TO_VOL(ACT_TEMP);                             //ADC位数转换为电压值计算，此电压值为电阻分压值
//  NTC_Res = (3.3f - NTC_VOL / 1000)/(NTC_VOL / 10000)/1000;   //计算NTC实际电阻值，3.3V：分压电阻供电电压，10000：NTC串联电阻10KΩ
//  
//  ACT_TEMP = 1/((log(10/NTC_Res))/3380+1/298.15f)-273.15f;    //计算NTC实时温度值，℃
//                                                              //10：25°时NTC的阻值10KΩ；3380：NTC的B值
//                                                              //298.15：摄氏度25°时的开尔文温度；273.15：摄氏度 = 开尔文温度 - 273.15
//  
//  return ACT_TEMP;
//}
