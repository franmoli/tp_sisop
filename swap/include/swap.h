#ifndef SWAP_H
#define SWAP_H

#include "swap-global.h"
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

//Archivos
int tamanio_archivo;
int archivo_seleccionado;
t_config *config_file;

//Funciones
static void *ejecutar_operacion(int client);
void crear_archivo_swap(int numero_particion);
void insertar_pagina_en_archivo(t_pagina *pagina);
void borrar_archivos_swap();
void liberar_memoria_y_finalizar();
void destruir_elementos_lista(void *elemento);

#endif