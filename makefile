lanfiles.app: lanfiles.o
	gcc lanfiles.o -o lanfiles.app
lanfiles.o: src/lanfiles.c src/lanfiles.h
	gcc src/lanfiles.c -c -o lanfiles.o
