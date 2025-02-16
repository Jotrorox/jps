# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -pthread `sdl2-config --cflags` -Iinclude
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -pthread

# Directories
SRCDIR = src
BUILDDIR = build
RSC = rsc

# Source and object files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCS))

TARGET = $(BUILDDIR)/simulation

# The embedded font header file generated from rsc/SNPro-Regular.ttf
EMBEDDED_FONT = include/font_data.hpp

all: $(BUILDDIR) $(EMBEDDED_FONT) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Build object files from source files.
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(EMBEDDED_FONT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link the executable.
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Generate the embedded font header from the TTF file.
$(EMBEDDED_FONT): $(RSC)/SNPro-Regular.ttf
	xxd -i $< > $@

clean:
	rm -rf $(BUILDDIR) $(EMBEDDED_FONT)