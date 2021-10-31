#ifndef SWAP_H
#define SWAP_H

#include "swap-global.h"
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

//Archivos
int archivo_seleccionado;
t_config *config_file;

//Funciones
static void *ejecutar_operacion(int client);
void crear_archivos_swap();
void insertar_pagina_en_archivo(t_pagina *pagina);
void leer_pagina_de_archivo();
void borrar_archivos_swap();
void liberar_memoria_y_finalizar();
void destruir_elementos_lista(void *elemento);

#endif