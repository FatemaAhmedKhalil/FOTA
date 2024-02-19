/***********************************************************************************************/
/***********************************************************************************************/
/*********************************** Author: Fatema Ahmed **************************************/
/****************************************** Layer: HAL *****************************************/
/*************************************** SWC: Bootloader ***************************************/
/**************************************** Version: 1.00 ****************************************/
/*************************************** Date: 25/01/2024 **************************************/
/***********************************************************************************************/
/***********************************************************************************************/

#include "STD_Types.h"
#include "BitOperations.h"

#include "Bootloader_Interface.h"
#include "Bootloader_Private.h"
#include "USART_Interface.h"
#include "CRC_Interface.h"
#include "FMI_Interface.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/********** Global Variables Deceleration**********/
static u8 BL_HostBuffer[BL_HostSize];

// BL_SendMessage(char* Format,...) For Test
void BL_SendMessage(char* Format,...)
{
	char Message[100] = {0};
	va_list args;
	va_start(args, Format);
	vsprintf(Message, Format, args);
	USART_voidTransmitString (USARTxNum, (u8*)Message);
	va_end(args);
}

void BL_voidJumpToUserApp(void)
{
	void( * AppResetHandler)(void);
	uint32_t Local_u32MSPValue,Local_u32ResetAddr;

	/* Get the MSP value of the user application*/
	Local_u32MSPValue = *((volatile uint32_t *)(FLASH_SECTOR_2_BASE_ADDR));

	/* assign this value into MSP*/
	__asm volatile("MSR MSP,%0"::"r"(Local_u32MSPValue));

	/* Get the Reset Handler address of the user application*/
	Local_u32ResetAddr = *((uint32_t *)(FLASH_SECTOR_2_BASE_ADDR+4));
	AppResetHandler = (void *)Local_u32ResetAddr;

	/* Call the Reset Handler of the use application*/
	AppResetHandler();

}

BL_Status BL_FeatchHostCmd()
{
	BL_Status Status = BL_NAKN;
	u8 DataLength = 0;
	memset(BL_HostBuffer, 0, BL_HostSize);

	// Get Length
	BL_HostBuffer[0] = USART_u8ReceiveByteSynchBlocking ( USART1 );

	DataLength = BL_HostBuffer[0];

	// Get Command
	BL_HostBuffer[1] = USART_u8ReceiveByteSynchBlocking ( USART1 );
	switch(BL_HostBuffer[1])
	{
		case CBL_GET_VER_CMD	: BL_voidGetVersion(BL_HostBuffer); break;
		case CBL_GET_HELP_CMD	: BL_voidGetHelp(BL_HostBuffer); 	break;
		case CBL_GET_CID_CMD	: BL_voidGetChipID(BL_HostBuffer); 	break;
		case CBL_GO_TO_ADDR_CMD:
			// BL_SendMessage("Jump to Address");
			break;
		case CBL_FLASH_ERASE_CMD: BL_voidFlashErase(BL_HostBuffer); break;
		case CBL_MEM_WRITE_CMD:
			// BL_SendMessage("Write to Flash");
			break;
		default: BL_voidSendNACK();
	}
	return Status;
}

u8 BL_u8CRC(u8 *ptru8Data, u8 u8DataLength, u32 u32HostCRC)
{
	u32 MCUCRC = 0;
	u8 Status = BL_NAKN;
	u32 DataBuffer = 0;

	for (int i = 0; i < u8DataLength; i++)
	{
		DataBuffer = ptru8Data[i];
		MCUCRC = CRC_u32Accumulate(&DataBuffer, 1);
	}

	CRC_voidReset();

	if (MCUCRC == u32HostCRC)
		Status = BL_AKN;

	return Status;
}

void BL_voidSendACK(u8 u8DataLength)
{
	u8 ACKValue[2] = {0};
	ACKValue[0] = BL_AKN;
	ACKValue[1] = u8DataLength;
	USART_voidTransmitByte (USARTxNum, ACKValue[0]);
	USART_voidTransmitByte (USARTxNum, ACKValue[1]);
}

void BL_voidSendNACK()
{
	USART_voidTransmitByte (USARTxNum, BL_NAKN);
}

void BL_voidGetVersion(u8 *ptru8HostBuffer)
{
	u8 Version[4] = {CBL_VENDOR_ID,
						CBL_SW_MAJOR_VENRSION,
						CBL_SW_MINOR_VENRSION,
						CBL_SW_PATCH_VENRSION};
	u16 HostPacketLength = 0;
	u32 CRCValue = 0;
	HostPacketLength = ptru8HostBuffer[0] + 1;
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - 4);

	if (BL_AKN == BL_u8CRC(&ptru8HostBuffer[0], HostPacketLength-4, CRCValue))
	{
		BL_voidSendACK(4);

		USART_u8TransmitArraySynch(USARTxNum, Version, sizeof(Version));
	}
	else
		BL_voidSendNACK();
}

