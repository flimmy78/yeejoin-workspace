/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include <stm32f10x.h>
#include <stm32f10x_fsmc.h>
#include "board.h"
#include <stm32f10x_wwdg.h>


static void RCC_Configuration(void);
static void GPIO_Configuration(void);
static void NVIC_Configuration(void);
static void exti_config(void);
static void spi_config(void);

#if ENABLE_RAND
static void ADC_Configration(void);
static void ADC_Channel_Config(void);
#endif

void i2c_config(void);

//static void sys_fsmc_nor_mux_config(void);
#if 1==STM32_USE_FSMC_NOR_MUX2LCD
static void lcd_fsmc_nor_mux_config(void);
#endif

#if USE_STM32_IWDG
static void iwdg_init(void);
#endif


#if 0
#define board_debug(x)	rt_kprintf x
#else
#define board_debug(x)
#endif


#if RT_USING_STM32_FSMC
static void fsmc_cfg(void);
#endif

#if EM_MULTI_BASE
static void decoder_pins_cfg(void);
static void double_mbus_indication_pins_cfg(void);
#endif

#if RT_USING_TL16C554
static void tl16c554_pins_cfg(void);
#endif

//void delay_us(int xus);

/**
 * @addtogroup STM32
 */

/*@{*/


/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init()
{
	RCC_Configuration();
	GPIO_Configuration();

#if RT_USING_STM32_FSMC
	fsmc_cfg();
#endif


	NVIC_Configuration();

	/* Configure the SysTick */
	SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );

	/* rand */
#if ENABLE_RAND
	ADC_Configration();
	ADC_Channel_Config();
#endif


	rt_hw_usart_init();
	rt_console_set_device(CONSOLE_DEVICE);

	board_debug(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));

	exti_config(); /* mark by David */

	spi_config();

	board_debug(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));

#if USE_STM32_IWDG
	iwdg_init();
#endif
	board_debug(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));


	return;
}

#if USE_STM32_IWDG
/*
 * These timings are given for a 40 kHz clock but the microcontroller's internal RC frequency can vary
 * from 30 to 60 kHz.
 * t0 = 1/f40 * cnt / pre_div
 * t1 = 1/f30 * cnt / pre_div = 4/3 * t0
 * t2 = 1/f60 * cnt / pre_div = 2/3 * t0
 *
 * Prescaler divider 	PR[2:0] bits 	Min timeout (ms) RL[11:0]=0x000 	Max timeout (ms) RL[11:0]=0xFFF
 * /4 			0 		0.1 					409.6
 */

/*
 * t = (reload_val + 1) * 1/f40 / pre_div
 *
 * t0(ms)	t1(ms)	t2(ms)
 * 0.1		0.13	0.067
 * 409.6	546.13	273.07
 * 100(0x3e7)	133.3   66.67
 *
 * 30(0x12b)	40	20
 * 90(0x383)	120	60
 *
 * IWDG_RELOAD_VALUE -- max 0xfff
 *
 * t0 = 1220ms, 256分频, 191(0xbf)
 * 0xbf * 9 = 0x6B7
 */
#if EM_MULTI_BASE
#define IWDG_RELOAD_VALUE (0xbf*2)
#else
#define IWDG_RELOAD_VALUE (0xbf)
#endif

static void iwdg_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(IWDG_RELOAD_VALUE);
#if 0
	rt_kprintf("IWDG->SR:0x%x\n", IWDG->SR);
	/* 在没有iwdg没有enable时, 状态标志可能不会更新, ysh>>IWDG->SR:0x3 */
	while (SET==IWDG_GetFlagStatus(IWDG_FLAG_PVU))
		; /* nothing, 等待寄存器更新完成 */
	while (SET==IWDG_GetFlagStatus(IWDG_FLAG_RVU))
		; /* nothing, 等待寄存器更新完成 */
#endif
	/* 禁止更新寄存器应该只是禁止软件来更新, 不会停止正在进行的更新(由硬件正在进行的更新) */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);

	/* IWDG_Enable(); 延迟开启看门狗 */
	return;
}
#endif


/**
 * This is the timer interrupt service routine.
 *
 */
void rt_hw_timer_handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

