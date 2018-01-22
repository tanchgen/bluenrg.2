################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/eeprom.c \
../src/init.c \
../src/logger.c \
../src/main.c \
../src/my_function.c \
../src/my_service.c \
../src/onewire.c \
../src/stm32xx_it.c \
../src/thermo.c \
../src/time.c 

OBJS += \
./src/eeprom.o \
./src/init.o \
./src/logger.o \
./src/main.o \
./src/my_function.o \
./src/my_service.o \
./src/onewire.o \
./src/stm32xx_it.o \
./src/thermo.o \
./src/time.o 

C_DEPS += \
./src/eeprom.d \
./src/init.d \
./src/logger.d \
./src/main.d \
./src/my_function.d \
./src/my_service.d \
./src/onewire.d \
./src/stm32xx_it.d \
./src/thermo.d \
./src/time.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F030 -DHSE_VALUE=16000000 -DSTM32F030x6 -DBLUENRG=1 -DUSE_STDPERIPH_DRIVER -DONE_WIRE=1 -I"/home/jet/work/workspace/pak.exp-2/inc" -I"/home/jet/work/workspace/pak.exp-2/system/inc" -I"/home/jet/work/workspace/pak.exp-2/system/inc/cmsis" -I"/home/jet/work/workspace/pak.exp-2/system/inc/stm32f0-stdperiph" -I"/home/jet/work/workspace/pak.exp-2/bluenrg/inc" -I"/home/jet/work/workspace/pak.exp-2/system/inc/stm32f0-hal" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


