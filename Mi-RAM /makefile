TARGET = MIRAM
UTILS_DIR = ../utils
$(TARGET) : mi-ram.o ../utils/utils.o
	gcc -Wall -I $(UTILS_DIR) mi-ram.o $(UTILS_DIR)/utils.o -o $(TARGET) -lcommons


$(UTILS_DIR)/utils.o : $(UTILS_DIR)/utils.c $(UTILS_DIR)/utils.h
    
mi-ram.o : mi-ram.c mi-ram.h

.PHONY : clean
clean:
	rm *.o $(TARGET)