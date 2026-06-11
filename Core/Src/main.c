/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 定义舵机
typedef struct {
  TIM_HandleTypeDef* htim;
  uint32_t channel;
} Servo_t;

Servo_t servo_A0 = {&htim1, TIM_CHANNEL_1};
Servo_t servo_A1 = {&htim1, TIM_CHANNEL_2};

// 数组用来保存遥控器接收值
int datachan[10];
int x, y, chx;                      // 遥控接收程序中用到的变量
int channel1;                       // 通道1
int channel2;                       // 通道2
int channel3;                       // 通道3
int channel4;                       // 通道4
int channel5;                       // 通道5
int channel6;                       // 通道6
int channel7;                       // 通道7
int pulsewidth_0;                   // 舵机控制函数舵机的脉冲宽500-2500
int pulsewidth_1;                   // 舵机控制函数舵机的脉冲宽500-2500
int servoC_0;                       // 舵机A0中间值一般在1500左右
int servoC_1;                       // 舵机A1中间值一般在1500左右
int fd, pj, cs, sj, c5, c6;         // 幅度,偏角,差速,升降,c5c6用于微调舵机初始位置。
int hx;                             // 滑翔角度
float ys;                           // 延时,
int fly = 0;                        // 起飞标志位

// PPM信号解析变量
volatile uint32_t last_capture = 0;
volatile uint8_t channel_count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void dataget(void);
void do_sv(void);
void writeMicroseconds(Servo_t* servo, int pulse);
int abs_val(int x);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// abs函数,控制遥控器死区
int abs_val(int x) {
  return (x < 0) ? -x : x;
}

// 舵机控制函数 - 替代Arduino的writeMicroseconds
void writeMicroseconds(Servo_t* servo, int pulse) {
  __HAL_TIM_SET_COMPARE(servo->htim, servo->channel, pulse);
}

// 通过控制脉冲宽度来控制舵机的角度
void do_sv() {
  writeMicroseconds(&servo_A0, pulsewidth_0);
  writeMicroseconds(&servo_A1, pulsewidth_1);
}

// 获取ppm函数 - 替代原来的pulseIn
void dataget() {
  // 这里简化处理，直接使用中断捕获的数据
  for(x = 0; x < 8; x++) {
    // datachan数组会在中断中被填充
  }
}

