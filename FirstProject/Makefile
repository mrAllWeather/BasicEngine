# Keep this for future
CC=g++
LDFLAGS=`pkg-config --static --libs glfw3 --cflags freetype2` -lGLEW -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -lGL -lpthread -ldl -std=c++11
CFLAGS=-std=c++11

SRCDIR= src
OBJDIR= obj
BINDIR= .
INCDIR= include

SOURCES := $(wildcard $(SRCDIR)/*.c*)
INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

rm = rm -f

TARGET = assign3_part1

# all: $(SOURCES) $(EXECUTABLE)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS) $(LDFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)
	echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	$(rm) $(OBJDIR)/*.o

.PHONY: remove
remove: clean
	$(rm) $(BINDIR)/$(TARGET)
