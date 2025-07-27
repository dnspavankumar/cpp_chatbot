CC = g++
CFLAGS = -std=c++11 -I./include
LDFLAGS = -lcurl
TARGET = app
SRC = app.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
