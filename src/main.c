/*==================================================================================================
*   Project              : RTD AUTOSAR 4.9
*   Platform             : CORTEXM
*   Peripheral           : S32K3XX
*   Dependencies         : none
*
*   Autosar Version      : 4.9.0
*   Autosar Revision     : ASR_REL_4_9_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 7.0.1
*   Build Version        : S32K3_RTD_7_0_1_D2602_ASR_REL_4_9_REV_0000_20260206
*
*   Copyright 2020 - 2026 NXP
*
*   NXP Proprietary. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Adc.h"
#include "Mcu.h"
#include "Mcl.h"
#include "CDD_Rm.h"
#include "Platform.h"
#include "check_example.h"
#include "Siul2_Port_Ip.h"
#include "Dma_Ip_Hw_Access.h"

/*==================================================================================================
 *                                      EXTERN DECLARATIONS
==================================================================================================*/
/* By default S32K342, K358 and K311 boards will have 3.3V voltage reference selected while S32K312, S32K344 and S32K396 will have 5V selected.
 * If you have S32K342 with default pin configuration or if you've manually selected 3.3V reference, please un-comment the following line: */
#define ADC_3V3_VREF_SELECTED
#ifdef ADC_3V3_VREF_SELECTED
#define ADC_BANDGAP         			5980U /* Vbandgap ~ 1.2V on 14 bits resolution, 3.3V VrefH */
#else
#define ADC_BANDGAP         			3932U /* Vbandgap ~ 1.2V on 14 bits resolution, 5V VrefH */
#endif

#define ADC_VREFH           			16383U /* 14 bits resolution */

#define BCTU_IRQ            			BCTU_IRQn
#define DMA_BCTU_FIFO1_IRQ          	DMATCD0_IRQn
#define DMA_BCTU_FIFO1_IRQ_HANDLER   	Dma0_Ch0_IRQHandler
#define DMA_BCTU_FIFO2_IRQ          	DMATCD16_IRQn
#define DMA_BCTU_FIFO2_IRQ_HANDLER   	Dma0_Ch16_IRQHandler

#define BUFFER_SIZE_1 					(24u)
#define BUFFER_SIZE_2 					(24u)
#define NUM_RESULTS_1         			(8u)
#define NUM_RESULTS_2         			(6u)
#define DMA_CH1_CONFIG_LIST_DIMENSION 	(2u)
#define DMA_CH17_CONFIG_LIST_DIMENSION 	(2u)

#define BCTU_FIFO_CHANNELNUMBER_MASK	(0x01FC0000)
#define BCTU_FIFO_ADCDATA_MASK			(0x00007FFF)
#define ADC_VREFH_CHANNELID				(55U)
#define ADC_BANDGAP_CHANNELID			(48U)

#define ADC_TOLERANCE(x,y)  (((x > y) ? (x - y) : (y - x)) > 200U) /* Check that the data is within tolerated range */

extern ISR(Adc_Sar_0_Isr);
extern ISR(Adc_Sar_1_Isr);

extern ISR(DMA_ADC_IRQ_HANDLER);
extern ISR(Bctu_0_Isr);
extern ISR(DMA_BCTU_FIFO1_IRQ_HANDLER);
extern ISR(DMA_BCTU_FIFO2_IRQ_HANDLER);

/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/
extern uint32 BctuDmaFifo1[NUM_RESULTS_1]; /* Used to store raw data from FIFO_1 */
extern uint32 BctuDmaFifo2[NUM_RESULTS_2]; /* Used to store raw data from FIFO_2 */

volatile uint8 VarNotification_0 = 0u;
volatile uint8 VarNotification_1 = 0u;
volatile uint8 BctuWatermarkNotif1 = 0u;
volatile uint8 BctuWatermarkNotif2 = 0u;
volatile uint8 BctuTrigNotif = 0u;
volatile uint8 BctuNewDataNotifAdc0 = 0u;
volatile uint8 BctuNewDataNotifAdc1 = 0u;
volatile uint8 Counter_Dma_1 = 0;
volatile uint8 Counter_Dma_2 = 0;

__attribute__(( aligned(32) )) static uint32 DmaCh1_DestinationBuffer[BUFFER_SIZE_1];
__attribute__(( aligned(32) )) static uint32 DmaCh17_DestinationBuffer[BUFFER_SIZE_2];

