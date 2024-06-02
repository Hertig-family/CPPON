################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CppON.cpp \
../LocalCppObj.cpp \
../SCppObj.cpp 

OBJS += \
./CppON.o \
./LocalCppObj.o \
./SCppObj.o 

CPP_DEPS += \
./CppON.d \
./LocalCppObj.d \
./SCppObj.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


