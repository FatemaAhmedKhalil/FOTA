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

void BL_FeatchHostCmd()
{
	u8 BL_HostBuffer[BL_HostSize] = {0};

	// Get Length
	 USART_u8ReceiveByteSynchBlocking (USARTxNum, &BL_HostBuffer[0]);

	// Get the Rest of Packet
	USART_voidReceiveArraySynch (USARTxNum , &BL_HostBuffer[1] , BL_HostBuffer[0]);

	switch(BL_HostBuffer[1]) // Switch on the Command
	{
		case CBL_GET_VER_CMD	: BL_voidGetVersion(BL_HostBuffer); break;
		case CBL_GET_HELP_CMD	: BL_voidGetHelp(BL_HostBuffer); 	break;
		case CBL_GET_CID_CMD	: BL_voidGetChipID(BL_HostBuffer); 	break;
		case CBL_GO_TO_ADDR_CMD	: break;
		case CBL_FLASH_ERASE_CMD: BL_voidFlashErase(BL_HostBuffer); break;
		case CBL_MEM_WRITE_CMD	: BL_voidFlashWrite(BL_HostBuffer); break;
		default: BL_voidSendNACK();
	}
}

u8 BL_u8CRC(u8 *ptru8Data, u8 u8DataLength, u32 u32HostCRC)
{
	u32 AccumualateCRC = 0;
	u8 Status = BL_NAKN;
	u32 DataBuffer = 0;

	for (int iterator = 0; iterator < u8DataLength; iterator++)
	{
		DataBuffer = ptru8Data[iterator];
		AccumualateCRC = CRC_u32Accumulate(&DataBuffer, 1);
	}

	CRC_voidReset();

	if (AccumualateCRC == u32HostCRC)
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
	CRCValue = *((u32 *)(ptru8HostBuffer + HostPacketLength - CRC_Length));

	if (BL_AKN == BL_u8CRC(ptru8HostBuffer, HostPacketLength-CRC_Length, CRCValue))
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
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - CRC_Length);

	if (BL_AKN == BL_u8CRC(ptru8HostBuffer, HostPacketLength-CRC_Length, CRCValue))
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
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - CRC_Length);

	if (BL_AKN == BL_u8CRC(&ptru8HostBuffer[0], HostPacketLength-CRC_Length, CRCValue))
	{
		ChipID = (u16)DBGMCU_IDCODE_REG & IDCODE_MASK;
		BL_voidSendACK(2);
		USART_u8TransmitArraySynch(USARTxNum, (u8*)&ChipID, 2);
	}
	else
		BL_voidSendNACK();
}

u8 BL_u8ExcuteFlashErase(u32 u32SectorNumber, u8 u8NumberOfSectors)
{
	u8 SectorStatus = INVALID_SECTOR_NUMBER;
	FMI_EraseTypeDef FlashErase;

	if((u32SectorNumber > FLASH_MAX_NUM_SECTROS-1) && (u8NumberOfSectors != CBL_FLASH_MASS_ERASE))
		SectorStatus = INVALID_SECTOR_NUMBER;

	else if((u32SectorNumber > FLASH_MAX_NUM_SECTROS) && (u8NumberOfSectors != CBL_FLASH_MASS_ERASE))
		SectorStatus = INVALID_SECTOR_NUMBER;

	else
	{
		if (u32SectorNumber == CBL_FLASH_MASS_ERASE)
			FlashErase.EraseType = FMI_MassErase;

		else
		{
			FlashErase.EraseType = FMI_SectorErase;
			FlashErase.StartSector = u32SectorNumber;

			u8 RemainingSectors = FLASH_MAX_NUM_SECTROS - u32SectorNumber;

			if(u32SectorNumber > RemainingSectors)
				u8NumberOfSectors = RemainingSectors;

			FlashErase.EndSector = u32SectorNumber + u8NumberOfSectors;
		}

		FlashErase.Parallelism = FMI_PAR_HWORD;

		FMI_voidUnlock();
		FMI_voidErase(&FlashErase);
		FMI_voidLock();

		SectorStatus = VALID_SECTOR_NUMBER;
	}
	return SectorStatus;
}

