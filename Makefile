CC = g++
LD = ld

LIB += -L/usr/local/lib
INCLUDE += -I/usr/local/include

CPPFLAGS += $(INCLUDE) -std=c++14
LDFLAGS += $(LIB) -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

CPP_SOURCES = $(wildcard *.cpp)
CPP_OBJS = $(patsubst %.cpp, $(OBJECTS)%.o, $(CPP_SOURCES))

TARGET = mashiro

$(TARGET) : 
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $(TARGET) $(CPP_SOURCES)

install :
	install -m 775 $(TARGET) /usr/local/bin

uninstall :
	rm -f /usr/local/bin/$(TARGET)

clean :
	-rm -f $(TARGET)

