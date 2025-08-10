################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v00X_it.c \
../User/main.c \
../User/oled_049.c \
../User/system_ch32v00X.c 

OBJS += \
./User/ch32v00X_it.o \
./User/main.o \
./User/oled_049.o \
./User/system_ch32v00X.o 

C_DEPS += \
./User/ch32v00X_it.d \
./User/main.d \
./User/oled_049.d \
./User/system_ch32v00X.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\MRS_DATA\workspace\ADC_POT_CH32V002J4M6_R2\Debug" -I"C:\MRS_DATA\workspace\ADC_POT_CH32V002J4M6_R2\Core" -I"C:\MRS_DATA\workspace\ADC_POT_CH32V002J4M6_R2\User" -I"C:\MRS_DATA\workspace\ADC_POT_CH32V002J4M6_R2\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

