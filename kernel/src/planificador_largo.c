#include "planificador_largo.h"

void iniciar_planificador_largo(){
    //Iniciar servidor y empiezo a escuchar procesos
    printf("Inicio planificador LARGO \n");

    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, iniciar_servidor_kernel, (void *)NULL);

    pthread_t hilo_planificador;
    pthread_create(&hilo_planificador, NULL, planificador_largo_plazo, (void *)NULL);

    pthread_t hilo_exit;
    pthread_create(&hilo_exit, NULL, hilo_salida_a_exit, (void *)NULL);
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
                sem_wait(&libre_para_inicializar_proceso);
                pthread_t hilo_proceso_cliente;
                pthread_create(&hilo_proceso_cliente, NULL, (void *)atender_proceso, (void *)socket_proceso_cliente);
            }
        }
    }
    return NULL;
}

//Atencion de nuevas operaciones
void atender_proceso (void* parametro ){
    bool inicializado = false;
    int socket_cliente = *(int*)parametro;
    t_proceso *carpincho = malloc(sizeof(t_proceso)); 
    //char *nombre_semaforo = NULL;
    char *nombre_semaforo = string_new();
    string_append(&nombre_semaforo, "Semaforo mock");
    t_task *task_aux;
    while(1) {
		t_paquete *paquete = recibir_paquete(socket_cliente);
        task_aux = malloc(sizeof(t_task));
        print_semaforos();
        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        switch(paquete->codigo_operacion) {
            case CLIENTE_TEST:
                log_info(logger_kernel, "Mensaje de prueba recibido correctamente por el cliente %d", socket_cliente);
                break;

            case NUEVO_CARPINCHO:
                if(!inicializado){
                    carpincho = nuevo_carpincho(socket_cliente);
                    inicializado = true;
                }
                break;
            case INIT_SEM:
                //nombre_semaforo = paquete->buffer;
                //agregar a lista de actividades
                task_aux->id = INIT_SEM;
                task_aux->nombre_semaforo = nombre_semaforo;
                task_aux->value = 2;
                list_add(carpincho->task_list, task_aux);
                break;
            case CLIENTE_DESCONECTADO:
                log_info(logger_kernel, "Desconectando cliente %d", socket_cliente);
                close(socket_cliente);
                return;
                break;
                
            case MEMALLOC:
            case MEMFREE:
            case MEMREAD:
            case MEMWRITE:
                enviar_paquete(paquete, socket_cliente_memoria);
                break;
            default:
                log_error(logger_kernel, "Codigo de operacion desconocido");
                break;
            

        }

        //Libero la memoria ocupada por el paquete
		free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);

	}
    return;
}

t_proceso *nuevo_carpincho(int socket_cliente){

    t_proceso *nuevo_proceso = malloc(sizeof(t_proceso));

    nuevo_proceso->id = socket_cliente;
    nuevo_proceso->status = NEW;
    nuevo_proceso->estimacion = config_kernel->ESTIMACION_INICIAL;
    nuevo_proceso->ejecucion_anterior = 0;
    nuevo_proceso->estimar = false;
    nuevo_proceso->termino_rafaga = false;
    nuevo_proceso->block = false;
    nuevo_proceso->task_list = list_create();

    pthread_t hilo_proceso;
    pthread_create(&hilo_proceso, NULL, proceso, (void*)nuevo_proceso);

    sem_wait(&mutex_listas);
    list_add(lista_new, nuevo_proceso);
    sem_post(&mutex_listas);
    
    sem_wait(&mutex_cant_procesos);
    cantidad_de_procesos++;
    sem_post(&mutex_cant_procesos);

    sem_post(&proceso_inicializado);
    avisar_cambio();
    sem_post(&libre_para_inicializar_proceso);

    return nuevo_proceso;
}

