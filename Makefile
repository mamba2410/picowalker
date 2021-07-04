# Makefile for Raspberry pico projects


## Handle init with nwd
#init:
#	@cp ${PICO_SDK_PATH}/external/pico_sdk_import.cmake ./
#	@if [ ! -d build ]; then mkdir build; fi
#	@cd build
#	@pwd
#	@cmake ..
#	@echo "Done initialising"

.PHONY : build
build:
	@echo "Building"
	@$(MAKE) --silent -C build

build_debug: build
	@echo "Sorry, do this manually. `cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..`"

send:
	# openocd -f tcl/interface/raspberrypi-swd.cfg -f tcl/target/rp2040.cfg

clean:
	@find build -name *.obj | xargs rm
	@rm -f build/*.elf build/*.uf2 build/*.bin build/*.dis build/*.hex

rebuild: clean build
