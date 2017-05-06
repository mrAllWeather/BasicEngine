# Keep this for future
CC=g++
LDFLAGS=`pkg-config --static --libs glfw3 --cflags freetype2` -lGLEW -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -lGL -lpthread -ldl -std=c++11
CFLAGS=-std=c++11
SOURCES=RenderText.cpp Bouncer.cpp Light.cpp Camera.cpp SceneLoader.cpp StaticMesh.cpp ComplexMesh.cpp ObjLoader.cpp TextureLoader.cpp Seconds_Per_Frame_Counter.cpp File_IO.cpp Scene.cpp ShaderLoader.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE := assign2

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o: #M make object files
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm $(EXECUTABLE) *.o
