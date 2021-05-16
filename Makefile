# Makefile for UD CISC user-level thread library

CC = gcc
CFLAGS = -g

LIBOBJS = t_lib.o 

TSTOBJS = test00.o partialorder.o rendezvous.o

# specify the executable 

EXECS = test00 partialorder rendezvous

# specify the source files

LIBSRCS = t_lib.c

TSTSRCS = test00.c


# ar creates the static thread library



t_lib.a: ${LIBOBJS} Makefile
	ar rcs t_lib.a ${LIBOBJS}

test00: test00.o t_lib.a Makefile
	${CC} ${CFLAGS} test00.o t_lib.a -o test00

# here, we specify how each file should be compiled, what
# files they depend on, etc.

t_lib.o: t_lib.c t_lib.h Makefile
	${CC} ${CFLAGS} -c t_lib.c

test00.o: test00.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test00.c

rendezvous: rendezvous.o t_lib.a Makefile
	${CC} ${CFLAGS} rendezvous.o t_lib.a -o rendezvous

rendezvous.o: rendezvous.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c rendezvous.c

partialorder: partialorder.o t_lib.a Makefile
	${CC} ${CFLAGS} partialorder.o t_lib.a -o partialorder

partialorder.o: partialorder.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c partialorder.c

test8.o: test8.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test8.c

test8: test8.o t_lib.a Makefile
	${CC} ${CFLAGS} test8.o t_lib.a -o test8

test6.o: test6.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test6.c

test6: test6.o t_lib.a Makefile
	${CC} ${CFLAGS} test6.o t_lib.a -o test6

test5.o: test5.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test5.c

test5: test5.o t_lib.a Makefile
	${CC} ${CFLAGS} test5.o t_lib.a -o test5

%.o: %.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c %.c

%: %.o t_lib.a Makefile
	${CC} ${CFLAGS} %.o t_lib.a -o %

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS} linked.o
