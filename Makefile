CFLAGS := -ggdb -std=c2x -Wall -Wextra -Wpedantic \
		   -Wmissing-prototypes -Wshadow \
		   -Warith-conversion -Wconversion \
		   -Werror \
		   `pkg-config raylib --cflags`
LIBS := `pkg-config raylib --libs` -lm

ceys: ceys.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