/* DMA Logic Channel 1 Initial Address Configuration */
Mcl_DmaChannelTransferListType DmaCh1_TransferList[DMA_CH1_CONFIG_LIST_DIMENSION] =
{
		{MCL_DMA_CH_SET_SOURCE_ADDRESS, (Dma_Ip_uintPtrType)&BctuDmaFifo1[0U]},
		{MCL_DMA_CH_SET_DESTINATION_ADDRESS, (Dma_Ip_uintPtrType)&DmaCh1_DestinationBuffer[0U]},
};

/* DMA Logic Channel 17 Initial Address Configuration */
Mcl_DmaChannelTransferListType DmaCh17_TransferList[DMA_CH17_CONFIG_LIST_DIMENSION] =
{
		{MCL_DMA_CH_SET_SOURCE_ADDRESS, (Dma_Ip_uintPtrType)&BctuDmaFifo2[0U]},
		{MCL_DMA_CH_SET_DESTINATION_ADDRESS, (Dma_Ip_uintPtrType)&DmaCh17_DestinationBuffer[0U]},
};

uint32 g_DmaCh1_CallbackCounter = 0U;
uint32 g_DmaCh17_CallbackCounter = 0U;
Mcl_DmaChannelStatusType t_GetChannel1Status;
Mcl_DmaChannelStatusType t_GetChannel17Status;

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/
void Notification_0(void)
{
	VarNotification_0++;
}

void Notification_1(void)
{
	VarNotification_1++;
}

void Mcl_DmaCh1_Callback(void)
{
	g_DmaCh1_CallbackCounter++;
}

void Mcl_DmaCh17_Callback(void)
{
	g_DmaCh17_CallbackCounter++;
}

#if (ADC_ENABLE_CTU_CONTROL_MODE_API == STD_ON)
void WatermarkNotification1(void)
{
	BctuWatermarkNotif1++;

	/* Count times BctuDmaFifo1 is to be transfered to DmaCh1_DestinationBuffer before it fills up */
	Counter_Dma_1 = (Counter_Dma_1 == (BUFFER_SIZE_1/NUM_RESULTS_1) - 1) ? 0 : (Counter_Dma_1 + 1);
}

void WatermarkNotification2(void)
{
	BctuWatermarkNotif2++;

	/* Count times BctuDmaFifo1 is to be transfered to DmaCh17_DestinationBuffer before it fills up */
	Counter_Dma_2 = (Counter_Dma_2 == (BUFFER_SIZE_2/NUM_RESULTS_2) - 1) ? 0 : (Counter_Dma_2 + 1);
}

void BctuTriggerNotification(void)
{
	BctuTrigNotif++;
}

void NewDataNotificationAdc0(void)
{
	BctuNewDataNotifAdc0++;
}

void NewDataNotificationAdc1(void)
{
	BctuNewDataNotifAdc1++;
}

#endif /* (ADC_ENABLE_CTU_CONTROL_MODE_API == STD_ON) */


