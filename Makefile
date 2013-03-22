CC = g++
CFLAGS = -arch x86_64
INCLUDES = -Iinclude/
LFLAGS = -Llib/
LIBS = -lfbxsdk-2013.3-static
SRCS = src/*.cpp src/JsonBox/*.cpp
FRAMEWORKS = -framework cocoa
TARGET = fbx2json
BUILD = bin

all: $(TARGET)
	@echo ${TARGET} has been compiled
	
$(TARGET):
	mkdir -p ${BUILD}
	$(CC) $(CFLAGS) $(INCLUDES) -o ${BUILD}/$(TARGET) $(SRCS) $(FRAMEWORKS) $(LFLAGS) $(LIBS)

clean:
	$(RM) ${BUILD}