CC =		gcc
COPTS =		-O2 -Wall -fPIC -DPIC -DFP_LAYOUT_NEED_NOTHING -pthread
COLLECTD_SRC ?=	/usr/ports/pobj/collectd-4.10.2/collectd-4.10.2/src/

all: pf.so

test: pfcmd

clean:
	rm -f *.o *.so pfcmd

pfcmd: pf.c
	${CC} -DTEST ${COPTS} -I${COLLECTD_SRC} -o pfcmd pf.c

pf.so: pf.c
	${CC} -shared ${COPTS} -I${COLLECTD_SRC} -o pf.so pf.c
