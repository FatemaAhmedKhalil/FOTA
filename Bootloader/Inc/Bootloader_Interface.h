/***********************************************************************************************/
/***********************************************************************************************/
/*********************************** Author: Fatema Ahmed **************************************/
/****************************************** Layer: HAL *****************************************/
/*************************************** SWC: Bootloader ***************************************/
/**************************************** Version: 1.00 ****************************************/
/*************************************** Date: 25/01/2024 **************************************/
/***********************************************************************************************/
/***********************************************************************************************/

#ifndef BOOTLOADER_INTERFACE_H_
#define BOOTLOADER_INTERFACE_H_

#define BL_HostSize		200

/******************* Bootloader Frame *********************/
/* |	Data Length	|	 CMD	|	Data	|	CRC		| */
/* |	  1 Byte	|	1 Byte	|			|	4 Bytes	| */
/**********************************************************/

/********** Bootloader Commands **********/
#define CBL_GET_VER_CMD              0x10
#define CBL_GET_HELP_CMD             0x11
#define CBL_GET_CID_CMD              0x12
#define CBL_GET_RDP_STATUS_CMD       0x13
#define CBL_GO_TO_ADDR_CMD           0x14
#define CBL_FLASH_ERASE_CMD          0x15
#define CBL_MEM_WRITE_CMD            0x16
#define CBL_ED_W_PROTECT_CMD         0x17
#define CBL_MEM_READ_CMD             0x18
#define CBL_READ_SECTOR_STATUS_CMD   0x19
#define CBL_OTP_READ_CMD             0x20
#define CBL_CHANGE_ROP_Level_CMD     0x21

/********** Version Information **********/
#define CBL_VENDOR_ID				100
#define CBL_SW_MAJOR_VENRSION		1
#define CBL_SW_MINOR_VENRSION		1
#define CBL_SW_PATCH_VENRSION		0

/************* Flash Status *************/
#define INVALID_SECTOR_NUMBER         0x00
#define VALID_SECTOR_NUMBER           0x01
#define UNSUCCESSFUL_ERASE            0x02
#define SUCCESSFUL_ERASE              0x03
#define INVALID_ADDRESS				  0x04
#define VALID_ADDRESS         		  0x05

#define FLASH_PAYLOAD_WRITE_FAILED    0x00
#define FLASH_PAYLOAD_WRITE_PASSED    0x01

#define CBL_FLASH_MASS_ERASE		0xFF
#define FLASH_MAX_NUM_SECTROS		8

/************* Acknowledgment Status *************/
typedef enum {
	BL_NAKN = 0xAB,
	BL_AKN = 0xCD
} BL_Status;

/************* USART Number *************/
#define USARTxNum		USART1

/********************************************************************************************************************
* @fn             : BL_voidApplication
* @brief          : Execute the Application
* @param[in]      :
* @retval         :
**********************************************************************************************************************
* @attention
*
*
**********************************************************************************************************************/
void BL_voidApplication(void);

/********************************************************************************************************************
* @fn             : BL_FeatchHostCmd
* @brief          : Execute Bootloader Command
* @param[in]      :
* @retval         : Acknowledgment Status
**********************************************************************************************************************
* @attention
*
*
**********************************************************************************************************************/
void BL_FeatchHostCmd();

#endif
