CC  = g++
CFLAGS = -O3

OBJS = Array1.o GArray.o Util.o extl2.o itemset.o memman.o \
	partition.o Graph.o

TARGET = genmax 

default: $(TARGET)

clean:
	rm -rf *~ *.o $(TARGET)

.SUFFIXES: .o .cc .c

genmax:  ExtendF5.o $(OBJS) stats.h
	$(CC) $(CFLAGS) -o genmax ExtendF5.c $(OBJS) $(LIBS)
	strip genmax

genmaxnot:  NExtendF5.o $(OBJS) stats.h
	$(CC) $(CFLAGS) -o genmaxnot NExtendF5.c $(OBJS) $(LIBS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

.c.o:
	$(CC) $(CFLAGS) -c $<

# dependencies
# $(OBJS): $(HFILES)
Array1.o: Array1.h
GArray.o: GArray.h
Util.o: Util.h
extl2.o: extl2.h
itemset.o: itemset.h
memman.o: memman.h
partition.o: partition.h
Graph.o: Graph.h
