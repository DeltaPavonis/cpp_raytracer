CXX := g++-11

CXXFLAGS := -std=c++20 -D_FILE_OFFSET_BITS=64 -fopenmp -Wall -Wextra -Wpedantic -Wold-style-cast -Wshadow -Wcast-qual -Wwrite-strings -Wdisabled-optimization -Wfloat-equal -Wformat=2 -Wformat-overflow -Wformat-truncation -Wundef -fno-common -Wconversion -fdiagnostics-show-option -fdiagnostics-color=auto -Wuninitialized -Wno-unused-function -fstrict-aliasing -D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wno-unused -Wno-unused-result -Wno-unused-parameter -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion
# Consider adding -Wsign-conversion to the above
CXXFLAGS += -MMD -MP
CXXFLAGS += -I /home/kxiao/devel/competitive/kxlib # for kx.h

LDFLAGS := -L.
LDLIBS := -lm -lrt -lpthread

SANITIZER := -fsanitize=address,leak,undefined
CXXDEBUG := -g -fno-omit-frame-pointer -fanalyzer -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
CXXRELEASE += -DNDEBUG -O2

MAKEFLAGS := -j

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
DEP := $(SRC:.cpp=.d)
DEBUG := $(SRC:.cpp=.dbg)
RELEASE := $(SRC:.cpp=.rel)

.PHONY: all clean

all: $(DEBUG) $(RELEASE)

ifneq ($(MAKECMDGOALS), clean, install) # don't create .d for clean and install goals
-include $(DEP)
endif

%.dbg:%.o
	$(CXX) $(CXXFLAGS) $(CXXDEBUG) $(SANITIZER) $< -o $@ $(LDFLAGS) $(LDLIBS)

%.rel:%.o
	$(CXX) $(CXXFLAGS) $(CXXRELEASE) $< -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(OBJ) $(DEP) $(RELEASE) $(DEBUG) *~ *.out *.dbg *.rel compile_commands.json *.d
