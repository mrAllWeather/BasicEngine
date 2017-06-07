# Keep this for future
OS		:= $(shell uname)

CC		:= g++
CFLAGS 	:= -Wall -Wextra -pedantic -std=c++11 -O2
LDFLAGS	:=`pkg-config --cflags glfw3`
CLIBS	:=`pkg-config --static --libs glfw3` -lGLEW

SRCDIR= src
OBJDIR= obj
BINDIR= .
INCDIR= include

SOURCES := $(wildcard $(SRCDIR)/*.c*)
INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)


TARGET = assign3

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

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) $(OBJECTS) $(BINDIR)/$(TARGET)
