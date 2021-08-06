TARGET = MIRAM
UTILS  = ../utils
PROG   = mi-ram.c $(UTILS)/utils.c #memoria-virtual.c paginacion.c segmentacion.c mapa.c
FLAGS  = -I$(UTILS) -lcommons -lreadline -pthread -lm -lnivel-gui -lncurses

$(TARGET) : $(PROG)
	gcc $(PROG) -o $(TARGET) $(FLAGS)

.PHONY : clean
clean :
	rm -f $(TARGET) *.o