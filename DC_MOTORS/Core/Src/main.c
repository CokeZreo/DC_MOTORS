/* ============================================================================
 *  DC_MOTORS  main.c —— 最终版逐行注释（逻辑与你烧录通过的版本完全一致）
 *  编码 UTF-8
 *  硬件连接：
 *    PB1  → Q1 半桥A 上管  TIM3_CH4   正转 PWM
 *    PB0  → Q2 半桥A 下管  GPIO       低电平导通（XJNG2103 的 LIN 低有效）
 *    PB11 → Q3 半桥B 上管  TIM2_CH4   反转 PWM（TIM2 部分重映射）
 *    PB10 → Q4 半桥B 下管  GPIO       低电平导通
 *    PB14/PB15 → 方向指示灯（接 VCC，低电平点亮）
 *    PA5  → ADC1_IN5 电压检测（电阻分压 4:1）
 *    PA6  → ADC2_IN6 电流检测（10mΩ 分流 + 20 倍放大 + 1.2V 偏置）
 * ========================================================================== */

/* ======================= 头文件 ======================= */

#include "main.h"                       /* HAL 总头：stm32f1xx.h、Error_Handler 声明 */
#include "adc.h"                        /* ADC1/ADC2 初始化函数与 hadc1/hadc2 句柄 */
#include "tim.h"                        /* TIM2/TIM3 初始化函数与 htim2/htim3 句柄 */
#include "gpio.h"                       /* MX_GPIO_Init()：引脚初始化 */

/* USER CODE BEGIN Includes */
#include "key.h"                        /* KEY_Scan()：按键扫描函数 */
#include "ADC_Detection.h"              /* GET_ADC1_VALUE_AVG / GET_ADC2_VALUE_AVG */
#include "OLED.h"                       /* OLED_Init / ShowString / ShowChinese / Refresh */
#include <stdio.h>                      /* sprintf()：把数字格式化成字符串 */
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* USER CODE BEGIN PD */

#define PWM_PERIOD        3600          
#define DUTY_MIN          20            
#define DUTY_MAX          90            
#define DUTY_STEP         10            

#define OLED_REFRESH_MS   50            
#define CUR_FILTER_SHIFT  1             

#define ADC_VREF_MV       3280UL        
#define CUR_V0_MV         1220.0f       
#define CUR_K             5.00f         

/* USER CODE END PD */

/* USER CODE BEGIN PM */
/* USER CODE END PM */



/* USER CODE BEGIN PV */

uint8_t  Key_STATE = 0;                 
uint8_t  Dir       = 0;                 
uint32_t Duty      = DUTY_MIN;          

uint16_t adc_v     = 0;                 
uint16_t adc_i     = 0;                 
uint32_t Volt      = 0;                 
uint32_t oledTick  = 0;                 

/* USER CODE END PV */



void SystemClock_Config(void);          /* CubeMX 生成的时钟配置函数 */

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */



/* USER CODE BEGIN 0 */

static int32_t s_CurShow  = 0;          
static uint8_t s_CurFirst = 0;          

               
static void OLED_ShowDashboard(uint32_t VoltMv, int32_t CurrX100, uint32_t DutyPct, uint8_t DirNow)
{
    char Line[24];                      

    /* ---------- 第 1 行 y=0：电压: 12.34V ---------- */
    OLED_ShowChinese(0,  0, CN_DIAN, 16, 1);        
    OLED_ShowChinese(16, 0, CN_YA,   16, 1);        
    OLED_ShowString(32, 0, (uint8_t *)":", 16, 1);  
    sprintf(Line, "%3d.%02dV",
            (int)(VoltMv / 1000),                   
            (int)((VoltMv % 1000) / 10));           
    OLED_ShowString(40, 0, (uint8_t *)Line, 16, 1); 

    /* ---------- 第 2 行 y=16：电流:205.37mA ---------- */
    OLED_ShowChinese(0,  16, CN_DIAN, 16, 1);       
    OLED_ShowChinese(16, 16, CN_LIU,  16, 1);       
    OLED_ShowString(32, 16, (uint8_t *)":", 16, 1); 
    sprintf(Line, "%3d.%02dmA",
            (int)(CurrX100 / 100),                  
            (int)(CurrX100 % 100));                 
    OLED_ShowString(40, 16, (uint8_t *)Line, 16, 1);

    /* ---------- 第 3 行 y=32：PWM : 20% ---------- */
    OLED_ShowString(4,  32, (uint8_t *)"PWM", 16, 1);   
    OLED_ShowString(32, 32, (uint8_t *)":", 16, 1);     
    sprintf(Line, "%2d%%", (int)DutyPct);               
    OLED_ShowString(48, 32, (uint8_t *)Line, 16, 1);    

    /* ---------- 第 4 行 y=48：转向:正转 ---------- */
    OLED_ShowChinese(0,  48, CN_ZHUAN, 16, 1);      
    OLED_ShowChinese(16, 48, CN_XIANG, 16, 1);      
    OLED_ShowString(32, 48, (uint8_t *)":", 16, 1); 

    if (DirNow == 1)                                
    {
        OLED_ShowChinese(40, 48, CN_ZHENG, 16, 1);  
        OLED_ShowChinese(56, 48, CN_ZHUAN, 16, 1);  
    }
    else if (DirNow == 2)                           
    {
        OLED_ShowChinese(40, 48, CN_FAN,  16, 1);   
        OLED_ShowChinese(56, 48, CN_ZHUAN, 16, 1);  
    }
    else                                            
    {
        OLED_ShowChinese(40, 48, CN_TING, 16, 1);   
        OLED_ShowChinese(56, 48, CN_ZHI,  16, 1);  
    }
}

