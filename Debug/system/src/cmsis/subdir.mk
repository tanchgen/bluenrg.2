################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/cmsis/system_stm32f0xx.c 

S_UPPER_SRCS += \
../system/src/cmsis/startup_stm32f030x6.S 

OBJS += \
./system/src/cmsis/startup_stm32f030x6.o \
./system/src/cmsis/system_stm32f0xx.o 

C_DEPS += \
./system/src/cmsis/system_stm32f0xx.d 

S_UPPER_DEPS += \
./system/src/cmsis/startup_stm32f030x6.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/cmsis/%.o: ../system/src/cmsis/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -x assembler-with-cpp -DDEBUG -DSTM32F030 -DHSE_VALUE=16000000 -DSTM32F030x6 -DBLUENRG=1 -DWATCHDOG=0 -DUSE_STDPERIPH_DRIVER -DONE_WIRE=1 -I../inc -I../system/inc -I../system/inc/cmsis -I../system/inc/stm32f0-stdperiph -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

system/src/cmsis/%.o: ../system/src/cmsis/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F030 -DHSE_VALUE=16000000 -DSTM32F030x6 -DBLUENRG=1 -DWATCHDOG=0 -DUSE_STDPERIPH_DRIVER -DONE_WIRE=1 -I"/home/jet/work/workspace/expedition/inc" -I"/home/jet/work/workspace/expedition/system/inc" -I"/home/jet/work/workspace/expedition/system/inc/cmsis" -I"/home/jet/work/workspace/expedition/system/inc/stm32f0-stdperiph" -I"/home/jet/work/workspace/expedition/bluenrg/inc" -I"/home/jet/work/workspace/expedition/system/inc/stm32f0-hal" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


