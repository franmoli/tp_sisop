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
    int base;
    int size;
    char *file;
} t_pagina_almacenada;

//Variables globales
int archivo_seleccionado;
t_list *lista_paginas_almacenadas;
t_list *lista_mapeos;

//Funciones
void crear_archivos_swap();
void insertar_pagina_en_archivo(t_pagina *pagina);
void leer_pagina_de_archivo(int numero_pagina);
void borrar_archivos_swap();
void siguiente_archivo();

#endif