#ifndef KERNEL_GLOBAL_H
#define KERNEL_GLOBAL_H

// Acá estan todas las declaraciones y librerias usadas por todos los archivos .c

#include "config_utils.h"
#include "server.h"
#include "planificador_corto.h"
#include "planificador_largo.h"
#include "planificador_mediano.h"
#include <pthread.h>
#include <semaphore.h>

//tipos propios para listas
typedef enum {
    NEW = 1,
    READY = 2,
    EXEC = 3,
    BLOCKED = 4,
    S_BLOCKED = 5,
    S_READY = 6
} t_status;

typedef struct {
    int id;
    t_status status;
    // t_task_list *task_list; ????? cuales son las tareas que ejecuta el proceso
} t_proceso;

//Configuración
t_config_kernel *config_kernel;
//Logs
t_log *logger_kernel;
//listas
t_list *lista_new;
t_list *lista_ready;
t_list *lista_exec;
t_list *lista_blocked;
t_list *lista_s_blocked;
t_list *lista_s_ready;
//Semaforos
sem_t mutex_listas;
sem_t proceso_finalizo;

#endif