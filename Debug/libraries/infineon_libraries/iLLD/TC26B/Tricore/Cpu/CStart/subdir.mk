################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.c" \
"../libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.c" 

COMPILED_SRCS += \
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.src" \
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.src" 

C_DEPS += \
"./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.d" \
"./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.d" 

OBJS += \
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.o" \
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.o" 


# Each subdirectory must supply rules for building sources it contributes
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.src":"../libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.c" "libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.o":"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.src" "libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.src":"../libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.c" "libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.o":"libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.src" "libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cpu-2f-CStart

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cpu-2f-CStart:
	-$(RM) ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.o ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart0.src ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.o ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/IfxCpu_CStart1.src

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cpu-2f-CStart

