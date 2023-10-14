LIBS := `sdl2-config --libs`
CFLAGS := -ggdb -std=c2x -Wall -Wextra -Wpedantic \
		  -Wmissing-prototypes -Wshadow \
		  -Warith-conversion -Wconversion \
		  `sdl2-config --cflags`

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

.PHONY:
clean:
	-rm main
