################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fifo.c \
../src/flash.c \
../src/keypad.c \
../src/main.c \
../src/motor.c \
../src/prox.c \
../src/syscalls.c \
../src/system_stm32f0xx.c \
../src/tty.c \
../src/utils.c \
../src/wifi_uart.c 

OBJS += \
./src/fifo.o \
./src/flash.o \
./src/keypad.o \
./src/main.o \
./src/motor.o \
./src/prox.o \
./src/syscalls.o \
./src/system_stm32f0xx.o \
./src/tty.o \
./src/utils.o \
./src/wifi_uart.o 

C_DEPS += \
./src/fifo.d \
./src/flash.d \
./src/keypad.d \
./src/main.d \
./src/motor.d \
./src/prox.d \
./src/syscalls.d \
./src/system_stm32f0xx.d \
./src/tty.d \
./src/utils.d \
./src/wifi_uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F0 -DSTM32F091RCTx -DDEBUG -DSTM32F091 -DUSE_STDPERIPH_DRIVER -I"/Users/husaisma/Purdue/ECE 477/Interfacing-with-Keypad/Keypad_Interface/StdPeriph_Driver/inc" -I"/Users/husaisma/Purdue/ECE 477/Interfacing-with-Keypad/Keypad_Interface/inc" -I"/Users/husaisma/Purdue/ECE 477/Interfacing-with-Keypad/Keypad_Interface/CMSIS/device" -I"/Users/husaisma/Purdue/ECE 477/Interfacing-with-Keypad/Keypad_Interface/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


