################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.c" \
"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.c" \
"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.c" \
"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.c" 

COMPILED_SRCS += \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.src" \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.src" \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.src" \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.src" 

C_DEPS += \
"./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.d" \
"./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.d" \
"./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.d" \
"./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.d" 

OBJS += \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.o" \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.o" \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.o" \
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.o" 


# Each subdirectory must supply rules for building sources it contributes
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.src":"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.c" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.o":"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.src" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.src":"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.c" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.o":"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.src" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.src":"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.c" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.o":"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.src" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.src":"../libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.c" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.o":"libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.src" "libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-libraries-2f-infineon_libraries-2f-Service-2f-CpuGeneric-2f-StdIf

clean-libraries-2f-infineon_libraries-2f-Service-2f-CpuGeneric-2f-StdIf:
	-$(RM) ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.d ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.o ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_DPipe.src ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.d ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.o ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Pos.src ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.d ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.o ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_PwmHl.src ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.d ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.o ./libraries/infineon_libraries/Service/CpuGeneric/StdIf/IfxStdIf_Timer.src

.PHONY: clean-libraries-2f-infineon_libraries-2f-Service-2f-CpuGeneric-2f-StdIf

