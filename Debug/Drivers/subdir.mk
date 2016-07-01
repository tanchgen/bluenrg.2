################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/stm32f0xx_hal.c \
../Drivers/stm32f0xx_hal_cortex.c \
../Drivers/stm32f0xx_hal_gpio.c \
../Drivers/stm32f0xx_hal_iwdg.c \
../Drivers/stm32f0xx_hal_rcc.c \
../Drivers/stm32f0xx_hal_rcc_ex.c \
../Drivers/stm32f0xx_hal_spi.c \
../Drivers/stm32f0xx_hal_spi_ex.c \
../Drivers/stm32f0xx_hal_tim.c \
../Drivers/stm32f0xx_hal_tim_ex.c 

OBJS += \
./Drivers/stm32f0xx_hal.o \
./Drivers/stm32f0xx_hal_cortex.o \
./Drivers/stm32f0xx_hal_gpio.o \
./Drivers/stm32f0xx_hal_iwdg.o \
./Drivers/stm32f0xx_hal_rcc.o \
./Drivers/stm32f0xx_hal_rcc_ex.o \
./Drivers/stm32f0xx_hal_spi.o \
./Drivers/stm32f0xx_hal_spi_ex.o \
./Drivers/stm32f0xx_hal_tim.o \
./Drivers/stm32f0xx_hal_tim_ex.o 

C_DEPS += \
./Drivers/stm32f0xx_hal.d \
./Drivers/stm32f0xx_hal_cortex.d \
./Drivers/stm32f0xx_hal_gpio.d \
./Drivers/stm32f0xx_hal_iwdg.d \
./Drivers/stm32f0xx_hal_rcc.d \
./Drivers/stm32f0xx_hal_rcc_ex.d \
./Drivers/stm32f0xx_hal_spi.d \
./Drivers/stm32f0xx_hal_spi_ex.d \
./Drivers/stm32f0xx_hal_tim.d \
./Drivers/stm32f0xx_hal_tim_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/%.o: ../Drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F030 -DHSE_VALUE=8000000 -DUSE_HAL_DRIVER -DSTM32F030X6 -I"../inc" -I"../system/inc" -I"../system/inc/cmsis" -I"../system/inc/stm32f0-stdperiph" -I../bluenrg_inc -I../stm32_inc -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