int main(void)
{
	volatile boolean bStatus = TRUE;
#if (ADC_CALIBRATION == STD_ON)
	Adc_CalibrationStatusType CalibStatus;
#endif /* (ADC_CALIBRATION == STD_ON) */
	uint8 Index;
	uint8 Index2;

	/* Initialize Mcu Driver */
#if (MCU_PRECOMPILE_SUPPORT == STD_ON)
	Mcu_Init(NULL_PTR);
#elif (MCU_PRECOMPILE_SUPPORT == STD_OFF)
	Mcu_Init(&Mcu_Config);
#endif /* (MCU_PRECOMPILE_SUPPORT == STD_ON) */
	Mcu_InitClock(McuClockSettingConfig_0);
#if (MCU_NO_PLL == STD_OFF)
	while ( MCU_PLL_LOCKED != Mcu_GetPllStatus() )
	{
		/* Busy wait until the System PLL is locked */
	}
	Mcu_DistributePllClock();
#endif
	Mcu_SetMode(McuModeSettingConf_0);

    /* Initialize all pins using the Siul2_Port driver */
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

	/* Initialize Mcl driver */
	Mcl_Init(NULL_PTR);

	/* Initialize Rm driver */
	Rm_Init(NULL_PTR);

	/* Initialize Platform Driver */
	Platform_Init(NULL_PTR);
	Platform_InstallIrqHandler(BCTU_IRQ, Bctu_0_Isr, NULL_PTR);
	Platform_InstallIrqHandler(DMA_BCTU_FIFO1_IRQ, DMA_BCTU_FIFO1_IRQ_HANDLER, NULL_PTR);
	Platform_InstallIrqHandler(DMA_BCTU_FIFO2_IRQ, DMA_BCTU_FIFO2_IRQ_HANDLER, NULL_PTR);
	Platform_SetIrq(BCTU_IRQ, TRUE);
	Platform_SetIrq(DMA_BCTU_FIFO1_IRQ, TRUE);
	Platform_SetIrq(DMA_BCTU_FIFO2_IRQ, TRUE);

#if (ADC_PRECOMPILE_SUPPORT == STD_ON)
	Adc_Init(NULL_PTR);
#else
	Adc_Init(&Adc_Config);
#endif /* (ADC_PRECOMPILE_SUPPORT == STD_ON) */

#if (ADC_CALIBRATION == STD_ON)
	/* Calibrate the ADC hardware units. */
	for(Index = 0; Index <= 1; Index++)
	{
		/* Call Calibration function multiple times, to mitigate instability of board source */
		for(Index2 = 0; Index2 <= 5; Index2++)
		{
			Adc_Calibrate(Index, &CalibStatus);
			if(CalibStatus.AdcUnitSelfTestStatus == E_OK)
			{
				break;
			}
		}
		/* Fail if calibration did not succeed after multiple attempts. */
		if(Index2 > 5)
		{
			bStatus = FALSE;
		}
	}
#endif /* (ADC_CALIBRATION == STD_ON) */

	if (bStatus)
	{
		/* Part 1: Example with BCTU controlled mode with triggers using all ADC instances in series, data
		 * conversion is updated by DMA to BctuDmaFifo1 array.                                                 */
		/*******************************************************************************************************/
		Adc_EnableCtuControlMode(AdcHwUnit_0);
		Adc_EnableCtuControlMode(AdcHwUnit_1);

		for(uint8 AdcHwUnitTrigger = 0; AdcHwUnitTrigger < (BUFFER_SIZE_1/NUM_RESULTS_1); AdcHwUnitTrigger++)
		{
			/* Update Dma Ch1 TCD to point to new buffer source address */
			DmaCh1_TransferList[1].Value = (Dma_Ip_uintPtrType)&DmaCh1_DestinationBuffer[Counter_Dma_1 * NUM_RESULTS_1];

			/* SW start conversion for trigger which uses LIST and FIFO for each ADCHwUnit's trigger. Done sequentially */
			Adc_CtuStartConversion(AdcHwUnitTrigger);

			while (BctuWatermarkNotif1 == 0u)
			{}

			/* Set Logic Channel 1 -> Source and Destination Address */
			Mcl_SetDmaChannelTransferList(DMA_LOGIC_CH_1, DmaCh1_TransferList, DMA_CH1_CONFIG_LIST_DIMENSION);

			/* Start transfer for logic Channel 1 */
			Mcl_SetDmaChannelCommand(DMA_LOGIC_CH_1, MCL_DMA_CH_START_SERVICE);
			Mcl_GetDmaChannelStatus(DMA_LOGIC_CH_1, &t_GetChannel1Status);
			while(t_GetChannel1Status.Done != TRUE)
			{
				Mcl_GetDmaChannelStatus(DMA_LOGIC_CH_1, &t_GetChannel1Status);
			}

			/* Handle buffer FULL state */
			if(Counter_Dma_1 == 0u)
			{
				bStatus = TRUE;
			}
			else
			{
				bStatus = FALSE;
			}

			BctuWatermarkNotif1 = 0;
		}

		Adc_DisableCtuControlMode(AdcHwUnit_0);
		Adc_DisableCtuControlMode(AdcHwUnit_1);

		/* Part 2: Example with BCTU controlled mode for parallel conversions from all ADC instances using single
		 * list, data conversion is updated by DMA to BctuDmaFifo2 array.                                      */
		/*******************************************************************************************************/
		Adc_EnableCtuControlMode(AdcHwUnit_0);
		Adc_EnableCtuControlMode(AdcHwUnit_1);

		for(uint8 i = 0; i < (BUFFER_SIZE_2/NUM_RESULTS_2); i++)
		{
			/* Update Dma Ch17 TCD to point to new buffer source address */
			DmaCh17_TransferList[1].Value = (Dma_Ip_uintPtrType)&DmaCh17_DestinationBuffer[Counter_Dma_2 * NUM_RESULTS_2];

			/* SW start conversion for trigger which uses LIST and FIFO for all ADCHwUnit's trigger. Done in parallel */
			Adc_CtuStartConversion(AdcHwTrigger_3);
			while (BctuWatermarkNotif2 == 0u)
			{}

			/* Set Logic Channel 17 -> Source and Destination Address */
			Mcl_SetDmaChannelTransferList(DMA_LOGIC_CH_17, DmaCh17_TransferList, DMA_CH17_CONFIG_LIST_DIMENSION);

			/* Start transfer for logic Channel 17 */
			Mcl_SetDmaChannelCommand(DMA_LOGIC_CH_17, MCL_DMA_CH_START_SERVICE);
			Mcl_GetDmaChannelStatus(DMA_LOGIC_CH_17, &t_GetChannel17Status);
			while(t_GetChannel17Status.Done != TRUE)
			{
				Mcl_GetDmaChannelStatus(DMA_LOGIC_CH_17, &t_GetChannel17Status);
			}

			/* Handle buffer FULL state */
			if(Counter_Dma_2 == 0u)
			{
				bStatus = TRUE;
			}
			else
			{
				bStatus = FALSE;
			}

			BctuWatermarkNotif2 = 0;
		}

		Adc_DisableCtuControlMode(AdcHwUnit_0);
		Adc_DisableCtuControlMode(AdcHwUnit_1);
	}

	/* Check results transfered to both buffers are correct. For more information on how to read the RAW FIFO
	 * data stored in this example, refer to S32K3xx Reference Manual to 'BCTU Register Descriptions' chapter,
	 * and search for 'FIFO Result Data (FIFO1DR - FIFO2DR)' register. */
	if(bStatus)
	{
		/* Check DmaCh1_DestinationBuffer */
		for(uint8 i = 0; i < (BUFFER_SIZE_1) && bStatus; i++)
		{
            /* Need to shift 1 bit because of DMA will always transfer the full 15 bits width of data from the
             * CDR register but result from BCTU/ADC returns data with Resolution 14 bits and right alignment
             * which were done by configuration. */
			if((((DmaCh1_DestinationBuffer[i] & BCTU_FIFO_CHANNELNUMBER_MASK) >> 18U) == ADC_BANDGAP_CHANNELID) && \
			   (ADC_TOLERANCE(((DmaCh1_DestinationBuffer[i] & BCTU_FIFO_ADCDATA_MASK) >> 1U), ADC_BANDGAP)))
			{
				bStatus = FALSE;
				break;
			}
			else if((((DmaCh1_DestinationBuffer[i] & BCTU_FIFO_CHANNELNUMBER_MASK) >> 18U) == ADC_VREFH_CHANNELID) && \
					 (ADC_TOLERANCE(((DmaCh1_DestinationBuffer[i] & BCTU_FIFO_ADCDATA_MASK) >> 1U), ADC_VREFH)))
			{
				bStatus = FALSE;
				break;
			}
		}

		/* Check DmaCh17_DestinationBuffer */
		for(uint8 i = 0; i < (BUFFER_SIZE_2) && bStatus; i++)
		{
			/* Need to shift 1 bit because of DMA will always transfer the full 15 bits width of data from the
			 * CDR register but result from BCTU/ADC returns data with Resolution 14 bits and right alignment
			 * which were done by configuration. */
			if((((DmaCh17_DestinationBuffer[i] & BCTU_FIFO_CHANNELNUMBER_MASK) >> 18U) == ADC_BANDGAP_CHANNELID) && \
				(ADC_TOLERANCE(((DmaCh17_DestinationBuffer[i] & BCTU_FIFO_ADCDATA_MASK) >> 1U), ADC_BANDGAP)))
			{
				bStatus = FALSE;
				break;
			}
			else if((((DmaCh17_DestinationBuffer[i] & BCTU_FIFO_CHANNELNUMBER_MASK) >> 18U) == ADC_VREFH_CHANNELID) && \
					(ADC_TOLERANCE(((DmaCh17_DestinationBuffer[i] & BCTU_FIFO_ADCDATA_MASK) >> 1U), ADC_VREFH)))
			{
				bStatus = FALSE;
				break;
			}
		}
	}

	Adc_DeInit();

	Exit_Example(bStatus);

	while(1);

	return 0;
}

#ifdef __cplusplus
}
#endif

/** @} */
