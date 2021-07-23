TARGET = Cliente
UTILS  = ../utils
PROG   = discordiador.c utils_disc.c tareas.c sabotaje.c\
         comunicacion.c $(UTILS)/utils.c
FLAGS  = -I$(UTILS) -Wall -lcommons -lreadline -pthread

$(TARGET) : $(PROG)
	gcc $(PROG) -o $(TARGET) $(FLAGS)


.PHONY : clean
clean :
	rm -f Cliente$(TARGET) *.o

#Cliente : discordiador.c ../utils/utils.c
#	gcc discordiador.c ../utils/utils.c -o Cliente -lcommons -lreadline -pthread
#
#.PHONY : clean
#clean:
#	rm Cliente
