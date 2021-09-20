#include "swap.h"

int main(int argc, char **argv) {
    //Inicio del programa
    system("clear");
    logger_swap = log_create("./cfg/swap.log", "SWAP", true, LOG_LEVEL_INFO);
    log_info(logger_swap, "Programa inicializado correctamente");

    //Se carga la configuración
    log_info(logger_swap, "Iniciando carga del archivo de configuración");
    config_file = leer_config_file("./cfg/swap.cfg");
    config_swap = generar_config_swap(config_file);
    
    printf("IP: %s\n", config_swap->IP);
    printf("PUERTO: %d\n", config_swap->PUERTO);
    printf("TAMANIO_SWAP: %d\n", config_swap->TAMANIO_SWAP);
    printf("TAMANIO_PAGINA: %d\n", config_swap->TAMANIO_PAGINA);
    printf("MARCOS_MAXIMOS: %d\n", config_swap->MARCOS_MAXIMOS);
    printf("RETARDO_SWAP: %d\n", config_swap->RETARDO_SWAP);
    printf("ARCHIVOS:\n");
    t_list_iterator *paths_iterator = list_iterator_create(config_swap->ARCHIVOS_SWAP);
    while(list_iterator_has_next(paths_iterator)) {
        printf("- %s\n", (char*) list_iterator_next(paths_iterator));
    }
    list_iterator_destroy(paths_iterator);
    log_info(logger_swap, "Configuración cargada correctamente");

    //Fin del programa
    log_info(logger_swap, "Programa finalizado con éxito");
    log_destroy(logger_swap);
    config_destroy(config_file);
    free(config_swap->ARCHIVOS_SWAP);
    free(config_swap);
    return 1;
}