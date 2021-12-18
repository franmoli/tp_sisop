#include "config_utils.h"

t_config *leer_config_file(char *configName) {
    return config_create(configName);
}

t_config_kernel *generar_config_kernel(t_config *config) {

    //Formateo los array recibidos por configuración
    char *io_devices_unformatted = config_get_string_value(config, "DISPOSITIVOS_IO");
    char *io_devices_substring = string_substring(io_devices_unformatted, 1, strlen(io_devices_unformatted)-2);
    char **io_devices_formatted = string_split(io_devices_substring, ","); 
    char *io_durations_unformatted = config_get_string_value(config, "DURACIONES_IO");
    char *io_durations_substring = string_substring(io_durations_unformatted, 1, strlen(io_durations_unformatted)-2);
    char **io_durations_formatted = string_split(io_durations_substring, ",");

    free(io_durations_substring);
    free(io_devices_substring);

    //Agrego los elementos de los array a una lista
    t_list *io_devices = list_create();
    int contador_devices = 0;
    while(io_devices_formatted[contador_devices]) {
        string_trim_left(&io_devices_formatted[contador_devices]);
        string_trim_right(&io_devices_formatted[contador_devices]);
        list_add(io_devices, io_devices_formatted[contador_devices]);
        contador_devices++;
    }

    t_list *io_durations = list_create();
    int contador_durations = 0;
    while(io_durations_formatted[contador_durations]) {
        string_trim_left(&io_durations_formatted[contador_durations]);
        string_trim_right(&io_durations_formatted[contador_durations]);
        list_add(io_durations, io_durations_formatted[contador_durations]);
        contador_durations++;
    }

    //Cargo la configuración
    t_config_kernel *config_kernel = malloc(sizeof(t_config_kernel));
    config_kernel->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    config_kernel->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
    config_kernel->IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
    config_kernel->PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
    config_kernel->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_kernel->RETARDO_CPU = config_get_int_value(config, "RETARDO_CPU");
    config_kernel->GRADO_MULTIPROCESAMIENTO = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
    config_kernel->GRADO_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    config_kernel->ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
    config_kernel->ALFA = strtof(config_get_string_value(config, "ALFA"), NULL);
    config_kernel->DISPOSITIVOS_IO = io_devices;
    config_kernel->DURACIONES_IO = io_durations;
    config_kernel->TIEMPO_DEADLOCK = config_get_int_value(config, "TIEMPO_DEADLOCK");

    // libero memoria utilizada en el formateo
    //free(io_devices_unformatted);
    //free(io_devices_substring);
    free(io_devices_formatted);

    //free(io_durations_unformatted);
    free(io_durations_formatted);
    //free(io_durations_substring);


    return config_kernel;
}

t_config_memoria *generarConfigMemoria(t_config *config){
    t_config_memoria *config_memoria = malloc(sizeof(t_config_memoria));
    config_memoria->IP = config_get_string_value(config, "IP");
    config_memoria->PUERTO = config_get_string_value(config, "PUERTO");
    config_memoria->TAMANIO = config_get_int_value(config, "TAMANIO");   
    config_memoria->ALGORITMO_REEMPLAZO_MMU = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
    config_memoria->TIPO_ASIGNACION = config_get_string_value(config, "TIPO_ASIGNACION");
    config_memoria->CANTIDAD_ENTRADAS_TLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    config_memoria->ALGORITMO_REEMPLAZO_TLB = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
    config_memoria->RETARDO_ACIERTO_TLB = config_get_int_value(config, "RETARDO_ACIERTO_TLB");
    config_memoria->RETARDO_FALLO_TLB = config_get_int_value(config, "RETARDO_FALLO_TLB");
    config_memoria->MARCOS_POR_CARPINCHO = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
    config_memoria->TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
    config_memoria->PATH_DUMP_TLB = config_get_string_value(config, "PATH_DUMP_TLB");
    return config_memoria;
}

t_config_swap *generar_config_swap(t_config *config) {
    //Formateo los array recibidos por configuración
    char *file_paths_unformatted = config_get_string_value(config, "ARCHIVOS_SWAP");
    //Agrego los elementos del array a una lista
    t_list *file_paths = list_create();
    if(strlen(file_paths_unformatted) > 2) {
        char *file_paths_substring = string_substring(file_paths_unformatted, 1, strlen(file_paths_unformatted)-2);
        char **file_paths_formatted = string_split(file_paths_substring, ",");

        int contador = 0;
        while(file_paths_formatted[contador] != NULL) {
            string_trim(&file_paths_formatted[contador]);
            list_add(file_paths, file_paths_formatted[contador]);
            contador += 1;
        }

        free(file_paths_substring);
        free(file_paths_formatted);
    }

    //Cargo la configuración
    t_config_swap *config_swap = malloc(sizeof(t_config_swap));
    config_swap->IP = config_get_string_value(config, "IP");
    config_swap->PUERTO = config_get_string_value(config, "PUERTO");
    config_swap->TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
    config_swap->TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
    config_swap->MARCOS_MAXIMOS = config_get_int_value(config, "MARCOS_MAXIMOS");
    config_swap->RETARDO_SWAP = config_get_int_value(config, "RETARDO_SWAP");
    config_swap->ARCHIVOS_SWAP = file_paths;
    config_swap->TIPO_ASIGNACION = config_get_string_value(config, "TIPO_ASIGNACION");
    
    return config_swap;
}

t_config_matelib *generar_config_matelib(t_config *config){

    t_config_matelib *config_matelib =  malloc(sizeof(t_config_matelib));
    config_matelib->IP_KERNEL = config_get_string_value(config,"IP_KERNEL");
    config_matelib->PUERTO_KERNEL = config_get_string_value(config,"PUERTO_KERNEL");
    config_matelib->IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
    config_matelib->PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
    char *log = config_get_string_value(config,"LOG_LEVEL");
    printf("EL LOG ES %s\n",log);

    if(!strcmp(log,"LOG_LEVEL_TRACE"))
        config_matelib->LOG_LEVEL = LOG_LEVEL_TRACE;
    if(!strcmp(log,"LOG_LEVEL_DEBUG"))
        config_matelib->LOG_LEVEL = LOG_LEVEL_DEBUG;
    if(!strcmp(log,"LOG_LEVEL_INFO"))
        config_matelib->LOG_LEVEL = LOG_LEVEL_INFO;
    if(!strcmp(log,"LOG_LEVEL_WARNING"))
        config_matelib->LOG_LEVEL = LOG_LEVEL_WARNING;
    if(!strcmp(log,"LOG_LEVEL_ERROR"))
        config_matelib->LOG_LEVEL = LOG_LEVEL_ERROR;

    return config_matelib;
}

void liberar_config(t_config* config){    
    config_destroy(config);
}