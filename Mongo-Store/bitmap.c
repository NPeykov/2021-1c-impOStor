/*
 * bitmap.c
 *
 *  Created on: 19 jul. 2021
 *      Author: utnso
 */


#include "bitmap.h"

t_bitarray* crear_bitmap(char *ubicacion, int cant_bloques){

	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_TRACE);
	struct stat file_st;
	size_t size = (size_t) cant_bloques / 8;
	//printf("\nSize = %d\n", size);
	char *rutaBitmap = malloc(strlen(ubicacion) );
	strcpy(rutaBitmap, ubicacion);
//	strcat(rutaBitmap, "/Bitmap.bin");

	int fd = open("/home/utnso/workspace/mnt/Bitmap.bin", O_CREAT);
	if (fd == -1) {
		log_error(mongoLogger, "Error al crear el Bitmap");
		exit(1);
	}
	fstat(fd, &file_st);

	char* bmap = mmap(NULL, file_st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);
	if (bmap == MAP_FAILED) {

		close(fd);
		exit(1);
	}

	t_bitarray* bitmap = bitarray_create_with_mode((char*) bmap, size, MSB_FIRST);


	msync(bitmap, size, MS_SYNC);
	free(rutaBitmap);
	return bitmap;
}


int obtener_bloque_libre(t_bitarray* bitmap){
	size_t tamanio = bitarray_get_max_bit(bitmap);
	int i;
	for(i=0; i<tamanio; i++){
		if(bitarray_test_bit(bitmap, i)== 0){
			return i;
		}
	}
	return -1;
}
void ocupar_bloque(t_bitarray* bitmap, int bloque){
	bitarray_set_bit(bitmap,bloque);
	return;
}
void liberar_bloque(t_bitarray* bitmap, int bloque){
	bitarray_clean_bit(bitmap,bloque);
	return;
}















