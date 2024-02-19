#include "STD_Types.h"
#include "BitOperations.h"

#include "RCC_Interface.h"
#include "GPIO_Interface.h"
#include "USART_Interface.h"
#include "Bootloader_Interface.h"
#include "FMI_Interface.h"

USART_Initialization UART = {
		.BaudRate = 115200,
		.DataWidth = USART_MODE_8BIT,
		.Oversampling = USART_OVER_SAMPLING_16,
		.Parity_Enable = USART_DISABLE,
		.Parity_Selection = USART_EVEN_PARITY,
		.TransferDirection = USART_TX_RX,
		.StopBits = USART_STOP_BIT_1,
		.HardwareFlowControl = USART_DISABLE};

USART_ClockInitialization UARTCLOCK = { USART_DISABLE,
										USART_ClockPolarity_Outside_SteadyLow,
										USART_ClockPhase_FirstTransition,
										USART_LastBitClkPulse_LastDataNotOutput};
int main(void)
{
	/***** RCC Configurations *****/
	RCC_u8SystemClkInitialization();

	RCC_u8Enable(RCC_AHB1, RCC_GPIOA);		// Enable Port A
	RCC_u8Enable (RCC_APB2, RCC_USART1);	// Enable USART1

	/***** Push Button Configurations *****/
	GPIO_u8SetGPIOPinMode (GPIO_PORTA, GPIO_PIN0, GPIO_MODER_INPUT);
	GPIO_u8SetPullResMode (GPIO_PORTA, GPIO_PIN0, GPIO_PUPDR_PULLUP);

	/***** USART Configurations *****/
	GPIO_u8SetGPIOPinMode (GPIO_PORTA, GPIO_PIN9, GPIO_MODER_ALTERNATE);
	GPIO_u8SetGPIOPinMode (GPIO_PORTA, GPIO_PIN10, GPIO_MODER_ALTERNATE);
	GPIO_voidSetAlternativeFunction(GPIO_PORTA, GPIO_PIN9, AF7);
	GPIO_voidSetAlternativeFunction(GPIO_PORTA, GPIO_PIN10, AF7);

	USART_voidInitialization(&UART, &UARTCLOCK, USART1);
	USART_Enable(USART1);

	FMI_WriteTypeDef FlashProgram;
	FlashProgram.BaseAddress = 0x08008030;
	FlashProgram.Parallelism = FMI_PAR_WORD;
	FlashProgram.Data = 0x0020043;
	FMI_voidUnlock();
	FMI_u8FlashWrite(&FlashProgram);
	FMI_voidLock();

//	u8 ButtonValue = 0;
	while(1)
	{
		/***** Bootloader *****/
		BL_FeatchHostCmd();

//		GPIO_u8GetPinValue (GPIO_PORTA, GPIO_PIN0, &ButtonValue);
//		if (ButtonValue == 0)
//			BL_FeatchHostCmd();
//		else
//			BL_voidJumpToUserApp();
	}
}
