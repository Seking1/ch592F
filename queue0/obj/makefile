################################################################################
# MRS Version: 1.9.1
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include StdPeriphDriver/subdir.mk
-include Startup/subdir.mk
-include RVMSIS/subdir.mk
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
queue0.hex \

SECONDARY_LIST += \
queue0.lst \

SECONDARY_SIZE += \
queue0.siz \


# All Target
all: queue0.elf secondary-outputs

# Tool invocations
queue0.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T "C:\MRS_DATA\workspace\queue0\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"../" -L"C:\MRS_DATA\workspace\queue0\StdPeriphDriver" -Xlinker --print-memory-usage -Wl,-Map,"queue0.map" --specs=nano.specs --specs=nosys.specs -o "queue0.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@	@
queue0.hex: queue0.elf
	@	riscv-none-embed-objcopy -O ihex "queue0.elf"  "queue0.hex"
	@	@
queue0.lst: queue0.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "queue0.elf" > "queue0.lst"
	@	@
queue0.siz: queue0.elf
	@	riscv-none-embed-size --format=berkeley "queue0.elf"
	@	@
# Other Targets
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) queue0.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
