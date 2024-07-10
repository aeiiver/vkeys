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
	-rm ceys

CFLAGS_2 := -ggdb -std=c2x -Wall -Wextra -Wpedantic \
		   -Wmissing-prototypes -Wshadow \
		   -Warith-conversion -Wconversion \
		   -Werror \
		   `pkg-config raylib --cflags`
LIBS_2 := `pkg-config raylib --libs` -lm

ceys: ceys.c
	$(CC) $(CFLAGS_2) -o $@ $< $(LIBS_2)
