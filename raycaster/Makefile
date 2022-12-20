
CC=gcc
SRC=raycaster.c
OUT=raycaster
FLAGS=-o
LINKER=-lglu32 -lopengl32 -lfreeglut

all: raycaster.c
	$(CC) $(SRC) $(FLAGS) $(OUT) $(LINKER)

run:
	make
	./raycaster.exe

clean:
	rm *.exe