TARGET = mandelbrot
LIBS += $(if $(shell pkg-config --exists ncursesw && echo y),\
	$(shell pkg-config --libs ncursesw),\
	$(if $(shell pkg-config --exists ncurses && echo y),\
		$(shell pkg-config --libs ncurses),-lcurses))
CFLAGS = -Wall -Werror -std=c99 -lm

all: $(TARGET)

$(TARGET): m2.c
	mpicc -o $@ $< $(CFLAGS) $(LIBS)

clean:
	-rm -f $(TARGET)

.PHONY: all clean
