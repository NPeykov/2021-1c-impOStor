mongo : mongo-store.c ../utils/utils.c mongo-store.h
	gcc mongo-store.c ../utils/utils.c -o mongo -lcommons -lreadline -pthread
	
.PHONY : clean
clean:
	rm mongo