CC = /usr/bin/x86_64-w64-mingw32-g++-posix
FLAGS =  -std=c++17 -D_GLIBCXX_USE_NANOSLEEP
CC_FLAGS = -static -O3 
DEBUG_FLAGS = -g -S
LIB_FLAGS = -I /mnt/d/git/congl/

main: 
	$(CC) $(FLAGS) $(CC_FLAGS) main.cpp -o main.exe $(LIB_FLAGS)

debug:
	$(CC) $(FLAGS) $(DEBUG_FLAGS) main.cpp -o main.exe $(LIB_FLAGS)

