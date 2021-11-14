#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

#include "swap-global.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

//Estructuras
typedef struct {
    int numero_pagina;
    int id_proceso;
    int base;
    int size;
    int file;
} t_pagina_almacenada;

typedef struct {
    int numero_archivo;
    int espacio_disponible;
} t_informacion_archivo;

//Variables globales
t_list *archivos_abiertos;
t_list *lista_paginas_almacenadas;
t_list *lista_mapeos;

//Funciones
void crear_archivos_swap();
void insertar_pagina_en_archivo(t_pagina *pagina);
void leer_pagina_de_archivo(int numero_pagina);
void borrar_archivos_swap();
int seleccionar_archivo_escritura(int proceso_a_guardar, int bytes_pagina);

#endif