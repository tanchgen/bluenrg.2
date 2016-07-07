################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/_write.c \
../src/main.c \
../src/my_function.c \
../src/onewire.0.c \
../src/onewire.c \
../src/stm32xx_it.c 

OBJS += \
./src/_write.o \
./src/main.o \
./src/my_function.o \
./src/onewire.0.o \
./src/onewire.o \
./src/stm32xx_it.o 

C_DEPS += \
./src/_write.d \
./src/main.d \
./src/my_function.d \
./src/onewire.0.d \
./src/onewire.d \
./src/stm32xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra  -g -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F030 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -DUSE_HAL_DRIVER -DSTM32F030X6 -DSTM32F030x6 -I"../inc" -I"../system/inc" -I"../system/inc/cmsis" -I"../system/inc/stm32f0-stdperiph" -I../bluenrg_inc -I../stm32_inc -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


