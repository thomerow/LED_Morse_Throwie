################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
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
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
LED_Morse_Throwie.lss \

FLASH_IMAGE += \
LED_Morse_Throwie.hex \

EEPROM_IMAGE += \
LED_Morse_Throwie.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: LED_Morse_Throwie.elf secondary-outputs

# Tool invocations
LED_Morse_Throwie.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,LED_Morse_Throwie.map -mmcu=attiny45 -o "LED_Morse_Throwie.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

LED_Morse_Throwie.lss: LED_Morse_Throwie.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S LED_Morse_Throwie.elf  >"LED_Morse_Throwie.lss"
	@echo 'Finished building: $@'
	@echo ' '

LED_Morse_Throwie.hex: LED_Morse_Throwie.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex LED_Morse_Throwie.elf  "LED_Morse_Throwie.hex"
	@echo 'Finished building: $@'
	@echo ' '

LED_Morse_Throwie.eep: LED_Morse_Throwie.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex LED_Morse_Throwie.elf  "LED_Morse_Throwie.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: LED_Morse_Throwie.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=attiny45 LED_Morse_Throwie.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) LED_Morse_Throwie.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
