CC = gcc
CFLAGS := -ggdb3 -O2 -Wall -std=c11 -D_FILE_OFFSET_BITS=64
CFLAGS += -Wno-unused-function -Wvla

LDLIBS := $(shell pkg-config fuse --cflags --libs)

FS_NAME := fisopfs

SRCS := fisopfs.c fs_utils.c
OBJS := $(SRCS:.c=.o)

all: build

build: $(FS_NAME)

$(FS_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

docker-build:
	./dock build

docker-run:
	./dock run

docker-exec:
	./dock exec

clean:
	rm -rf $(OBJS) $(FS_NAME)

.PHONY: all build clean
