/***********************************************************************************************/
/***********************************************************************************************/
/*********************************** Author: Fatema Ahmed **************************************/
/***************************************** Layer: MCAL *****************************************/
/********************************* SWC: Flash Memory Interface *********************************/
/**************************************** Version: 1.00 ****************************************/
/*************************************** Date: 5/02/2024 ***************************************/
/***********************************************************************************************/
/***********************************************************************************************/

#ifndef FLASHMEMORYINTERFACE_REGISTER_H_
#define FLASHMEMORYINTERFACE_REGISTER_H_

typedef struct
{
	u32	ACR;		// Flash access control register
	u32 KEYR;		// Flash key register
	u32 OPTKEYR;	// Flash option key register
	u32 SR;			// Flash status register
	u32 CR;			// Flash control register
	u32 OPTCR;		// Flash option control register (
}FMI_MemoryMap;

#define	FMI			( (volatile FMI_MemoryMap *)(0x40023C00) )


#endif
