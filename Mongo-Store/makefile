TARGET = mongo
UTILS  = ../utils
PROG   = mongo-store.c bitmap.c $(UTILS)/utils.c sabotaje_mongo.c
FLAGS  = -I$(UTILS) -lcommons -lreadline -pthread -lm

$(TARGET) : $(PROG)
	gcc $(PROG) -o $(TARGET) $(FLAGS)

.PHONY : clean
clean :
	rm -f $(TARGET) *.o