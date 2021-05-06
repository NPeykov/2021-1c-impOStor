TARGET = ServerMONGOSTORE
UTILS_DIR = ../server_utils
$(TARGET) : mongo-store.o ../server_utils/utils.o
	gcc -Wall -I $(UTILS_DIR) mongo-store.o $(UTILS_DIR)/utils.o -o $(TARGET) -lcommons


$(UTILS_DIR)/utils.o : $(UTILS_DIR)/utils.c $(UTILS_DIR)/utils.h
    
mongo-store.o : mongo-store.c mongo-store.h

.PHONY : clean
clean:
	rm *.o $(TARGET)