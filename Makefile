#Copyright 2021 Zipline International Inc. All rights reserved.
CC = g++
CFLAGS = -std=c++20 -Wall -Wextra
FILES := $(wildcard *.cpp)
HEADERS := $(wildcard *.h)
BINARY = zip_scheduler

all: $(FILES) $(HEADERS)
	$(CC) $(CFLAGS) $(FILES) -o $(BINARY)
clean:
	rm -f $(BINARY)
