RACK_DIR ?= ../../..
.NOTPARALLEL:


# Target paths

include $(RACK_DIR)/arch.mk

ifdef ARCH_LIN
	openmpt = lib/openmpt.a
endif

ifdef ARCH_MAC
	openmtp = lib/openmpt.a
endif

ifdef ARCH_WIN
	openmpt = lib/openmpt.a
endif

nanovg = include/openmpt.h

DEPS += $(openmpt)
include $(RACK_DIR)/dep.mk


# Targets

$(openmpt):
	$(WGET) "https://github.com/OpenMPT/openmpt/archive/libopenmpt-0.3.10.tar.gz"
	$(UNTAR) libopenmpt-0.3.10.tar.gz
	$(MAKE) -C openmpt-libopenmpt-0.3.10


clean:
	git clean -fdx
	git submodule foreach git clean -fdx
