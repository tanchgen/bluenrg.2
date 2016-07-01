################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/bluenrg/bluenrg_gap_aci.c \
../Drivers/bluenrg/bluenrg_gatt_aci.c \
../Drivers/bluenrg/bluenrg_hal_aci.c \
../Drivers/bluenrg/bluenrg_itf_template.c \
../Drivers/bluenrg/bluenrg_l2cap_aci.c \
../Drivers/bluenrg/gp_timer.c \
../Drivers/bluenrg/hci.c \
../Drivers/bluenrg/list.c \
../Drivers/bluenrg/osal.c \
../Drivers/bluenrg/stm32_bluenrg_ble.c 

OBJS += \
./Drivers/bluenrg/bluenrg_gap_aci.o \
./Drivers/bluenrg/bluenrg_gatt_aci.o \
./Drivers/bluenrg/bluenrg_hal_aci.o \
./Drivers/bluenrg/bluenrg_itf_template.o \
./Drivers/bluenrg/bluenrg_l2cap_aci.o \
./Drivers/bluenrg/gp_timer.o \
./Drivers/bluenrg/hci.o \
./Drivers/bluenrg/list.o \
./Drivers/bluenrg/osal.o \
./Drivers/bluenrg/stm32_bluenrg_ble.o 

C_DEPS += \
./Drivers/bluenrg/bluenrg_gap_aci.d \
./Drivers/bluenrg/bluenrg_gatt_aci.d \
./Drivers/bluenrg/bluenrg_hal_aci.d \
./Drivers/bluenrg/bluenrg_itf_template.d \
./Drivers/bluenrg/bluenrg_l2cap_aci.d \
./Drivers/bluenrg/gp_timer.d \
./Drivers/bluenrg/hci.d \
./Drivers/bluenrg/list.d \
./Drivers/bluenrg/osal.d \
./Drivers/bluenrg/stm32_bluenrg_ble.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/bluenrg/%.o: ../Drivers/bluenrg/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F030 -DHSE_VALUE=8000000 -DUSE_HAL_DRIVER -DSTM32F030x6 -DBLENRG=0 -DWATCHDOG=0 -DONEWIRE=1 -I"../inc" -I"../system/inc" -I"../system/inc/cmsis" -I"../system/inc/stm32f0-stdperiph" -I../bluenrg_inc -I../stm32_inc -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


