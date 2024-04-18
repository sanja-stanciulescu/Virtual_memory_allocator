#Copyright by Ana Stanciulescu 312CA 04.04.2024

CC= gcc
CFLAGS= -g -Wall -Wextra -std=c99 -pedantic -ggdb3

TARGETS = sfl

build: $(TARGETS)

sfl: sfl.c
	$(CC) $(CFLAGS) sfl.c -o sfl

run_sfl:
	./sfl

pack:
	zip -FSr 312CA_StanciulescuAna_Tema1.zip README Makefile *.c *.h

clean: 
	rm -f $(TARGETS)

.PHONY: pack clean