/* USER CODE END 0 */



int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();                           

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  SystemClock_Config();                 

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  MX_GPIO_Init();                       
  MX_ADC1_Init();                       
  MX_ADC2_Init();                       
  MX_TIM2_Init();                       
  MX_TIM3_Init();                       

  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);  
  HAL_ADCEx_Calibration_Start(&hadc2);  
  OLED_Init();                          
  OLED_ColorTurn(0);                    
  OLED_DisplayTurn(0);                  
  OLED_Clear();                         
  /* USER CODE END 2 */

  while (1)                             
  {
    
	  
    Key_STATE = KEY_Scan(0);            


    if (Key_STATE == 1 && Duty < DUTY_MAX) Duty += DUTY_STEP;  
    if (Key_STATE == 2 && Duty > DUTY_MIN) Duty -= DUTY_STEP;  


    if (Key_STATE == 4)
    {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);               
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);               
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_SET);   
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);   
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);   
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);   
        Dir = 0;                                               
    }

    if (Key_STATE == 3)
    {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);               
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);               
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_SET);   
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);   

        HAL_Delay(10);                                          


        if (Dir == 1) Dir = 2;                                 
        else          Dir = 1;                                 
		
        Duty = DUTY_MIN;                                       

        if (Dir == 1)                                          
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_SET);    
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);  
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, PWM_PERIOD * Duty / 100); 
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);               
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);    
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  
        }
        else                                                   
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);    
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_RESET);  
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, PWM_PERIOD * Duty / 100); 
            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);               
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);  
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);    
        }
    }


    if (Dir == 1)                                              
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, PWM_PERIOD * Duty / 100);
    else if (Dir == 2)                                         
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, PWM_PERIOD * Duty / 100);

    if (HAL_GetTick() - oledTick >= OLED_REFRESH_MS)    
    {
        int32_t mv_i;                                   
        int32_t CurX100;                                

        oledTick = HAL_GetTick();                       

        adc_v = GET_ADC1_VALUE_AVG(20);                 
        adc_i = GET_ADC2_VALUE_AVG(20);                 

        Volt = (uint32_t)adc_v * ADC_VREF_MV * 4 / 4095;

        mv_i    = (int32_t)adc_i * ADC_VREF_MV / 4095;  
        CurX100 = (int32_t)((mv_i - CUR_V0_MV) * CUR_K * 100.0f); 
        if (CurX100 < 0) CurX100 = -CurX100;            
        if (CurX100 < 500) CurX100 = 0;                 

        if (s_CurFirst == 0)                            
        {
            s_CurShow  = CurX100;                       
            s_CurFirst = 1;                             
        }
        else                                            
        {
            s_CurShow += (CurX100 - s_CurShow) / (1 << CUR_FILTER_SHIFT); 
        }

        OLED_ShowDashboard(Volt, s_CurShow, (Dir == 0) ? 0 : Duty, Dir); 
        OLED_Refresh();                                 
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}



void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};           
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};           
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};         

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; 
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;            
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;   
  RCC_OscInitStruct.HSIState       = RCC_HSI_ON;            
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;            
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;     
  RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;          
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)      
  {
    Error_Handler();                                        
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; 
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;         
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;           
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;           
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) 
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;     
  PeriphClkInit.AdcClockSelection    = RCC_ADCPCLK2_DIV6;     
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();                      
  while (1)                             
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
