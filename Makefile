CC =		gcc
COPTS =		-O2 -Wall -fPIC -DPIC -DFP_LAYOUT_NEED_NOTHING -pthread -I . -I${COLLECTD_SRC}
COLLECTD_SRC ?=	/usr/ports/pobj/collectd-4.10.2/collectd-4.10.2/src/

all: pf.so pfrules.so

test: pfcmd pfrulescmd

clean:
	rm -f *.o *.so pfcmd pfrulescmd

pf.so: pf.c
	${CC} -shared ${COPTS} -o pf.so pf.c

pfrules.so: pfrules.c
	${CC} -shared ${COPTS} -o pfrules.so pfrules.c

pfcmd: pf.c
	${CC} -DTEST ${COPTS} -o pfcmd pf.c

pfrulescmd: pfrules.c pfutils.c
	${CC} -DTEST ${COPTS} -c pfrules.c
	${CC} -DTEST ${COPTS} -c pfutils.c
	${CC} -o pfrulescmd pfrules.o pfutils.o
