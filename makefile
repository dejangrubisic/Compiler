CC = g++
CFLAGS = -std=c++11 -g -Wall

TARGET = schedule

all: $(TARGET)
$(TARGET): $(TARGET).cpp 
	$(CC) $(CFLAGS) -o schedule $(TARGET).cpp
clean: 
	$(RM) $(TARGET)