void *planificador_largo_plazo(void *_){

    while(1){
        print_semaforos();
        sem_wait(&cambio_de_listas);
        if(multiprogramacion_disponible){
            if(list_size(lista_s_ready)){
                //Se saca de suspended y se pasa a ready
                mover_proceso_de_lista(lista_s_ready, lista_ready, 0, READY);

                multiprogramacion_disponible = multiprogramacion_disponible - 1;

            }else if(list_size(lista_new)){

                //Se saca de new y se pasa a ready
                mover_proceso_de_lista(lista_new, lista_ready, 0, READY);
                multiprogramacion_disponible = multiprogramacion_disponible - 1;
            }
        }
    }
    return NULL;
}

void *hilo_salida_a_exit(void *multiprogramacion_disponible_p){


    while(1){
        sem_wait(&salida_a_exit);
        
        //printf("Ready: %d\n", list_size(lista_ready));
        //printf("Block: %d\n", list_size(lista_blocked));
        //printf("Exec: %d\n", list_size(lista_exec));
        bool encontrado = false;
        //int tamanio_lista_exec = list_size(lista_exec);
        int tamanio_lista_blocked = list_size(lista_blocked);
        int tamanio_lista_ready = list_size(lista_ready);
        int index = 0;


        //Busco en exec
        //printf("Exec: %d \n", list_size(lista_exec));        
        while(!encontrado && (index < list_size(lista_exec))){
            //printf("Exec: %d \n", list_size(lista_exec));
            t_proceso *aux = list_get(lista_exec, index);
            if(aux->salida_exit){
                   // printf("Espera en exit - procesos\n");
                    sem_wait(&mutex_cant_procesos);
                    //printf("Pasó en exit\n");  
                    //printf(".");                  
                    sem_wait(&mutex_listas);
                        aux = list_remove(lista_exec, index );
                    sem_post(&mutex_listas);
                    cantidad_de_procesos--;
                    sem_post(&mutex_cant_procesos);
                    //printf("Post-procesos-planif-exit\n");
                 //printf("Saco proceso %d\n", aux->id);
                 //printf(".");
                encontrado = true;
            }
            index ++;
        }

        index = 0;

        //Busco en ready
        while(!encontrado && (index < tamanio_lista_ready)){
            t_proceso *aux = list_get(lista_ready, index);
            if(aux->salida_exit){
                
                    sem_wait(&mutex_cant_procesos);
                    sem_wait(&mutex_listas);
                        aux = list_remove(lista_ready, index );
                    sem_post(&mutex_listas);
                    cantidad_de_procesos--;
                    sem_post(&mutex_cant_procesos);
                

                //printf("Saco proceso %d\n", aux->id);
                encontrado = true;
            }
            index ++;
        }

        while(!encontrado && (index < tamanio_lista_blocked)){
            t_proceso *aux = list_get(lista_blocked, index);
            if(aux->salida_exit){
               
                sem_wait(&mutex_listas);
                aux = list_remove(lista_blocked, index);
                sem_post(&mutex_listas);
                
                //printf("Saco proceso %d\n", aux->id);
                encontrado = true;
            }
            index ++;
        }

        if(encontrado){
            sem_wait(&mutex_multiprogramacion);
            multiprogramacion_disponible = multiprogramacion_disponible + 1;
            sem_post(&mutex_multiprogramacion);
            sem_post(&proceso_finalizo_o_suspended);
            sem_post(&liberar_multiprocesamiento);
            sem_post(&salida_a_exit_recibida);
        }
        
    }
    return NULL;
}

void print_semaforos(){
    int index = 0;
    t_semaforo *aux;
    printf("Printing semaphores %d:\n", list_size(lista_semaforos));
    while(index < list_size(lista_semaforos)){
        aux = list_get(lista_semaforos, index);
        printf("Semaforo \"%s\" - Valor%d - Solicitantes %d\n",aux->nombre_semaforo, aux->value, list_size(aux->solicitantes));
        printf("\n");
        index++;
    }
}
