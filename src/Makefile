CC = gcc
CFLAGS = -mmacosx-version-min=10.4  -Wall -g -framework IOKit
CPPFLAGS = -DCMD_TOOL_BUILD

all: fanspeed.o
	$(CC) $(CFLAGS) -o fanspeed fanspeed.o
clean:
	-rm -f fanspeed fanspeed.o
