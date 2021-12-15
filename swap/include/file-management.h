#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

#include "swap-global.h"
#include "asignaciones.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

//Estructuras
typedef struct {
    int numero_marco;
    int file;
    int base;
    bool esta_libre;
    int proceso_asignado;
} t_entrada_tabla_marcos;

typedef struct {
    int numero_pagina;
    int id_proceso;
    t_entrada_tabla_marcos *marco;
    int cantidad_contenidos;
    t_list *sizes_contenidos;
} t_pagina_almacenada;

typedef struct {
    int numero_archivo;
    int espacio_disponible;
} t_informacion_archivo;

//Variables globales
t_list *archivos_abiertos;
t_list *lista_paginas_almacenadas;
t_list *lista_mapeos;

//Funciones de archivos
void crear_archivos_swap();
int seleccionar_archivo_escritura(int proceso_a_guardar, int bytes_pagina);
void escribir_en_archivo_swap(int archivo, void *mapping, int offset_final);
void borrar_archivos_swap();

//Funciones de marcos
t_entrada_tabla_marcos* seleccionar_marco(int numero_marco);
t_entrada_tabla_marcos* seleccionar_marco_libre(int archivo_seleccionado);
t_entrada_tabla_marcos* seleccionar_marco_libre_fija(int proceso, int archivo_seleccionado);
t_entrada_tabla_marcos* instanciar_marco_global(int file);
void instanciar_marcos_fija(int file);

//Funciones de páginas
bool insertar_pagina_en_archivo(t_pagina_enviada_swap *pagina);
t_pagina_enviada_swap leer_pagina_de_archivo(int numero_pagina);
void eliminar_pagina(int numero_pagina);

//Esquemas de asignación
bool asignacion_global_de_pagina(int posicion_archivo, char *path_archivo, int archivo, t_pagina_enviada_swap *pagina);
bool asignacion_fija_de_pagina(int posicion_archivo, char *path_archivo, int archivo, t_pagina_enviada_swap *pagina);

#endif