#include "mapa.h"

void verificarCreacion(char id, int err){
	if(err) {
        nivel_destruir(nivel);
        nivel_gui_terminar();
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(err));
	}
}



void crear_nivel(){
	caracter=65;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&columnas, &filas);

	nivel = nivel_crear("Colonia de Polus");

	sem_init(&mutexError,0,1);

	nivel_gui_dibujar(nivel);
}

int nuevoTripuMapa(int posicionX, int posicionY){
	sem_wait(&mutexError);
	error = personaje_crear(nivel, (char)caracter, posicionX, posicionY);
	verificarCreacion((char)caracter, error);
	sem_post(&mutexError);

	nivel_gui_dibujar(nivel);
	caracter++;
	return (caracter-1);
}

void moverTripuMapa(int caracterRepresentativo, int difX, int difY){
	item_desplazar(nivel, (char)caracterRepresentativo, difX, difY);

	nivel_gui_dibujar(nivel);
}

void eliminarTripuMapa(int caracterRepresentativo){
	item_borrar(nivel, (char)caracterRepresentativo);

	nivel_gui_dibujar(nivel);
}
