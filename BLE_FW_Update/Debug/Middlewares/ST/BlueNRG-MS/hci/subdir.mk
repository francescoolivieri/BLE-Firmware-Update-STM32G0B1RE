################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/ST/BlueNRG-MS/hci/hci_le.c 

OBJS += \
./Middlewares/ST/BlueNRG-MS/hci/hci_le.o 

C_DEPS += \
./Middlewares/ST/BlueNRG-MS/hci/hci_le.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/ST/BlueNRG-MS/hci/%.o Middlewares/ST/BlueNRG-MS/hci/%.su Middlewares/ST/BlueNRG-MS/hci/%.cyclo: ../Middlewares/ST/BlueNRG-MS/hci/%.c Middlewares/ST/BlueNRG-MS/hci/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G0B1xx -c -I../BlueNRG-MS/Target -I"C:/Users/Francesco Olivieri/Documents/ST-Libraries/STM32CubeExpansion_Crypto_V4.1.0/Middlewares/ST/STM32_Cryptographic/include" -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/ST/BlueNRG-MS/utils -I../Middlewares/ST/BlueNRG-MS/includes -I../Middlewares/ST/BlueNRG-MS/hci/hci_tl_patterns/Basic -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-ST-2f-BlueNRG-2d-MS-2f-hci

clean-Middlewares-2f-ST-2f-BlueNRG-2d-MS-2f-hci:
	-$(RM) ./Middlewares/ST/BlueNRG-MS/hci/hci_le.cyclo ./Middlewares/ST/BlueNRG-MS/hci/hci_le.d ./Middlewares/ST/BlueNRG-MS/hci/hci_le.o ./Middlewares/ST/BlueNRG-MS/hci/hci_le.su

.PHONY: clean-Middlewares-2f-ST-2f-BlueNRG-2d-MS-2f-hci

