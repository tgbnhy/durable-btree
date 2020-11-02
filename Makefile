Btree : main.o mmap.o btree.o
	cc -o Btree main.o mmap.o btree.o
main.o : main.c btree.h
	cc -c main.c
mmap.o:mmap.c mmap.h
	cc -c mmap.c
btree.o:btree.c
	cc -c btree.c
clean : 
	rm main.o mmap.o btree.o
