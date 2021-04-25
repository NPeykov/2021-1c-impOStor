# tp-2021-1c-impOStor

# DISCORDINADOR

PROCESO MULTIHILO. 1 HILO 1 TRIPULANTE

ESTADOS: NEW READY EXEC BLOQUEADO (I/0 o EMERGENCIA) EXIT

COLAS DE PLANIFICACIÓN:
  
  - FIFO
  - RR (1 QUANTUM = 1 MOVIEMIENTO X o Y)

COLAS DE BLOQUEADOS: 
  -I/0
  -EMERGENCIAS

MENSAJES:
  - INICIAR_PLANIFICACION: 
  - PAUSAR_PLANIFICACION:
  

# MI-RAM

  *Servidor que representa el funcionamiento de la RAM
	
    Funciones:
      -Atender Tripulantes
      -Mantener mapa de tripulantes actualizado
      -Dump de contenido de memoria (por signal)
			
    Inicializacion:
      -Reservar espacio en memoria
      -Dibujar el mapa vacío
      -Iniciar el Servidor
			
	Administracion de Memoria
	
	  Memoria Compartida, 3 tipos de elementos:
			-TCB (Tripulante Control Block)
			-PCB (Patota Control Block)
			-Tareas
			
		Unico bloque con:
			Segmentacion pura:
				-Dos segmentos por patota (PCB y otro para las Tareas) y otro para tripulante (TCB)
				-Lugar a ocupar según: 
						Best Fit – Donde se desperdicie menos espacio
						First Fit – Primer lugar encontrado
				-Compactacion: Se unifican los segmentos ocupados al final de la memoria. Se ejecuta mediante signal o 
						cuando no haya espacio para un segmento recibido (Si luego de eso no se puede, 
						se deniega la solicitud) 
					
			Paginacion Simple con memoria virtual:
				-Tam. Paginas = RAM/n
				-PCB, instrucciones y TCB en forma contigua
				-Tamaño de pagina/marco según Config
				-SWAP: Según LRU o Clock
				
		Estructuras *Ver archivo del TP para más info.*
		
    Metodos del Servidor
		
      -Iniciar Tripulante
      -Recibir tareas de patota
      -Recibir la ubicación del tripulante
      -Enviar próxima tarea
      -Expulsar Tripulante
			
	Dibujado del mapa - *Ver anexo 1 del Archivo de TP
		
    Dump de Memoria
		
      -Se hace un archivo con el formato Dump_<Timestamp>.dmp (Temporal.h)
      Se visualizara distinto según el esquema de memoria utilizado


# MI-STORE
