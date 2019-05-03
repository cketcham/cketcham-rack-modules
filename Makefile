# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# Must follow the format in the Naming section of
# https://vcvrack.com/manual/PluginDevelopmentTutorial.html
SLUG = cketcham

# Must follow the format in the Versioning section of
# https://vcvrack.com/manual/PluginDevelopmentTutorial.html
VERSION = 0.6.13

# FLAGS will be passed to both the C and C++ compiler
#FLAGS += -Idep/openmpt-libopenmpt-0.3.10/soundlib -Idep/openmpt-libopenmpt-0.3.10/common
CFLAGS += 
CXXFLAGS += 

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
#LDFLAGS = -Ldep -Ldep/openmpt-libopenmpt-0.3.10/bin/libopenmpt.a
#LDFLAGS = -Ldep 

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(wildcard src/*.c)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

dep:
	$(MAKE) -C dep

