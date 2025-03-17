lanfiles.app: *.o
	gcc lanfiles.o -o lanfiles
lanfiles.o: src/lanfiles.c
	gcc src/lanfiles.c -c -o lanfiles.o
