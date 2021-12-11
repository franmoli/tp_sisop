#ifndef SWAP_H
#define SWAP_H

#include "swap-global.h"
#include "file-management.h"
#include "asignaciones.h"
#include <pthread.h>

//Variables globales
t_config *config_file;
sem_t mutex_operacion;

//Funciones
int ejecutar_operacion(int client);
void liberar_memoria_y_finalizar();
void destruir_elementos_lista(void *elemento);

#endif