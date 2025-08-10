################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
ADC_POT_CH32V002J4M6_R2.hex \

SECONDARY_LIST += \
ADC_POT_CH32V002J4M6_R2.lst \

SECONDARY_SIZE += \
ADC_POT_CH32V002J4M6_R2.siz \


# All Target
all: ADC_POT_CH32V002J4M6_R2.elf secondary-outputs

# Tool invocations
ADC_POT_CH32V002J4M6_R2.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T "C:\MRS_DATA\workspace\ADC_POT_CH32V002J4M6_R2\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"ADC_POT_CH32V002J4M6_R2.map" --specs=nano.specs --specs=nosys.specs -o "ADC_POT_CH32V002J4M6_R2.elf" $(OBJS) $(USER_OBJS) $(LIBS) -lprintf
	@	@
ADC_POT_CH32V002J4M6_R2.hex: ADC_POT_CH32V002J4M6_R2.elf
	@	riscv-none-elf-objcopy -O ihex "ADC_POT_CH32V002J4M6_R2.elf"  "ADC_POT_CH32V002J4M6_R2.hex"
	@	@
ADC_POT_CH32V002J4M6_R2.lst: ADC_POT_CH32V002J4M6_R2.elf
	@	riscv-none-elf-objdump --all-headers --demangle --disassemble -M xw "ADC_POT_CH32V002J4M6_R2.elf" > "ADC_POT_CH32V002J4M6_R2.lst"
	@	@
ADC_POT_CH32V002J4M6_R2.siz: ADC_POT_CH32V002J4M6_R2.elf
	@	riscv-none-elf-size --format=berkeley "ADC_POT_CH32V002J4M6_R2.elf"
	@	@
# Other Targets
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) ADC_POT_CH32V002J4M6_R2.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
