#include "config_utils.h"

t_config *leer_config_file(char *configName) {
    return config_create(configName);
}

t_config_kernel *generarConfigKernel(t_config *config) {
    t_config_kernel *config_kernel = malloc(sizeof(t_config_kernel));
    config_kernel->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_kernel->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
    config_kernel->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_kernel->RETARDO_CPU = config_get_int_value(config, "RETARDO_CPU");
    config_kernel->GRADO_MULTIPROCESAMIENTO = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
    config_kernel->GRADO_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    config_kernel->ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
    config_kernel->ALFA = config_get_int_value(config, "ALFA");
    //faltan las 2 listas cargar
    return config_kernel;
}

t_config_memoria *generarConfigMemoria(t_config *config){
    t_config_memoria *config_memoria = malloc(sizeof(t_config_memoria));
    config_memoria->IP = config_get_string_value(config, "IP");
    config_memoria->PUERTO = config_get_int_value(config, "PUERTO");
    config_memoria->TAMANIO = config_get_int_value(config, "TAMANIO");   
    config_memoria->ALGORITMO_REEMPLAZO_MMU = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
    config_memoria->TIPO_ASIGNACION = config_get_string_value(config, "TIPO_ASIGNACION");
    config_memoria->CANTIDAD_ENTRADAS_TLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    config_memoria->ALGORITMO_REEMPLAZO_TLB = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
    config_memoria->RETARDO_ACIERTO_TLB = config_get_int_value(config, "RETARDO_ACIERTO_TLB");
    config_memoria->RETARDO_FALLO_TLB = config_get_int_value(config, "RETARDO_FALLO_TLB");
    config_memoria->MARCOS_MAXIMOS = config_get_int_value(config, "MARCOS_MAXIMOS");
    config_memoria->TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
    return config_memoria;
}

t_config_swap *generarConfigSwap(t_config *config){
    //Formateo los array recibidos por configuración
    char *file_paths_unformatted = config_get_string_value(config, "ARCHIVOS_SWAP");
    char **file_paths_formatted = string_split(string_substring(file_paths_unformatted, 1, strlen(file_paths_unformatted)-2), ", ");
    
    //Agrego los elementos del array a una lista
    t_list *file_paths = list_create();
    int contador = 0;
    while(file_paths_formatted[contador] != NULL) {
        list_add(file_paths, file_paths_formatted[contador]);
        contador += 2;
    }

    //Cargo la configuración
    t_config_swap *config_swap = malloc(sizeof(t_config_swap));
    config_swap->IP = config_get_string_value(config, "IP");
    config_swap->PUERTO = config_get_int_value(config, "PUERTO");
    config_swap->TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
    config_swap->TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
    config_swap->MARCOS_MAXIMOS = config_get_int_value(config, "MARCOS_MAXIMOS");
    config_swap->RETARDO_SWAP = config_get_int_value(config, "RETARDO_SWAP");
    config_swap->ARCHIVOS_SWAP = file_paths;

    return config_swap;
}
void liberar_config(t_config* config){    
    config_destroy(config);
}