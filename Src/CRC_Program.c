/***********************************************************************************************/
/***********************************************************************************************/
/*********************************** Author: Fatema Ahmed **************************************/
/***************************************** Layer: MCAL *****************************************/
/******************************************* SWC: CRC ******************************************/
/**************************************** Version: 1.00 ****************************************/
/*************************************** Date: 29/01/2024 **************************************/
/***********************************************************************************************/
/***********************************************************************************************/

#include "STD_Types.h"
#include "BitOperations.h"

#include "CRC_Interface.h"
#include "CRC_Register.h"

u32 CRC_u32Accumulate(u32* ptru32Data, u32 u32Length)
{
	u32 CRCResult;
	for(u32 i = 0; i < u32Length; i++)
		CRC->DR = ptru32Data[i];

	CRCResult = CRC->DR;

	return CRCResult;
}

void CRC_voidReset(void)
{
	CRC->CR = 1;
}