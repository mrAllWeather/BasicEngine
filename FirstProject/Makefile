# Keep this for future
OS		:= $(shell uname)

CC		:= g++
CFLAGS 	:= -Wall -Wextra -std=c++11 -O2 -fsanitize=address -fno-omit-frame-pointer
LDFLAGS	:=`pkg-config --cflags glfw3 freetype2`
CLIBS	:=`pkg-config --static --libs glfw3 freetype2` -lGLEW

SRCDIR= src
OBJDIR= obj
BINDIR= .
INCDIR= include

SOURCES := $(wildcard $(SRCDIR)/*.c*)
INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)


TARGET = assign3_part1

# For those who dare compile with Cygwin
ifeq ($(OS),GYGWIN)
	GLLIBS	:= -lopengl32 -lglfw3 -lglew32
	GLFLAGS := -DWIN32
	TARGET  +=.exe
endif

ifeq ($(OS),Darwin)
	CLIBS	+=-framework OpenGL
else
	CLIBS	+=-lGL -lGLU
endif

.PHONY: all clean remove

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(CLIBS)
	@echo "Compiled "$<" successfully!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)
	@echo "Compiled "$<" successfully!"

clean:
	$(RM) $(OBJDIR)/*.o

remove: clean
	$(RM) $(BINDIR)/$(TARGET)
