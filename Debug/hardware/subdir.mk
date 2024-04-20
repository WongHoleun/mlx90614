################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hardware/MLX90614.c \
../hardware/V8.c 

OBJS += \
./hardware/MLX90614.o \
./hardware/V8.o 

C_DEPS += \
./hardware/MLX90614.d \
./hardware/V8.d 


# Each subdirectory must supply rules for building sources it contributes
hardware/%.o hardware/%.su hardware/%.cyclo: ../hardware/%.c hardware/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"S:/STM32/CUBEIDE/mlx90614_stm32f411/hardware" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-hardware

clean-hardware:
	-$(RM) ./hardware/MLX90614.cyclo ./hardware/MLX90614.d ./hardware/MLX90614.o ./hardware/MLX90614.su ./hardware/V8.cyclo ./hardware/V8.d ./hardware/V8.o ./hardware/V8.su

.PHONY: clean-hardware