u8 BL_voidFlashErase(u8 *ptru8HostBuffer)
{
	u8 EraseStatus = UNSUCCESSFUL_ERASE;
	u16 HostPacketLength = 0;
	u32 CRCValue = 0;
	HostPacketLength = ptru8HostBuffer[0] + 1;
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - CRC_Length);

	if (BL_AKN == BL_u8CRC(ptru8HostBuffer, HostPacketLength-CRC_Length, CRCValue))
	{
		EraseStatus = BL_u8ExcuteFlashErase(*((u32*)&ptru8HostBuffer[2]), ptru8HostBuffer[6]);

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

u8 BL_voidFlashWrite(u8 *ptru8HostBuffer)
{
	u8 WriteStatus = FLASH_PAYLOAD_WRITE_FAILED;
	u8 AddressStatus = INVALID_ADDRESS;
	u16 HostPacketLength = 0;
	u32 CRCValue = 0;
	HostPacketLength = ptru8HostBuffer[0] + 1;
	CRCValue = *(u32 *)(ptru8HostBuffer + HostPacketLength - CRC_Length);

	if (BL_AKN == BL_u8CRC(ptru8HostBuffer, HostPacketLength-CRC_Length, CRCValue))
	{
		BL_voidSendACK(1);
		u32 BaseAddress = *((u32*)&ptru8HostBuffer[2]);
		AddressStatus = BL_AddressVarification(BaseAddress);
		if (AddressStatus == VALID_ADDRESS)
			WriteStatus = BL_u8ExcuteFlashPayloadWrite((u8*)&ptru8HostBuffer[7], BaseAddress, ptru8HostBuffer[6]);
		else
			WriteStatus = FLASH_PAYLOAD_WRITE_FAILED;
	}
	else
	{
		BL_voidSendNACK();
		WriteStatus = FLASH_PAYLOAD_WRITE_FAILED;
	}

	USART_voidTransmitByte(USARTxNum, WriteStatus);
	return WriteStatus;
}

u8 BL_AddressVarification(u32 u32Address)
{
	u8 AddressStatus;

	if((u32Address >= FLASH_START) && (u32Address <= FLASH_END))
		AddressStatus = VALID_ADDRESS;
	else if((u32Address >= SRAM_START) && (u32Address <= SRAM_END))
		AddressStatus = VALID_ADDRESS;
	else
		AddressStatus = INVALID_ADDRESS;

	return AddressStatus;
}

u8 BL_u8ExcuteFlashPayloadWrite(u8 *ptru8Data, u32 u32StartAddress, u8 u8Length)
{
	u8 PayloadStatus = FLASH_PAYLOAD_WRITE_FAILED;

	// Address in Flash
	if((u32StartAddress >= FLASH_START) && (u32StartAddress <= FLASH_END))
	{
		FMI_WriteTypeDef FlashProgram;
		FlashProgram.Parallelism = FMI_PAR_BYTE;

		FMI_voidUnlock();
		for(u8 iterator = 0; iterator < u8Length; iterator++)
		{
			FlashProgram.Data = ptru8Data[iterator];
			FlashProgram.BaseAddress = u32StartAddress+iterator;
			if (FMI_u8FlashWrite(&FlashProgram) == 0)
				PayloadStatus = FLASH_PAYLOAD_WRITE_PASSED;
		}
		FMI_voidLock();
	}

	// Address in SRAM
	else
	{
		u8* DistinationSRAM = (u8*)u32StartAddress;

		for(u32 Iterator = 0; Iterator < u8Length; Iterator++)
			DistinationSRAM[Iterator] = ptru8Data[Iterator];
	}

	return PayloadStatus;
}
