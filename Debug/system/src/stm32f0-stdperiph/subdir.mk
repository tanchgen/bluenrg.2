################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/stm32f0-stdperiph/stm32f0xx_dma.c \
../system/src/stm32f0-stdperiph/stm32f0xx_exti.c \
../system/src/stm32f0-stdperiph/stm32f0xx_gpio.c \
../system/src/stm32f0-stdperiph/stm32f0xx_rcc.c \
../system/src/stm32f0-stdperiph/stm32f0xx_rtc.c \
../system/src/stm32f0-stdperiph/stm32f0xx_spi.c \
../system/src/stm32f0-stdperiph/stm32f0xx_usart.c 

OBJS += \
./system/src/stm32f0-stdperiph/stm32f0xx_dma.o \
./system/src/stm32f0-stdperiph/stm32f0xx_exti.o \
./system/src/stm32f0-stdperiph/stm32f0xx_gpio.o \
./system/src/stm32f0-stdperiph/stm32f0xx_rcc.o \
./system/src/stm32f0-stdperiph/stm32f0xx_rtc.o \
./system/src/stm32f0-stdperiph/stm32f0xx_spi.o \
./system/src/stm32f0-stdperiph/stm32f0xx_usart.o 

C_DEPS += \
./system/src/stm32f0-stdperiph/stm32f0xx_dma.d \
./system/src/stm32f0-stdperiph/stm32f0xx_exti.d \
./system/src/stm32f0-stdperiph/stm32f0xx_gpio.d \
./system/src/stm32f0-stdperiph/stm32f0xx_rcc.d \
./system/src/stm32f0-stdperiph/stm32f0xx_rtc.d \
./system/src/stm32f0-stdperiph/stm32f0xx_spi.d \
./system/src/stm32f0-stdperiph/stm32f0xx_usart.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/stm32f0-stdperiph/%.o: ../system/src/stm32f0-stdperiph/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F030 -DHSE_VALUE=16000000 -DSTM32F030x6 -DBLUENRG=1 -DUSE_STDPERIPH_DRIVER -DONE_WIRE=1 -I"/home/jet/work/workspace/expedition/inc" -I"/home/jet/work/workspace/expedition/system/inc" -I"/home/jet/work/workspace/expedition/system/inc/cmsis" -I"/home/jet/work/workspace/expedition/system/inc/stm32f0-stdperiph" -I"/home/jet/work/workspace/expedition/bluenrg/inc" -I"/home/jet/work/workspace/expedition/system/inc/stm32f0-hal" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


