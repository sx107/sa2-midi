################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/i2c.c \
../Src/main.c \
../Src/midi.c \
../Src/midi_sa2_handler.c \
../Src/sa2.c \
../Src/si5351.c \
../Src/stm32f1xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c 

C_DEPS += \
./Src/i2c.d \
./Src/main.d \
./Src/midi.d \
./Src/midi_sa2_handler.d \
./Src/sa2.d \
./Src/si5351.d \
./Src/stm32f1xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d 

OBJS += \
./Src/i2c.o \
./Src/main.o \
./Src/midi.o \
./Src/midi_sa2_handler.o \
./Src/sa2.o \
./Src/si5351.o \
./Src/stm32f1xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o 


# Each subdirectory must supply rules for building sources it contributes
Src/i2c.o: ../Src/i2c.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/i2c.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/main.o: ../Src/main.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/main.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/midi.o: ../Src/midi.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/midi.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/midi_sa2_handler.o: ../Src/midi_sa2_handler.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/midi_sa2_handler.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/sa2.o: ../Src/sa2.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/sa2.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/si5351.o: ../Src/si5351.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/si5351.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/stm32f1xx_it.o: ../Src/stm32f1xx_it.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/stm32f1xx_it.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/syscalls.o: ../Src/syscalls.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/syscalls.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Src/sysmem.o: ../Src/sysmem.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -c -I../Inc -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Include" -I"E:/Sx/Programming/stm-workspace/stm32-sa2/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/sysmem.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

