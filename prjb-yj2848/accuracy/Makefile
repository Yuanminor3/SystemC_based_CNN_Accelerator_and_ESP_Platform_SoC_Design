#
# This Makefile shows some tricks useful when more than one target program has
# to be compiled and most of the rules are shared. Note, however, that for a
# single executable with single source file a simpler Makefile would work as
# well. Nevertheless, knowing these tricks will allow you to minimize repeated
# lines of code.
#

#
# Automatic variables in make:
#
# $@ -> the target
# $< -> first dependency
# $^ -> all dependencies
#

#
# Builtin functions in make:
#
# wildcard -> all files matching the pattern
# subst -> replace string
# $(VAR:suffix_a=suffix_b) -> replace suffix
# %-suffix_a: %suffix_b %suffix_c -> replace target suffix (suffix_a) with suffix_b and suffix_c to create the corresponding dependency
# @ -> hide the corresponding command when running the target
#

#
# Aliases
#

# Define V=1 for a more verbose compilation
ifndef V
        QUIET_CXX           = @echo '   ' CXX $@;
        QUIET_MKDIR         = @echo '   ' MKDIR $@;
        QUIET_RUN           = @echo '   ' RUN $@;
        QUIET_UNZIP         = @echo '   ' UNZIP $@;
        QUIET_WGET          = @echo '   ' WGET $@;
        QUIET_CLEAN         = @echo '   ' CLEAN $@;
endif

RM = rm -rf
IMAGE ?= cat
INPUT = ./data/$(IMAGE).bmp
LAYER ?= 0
ARCH ?= SMALL
SYS_OPT ?=

#
# Environment variables
#

OPENCV ?= /opt/opencv

#
# Compile and Link Flags
# Keep the variable name as below. This allows make to automatically infer
# compiler and linker rules.
#

# Include folders for header files
INCDIR = -I$(OPENCV)/include -I./mojo

# Compile flags
CXXFLAGS ?= -O3 -Wall -Werror -fmax-errors=5 -std=c++11
CXXFLAGS += $(INCDIR) -D$(ARCH) $(SYS_OPT)

# Link flags
LDLIBS = -L$(OPENCV)/lib64

LDFLAGS = \
	-lopencv_videostab \
	-lopencv_photo \
	-lopencv_stitching \
	-lopencv_objdetect \
	-lopencv_video \
	-lopencv_ml \
	-lopencv_calib3d \
	-lopencv_features2d \
	-lopencv_imgcodecs \
	-lopencv_highgui \
	-lopencv_flann \
	-lopencv_imgproc \
	-lopencv_core

# List of source files.
SRCS := dwarf.cpp

# In this Makefile each source produces a corresponding executable.
OBJS := $(SRCS:.cpp=)

# Find all headers for dependency (i.e. recompile if one header changes)
HDRS := $(wildcard ./mojo/*.h) $(wildcard ./*.h)

# The first target gets executed by default when typing simply make.
all: $(OBJS)

# Let mak know the suffix of source and object files (if any)
.SUFFIX: .c .cpp

# Compile
# The following rules are inferred by make. Overriding to add $(QUIET_CXX)
# Try removing the following target and you will still be able to compile!
.cpp:
	$(QUIET_CXX) $(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS) $(LDFLAGS)

# Add header files dependency
$(OBJS): $(HDRS)

# Create a target to clean from compiled objects
clean:
	$(QUIET_CLEAN) $(RM) $(OBJS) out_*_pvc.txt *.log

# Create a target to also clean from data and or results
distclean: clean
	$(QUIET_CLEAN) $(RM) models

accuracy:
	./accuracy_sys.sh

# Declare as .PHONY all targets that do not generate a file whose name matches
# the name of the target (e.g. make clean won't generate a file named clean,
# while make dwarf will compile an object file named dwarf)
.PHONY: all clean distclean


#
# Get Data Sets and Pretrained Models
#

# List of data files to download (this list could contain more files)
MOJO_PRETRAINED := ./models/dwarf7.mojo

$(MOJO_PRETRAINED):
		@mkdir -p ./models
		@cp -f /opt/dwarf_model/dwarf7.mojo $@

# Add a PHONY to get all data at once
data: $(INPUT) $(MOJO_PRETRAINED)

.PHONY: data


#
# Set data dependencies for each target application
#

# Declare specific dependency for each target executable
dwarf-data: data


#
# Run
#

# Define list of targets by replacing the suffix
RUNS := $(SRCS:.cpp=-run)

# Declare multiple PHONY targets to run compiled programs % is used to add
# dependencies based on the target name.  (e.g. dwarf-run depends on dwarf-data and
# dwarf). Note that we add OPENCV/lib to LD_LIBRARY_PATH because libraries are
# dynamically linked when the program is loaded into memory and not at compile
# time.
$(RUNS): %-run : %-data %
	$(QUIET_RUN) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(OPENCV)/lib64 ./$(@:-run=) $(INPUT) $(LAYER)

accuracy_comp_SMALL:
	./accuracy_comp.sh small

accuracy_comp_MEDIUM:
	./accuracy_comp.sh medium

accuracy_comp_FAST:
	./accuracy_comp.sh fast

accuracy_sys_SMALL:
	./accuracy_sys.sh SMALL

accuracy_sys_MEDIUM:
	./accuracy_sys.sh MEDIUM

accuracy_sys_FAST:
	./accuracy_sys.sh FAST_

.PHONY: $(RUNS) accuracy_comp_SMALL accuracy_comp_MEDIUM accuracy_comp_FAST accuracy_sys_SMALL accuracy_sys_MEDIUM accuracy_sys_FAST
