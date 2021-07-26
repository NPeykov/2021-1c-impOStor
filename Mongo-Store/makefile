TARGET = mongo
UTILS  = ../utils
PROG   = mongo-store.c bitmap.c $(UTILS)/utils.c
FLAGS  = -I$(UTILS) -lcommons -lreadline -pthread -lm

$(TARGET) : $(PROG)
	gcc $(PROG) -o $(TARGET) $(FLAGS)

.PHONY : clean
clean :
	rm -f $(MONGO) *.o


#mongo : mongo-store.c ../utils/utils.c mongo-store.h
#	gcc mongo-store.c ../utils/utils.c -o mongo -lcommons -lreadline -pthread -lm
#	
#.PHONY : clean
#clean:
#	rm mongo