// map函数实现
int map_func(int value, int in_min, int in_max, int out_min, int out_max) {
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// 延时函数（微秒级）
void delayMicroseconds(int us) {
  // 简单的延时实现
  volatile int count = us * 18; // 大概的延时循环
  while(count--);
}

// 串口重定向
int _write(int file, char *ptr, int len) {
  HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    //Serial.begin(115200);               //开启后可进行串口调试
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);   // 舵机端口A0
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);   // 舵机端口A1

    servoC_0 = 1500;                            // 舵机A0角度初始值
    servoC_1 = 1500;                            // 舵机A1角度初始值
    fd = 500;                                   // 幅度初始值500
    ys = 10000;                                 // 延时初始值10000单位微秒
    pj = 0;                                     // 偏角初始0
    cs = 0;                                     // 差速初始0
    sj = 0;                                     // 升降初始0

    pulsewidth_0 = servoC_0;                    // 舵机A0脉冲宽度设为初始值
    pulsewidth_1 = servoC_1;                    // 舵机A0脉冲宽度设为初始值
    writeMicroseconds(&servo_A0, pulsewidth_0); // 输出相应脉冲宽度控制舵机A0
    writeMicroseconds(&servo_A1, pulsewidth_1); // 输出相应脉冲宽度控制舵机A1
    HAL_Delay(3000);                            // 等待三秒

    // 启动PPM输入捕获
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      // 主程序重复执行 - 对应Arduino的loop()
    dataget();                                        // 采集接收机返回值
    channel1 = datachan[0];                          // 将1通道返回值保存在变量channel1
    channel2 = datachan[1];                          // 将2通道返回值保存在变量channel2
    channel3 = datachan[2];                          // 将3通道返回值保存在变量channel3
    channel4 = datachan[3];                          // 将4通道返回值保存在变量channel4
    channel5 = datachan[4];                          // 将5通道返回值保存在变量channel5
    channel6 = datachan[5];                          // 将6通道返回值保存在变量channel6
    channel7 = datachan[6];                          // 将71通道返回值保存在变量channel7

    //pj=map(channel4,595,1595,-250,250);                       //映射通道四的值用于控制偏角左倾或右倾 此处仿生蝴蝶不用扑翼机可以用
    sj = map_func(channel2, 595, 1595, -100, 100);
    if(abs_val(sj) < 5) sj = 0;      // 映射通道2 的值给变量sj 用于控制蝴蝶升降 sj范围是-100到100
    ys = map_func(channel3, 595, 1595, 10000, 8500);       // 映射通道3 的值给变量ys 用于控制蝴蝶扑翼频率 ys范围是10000到8500
    cs = map_func(channel1, 595, 1595, -150, 150);
    if(abs_val(cs) < 5) cs = 0;       // 映射通道1 的值给变量cs 用于控制蝴蝶两侧幅度差异 可以控制转向 cs范围是-150到150
    c5 = map_func(channel5, 595, 1595, 0, 200);           // 映射通道5 的值给变量c5 用于控制A0舵机初始位置
    c6 = map_func(channel6, 595, 1595, 0, 200);           // 映射通道6 的值给变量c6 用于控制A1舵机初始位置
    servoC_0 = 1500 + c5;                                          // 通过旋钮来微调舵机A0初始位置
    servoC_1 = 1500 - c6;                                          // 通过旋钮来微调舵机A1初始位置

    if(channel7 > 1200)                                            // 通道7为三挡  分别控制三种幅度分别是  600  700  800
        fd = 400;                                                    //
    else if(700 < channel7 && channel7 < 1200)                       //
        fd = 450;                                                    //
    else if(channel7 < 700)
        fd = 500;                              //

    if(ys > 9000)                                                    // 当通道3在最下端时映射ys的值为10000左右  这时候fly为0 当ys小于9000时fly为1，启动
        fly = 0;                                                            //
    else if(ys < 9100)
        fly = 1;                                                     //

    if(!fly) {                                                         // fly为0时不启动
        pulsewidth_0 = servoC_0 - sj - cs * 2;                                         //
        pulsewidth_1 = servoC_1 + sj - cs * 2;                                            // 在不启动时只能通过通道2来控制升降cs在静止时控制倾角
        do_sv();                                                    // 舵机按照上面的值进行执行
    }

    if(fly) {                                                           // fly为1时启动
        // 将扑翼的半个周期分为18份分别是 cos0 cos10 cos20 cos30...cos170
        for (int i = 0; i < 18; i++) {
            // y=Acos(wx）+h     其中A控制了余弦函数的幅度  h控制了余弦函数的0点位置
            pulsewidth_0 = (servoC_0 - sj) - (fd + cs) * cos((10 * i) / 180.0 * 3.14);
            pulsewidth_1 = (servoC_1 + sj) + (fd - cs) * cos((10 * i) / 180.0 * 3.14);     // 通过改变参数来实现控制舵机扑翼的各种动作
            do_sv();                                                         // 将计算好的数值即脉冲宽度给舵机 执行相应的角度
            delayMicroseconds((int)ys);            // ys越小 半个周期的时间越短 速度就越快实现了通道3控制频率
        }                                                                  //

        for (int i = 18; i > 0; i--) {
            pulsewidth_0 = (servoC_0 - sj) - (fd + cs) * cos((10 * i) / 180.0 * 3.14);               //
            pulsewidth_1 = (servoC_1 + sj) + (fd - cs) * cos((10 * i) / 180.0 * 3.14);               //
            do_sv();                                                            //
            delayMicroseconds((int)ys);                      //
        }
    }

    //Serial.println(pj);//偏角
    //Serial.println(ys);//延时
    //Serial.println(fd);//幅度
    //Serial.println(cs);//差速
    //Serial.println(sj);//升降
    //Serial.println(hx);//滑翔角度
    //Serial.println(pulsewidth_0);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// PPM输入捕获中断回调 - 替代原来的pulseIn函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t pulse_width;

        if (current_capture >= last_capture) {
            pulse_width = current_capture - last_capture;
        } else {
            pulse_width = (0xFFFF - last_capture) + current_capture;
        }

        if (pulse_width > 5000) {  // 结束脉冲较大一般大于5000
            channel_count = 0;
        } else if (channel_count < 8) {
            datachan[channel_count++] = pulse_width;  // 将这8个脉冲分别保存在数组中
        }

        last_capture = current_capture;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
