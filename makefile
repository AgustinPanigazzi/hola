.PHONY: clean

comp=g++

clienteComp: cliente.o
	$(comp) -o clienteComp cliente.o

cliente.o: cliente.c
	$(comp) -c cliente.c
	
estructura_listaComp: estructura_lista.o
	$(comp) -o estructura_listaComp estructura_lista.o

estructura_lista.o: estructura_lista.c
	$(comp) -c estructura_lista.c

	
clean:
	rm *.o
