################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/All_Init.c \
../code/PID.c \
../code/PIT_Demo.c \
../code/PTS_Deal.c \
../code/Show_Data.c \
../code/clip.c \
../code/exposure_adjust.c \
../code/image_deal.c \
../code/image_storage.c \
../code/infrared_nec.c \
../code/kalman_filter.c \
../code/key.c \
../code/ladrc.c \
../code/menu.c \
../code/menu_list.c \
../code/parameter_flash.c \
../code/pico_gy.c \
../code/pico_link_ii.c \
../code/w25n04.c 

COMPILED_SRCS += \
./code/All_Init.src \
./code/PID.src \
./code/PIT_Demo.src \
./code/PTS_Deal.src \
./code/Show_Data.src \
./code/clip.src \
./code/exposure_adjust.src \
./code/image_deal.src \
./code/image_storage.src \
./code/infrared_nec.src \
./code/kalman_filter.src \
./code/key.src \
./code/ladrc.src \
./code/menu.src \
./code/menu_list.src \
./code/parameter_flash.src \
./code/pico_gy.src \
./code/pico_link_ii.src \
./code/w25n04.src 

C_DEPS += \
./code/All_Init.d \
./code/PID.d \
./code/PIT_Demo.d \
./code/PTS_Deal.d \
./code/Show_Data.d \
./code/clip.d \
./code/exposure_adjust.d \
./code/image_deal.d \
./code/image_storage.d \
./code/infrared_nec.d \
./code/kalman_filter.d \
./code/key.d \
./code/ladrc.d \
./code/menu.d \
./code/menu_list.d \
./code/parameter_flash.d \
./code/pico_gy.d \
./code/pico_link_ii.d \
./code/w25n04.d 

OBJS += \
./code/All_Init.o \
./code/PID.o \
./code/PIT_Demo.o \
./code/PTS_Deal.o \
./code/Show_Data.o \
./code/clip.o \
./code/exposure_adjust.o \
./code/image_deal.o \
./code/image_storage.o \
./code/infrared_nec.o \
./code/kalman_filter.o \
./code/key.o \
./code/ladrc.o \
./code/menu.o \
./code/menu_list.o \
./code/parameter_flash.o \
./code/pico_gy.o \
./code/pico_link_ii.o \
./code/w25n04.o 


# Each subdirectory must supply rules for building sources it contributes
code/%.src: ../code/%.c code/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -cs --dep-file="$(basename $@).d" --misrac-version=2004 -D__CPU__=tc26xb "-fF:/智能车摄像头/代码/调试/后车/LADRC-V0.2/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<" && \
	if [ -f "$(basename $@).d" ]; then sed.exe -r  -e 's/\b(.+\.o)\b/code\/\1/g' -e 's/\\/\//g' -e 's/\/\//\//g' -e 's/"//g' -e 's/([a-zA-Z]:\/)/\L\1/g' -e 's/\d32:/@TARGET_DELIMITER@/g; s/\\\d32/@ESCAPED_SPACE@/g; s/\d32/\\\d32/g; s/@ESCAPED_SPACE@/\\\d32/g; s/@TARGET_DELIMITER@/\d32:/g' "$(basename $@).d" > "$(basename $@).d_sed" && cp "$(basename $@).d_sed" "$(basename $@).d" && rm -f "$(basename $@).d_sed" 2>/dev/null; else echo 'No dependency file to process';fi
	@echo 'Finished building: $<'
	@echo ' '

code/%.o: ./code/%.src code/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-code

clean-code:
	-$(RM) ./code/All_Init.d ./code/All_Init.o ./code/All_Init.src ./code/PID.d ./code/PID.o ./code/PID.src ./code/PIT_Demo.d ./code/PIT_Demo.o ./code/PIT_Demo.src ./code/PTS_Deal.d ./code/PTS_Deal.o ./code/PTS_Deal.src ./code/Show_Data.d ./code/Show_Data.o ./code/Show_Data.src ./code/clip.d ./code/clip.o ./code/clip.src ./code/exposure_adjust.d ./code/exposure_adjust.o ./code/exposure_adjust.src ./code/image_deal.d ./code/image_deal.o ./code/image_deal.src ./code/image_storage.d ./code/image_storage.o ./code/image_storage.src ./code/infrared_nec.d ./code/infrared_nec.o ./code/infrared_nec.src ./code/kalman_filter.d ./code/kalman_filter.o ./code/kalman_filter.src ./code/key.d ./code/key.o ./code/key.src ./code/ladrc.d ./code/ladrc.o ./code/ladrc.src ./code/menu.d ./code/menu.o ./code/menu.src ./code/menu_list.d ./code/menu_list.o ./code/menu_list.src ./code/parameter_flash.d ./code/parameter_flash.o ./code/parameter_flash.src ./code/pico_gy.d ./code/pico_gy.o ./code/pico_gy.src ./code/pico_link_ii.d ./code/pico_link_ii.o ./code/pico_link_ii.src ./code/w25n04.d ./code/w25n04.o ./code/w25n04.src

.PHONY: clean-code