#if 0 //USE_STM32_IWDG
	/* Reloads IWDG counter with value defined in the reload register */
	/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
	IWDG->KR = 0xAAAA;
#endif

#if USE_STM32_WWDG
	extern volatile int is_need_fed_wwdg;
	if (0 != is_need_fed_wwdg)
		WWDG->CR = WWDG_RELOAD_VALUE;
#endif

	rt_tick_increase();

#ifdef RT_USING_LWIP
	extern void snmp_inc_sysuptime(void);
	snmp_inc_sysuptime();
#endif

	/* leave interrupt */
	rt_interrupt_leave();
}


static void RCC_Configuration(void)
{
	uint32_t gpio_rcc = 0;

#if RT_EXT_SRAM_MULTIPLEXED
	gpio_rcc = RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
			   RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE;
#else
	gpio_rcc = RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
			   RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF
			   | RCC_APB2Periph_GPIOG;
#endif
	RCC_APB2PeriphClockCmd(gpio_rcc, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	/* Enable SPI1 GPIOB clocks, sst25vf016b */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	/* Enable SPI2 clocks, enc28j60 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* use for ade7880 i2c */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);  
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	/* si4432 */
	/* use for ade7880 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

#if ENABLE_RAND
	/* stm32 temperature */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
#endif

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/*
	* USART
	*/
#ifdef RT_USING_UART1
	/* Enable USART1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

#if RT_USE_UART1_REMAP
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
#endif
#endif

#ifdef RT_USING_UART2
#if 1==RT_USE_UART2_REMAP
	/* Enable the USART2 Pins Software Remapping */
	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
#endif
	/* Enable USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif


//#ifdef RT_USING_UART3	/* 串口3会影响I2C????????? */
#if 0
	/* Enable USART3 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#if RT_UART3_USE_DMA
	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#endif
#endif

#ifdef RT_USING_UART4
	/* Enable UART4 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
#if RT_UART4_USE_DMA
	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
#endif
#endif

#ifdef RT_USING_UART5
	/* Enable UART5 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
#endif
#if 1
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
#endif
}



static void uart_pins_cfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef RT_USING_UART1
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
#endif

#ifdef RT_USING_UART2
	/* Configure USART2 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

	/* Configure USART2 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART2_GPIO, &GPIO_InitStructure);
#endif

#ifdef RT_USING_UART3
	/* Configure USART3 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

	/* Configure USART3 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
#endif

#ifdef RT_USING_UART4
	/* Configure UART4 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = UART4_GPIO_RX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);

	/* Configure USART4 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART4_GPIO_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);
#endif

#ifdef RT_USING_UART5
	/* Configure UART5 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = UART5_GPIO_RX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART5_GPIO_RX_PORT, &GPIO_InitStructure);

	/* Configure USART5 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART5_GPIO_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART5_GPIO_TX_PORT, &GPIO_InitStructure);
#endif
}

/*
 *  PERIPHERALS	MODES	REMAP	FUNCTIONS	PINS
 *  SPI1	Mode	0	SPI1_MISO	PA6
 *  SPI1	Mode	0	SPI1_MOSI	PA7
 *  SPI1	Mode	0	SPI1_SCK	PA5
 *  USART1	Mode	0	USART1_RX	PA10
 *  USART1	Mode	0	USART1_TX	PA9
 */
/*
 * CPU中所对应SD卡接口	SD卡接口
 * 引脚号	管脚		注释	引脚号		注释	备注
 * 78	PC10		SDIO-D2	1		PC10-SDIO-D2
 * 79	PC11		SDIO-D3	2		PC11-SDIO-D3
 * 83	PD2		SDIO-CMD	3		PD2-SDIO-CMD
 * 80	PC12		SDIO-CK	5		PC12-SDIO-CK
 * 65	PC8		SDIO-D0	7		PC8-SDIO-D0
 * 66	PC9		SDIO-D1	8		PC9-SDIO-D1
 */
static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	uart_pins_cfg();

	/* led */
#if EM_MULTI_BASE
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = led1_pin;
	GPIO_Init(led1_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = led2_pin;
	GPIO_Init(led2_gpio, &GPIO_InitStructure);

	led_off(led1_gpio, led1_pin);
	led_off(led2_gpio, led2_pin);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = s2p_74hc164_clk_pin;
	GPIO_Init(s2p_74hc164_clk_gpio, &GPIO_InitStructure);
	clr_port_pin(s2p_74hc164_clk_gpio, s2p_74hc164_clk_pin);

	GPIO_InitStructure.GPIO_Pin   = s2p_74hc164_data_pin;
	GPIO_Init(s2p_74hc164_data_gpio, &GPIO_InitStructure);
	clr_port_pin(s2p_74hc164_data_gpio, s2p_74hc164_data_pin);

	decoder_pins_cfg();
	double_mbus_indication_pins_cfg();

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = ade7880_range_switch_pin;
	GPIO_Init(ade7880_range_switch_gpio, &GPIO_InitStructure);
	//set_7880_to_lagre_scale();
	set_7880_to_small_scale();      
#else
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = led1_pin;
	GPIO_Init(led1_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = led2_pin;
	GPIO_Init(led2_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = led3_pin;
	GPIO_Init(led3_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = led4_pin;
	GPIO_Init(led4_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = led5_pin;
	GPIO_Init(led5_gpio, &GPIO_InitStructure);

	led_off(led1_gpio, led1_pin);
	led_off(led2_gpio, led2_pin);
	led_off(led3_gpio, led3_pin);
	led_off(led4_gpio, led4_pin);
	led_off(led5_gpio, led5_pin);
#endif

#if 0	/* buzzer */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = buzzer_pin;
	GPIO_Init(buzzer_gpio, &GPIO_InitStructure);
#endif

	/* 485en-1 */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = driver_oen_485_1_pin;
	GPIO_Init(driver_oen_485_1_gpio, &GPIO_InitStructure);
	tx_disable_rev_en_485_1();

	/* 485en-2 */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = driver_oen_485_2_pin;
	GPIO_Init(driver_oen_485_2_gpio, &GPIO_InitStructure);
	tx_disable_rev_en_485_2();

	/* 485en-3 */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = driver_oen_485_3_pin;
	GPIO_Init(driver_oen_485_3_gpio, &GPIO_InitStructure);
	tx_disable_rev_en_485_3();

#if RT_USING_ENC28J60 /* ENC28J60 */
	/*
	 * stm32f103ve使用SPI2与ENC28J60连接
	 * NSS	-- PB12
	 * SCK	-- PB13
	 * MISO	-- PB14
	 * MOSI	-- PB15
	 *
	 * ENC28J60的/int引脚连接至stm32f103ve的PA8
	 * ENC28J60的/RST引脚连接至stm32f103ve的PC6
	 *
	 * ENC28J60的/int引脚连接至stm32f103ve的PB10
	 * ENC28J60的/RST引脚连接至stm32f103ve的PB11
	 */

	/* Configure SPI pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_SPI_SCK | ENC28J60_SPI_MISO | ENC28J60_SPI_MOSI;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(ENC28J60_SPI_PORT, &GPIO_InitStructure);

	/* (ENC28J60_CS_PORT, ENC28J60_CS_PIN), Configure SPI2 pin: NSS */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_SPI_NSS;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(ENC28J60_SPI_NSS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ENC28J60_SPI_NSS_PORT, ENC28J60_SPI_NSS);

	/* /int */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_INT_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(ENC28J60_INT_PORT, &GPIO_InitStructure);

	/* /rst */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_RST_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(ENC28J60_RST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ENC28J60_RST_PORT, ENC28J60_RST_PIN);
#endif
#if RT_USING_SERIAL_FLASH /* spi flash */
	/* Configure SPI pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin 	= SF_SPI_SCK | SF_SPI_MISO | SF_SPI_MOSI;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(SF_SPI_PORT, &GPIO_InitStructure);

	/* (SF_CS_PORT, SF_CS_PIN), Configure SPI2 pin: NSS */
	GPIO_InitStructure.GPIO_Pin 	= SF_SPI_NSS;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(SF_SPI_NSS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SF_SPI_NSS_PORT, SF_SPI_NSS);

	/* /hold */
	GPIO_InitStructure.GPIO_Pin 	= SF_HOLD_PIN ;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(SF_HOLD_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SF_HOLD_PORT, SF_HOLD_PIN);

	/* /wp */
	GPIO_InitStructure.GPIO_Pin 	= SF_WP_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(SF_WP_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SF_WP_PORT, SF_WP_PIN);
#endif

#if RT_USING_TL16C554
	tl16c554_pins_cfg();
#endif
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PRIORITY_GRP);

#ifdef RT_USING_UART1
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART1_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#ifdef RT_USING_UART2
	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART2_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART2_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#ifdef RT_USING_UART3
	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART3_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART3_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#if RT_UART3_USE_DMA
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#endif

#ifdef RT_USING_UART4
	/* Enable the UART4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART4_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART4_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_USING_UART5
	/* Enable the UART4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART5_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART5_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
	//nvic_cfg_app();
}



/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void nvic_cfg_app(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

#if RT_USING_ENC28J60
	/* 允许ENC28J60中断 */
	NVIC_InitStructure.NVIC_IRQChannel = ENC28J60_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ENC28J60_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = ENC28J60_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if RT_USING_RTC
	/* RTC_IRQn, RTCAlarm_IRQn */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = RTC_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = RTC_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if EM_ALL_TYPE_BASE
	/* tim5 -- 用于计量电能脉冲时间 */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM5_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIM5_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	NVIC_InitStructure.NVIC_IRQChannel = TL16C554_1_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TL16_1_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TL16_1_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TL16C554_2_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TL16_2_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = TL16_2_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return;
}



void reset_whole_system(void)
{
	printf_syn("app will reset cm3!\n");

	NVIC_SystemReset();

	return;
}


#if STM32_EXT_SRAM

static void fsmc_for_sram_configration(void)
{
	FSMC_NORSRAMInitTypeDef  	FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  	timing;

	FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM1);

	/*
	 * ---- 初期调试用参数
	 * p.FSMC_AddressSetupTime = 15;
	 * p.FSMC_AddressHoldTime = 15;
	 * p.FSMC_DataSetupTime = 15;
	 * p.FSMC_BusTurnAroundDuration = 15;
	 * p.FSMC_CLKDivision = 8;
	 * p.FSMC_DataLatency = 15;
	 */
	timing.FSMC_AddressSetupTime 	  = 0x01; /* It is not used with synchronous NOR Flash memories. */
	timing.FSMC_AddressHoldTime 	  = 0x01; /* It is not used with synchronous NOR Flash memories.*/
	timing.FSMC_DataSetupTime 	  = 0x01; /* It is used for SRAMs, ROMs and asynchronous multiplexed NOR Flash memories. */
	timing.FSMC_BusTurnAroundDuration = 0x00; /* It is only used for multiplexed NOR Flash memories. */

	timing.FSMC_CLKDivision		  = 0x00; /* It is not used for asynchronous NOR Flash, SRAM or ROM accesses. */
	timing.FSMC_DataLatency		  = 0x00; /* It must be set to 0 in case of a CRAM */
	timing.FSMC_AccessMode		  = FSMC_AccessMode_B;

	FSMC_NORSRAMInitStructure.FSMC_Bank 		  = FSMC_Bank1_NORSRAM1;
#if RT_EXT_SRAM_MULTIPLEXED
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux 	  = FSMC_DataAddressMux_Enable;
#else
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux 	  = FSMC_DataAddressMux_Disable;
#endif
	FSMC_NORSRAMInitStructure.FSMC_MemoryType 	  = FSMC_MemoryType_PSRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth 	  = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode 	  = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait   = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode 	  = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive   = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation 	  = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal 	  = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode 	  = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst 	  = FSMC_WriteBurst_Disable;

	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct	= &timing;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct	= &timing;
	/* FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure); */

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

#else /* STM32_EXT_SRAM */

static void fsmc_for_tl16c554_cfg(void)
{

	FSMC_NORSRAMInitTypeDef  	FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  	timing;

	FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM1);

	/*
	 * ---- 初期调试用参数
	 * p.FSMC_AddressSetupTime = 15;
	 * p.FSMC_AddressHoldTime = 15;
	 * p.FSMC_DataSetupTime = 15;
	 * p.FSMC_BusTurnAroundDuration = 15;
	 * p.FSMC_CLKDivision = 8;
	 * p.FSMC_DataLatency = 15;
	 */
	timing.FSMC_AddressSetupTime 	  = 0x02; /* It is not used with synchronous NOR Flash memories. */
	timing.FSMC_AddressHoldTime 	  = 0x05; /* It is not used with synchronous NOR Flash memories.*/
	timing.FSMC_DataSetupTime 	  = 0x03; /* It is used for SRAMs, ROMs and asynchronous multiplexed NOR Flash memories. */
	timing.FSMC_BusTurnAroundDuration = 0x00; /* It is only used for multiplexed NOR Flash memories. */

	timing.FSMC_CLKDivision		  = 0x00; /* It is not used for asynchronous NOR Flash, SRAM or ROM accesses. */
	timing.FSMC_DataLatency		  = 0x00; /* It must be set to 0 in case of a CRAM */

	timing.FSMC_AccessMode		  = FSMC_AccessMode_A;

	FSMC_NORSRAMInitStructure.FSMC_Bank 		  = FSMC_Bank1_NORSRAM1;
#if RT_EXT_SRAM_MULTIPLEXED
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux 	  = FSMC_DataAddressMux_Enable;
#else
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux 	  = FSMC_DataAddressMux_Disable;
#endif
	FSMC_NORSRAMInitStructure.FSMC_MemoryType 	  = FSMC_MemoryType_PSRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth 	  = FSMC_MemoryDataWidth_8b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode 	  = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait   = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode 	  = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive   = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation 	  = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal 	  = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode 	  = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst 	  = FSMC_WriteBurst_Disable;

	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct	= &timing;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct	= &timing;
	/* FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure); */

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);

}

#endif

#if RT_USING_STM32_FSMC

/*
 * stm32f103ve
 *
 * PERIPHERALS	MODES	REMAP	FUNCTIONS	PINS
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A16	PD11
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A17	PD12
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A18	PD13
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A19	PE3
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A20	PE4
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A21	PE5
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A22	PE6
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A23	PE2
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_CLK	PD3
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA0	PD14
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA1	PD15
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA10	PE13
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA11	PE14
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA12	PE15
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA13	PD8
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA14	PD9
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA15	PD10
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA2	PD0
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA3	PD1
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA4	PE7
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA5	PE8
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA6	PE9
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA7	PE10
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA8	PE11
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA9	PE12
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NADV	PB7
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NBL0	PE0
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NBL1	PE1
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NE1	PD7
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NOE	PD4
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NWAIT	PD6
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NWE	PD5
 */
/*
 * stm32f103ze
 *
 * PERIPHERALS	MODES		REMAP	FUNCTIONS		PINS
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A17	PD12
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A0		PF0
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A1		PF1
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A2		PF2
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A3		PF3
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A4		PF4
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A5		PF5
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A6		PF12
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A7		PF13
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A8		PF14
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A9		PF15
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A10	PG0
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A11	PG1
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A12	PG2
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A13	PG3
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A14	PG4
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A15	PG5
 * FSMC_NOR_RAM	18-bit		0	FSMC_NOR_RAM_A16	PD11
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D0		PD14
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D1		PD15
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D2		PD0
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D3		PD1
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D4		PE7
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D5		PE8
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D6		PE9
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D7		PE10
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_NOE	PD4
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D8		PE11
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D9		PE12
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D10	PE13
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D11	PE14
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D12	PE15
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D13	PD8
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D14	PD9
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_D15	PD10
 * FSMC_NOR_RAM	16-bit-SRAM	0	FSMC_NOR_RAM_NWE	PD5
 * FSMC_NOR_RAM	ChipSelect1	0	FSMC_NOR_RAM_NE1	PD7
 * FSMC_NOR_RAM	ByteEnable	0	FSMC_NOR_RAM_NBL0	PE0
 * FSMC_NOR_RAM	ByteEnable	0	FSMC_NOR_RAM_NBL1	PE1
 *
 *
 * PD -- 0, 1, 4, 5, 7-12, 14, 15,
 * PE -- 0, 1, 7-15,
 * PF -- 0-5,  12-15
 * PG -- 0-5,
 * */
static void fsmc_gpio_cfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* PD */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                              GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                              GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* PE */
#if 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_7 | GPIO_Pin_8 |
				      GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
				      GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
#else
	/* #define tl16c554_1_cha_tx_en_gpio	GPIOE
	 * #define tl16c554_1_cha_tx_en_pin	GPIO_Pin_0
	 *  */
	GPIO_InitStructure.GPIO_Pin = /* GPIO_Pin_0 | */ GPIO_Pin_1 | GPIO_Pin_7 | GPIO_Pin_8 |
				      GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
				      GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
#endif
	GPIO_Init(GPIOE, &GPIO_InitStructure);

#if RT_EXT_SRAM_MULTIPLEXED
	/* PB */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#else
	/* PF */
	/* 3, 4, 5, 8, 9 for led-ctrl */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | /* GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | */
	                              GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* PG */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif

	return;
}

static void fsmc_cfg(void)
{
	fsmc_gpio_cfg();
#if STM32_EXT_SRAM
	fsmc_for_sram_configration();
#else
	fsmc_for_tl16c554_cfg();
#endif
}
#endif

/*
 *****************************************************************************
 * 函数名       EXTI_Configuration
 * 描述           配置EXTI线
 *****************************************************************************
 */
static void exti_config(void)
{
#if RT_USING_ENC28J60
	EXTI_InitTypeDef EXTI_InitStructure;

	/* 将EXTI线连接到enc28j60'/int */
	GPIO_EXTILineConfig(ENC28J60_INT_PORT_SOURCE, ENC28J60_INT_PIN_SOURCE);
	/* 配置EXTI线上出现下降沿，则产生中断 */
	EXTI_InitStructure.EXTI_Line 	= ENC28J60_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif

#if RT_USING_TL16C554
	EXTI_InitTypeDef EXTI_InitStructure;

	GPIO_EXTILineConfig(TL16C554_1_INT_PORT_SOURCE, TL16C554_1_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= TL16C554_1_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	GPIO_EXTILineConfig(TL16C554_2_INT_PORT_SOURCE, TL16C554_2_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= TL16C554_2_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

#endif
	return;
}

static void spi_config(void)
{
#if RT_USING_SERIAL_FLASH || EM_ALL_TYPE_BASE
	SPI_InitTypeDef   SPI_InitStructure;
#endif
#if RT_USING_SERIAL_FLASH
	extern void spiflash_spi_cfg(void);
	spiflash_spi_cfg();
#endif

#if EM_ALL_TYPE_BASE
	/* SPI2 configuration */
	/*
	 * SPI2 clk -- APB1, fCLK1(max = 36MHz)
	 * enc28j60's clk(max) = 10MHz
	 * DS80349C.errata #1:When the SPI clock from the host microcontroller is run at frequencies of less than 8 MHz,
	 * reading or writing to the MAC registers may be unreliable.
	 */
	SPI_InitStructure.SPI_Direction 	= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize 		= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL 		= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA 		= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS 		= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit 		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial 	= 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	SPI_Cmd(SPI2, ENABLE);
#endif

	return;
}

#if 0
void delay_us(int xus)
{
	int i;

	xus *= 6;
	for (i=0; i<xus; i++)
		;

	return;
}
#endif


#if RT_USING_SERIAL_FLASH
void spiflash_spi_cfg(void)
{
	SPI_InitTypeDef   SPI_InitStructure;
#if 0
	/*
	 * SPI1 -- sst25vf016b
	 *
	 * SPI1 clk -- APB2, fCLK2(max = 72MHz)
	 */
	SPI_InitStructure.SPI_Direction 	= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize 		= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL 		= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA 		= SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS 		= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; /* mark by David */
	SPI_InitStructure.SPI_FirstBit 		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial 	= 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
#else
	/*
	 * wx25
	 * SPI3 clk -- APB1, fCLK1(max = 36MHz)
	 */
	SPI_InitStructure.SPI_Direction 	= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize 		= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL 		= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA 		= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS 		= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit 		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial 	= 7;
	SPI_Init(SF_SPIX, &SPI_InitStructure);

	SPI_Cmd(SF_SPIX, ENABLE);
#endif

	return;
}
#endif

#if ENABLE_RAND
static void ADC_Configration(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

	ADC_TempSensorVrefintCmd(ENABLE); 
}

static void ADC_Channel_Config(void)
{
	//ADC_SampleTime_1Cycles5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_1Cycles5);
}

u16 Get_ADCValue(void)
{
	u16 ret;

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	ret =  ADC_GetConversionValue(ADC1);
	return ret;
}
#endif

#if RT_USING_TL16C554
static void tl16c554_pins_cfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = tl16c554_1_csa_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(tl16c554_1_csa_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = tl16c554_1_csb_pin;
	GPIO_Init(tl16c554_1_csb_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_1_csc_pin;
	GPIO_Init(tl16c554_1_csc_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_1_csd_pin;
	GPIO_Init(tl16c554_1_csd_gpio, &GPIO_InitStructure);

	tl16c554_1_csa_deact();
	tl16c554_1_csb_deact();
	tl16c554_1_csc_deact();
	tl16c554_1_csd_deact();

	GPIO_InitStructure.GPIO_Pin = tl16c554_2_csa_pin;
	GPIO_Init(tl16c554_2_csa_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_csb_pin;
	GPIO_Init(tl16c554_2_csb_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_csc_pin;
	GPIO_Init(tl16c554_2_csc_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_csd_pin;
	GPIO_Init(tl16c554_2_csd_gpio, &GPIO_InitStructure);

	tl16c554_2_csa_deact();
	tl16c554_2_csb_deact();
	tl16c554_2_csc_deact();
	tl16c554_2_csd_deact();

	GPIO_InitStructure.GPIO_Pin = tl16c554_1_reset_pin;
	GPIO_Init(tl16c554_1_reset_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_reset_pin;
	GPIO_Init(tl16c554_2_reset_gpio, &GPIO_InitStructure);

	tl16c554_1_reset_act();
	tl16c554_2_reset_act();

	/* 485 tx_en, rx_en pin cfg */
	GPIO_InitStructure.GPIO_Pin = tl16c554_x_chx_rx_en_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(tl16c554_x_chx_rx_en_gpio, &GPIO_InitStructure);

	tl16c554_x_485rx_enable();

	GPIO_InitStructure.GPIO_Pin = tl16c554_1_cha_tx_en_pin;
	GPIO_Init(tl16c554_1_cha_tx_en_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_1_chb_tx_en_pin;
	GPIO_Init(tl16c554_1_chb_tx_en_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_1_chc_tx_en_pin;
	GPIO_Init(tl16c554_1_chc_tx_en_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_1_chd_tx_en_pin;
	GPIO_Init(tl16c554_1_chd_tx_en_gpio, &GPIO_InitStructure);

	tl16c554_1_cha_485tx_disable();
	tl16c554_1_chb_485tx_disable();
	tl16c554_1_chc_485tx_disable();
	tl16c554_1_chd_485tx_disable();

	GPIO_InitStructure.GPIO_Pin = tl16c554_2_cha_tx_en_pin;
	GPIO_Init(tl16c554_2_cha_tx_en_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_chb_tx_en_pin;
	GPIO_Init(tl16c554_2_chb_tx_en_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_chc_tx_en_pin;
	GPIO_Init(tl16c554_2_chc_tx_en_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = tl16c554_2_chd_tx_en_pin;
	GPIO_Init(tl16c554_2_chd_tx_en_gpio, &GPIO_InitStructure);

	tl16c554_2_cha_485tx_disable();
	tl16c554_2_chb_485tx_disable();
	tl16c554_2_chc_485tx_disable();
	tl16c554_2_chd_485tx_disable();

	GPIO_InitStructure.GPIO_Pin 	= tl16c554_1_intx_pin;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPD;
	GPIO_Init(tl16c554_1_intx_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin 	= tl16c554_2_intx_pin;
	GPIO_Init(tl16c554_2_intx_gpio, &GPIO_InitStructure);
}
#endif

/*@}*/


