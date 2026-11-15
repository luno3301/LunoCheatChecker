CMAKE ?= cmake
BUILD_DIR ?= build
CMAKE_ARGS ?= -G "MSYS Makefiles" -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/usr/bin/make.exe -DCMAKE_PREFIX_PATH=C:/msys64/mingw64

.PHONY: all configure clean scanner-debug scanner-release

all: configure
	$(CMAKE) --build "$(BUILD_DIR)"

configure:
	$(CMAKE) -S . -B "$(BUILD_DIR)" $(CMAKE_ARGS)

clean:
	$(CMAKE) --build "$(BUILD_DIR)" --target clean

scanner-release:
	$(CMAKE) --preset scanner-release
	$(CMAKE) --build --preset scanner-release

scanner-debug:
	$(CMAKE) --preset scanner-debug
	$(CMAKE) --build --preset scanner-debug
