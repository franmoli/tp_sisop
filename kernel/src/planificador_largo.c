#include "planificador_largo.h"

void iniciar_planificador_largo(){
    printf("Inicio planificador LARGO \n");
    //Iniciar servidor y empiezo a escuchar procesos
    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, iniciar_servidor_kernel, (void *)NULL);
    pthread_join(hilo_servidor, NULL);
    //TODO: crear hilo planificador que elementos de new a ready segun tengan disponible por multiprogramacion
}

void atender_proceso (void* parametro ){

    int socket_cliente = *(int*)parametro;
    printf("Atiendo proceso socket: %d", socket_cliente);
    while(1) {
		t_paquete *paquete = recibir_paquete(socket_cliente);

        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        switch(paquete->codigo_operacion) {
            case CLIENTE_TEST:
                log_info(logger_kernel, "Mensaje de prueba recibido correctamente por el cliente %d", socket_cliente);
                break;
            // TODO: case NUEVO_CARPINCHO:
            // sumar a lista new
            //TODO: case OPERACION_SARASA
            // agregar a lista de operaciones del proceso     
            default:
                log_error(logger_kernel, "Codigo de operacion desconocido");
                break;
        }

        //Libero la memoria ocupada por el paquete
		free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);

        //Salgo del ciclo
        break; 
	}
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
