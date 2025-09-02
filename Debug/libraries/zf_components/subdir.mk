################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../libraries/zf_components/seekfree_assistant.c" \
"../libraries/zf_components/seekfree_assistant_interface.c" 

COMPILED_SRCS += \
"libraries/zf_components/seekfree_assistant.src" \
"libraries/zf_components/seekfree_assistant_interface.src" 

C_DEPS += \
"./libraries/zf_components/seekfree_assistant.d" \
"./libraries/zf_components/seekfree_assistant_interface.d" 

OBJS += \
"libraries/zf_components/seekfree_assistant.o" \
"libraries/zf_components/seekfree_assistant_interface.o" 


# Each subdirectory must supply rules for building sources it contributes
"libraries/zf_components/seekfree_assistant.src":"../libraries/zf_components/seekfree_assistant.c" "libraries/zf_components/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/zf_components/seekfree_assistant.o":"libraries/zf_components/seekfree_assistant.src" "libraries/zf_components/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"libraries/zf_components/seekfree_assistant_interface.src":"../libraries/zf_components/seekfree_assistant_interface.c" "libraries/zf_components/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/Smart_car/code/debugging/back_car/LADRC-V0.3/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"libraries/zf_components/seekfree_assistant_interface.o":"libraries/zf_components/seekfree_assistant_interface.src" "libraries/zf_components/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-libraries-2f-zf_components

clean-libraries-2f-zf_components:
	-$(RM) ./libraries/zf_components/seekfree_assistant.d ./libraries/zf_components/seekfree_assistant.o ./libraries/zf_components/seekfree_assistant.src ./libraries/zf_components/seekfree_assistant_interface.d ./libraries/zf_components/seekfree_assistant_interface.o ./libraries/zf_components/seekfree_assistant_interface.src

.PHONY: clean-libraries-2f-zf_components

