CXX := g++-11

CXXFLAGS := -std=c++20 -D_FILE_OFFSET_BITS=64 -fopenmp -Wall -Wextra -Wpedantic -Wold-style-cast \
			-Wshadow -Wcast-qual -Wwrite-strings -Wdisabled-optimization -Wfloat-equal -Wformat=2 \
			-Wformat-overflow -Wformat-truncation -Wundef -fno-common -Wconversion \
			-fdiagnostics-show-option -fdiagnostics-color=auto -Wuninitialized \
			-Wno-unused-function -fstrict-aliasing -D_FORTIFY_SOURCE=2 -fstack-protector-strong \
			-Wno-unused -Wno-unused-result -Wno-unused-parameter -Wcast-align -Wnon-virtual-dtor \
			-Woverloaded-virtual -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches \
			-Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion
# Consider adding -Wsign-conversion to the above
CXXFLAGS += -MMD -MP
# ^^ -MMD -MP are used to  automatically generate dependencies for files (see https://stackoverflow.com/questions/11855386/using-g-with-mmd-in-makefile-to-automatically-generate-dependencies)

LDFLAGS := -L .
# ^^ The `.` is needed
LDLIBS := -lm -lrt -lpthread

SANITIZER := -fsanitize=address,leak,undefined
CXXDEBUG := -g -fno-omit-frame-pointer -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
# `CXXDEBUG` previously had `-fanalyzer`, but this slowed down compilation a ton 
CXXRELEASE := -DNDEBUG -O2

MAKEFLAGS := -j
# ^ enable parallel compiling

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
# OLD:
# DEP := $(SRC:.cpp=.d)
DEP := $(SRC:.cpp=.dbg.d)
DEP += $(SRC:.cpp=.rel.d)  
# Now add both .dbg.d and .rel.d. Note the += not := for the second DEP; := which would reset DEP, which is not what we want.
# ^^ as for why we don't do .cpp=.d, it's because this Makefile doesn't seem to generate dependency files like "main.d".
# Instead, it generates like main.dbg.d and main.rel.d. Perhaps `make` generates .d for .o files. Like main.o gets a main.d,
# and now we instead have a .dbg.o and a .rel.o so it generates a .dbg.d and a .rel.d instead, so those are what we need to have the
# dependencies as.
# "Couldn't we just do SRC:.o=.d" because every x.o needs x.d? Isn't this more concise? Well I thought it would work but
# it did not; the line DEP := $(SRC:.o=.d) causes "main.cpp:7: *** missing separator.  Stop.". To reproduce this error, make sure
# the code equal to the first Git commit in `cpp_raytracer` when this Makefile was added.
DEBUG := $(SRC:.cpp=.dbg)
RELEASE := $(SRC:.cpp=.rel)

.PHONY: all clean

all: $(DEBUG) $(RELEASE)

ifneq ($(MAKECMDGOALS), clean, install) # don't create .d for clean and install goals
-include $(DEP)
endif

# first generate INDEPENDENT object code files for the debug and release builds
# Specifically, we generate a .dbg.o and a .rel.o file for every main

%.dbg.o:%.cpp
	# @echo "sources: $(SRC)"
	# @echo "OBJ: $(OBJ)"
	# @echo "DEP: $(DEP)"
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(SANITIZER) $< -c -o $@ $(LDFLAGS)

%.rel.o:%.cpp
	$(CXX) $(CXXFLAGS) $(CXXRELEASE) $< -c -o $@ $(LDFLAGS)

# now link the object files to generate the final executables; the debug and release executables
# have file suffix .dbg and .rel, respectively
%.dbg:%.dbg.o
	$(CXX) $(CXXFLAGS) $(SANITIZER) $< -o $@ $(LDFLAGS) $(LDLIBS)
#^^ need to pass SANITIZER to the above or else "ubsan" errors appear; see https://stackoverflow.com/questions/31803705/using-gcc-undefined-behavior-sanitizer

%.rel:%.rel.o
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

# Need to pass CXXFLAGS to the above because otherwise -fopenmp errors start showing up

clean:
	$(RM) $(OBJ) $(DEP) $(RELEASE) $(DEBUG) *~ *.out *.dbg *.rel *.dbg.o *.rel.o compile_commands.json *.d
