/***********************************************************************************************/
/***********************************************************************************************/
/*********************************** Author: Fatema Ahmed **************************************/
/****************************************** Layer: HAL *****************************************/
/*************************************** SWC: Bootloader ***************************************/
/**************************************** Version: 1.00 ****************************************/
/*************************************** Date: 25/01/2024 **************************************/
/***********************************************************************************************/
/***********************************************************************************************/

#ifndef BOOTLOADER_PRIVATE_H_
#define BOOTLOADER_PRIVATE_H_

/****** INCODE Register to get Chip ID ******/
#define DBGMCU_IDCODE_REG		*((volatile u32 *)0xE0042000)
#define IDCODE_MASK				0x00000fff

/******* CRC Bytes in the Packet *******/
#define CRC_Length				4

/****** Basic Implementation of Flash and SRAM ******/
#define FLASH_SIZE				(512*1024)
#define SRAM_SIZE				(128*1024)

#define FLASH_START				0x08000000UL
#define FLASH_END				((FLASH_START)+(FLASH_SIZE))

#define SRAM_START				0x2000000UL
#define SRAM_END				((SRAM_START)+(SRAM_SIZE))

/********************** Function Deceleration **********************/
u8 BL_u8CRC(u8 *ptru8Data, u8 u8DataLength, u32 u32HostCRC);
void BL_voidSendACK(u8 u8DataLength);
void BL_voidSendNACK();
void BL_voidGetVersion(u8 *ptru8HostBuffer);
void BL_voidGetHelp(u8 *ptru8HostBuffer);
void BL_voidGetChipID(u8 *ptru8HostBuffer);
u8 BL_u8ExcuteFlashErase(u32 u32SectorNumber, u8 u8NumberOfSectors);
u8 BL_voidFlashErase(u8 *ptru8HostBuffer);
u8 BL_voidFlashWrite(u8 *ptru8HostBuffer);
u8 BL_AddressVarification(u32 u32Address);
u8 BL_u8ExcuteFlashPayloadWrite(u16 *ptru8Data, u32 u32StartAddress, u8 u8Length);

#endif
