# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -pthread `sdl2-config --cflags` -Iinclude
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_gfx -pthread

# Directories
SRCDIR = src
BUILDDIR = build
RSC = rsc

# Source and object files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

TARGET = $(BUILDDIR)/simulation

EMBEDDED_FONT = include/font_data.hpp

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(EMBEDDED_FONT): $(RSC)/SNPro-Regular.ttf
	xxd -i $< > $@

clean:
	rm -rf $(BUILDDIR)

release: CXXFLAGS += -DNDEBUG -O3 -flto
release: clean all

debug : CXXFLAGS += -g -DDEBUG
debug : clean all

run: all
	$(TARGET)

help:
	@echo "Usage: make [all|clean|release|debug|run|help]"
	@echo "  all:     Build the simulation"
	@echo "  clean:   Remove build files"
	@echo "  release: Build the simulation with optimizations"
	@echo "  debug:   Build the simulation with debugging symbols"
	@echo "  run:     Build and run the simulation"
	@echo "  help:    Display this help message"

.PHONY: all clean release debug run help