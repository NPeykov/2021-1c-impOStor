#ifndef MAPA_H_
#define MAPA_H_

#include <semaphore.h>
#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include <stdio.h>

sem_t mutexError;

NIVEL* nivel;
int columnas, filas;
int error;

int caracter;

//-----Prototipos-------

//
//Verifica que la creacion de un tripulante no sea erronea
// #verificarCreacion(caracter, codError)
//
void verificarCreacion(char , int );

//
//Crea el nivel y lo dibuja vacio
//
void crear_nivel();

//
//Crea al nuevo tripulante y lo dibuja, devuelve el ASCII del caracter que representa al tripulante
// #nuevoTripuMapa(posX,posY)
//
int nuevoTripuMapa(int , int );

//
//Actualiza la posicion del tripulante y actualiza el mapa
// #moverTripuMapa(ASCIICaracter, difX,difY)
//
void moverTripuMapa(int , int , int );

//
//Elimina al tripulante del mapa segun su caracter representativo
// #eliminarTripuMapa(ASCIICaracter)
//
void eliminarTripuMapa(int );

#endif
