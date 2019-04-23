/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * @brief    A project template for M031 MCU.
 *
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

//#define MODULE_PWM
#define MODULE_ANALOG_PWM

#define LED_REVERSE(x)		(100-x)			// because lED in EVM schematic , need to reverse level

#define ANALOG_PWMWIDTH  	(100)
#define ANALOG_PWMFREQ	(2000)

#define ANALOG_PWMTIMER	(ANALOG_PWMFREQ*ANALOG_PWMWIDTH)
uint16_t ANALOG_CNT = 0;
	
uint8_t DUTY_LED = 0;
uint8_t FLAG_LED = 1;
uint8_t CNT_LED = 0;	


void TMR0_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER0) == 1)
    {
        TIMER_ClearIntFlag(TIMER0);
		ANALOG_CNT++;
    }
}

void ANALOG_PWM_TIMER0_Init(void)
{
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, ANALOG_PWMTIMER);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);	
    TIMER_Start(TIMER0);
}


void ANALOG_PWM_ConfigOutputChannel(uint8_t duty)
{
    if(ANALOG_CNT > ANALOG_PWMWIDTH)
    {
    	ANALOG_CNT = 0;
    }

    if(ANALOG_CNT <= duty)
    {
    	PB14 = 1;	//return 1;
    }

    else     //if((cnt > duty)) && (cnt <= period))
    {
    	PB14 = 0;	//return 0;
    }
}

void ANALOG_PWM_Init(void)
{
    GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);

	ANALOG_PWM_TIMER0_Init();
	
}

void PWM1_CH1_Init(void)	//PB14
{
    PWM_ConfigOutputChannel(PWM1, 1, 20000, LED_REVERSE(0));
    PWM_EnableOutput(PWM1, PWM_CH_1_MASK);

    /* Start PWM module */
    PWM_Start(PWM1, PWM_CH_1_MASK);

}

void TMR3_IRQHandler(void)
{
	static uint32_t LOG = 0;
	static uint16_t CNT = 0;

    if(TIMER_GetIntFlag(TIMER3) == 1)
    {
        TIMER_ClearIntFlag(TIMER3);

		#if defined (MODULE_PWM)
		if (CNT_LED++ >= 18)
		{		
			CNT_LED = 0;
			PWM_ConfigOutputChannel(PWM1, 1, 20000, LED_REVERSE(DUTY_LED));
//			printf("DUTY : %4d\r\n" ,DUTY_LED );
			if (FLAG_LED)
			{
				if ( ++DUTY_LED == 100)
				{
					FLAG_LED = 0;
					DUTY_LED = 100;
				}
			}
			else
			{
				if ( --DUTY_LED == 0)
				{
					FLAG_LED = 1;
					DUTY_LED = 0;
				}			
			}
		}
		#endif

		#if defined (MODULE_ANALOG_PWM)
		if (CNT_LED++ >= 18)
		{		
			CNT_LED = 0;
//			printf("DUTY : %4d\r\n" ,DUTY_LED );	
			if (FLAG_LED)
			{
				if ( ++DUTY_LED == 100)
				{
					FLAG_LED = 0;
					DUTY_LED = 100;
				}
			}
			else
			{
				if ( --DUTY_LED == 0)
				{
					FLAG_LED = 1;
					DUTY_LED = 0;
				}			
			}
		}
		#endif

		
	
		if (CNT++ >= 1000)
		{		
			CNT = 0;
        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
		}
    }
}


void TIMER3_Init(void)
{
    TIMER_Open(TIMER3, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER3);
    NVIC_EnableIRQ(TMR3_IRQn);	
    TIMER_Start(TIMER3);
}


void UART0_Init(void)
{

    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

	/* Set UART receive time-out */
	UART_SetTimeoutCnt(UART0, 20);

	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());
	
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock (Internal RC 48MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable UART0 clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR3_MODULE);

	#if defined (MODULE_PWM)	
    CLK_EnableModuleClock(PWM1_MODULE);
	#endif

	#if defined (MODULE_ANALOG_PWM)	
    CLK_EnableModuleClock(TMR0_MODULE);
	#endif
	
	
    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PCLK0, CLK_CLKDIV0_UART0(1));
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);

	#if defined (MODULE_PWM)
    CLK_SetModuleClock(PWM1_MODULE, CLK_CLKSEL2_PWM1SEL_PCLK1, 0);
	#endif
	#if defined (MODULE_ANALOG_PWM)
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK0, 0);
	#endif


    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk))    |       \
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

	#if defined (MODULE_PWM)
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB14MFP_Msk)) |
                    SYS_GPB_MFPH_PB14MFP_PWM1_CH1;
	#endif

    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M031 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
    SYS_Init();

    UART0_Init();

	TIMER3_Init();

	#if defined (MODULE_PWM)
	PWM1_CH1_Init();
	#endif
	#if defined (MODULE_ANALOG_PWM)
	ANALOG_PWM_Init();

	#endif
	
    /* Got no where to go, just loop forever */
    while(1)
    {
		#if defined (MODULE_ANALOG_PWM)
		ANALOG_PWM_ConfigOutputChannel(LED_REVERSE(DUTY_LED));
		#endif
    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
