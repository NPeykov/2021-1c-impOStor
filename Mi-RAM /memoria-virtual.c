#include "memoria-virtual.h"



void llenar_archivo(int fd, int tamanio){
	void* buffer = malloc(tamanio);
	char a = '\0';
	memset(buffer,a,tamanio);
	write(fd, buffer, tamanio);
	free(buffer);
}

int crear_archivo_swap(){
	int fd = open("swap", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // Lo abre para lectura si existe, sino lo crea
	struct stat statfile;
	if(fstat(fd,&statfile)==-1)
		return -1;
	llenar_archivo(fd, TAM_SWAP);
	MEMORIA_VIRTUAL = mmap(NULL,TAM_SWAP,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	close(fd);
	return 0;
}

/*TODO:Reemplazada por traer_marco_valido
int posicion_libre_en_swap(){
	int posicionLibre = -1;
	int i=0;
	bool encontrado = false;
	while(!encontrado && i<=(bitarray_get_max_bit(BIT_ARRAY_SWAP)-1)){
		if(!bitarray_test_bit(BIT_ARRAY_SWAP, i)){
			posicionLibre = i;
			encontrado=true;
		}
		i++;
	}

	return posicionLibre;
}*/

void inicializar_bitmap_swap(){
	int bytes;

	div_t aux = div(marcos_en_swap, 8);

	if (aux.rem == 0) {
		bytes = aux.quot;
	} else {
		bytes = aux.quot + 1;
	}
	void *punteroABits = malloc(bytes);

	BIT_ARRAY_SWAP = bitarray_create(punteroABits, bytes);

	for(int i=0;i<marcos_en_swap;i++){
		bitarray_clean_bit(BIT_ARRAY_SWAP,i);
	}
}
