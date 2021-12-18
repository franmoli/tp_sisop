#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef struct {
    char *IP_MEMORIA;
    char *PUERTO_MEMORIA;
    char *IP_KERNEL;
    char *PUERTO_KERNEL;
    char *ALGORITMO_PLANIFICACION;
    t_list *DISPOSITIVOS_IO;
    t_list *DURACIONES_IO;
    int GRADO_MULTIPROGRAMACION;
    int GRADO_MULTIPROCESAMIENTO;
    int RETARDO_CPU;
    int ESTIMACION_INICIAL;
    float ALFA;
    int TIEMPO_DEADLOCK;
} t_config_kernel;

typedef struct {
    char *IP;
    int TAMANIO;
    char *PUERTO;
    char *ALGORITMO_REEMPLAZO_MMU;
    char *TIPO_ASIGNACION;
    int MARCOS_POR_CARPINCHO;
    int CANTIDAD_ENTRADAS_TLB;
    char *ALGORITMO_REEMPLAZO_TLB;
    float RETARDO_ACIERTO_TLB;
    float RETARDO_FALLO_TLB;
    int TAMANIO_PAGINA;
    char *PATH_DUMP_TLB;
} t_config_memoria;

typedef struct {
    char *IP;
    char *PUERTO;
    int TAMANIO_SWAP;
    int TAMANIO_PAGINA;
    t_list *ARCHIVOS_SWAP;
    int MARCOS_MAXIMOS;
    float RETARDO_SWAP;
    char *TIPO_ASIGNACION;
} t_config_swap;

typedef struct {
    char *IP_KERNEL;
    char *PUERTO_KERNEL;
    char *IP_MEMORIA;
    char *PUERTO_MEMORIA;
    char *LOG_LEVEL;
} t_config_matelib;

t_config *leer_config_file(char *);
t_config_kernel *generar_config_kernel(t_config *config);
t_config_memoria *generarConfigMemoria(t_config *config);
t_config_swap *generar_config_swap(t_config *config);
t_config_matelib *generar_config_matelib(t_config *config);
void liberar_config(t_config* config);

#endif