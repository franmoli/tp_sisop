#include "planificador_largo.h"

void iniciar_planificador_largo(){
    printf("Inicio planificador LARGO \n");
}

void atender_proceso (void* parametro ){

    int socket_cliente = *(int*)parametro;
    //int size = 0;
    //t_paquete *raw;
    printf("Atiendo proceso socket: %d", socket_cliente);
     
    return;
}

void *iniciar_servidor_kernel(void *_){
    int socket_servidor = iniciar_servidor(config_kernel->IP_KERNEL,config_kernel->PUERTO_KERNEL, logger_kernel);
    if(socket_servidor == -1){
        log_info(logger_kernel, "Fallo en la creacion del servidor");
    }else{
        //Espero por un proceso cliente y creo hilo para atenderlo
        while(1){
            int *socket_proceso_cliente = malloc(sizeof(int));
            *socket_proceso_cliente = esperar_cliente(socket_servidor, logger_kernel);
            if (*socket_proceso_cliente != -1) {
                pthread_t hilo_proceso_cliente;
                pthread_create(&hilo_proceso_cliente, NULL, (void *)atender_proceso, (void *)socket_proceso_cliente);
            }
        }
    }
    return NULL;
}
