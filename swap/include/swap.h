#ifndef SWAP_H
#define SWAP_H

#include "swap-global.h"
#include <pthread.h>

//Archivos
t_config *config_file;

//Funciones
static void *ejecutar_operacion(int client);
void liberar_memoria_y_finalizar();
void destruir_elementos_lista(void *elemento);

#endif