void BL_voidGetHelp(u8 *ptru8HostBuffer)
{
	u8 SuporrtedCMS[] = {CBL_GET_VER_CMD,
							CBL_GET_HELP_CMD,
							CBL_GET_CID_CMD,
							CBL_GET_RDP_STATUS_CMD,
							CBL_GO_TO_ADDR_CMD,
							CBL_FLASH_ERASE_CMD,
							CBL_MEM_WRITE_CMD,
							CBL_ED_W_PROTECT_CMD,
							CBL_MEM_READ_CMD,
							CBL_READ_SECTOR_STATUS_CMD,
							CBL_OTP_READ_CMD,
							CBL_CHANGE_ROP_Level_CMD};
	u16 HostPacketLength = 0;
	u32 CRCValue = 0;
	HostPacketLength = ptru8HostBuffer[0] + 1;
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - 4);

	if (BL_AKN == BL_u8CRC(&ptru8HostBuffer[0], HostPacketLength-4, CRCValue))
	{
		BL_voidSendACK((sizeof(SuporrtedCMS)/sizeof(SuporrtedCMS[0])));
		USART_u8TransmitArraySynch(USARTxNum, SuporrtedCMS, sizeof(SuporrtedCMS));
	}
	else
		BL_voidSendNACK();
}

void BL_voidGetChipID(u8 *ptru8HostBuffer)
{
	u16 ChipID = 0;
	u16 HostPacketLength = 0;
	u32 CRCValue = 0;
	HostPacketLength = ptru8HostBuffer[0] + 1;
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - 4);

	if (BL_AKN == BL_u8CRC(&ptru8HostBuffer[0], HostPacketLength-4, CRCValue))
	{
		ChipID = (u16)DBGMCU_IDCODE_REG & IDCODE_MASK;
		BL_voidSendACK(2);
		USART_u8TransmitArraySynch(USARTxNum, (u8*)&ChipID, 2);
	}
	else
		BL_voidSendNACK();
}

u8 BL_u8ExcuteFlashErase(u8 u8SectorNumber, u8 u8NumberOfSectors)
{
	u8 SectorStatus = INVALID_SECTOR_NUMBER;
	FMI_EraseTypeDef FlashErase;

	if((u8SectorNumber > FLASH_MAX_NUM_SECTROS-1) && (u8NumberOfSectors != CBL_FLASH_MASS_ERASE))
		SectorStatus = INVALID_SECTOR_NUMBER;

	else if((u8SectorNumber > FLASH_MAX_NUM_SECTROS) && (u8NumberOfSectors != CBL_FLASH_MASS_ERASE))
		SectorStatus = INVALID_SECTOR_NUMBER;

	else
	{
		SectorStatus = VALID_SECTOR_NUMBER;

		if (u8SectorNumber == CBL_FLASH_MASS_ERASE)
		{
			FlashErase.EraseType = FMI_MassErase;
			FlashErase.StartSector = FMI_SECTOR_0;
			FlashErase.EndSector = FMI_SECTOR_7;
		}

		else
		{
			FlashErase.EraseType = FMI_SectorErase;
			FlashErase.StartSector = u8SectorNumber;

			u8 RemainingSectors = FLASH_MAX_NUM_SECTROS - u8SectorNumber;

			if(u8SectorNumber > RemainingSectors)
				u8NumberOfSectors = RemainingSectors;

			FlashErase.EndSector = u8SectorNumber + u8NumberOfSectors;
		}

		FlashErase.Parallelism = FMI_PAR_HWORD;

		FMI_voidUnlock();
		FMI_voidErase(&FlashErase);
		FMI_voidLock();
	}
	return SectorStatus;
}

u8 BL_voidFlashErase(u8 *ptru8HostBuffer)
{
	u8 EraseStatus = UNSUCCESSFUL_ERASE;
	u16 HostPacketLength = 0;
	u32 CRCValue = 0;
	HostPacketLength = ptru8HostBuffer[0] + 1;
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - 4);

	if (BL_AKN == BL_u8CRC(&ptru8HostBuffer[0], HostPacketLength-4, CRCValue))
	{
		EraseStatus = BL_u8ExcuteFlashErase(ptru8HostBuffer[2], ptru8HostBuffer[6]);

		if (EraseStatus == VALID_SECTOR_NUMBER)
			EraseStatus = SUCCESSFUL_ERASE;

		BL_voidSendACK(1);
	}
	else
	{
		BL_voidSendNACK();
		EraseStatus = UNSUCCESSFUL_ERASE;
	}

	USART_voidTransmitByte(USARTxNum, EraseStatus);
	return EraseStatus;
}
