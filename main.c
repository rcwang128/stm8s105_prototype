// C Compiler for STM8 (COSMIC Software)
// Parser V4.10.2 - 02 Nov 2011
// Generator (Limited) V4.3.7 - 29 Nov 2011

#include "stm8s.h"
#include "stdio.h"

#define UART2_PRINT

#ifdef UART2_PRINT
	#ifdef _RAISONANCE_
	#define PUTCHAR_PROTOTYPE int putchar (char c)
	#define GETCHAR_PROTOTYPE int getchar (void)
	#elif defined (_COSMIC_)
	#define PUTCHAR_PROTOTYPE char putchar (char c)
	#define GETCHAR_PROTOTYPE char getchar (void)
	#else /* _IAR_ */
	#define PUTCHAR_PROTOTYPE int putchar (int c)
	#define GETCHAR_PROTOTYPE int getchar (void)
	#endif /* _RAISONANCE_ */
#endif

void CLK_Config(void);
void GPIO_Config(void);
void UART2_Config(void);
void TIM1_Config(void); // Input capture - channel 2
void TIM2_Config(void); // Set sample resolution
void TIM3_Config(void); // Test freq output
void TIM4_Config(void); // Delay function

// Define sample resolution (Hz) = 5Hz
// Min = 0.5Hz
extern unsigned int SAMP_RES = 5;


uint16_t msTime = 0;
void msDelay(uint16_t msTime);

extern uint16_t overflow_count_tim1 = 0;
extern uint16_t overflow_count_tim2 = 0;
extern uint16_t overflow_count_tim4 = 0;
extern unsigned int freq = 0;
extern unsigned int cap = 0;

void main(void)
{
	CLK_Config();
	GPIO_Config();
	UART2_Config();
	TIM3_Config(); // Test freq output
	TIM4_Config(); // Delay function
	TIM2_Config(); // Input capture - channel 2
	TIM1_Config(); // Set sample resolution
	printf("Test!\n");
	printf("This is a Example.\n");
	
	//printf("float test: %f\n", 12.345);

	enableInterrupts();
	
	msDelay(500);

	while(1)
	{
		//freq = (overflow_count_tim1 + 1) * SAMP_RES;
		//printf("# of cnt = %d\n", overflow_count_tim1);
		//overflow_count_tim1 = 0;
		printf("Detected Freq = %d Hz\n", freq-3);
		printf("Cap value = %d pF\n", cap);
		GPIO_WriteReverse(GPIOD, GPIO_PIN_0);
		msDelay(1000);
	}
}

void CLK_Config(void)
{
	CLK_DeInit();
	
	CLK_HSECmd(ENABLE);
	CLK_LSICmd(DISABLE);
	CLK_HSICmd(DISABLE);
	while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == FALSE);
	
	CLK_ClockSwitchCmd(ENABLE);
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
	CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1); // 16MHz
	
	CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);

	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1, ENABLE); // Has to enable for UART2 to work?
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2, ENABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER3, ENABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);

	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU, DISABLE);
	
	//CLK_CCOConfig(CLK_OUTPUT_MASTER);
	//CLK_CCOCmd(ENABLE);
}

void UART2_Config(void)
{
	UART2_DeInit();
	UART2_Init((uint32_t)115200, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO, UART2_SYNCMODE_CLOCK_DISABLE, UART2_MODE_TXRX_ENABLE);
	UART2_Cmd(ENABLE);
}
	
void GPIO_Config(void)
{
	GPIO_DeInit(GPIOD);
	GPIO_DeInit(GPIOE);
	GPIO_DeInit(GPIOC);
	GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(GPIOE, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(GPIOC, GPIO_PIN_2, GPIO_MODE_IN_FL_NO_IT);
}

void TIM3_Config(void)
{	
	TIM3_DeInit();
	//TIM3_TimeBaseInit(TIM3_PRESCALER_16, 999);	//16M / 16 / 1000 = 1k, f = 500Hz
	//TIM3_TimeBaseInit(TIM3_PRESCALER_16, 499); //16M / 16 / 500 = 2k, f = 1kHz
	TIM3_TimeBaseInit(TIM3_PRESCALER_16, 131); //16M / 16 / 500 = 2k, f = 1kHz
	TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
	TIM3_Cmd(ENABLE);
}

void TIM4_Config(void)
{	
	TIM4_DeInit();
	TIM4_TimeBaseInit(TIM4_PRESCALER_64, 249); //16M / 64 / 250 = 1k = 1ms
	//TIM4_ClearFlag(TIM4_FLAG_UPDATE);	
	TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
	TIM4_Cmd(ENABLE);
}

// approximately 1ms
void msDelay(uint16_t msTime)
{
	while (msTime != (overflow_count_tim4 - 1));
	overflow_count_tim4 = 0;
}

void TIM1_Config(void)
{	
	TIM1_DeInit();
	TIM1_TimeBaseInit(8, TIM1_COUNTERMODE_UP, 19, 0);	// 16M / 8 / 20 = 100k, 10us
	//TIM1_TimeBaseInit(128, TIM1_COUNTERMODE_UP, 6249, 0);	//10hz
	TIM1_ICInit(TIM1_CHANNEL_2, TIM1_ICPOLARITY_RISING, TIM1_ICSELECTION_DIRECTTI, 1, 1);
	//TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
	TIM1_ITConfig(TIM1_IT_CC2, ENABLE);
	TIM1_Cmd(ENABLE);
}

void TIM2_Config(void)
{
	TIM2_DeInit();
	TIM2_TimeBaseInit(TIM2_PRESCALER_512, 31250/SAMP_RES-1); //16M / 512 / = 31250
	TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
	TIM2_Cmd(ENABLE);
}


#ifdef UART2_PRINT
PUTCHAR_PROTOTYPE
{
  /* Write a character to the UART2 */
  UART2_SendData8(c);
  /* Loop until the end of transmission */
  while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
  return (c);
}

/**
  * @brief Retargets the C library scanf function to the USART.
  * @param None
  * @retval char Character to Read
  */
GETCHAR_PROTOTYPE
{
#ifdef _COSMIC_
  char c = 0;
#else
  int c = 0;
#endif
  /* Loop until the Read data register flag is SET */
  while (UART2_GetFlagStatus(UART2_FLAG_RXNE) == RESET);
    c = UART2_ReceiveData8();
  return (c);
}
#endif