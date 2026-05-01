# Makefile

# Define compilar
CC = gcc

# Define flags
CFLAGS = -Wall -g

# Program name
TARGET = vbm

# libs
LIBS = -lcurl -lcjson

# make command
all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o $(LIBS)

# link to object file
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# clean up files
clean:
	rm -rf $(TARGET) *.o
