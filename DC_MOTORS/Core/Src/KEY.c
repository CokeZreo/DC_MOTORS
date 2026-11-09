/* 使用此外设驱动函数方法 */

/**
  1，修改key.h中第7、8、9、10、11行，按键1、按键2等按键的IO口
      设置按键的IO口要在STM32CubeMx中使能
      
  2，在main.c中找到下面第5和第7行的代码，在他们中间复制第6行代码，调用头文件
          USER CODE BEGIN Includes
            #include "KEY.h"
          USER CODE END Includes
  
  3，在main.c文件中，int.main主函数前，定义获取按键值的变量
      在main.c找到第11和第13行，在他们中间复制第12行代码
          USER CODE BEGIN PM
            uint8_t Key_STATE = 0;    //获取按键按下的返回值
          USER CODE END PM
  
  4，下面是按键按下判断函数，在main.c文件中，主函数int main下，while语句里面复制下方函数
          Key_STATE = KEY_Scan(0);  //获取按键按下的返回值，参数为是否支持连按

          if(Key_STATE == 1)
          {
            //按键1按下想执行某些代码在这里写
          }
          
          if(Key_STATE == 2)
          {
            //按键2按下想执行某些代码在这里写
          }
          
          if(Key_STATE == 3)
          {
            //按键3按下想执行某些代码在这里写
          }
          
          if(Key_STATE == 4)
          {
            //按键4按下想执行某些代码在这里写
          }
          
          if(Key_STATE == 5)
          {
            //按键5按下想执行某些代码在这里写
          }
          
          HAL_Delay(500);//延时去抖动
  
  注意要点
    1，删除添加按键数量要修改KEY.h文件和本文件下第116、124、126行
        KEY.h文件中有备注如何修改
    2，STM32CubeMx中自带的变量类型有宏定义，被重复命名，也是为了方便代码的维护统一标准
          如：字符型：uint8_t（char）
              整形：  短整型uint16_t（short int）、整形uint32_t（int）
              浮点型：单精度float、双精度double
          类型转换：有两种方法
              a）隐式转换
                  int i = 10;
                  double d = 5.5;
                  double result = i + d; // int类型的i被隐式转换为double类型
              b）显式转换
                  double d = 5.5;
                  int i = (int)d; // 将double类型的d显式转换为int类型
           定义变量类型在stdint.h等效替换
                  typedef unsigned           char u8;
                  typedef unsigned short     int u16;
                  typedef unsigned           int u32;
    3，用户自定义代码写的时候要写到main.c文件下的，USER CODE BEGIN XX和USER CODE END XX之间
          否则CubeMx重新生成代码会覆盖用户自己写的代码

    4，定时器输出的方波要通过软件修改占空比，修改占空比方法如下，以定时器的ARR=1199为例
      a）定义变量，通过按键来控制变量，进而控制输出PWM的占空比
            USER CODE BEGIN PM
          u16 PWM=599;//调节PWM变量大小，可以修改方波占空比
            USER CODE END PM
      b）通过按钮控制PWM变量，写在main.c文件中，int main主函数下，while语句里面
          if(KEY2==0)
          {
            PWM = PWM+120;
            
            if(PWM>1100)              //软件防护，占空比不能达到100%
            {
              PWM = 1079;
            }
          }
          
          if(KEY3==0)
          {
            PWM = PWM-119;
            if(PWM<100)              //软件防护，占空比不能达到0%
            {
              PWM = 119;
            }
          }
          
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, PWM);//占空比赋值函数，PWM赋值给定时器3通道1
          HAL_Delay(500);           //延时去抖动
  */

#include "KEY.h"
#include "main.h"

/**
  * @简要     获取按键，按键扫描函数
  * @参数     该函数是否支持按键连按
  *             mode:0,不支持连续按;1,支持连续按;
  * @返回值   返回ADC1轮询第N个的ADC位数
  *               1，KEY1按下
  *               2，KEY2按下
  *               3，KEY3按下
  *               4，KEY4按下 
  *               5，KEY5按下
  */
uint8_t KEY_Scan(uint8_t mode)
{
  static uint8_t key_up=1;  //按键按松开标志
  if(mode)key_up=1;         //支持连按
  if(key_up&&(KEY1==0||KEY2==0||KEY3==0||KEY4==0))       //删除添加按键，这一行的KEY5==0要对应删除添加
  {
    HAL_Delay(10);//去抖动 
    key_up=0;
    if(KEY1==0)return KEY1_PRES;
    else if(KEY2==0)return KEY2_PRES;
    else if(KEY3==0)return KEY3_PRES;
    else if(KEY4==0)return KEY4_PRES;                    //删除添加按键，这一按键按下的语句要对应删除添加
  }
  else if(KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1)key_up=1; //删除添加按键，这一行的KEY5==1要对应删除添加
  return 0;// 无按键按下
}

/*                            定时器开启关闭函数                             */
/**
  *     HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);           //定时器3通道1输出开启
  *     HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);            //定时器3通道2输出关闭
  */
/*                         定时器互补输出开启关闭函数                        */
/**
  *     HAL_TIMEx_PWMN_Start(&htim1,TIM_CHANNEL_2);         //定时器1通道2互补输出开启
  *     HAL_TIMEx_PWMN_Stop(&htim1,TIM_CHANNEL_2);          //定时器1通道2互补输出关闭
  */
/*                             定时器PWM输出函数                            */
/**
  *     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, PWM);  //占空比赋值函数，pwm赋值给定时器3通道1
  */
/*                              DAC开启关闭函数                             */
/**
  *     HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);               //负载仪开启DAC1通道1
  *     HAL_DAC_Stop(&hdac1, DAC_CHANNEL_2);                //负载仪关闭DAC1通道2
  */
/*                                DAC输出函数                               */
/**
  *     HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, CUR_DACNUM); //DAC赋值函数，CUR_DACNUM赋值给DAC1通道1
  */
/*                        普通GPIO引脚高低电平输出函数                       */
/**
  *    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);       //PA11引脚高电平
  *    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);     //PA12引脚低电平
  */
