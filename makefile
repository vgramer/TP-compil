OBJ=tp.o tp_l.o tp_y.o print.o
CC=gcc
CFLAGS=-Wall -ansi -I./ -g 
LDFLAGS= -g -lfl
tp : $(OBJ)
	$(CC) -o tp $(OBJ) $(LDFLAGS)

tp.c :
	echo ''

tp.o: tp.c tp_y.h tp.h
	$(CC) $(CFLAGS) -c tp.c

print.o: print.c tp_y.h tp.h
	$(CC) $(CFLAGS) -c print.c

tp_l.o: tp_l.c tp_y.h
	$(CC) $(CFLAGS) -c tp_l.c

tp_l.c : tp.l tp_y.h tp.h
	flex --yylineno -otp_l.c tp.l

tp_y.o : tp_y.c
	$(CC) $(CFLAGS) -c tp_y.c

tp_y.h tp_y.c : tp.y tp.h
	bison -v -b tp_y -o tp_y.c -d tp.y

tp_lex : tp_l.c tp_lex.c tp_y.h
	$(CC) $(CFLAGS) -o tp_lex tp_l.c tp_lex.c $(LDFLAGS) 

.Phony: clean tests

clean:
	rm -f *~ tp*.exe* tp tp_lex *.o tp_l.* tp_y.* prog.c
	rm -f test/*~ test/*.out test/*.res test/*/*~ test/*/*.out test/*/*.res

tests:
	@make tp
	@./test.sh -r -v -a test
