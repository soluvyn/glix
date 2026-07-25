ABI      := arm64-v8a
PLATFORM := android-21
ID       := gpx
VER  := 1.1.0
BUILD    := build
MODULE   := module
ZIP      := $(ID)-$(VER).zip
SO       := $(BUILD)/lib$(ID).so

$(BUILD)/Makefile:
	cmake -B $(BUILD) \
		-DCMAKE_TOOLCHAIN_FILE=$(NDK)/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=$(ABI) \
		-DANDROID_PLATFORM=$(PLATFORM)

$(SO): $(BUILD)/Makefile
	cmake --build $(BUILD)

zip: $(SO)
	cmake -E copy $(SO) $(MODULE)/zygisk/$(ABI).so
	cd $(MODULE) && zip -9 -Z deflate -r ../$(ZIP) .
	rm -f $(MODULE)/zygisk/$(ABI).so
	rm -rf $(BUILD)

clean:
	rm -rf $(ZIP)
