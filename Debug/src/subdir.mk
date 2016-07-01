################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bluenrg_interface.c \
../src/bt01.c \
../src/clock.c \
../src/main.c \
../src/my_function.c \
../src/my_service.c \
../src/stm32f0xx_hal_msp.c \
../src/stm32xx_it.c 

OBJS += \
./src/bluenrg_interface.o \
./src/bt01.o \
./src/clock.o \
./src/main.o \
./src/my_function.o \
./src/my_service.o \
./src/stm32f0xx_hal_msp.o \
./src/stm32xx_it.o 

C_DEPS += \
./src/bluenrg_interface.d \
./src/bt01.d \
./src/clock.d \
./src/main.d \
./src/my_function.d \
./src/my_service.d \
./src/stm32f0xx_hal_msp.d \
./src/stm32xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F030 -DHSE_VALUE=8000000 -DUSE_HAL_DRIVER -DSTM32F030X6 -I"../inc" -I"../system/inc" -I"../system/inc/cmsis" -I"../system/inc/stm32f0-stdperiph" -I../bluenrg_inc -I../stm32_inc -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


