TARGET = ServerMIRAM
UTILS_DIR = ../server_utils
$(TARGET) : mi-ram.o ../server_utils/utils.o
	gcc -Wall -I $(UTILS_DIR) mi-ram.o $(UTILS_DIR)/utils.o -o $(TARGET) -lcommons


$(UTILS_DIR)/utils.o : $(UTILS_DIR)/utils.c $(UTILS_DIR)/utils.h
    
mi-ram.o : mi-ram.c mi-ram.h

.PHONY : clean
clean:
	rm *.o $(TARGET)