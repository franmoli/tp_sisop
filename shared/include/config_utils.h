#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "TAD.h"

typedef struct
{
    char *IP_MEMORIA;
    int PUERTO_MEMORIA;
    char *ALGORITMO_PLANIFICACION;
    t_list *DISPOSITIVOS_IO;
    t_list *DURACIONES_IO;
    int GRADO_MULTIPROGRAMACION;
    int GRADO_MULTIPROCESAMIENTO;
    int RETARDO_CPU;
    int ESTIMACION_INICIAL;
    int ALFA;
} t_config_kernel;

typedef struct
{
    char *IP;
    int TAMANIO;
    int PUERTO;
    char *ALGORITMO_REEMPLAZO_MMU;
    char *TIPO_ASIGNACION;
    int MARCOS_MAXIMOS;
    int CANTIDAD_ENTRADAS_TLB;
    char *ALGORITMO_REEMPLAZO_TLB;
    int RETARDO_ACIERTO_TLB;
    int RETARDO_FALLO_TLB;
    int TAMANIO_PAGINA;

} t_config_memoria;

typedef struct
{
    char *IP;
    int PUERTO;
    int TAMANIO_SWAP;
    int TAMANIO_PAGINA;
    t_list *ARCHIVOS_SWAP;
   int MARCOS_MAXIMOS;
   int RETARDO_SWAP;

} t_config_swap;


t_config *leer_config_file(char *);
t_config_kernel *generarConfigKernel(t_config *config);
t_config_memoria *generarConfigMemoria(t_config *config);
t_config_swap *generarConfigSwap(t_config *config);
void liberar_config(t_config* config);
#endif