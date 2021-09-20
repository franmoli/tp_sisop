#include "swap.h"

int main(int argc, char **argv) {
    //Limpio la consola
    system("clear");

    //Se carga la configuración
    config_file = leer_config_file("./cfg/swap.cfg");
    config_swap = generarConfigSwap(config_file);
    
    printf("IP: %s\n", config_swap->IP);
    printf("PUERTO: %d\n", config_swap->PUERTO);
    printf("TAMANIO_SWAP: %d\n", config_swap->TAMANIO_SWAP);
    printf("TAMANIO_PAGINA: %d\n", config_swap->TAMANIO_PAGINA);
    printf("MARCOS_MAXIMOS: %d\n", config_swap->MARCOS_MAXIMOS);
    printf("RETARDO_SWAP: %d\n", config_swap->RETARDO_SWAP);
    printf("ARCHIVOS:\n");
    t_list_iterator *paths_iterator = list_iterator_create(config_swap->ARCHIVOS_SWAP);
    while(list_iterator_has_next(paths_iterator)) {
        printf("- %s\n", list_iterator_next(paths_iterator));
    }

    //Fin del programa
    return 1;
}