#include "planificador_largo.h"

void iniciar_planificador_largo(){
    //Iniciar servidor y empiezo a escuchar procesos
    printf("Inicio planificador LARGO \n");

    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, iniciar_servidor_kernel, (void *)NULL);

    pthread_t hilo_planificador;
    pthread_create(&hilo_planificador, NULL, planificador_largo_plazo, (void *)NULL);
    
}



void *iniciar_servidor_kernel(void *_){
    int socket_servidor = iniciar_servidor(config_kernel->IP_KERNEL,config_kernel->PUERTO_KERNEL, logger_kernel);
    if(socket_servidor == -1){
        log_error(logger_kernel, "Fallo en la creacion del servidor");
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

void atender_proceso (void* parametro ){

    int socket_cliente = *(int*)parametro;
    while(1) {
		t_paquete *paquete = recibir_paquete(socket_cliente);

        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        switch(paquete->codigo_operacion) {
            case CLIENTE_TEST:
                log_info(logger_kernel, "Mensaje de prueba recibido correctamente por el cliente %d", socket_cliente);
                break;
            case NUEVO_CARPINCHO:
                nuevo_carpincho(socket_cliente);
                break;
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

void nuevo_carpincho(int socket_cliente){
    t_proceso *nuevo_proceso = malloc(sizeof(t_proceso));

    nuevo_proceso->id = socket_cliente;
    nuevo_proceso->status = NEW;
    nuevo_proceso->estimacion = config_kernel->ESTIMACION_INICIAL;
    nuevo_proceso->ejecucion_anterior = 0;
    nuevo_proceso->estimar = false;

    pthread_t hilo_proceso;
    pthread_create(&hilo_proceso, NULL, proceso, (void*)nuevo_proceso);

    sem_wait(&mutex_listas);
    list_add(lista_new, nuevo_proceso);
    sem_post(&mutex_listas);

    cantidad_de_procesos++;
    avisar_cambio();



}

void *planificador_largo_plazo(void *_){

    int multiprogramacion_disponible = config_kernel->GRADO_MULTIPROGRAMACION;

    while(1){
        if(multiprogramacion_disponible){
            if(list_size(lista_new)){

                //Se saca de new y se pasa a ready
                mover_proceso_de_lista(lista_new, lista_ready, 0, READY);

                multiprogramacion_disponible--;
            }
        }else{

            printf("me trabé listas:\n");
            printf("Ready: %d\n", list_size(lista_ready));
            printf("Block: %d\n", list_size(lista_blocked));
            printf("Exec: %d\n", list_size(lista_exec));
            printf(".");
            sem_wait(&proceso_finalizo_o_suspended);
            printf("me destrabé\n");
            multiprogramacion_disponible++;
        }
    }
    return NULL;
}