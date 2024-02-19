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

#define DBGMCU_IDCODE_REG		*((volatile uint32_t *)0xE0042000)
#define IDCODE_MASK				0x00000fff

/********************** Function Deceleration **********************/
u8 BL_u8CRC(u8 *ptru8Data, u8 u8DataLength, u32 u32HostCRC);
void BL_voidSendACK(u8 u8DataLength);
void BL_voidSendNACK();
void BL_voidGetVersion(u8 *ptru8HostBuffer);
void BL_voidGetHelp(u8 *ptru8HostBuffer);
void BL_voidGetChipID(u8 *ptru8HostBuffer);
u8 BL_u8ExcuteFlashErase(u8 u8SectorNumber, u8 u8NumberOfSectors);
u8 BL_voidFlashErase(u8 *ptru8HostBuffer);

#endif
