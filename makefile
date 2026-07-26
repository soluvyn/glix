CC := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang
CFLAGS := -shared -Os -s -fvisibility=hidden -std=c11

VERSION := $(shell sed -n 's/^version=//p' module/module.prop)
ZIP := glix-$(VERSION).zip
SO := module/zygisk/arm64-v8a.so

all: $(ZIP)

$(SO): glix.c zygisk.h
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ZIP): module/module.prop module/system/product/etc/sysconfig/glix.xml $(SO)
	rm -f $@
	cd module && zip -r ../$@ module.prop system zygisk

clean:
	rm -rf module/zygisk $(ZIP)

.PHONY: all clean
