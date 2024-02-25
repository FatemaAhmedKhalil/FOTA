################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Bootloader_Program.c \
../Src/CRC_Program.c \
../Src/FMI_Program.c \
../Src/GPIO_Program.c \
../Src/RCC_Program.c \
../Src/USART_Program.c \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/Bootloader_Program.o \
./Src/CRC_Program.o \
./Src/FMI_Program.o \
./Src/GPIO_Program.o \
./Src/RCC_Program.o \
./Src/USART_Program.o \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/Bootloader_Program.d \
./Src/CRC_Program.d \
./Src/FMI_Program.d \
./Src/GPIO_Program.d \
./Src/RCC_Program.d \
./Src/USART_Program.d \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F401CCUx -DSTM32 -DSTM32F4 -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Bootloader_Program.d ./Src/Bootloader_Program.o ./Src/Bootloader_Program.su ./Src/CRC_Program.d ./Src/CRC_Program.o ./Src/CRC_Program.su ./Src/FMI_Program.d ./Src/FMI_Program.o ./Src/FMI_Program.su ./Src/GPIO_Program.d ./Src/GPIO_Program.o ./Src/GPIO_Program.su ./Src/RCC_Program.d ./Src/RCC_Program.o ./Src/RCC_Program.su ./Src/USART_Program.d ./Src/USART_Program.o ./Src/USART_Program.su ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

