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
    carpincho->task_list = list_create();
    carpincho->socket_carpincho = socket_cliente;
   // t_task *task_aux = NULL;
    t_semaforo *semaforo_recibido = NULL;
    t_io *io_recibida = NULL;
    while(1) {

		t_paquete *paquete = NULL;
		paquete = recibir_paquete(socket_cliente);
        //task_aux = malloc(sizeof(t_task));
        
        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        //printf("Paquete recibido %d\n", paquete->codigo_operacion);
        t_task *task = malloc(sizeof(t_task));

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

                semaforo_recibido = malloc(sizeof(t_semaforo));
                semaforo_recibido->nombre_semaforo = NULL;
                deserializar(paquete, 4, U_INT, &semaforo_recibido->value, CHAR_PTR, &semaforo_recibido->nombre_semaforo);
                
                task->id = INIT_SEM;
                task->datos_tarea = semaforo_recibido;

                list_add(carpincho->task_list, task);
                
                break;
            case SEM_WAIT:

                semaforo_recibido = malloc(sizeof(t_semaforo));
                semaforo_recibido->nombre_semaforo = NULL;

                deserializar(paquete, 2, CHAR_PTR, &semaforo_recibido->nombre_semaforo);
                
                task->id = SEM_WAIT;
                task->datos_tarea = semaforo_recibido;

                list_add(carpincho->task_list, task);
                break;

            case SEM_POST:

                semaforo_recibido = malloc(sizeof(t_semaforo));
                semaforo_recibido->nombre_semaforo = NULL;

                deserializar(paquete, 2, CHAR_PTR, &semaforo_recibido->nombre_semaforo);
                
                task->id = SEM_POST;
                task->datos_tarea = semaforo_recibido;

                list_add(carpincho->task_list, task);

                break;
                
            case CLIENTE_DESCONECTADO:
                log_info(logger_kernel, "Desconectando cliente %d", socket_cliente);
                
                //Agregar task de desconexion en caso que se encuentre en exec con prioridad máxima en caso que sea una desconexion forzosa
                task->id = CLIENTE_DESCONECTADO;
                list_add_in_index(carpincho->task_list, 0, task);

                //Se se pide la salida a exit del proceso
                carpincho->salida_exit = true;
                sem_post(&salida_a_exit);

                return;
                
            case MEMALLOC:
            
                task->id = MEMALLOC;
                task->datos_tarea = paquete;
                list_add(carpincho->task_list, task);
                break;
            case MEMFREE:
                task->id = MEMFREE;
                task->datos_tarea = paquete;
                list_add(carpincho->task_list, task);
                break;
            case MEMREAD:
                task->id = MEMREAD;
                task->datos_tarea = paquete;
                list_add(carpincho->task_list, task);
                break;
            case MEMWRITE:
                task->id = MEMWRITE;
                task->datos_tarea = paquete;
                list_add(carpincho->task_list, task);
                break;
            case CALLIO:
                task->id = CALLIO;
                io_recibida = malloc(sizeof(t_io));
                io_recibida->nombre = NULL;
                io_recibida->mensaje = NULL;
                io_recibida->proceso_solicitante = NULL;
                io_recibida->duracion = 0;
                deserializar(paquete, 4, CHAR_PTR, &io_recibida->nombre, CHAR_PTR, &io_recibida->mensaje);

                task->datos_tarea = io_recibida;
                list_add(carpincho->task_list, task);
                
                break;
            default:
                log_error(logger_kernel, "Codigo de operacion desconocido");
                //exit(EXIT_FAILURE);
                carpincho->salida_exit = true;
                sem_post(&salida_a_exit);
                return;
                break;
            
        }

        //Libero la memoria ocupada por el paquete
		//free(paquete->buffer->stream);
        //free(paquete->buffer);
        //free(paquete);

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
    nuevo_proceso->salida_exit = false;
    nuevo_proceso->salida_block = false;
    nuevo_proceso->task_list = list_create();
    nuevo_proceso->socket_carpincho = socket_cliente;

    pthread_t hilo_proceso;
    pthread_create(&hilo_proceso, NULL, proceso, (void*)nuevo_proceso);

    sem_wait(&mutex_listas);
    list_add(lista_new, nuevo_proceso);
    sem_post(&mutex_listas);
    
    sem_wait(&mutex_cant_procesos);
    printf("Nuevo carpincho\n");
    cantidad_de_procesos++;
    sem_post(&mutex_cant_procesos);

    sem_post(&proceso_inicializado);
    avisar_cambio();
    sem_post(&libre_para_inicializar_proceso);

   
    return nuevo_proceso;
}

void *planificador_largo_plazo(void *_){

    while(1){
        sem_wait(&cambio_de_listas_largo);
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
        
        
        bool encontrado = false;
        int tamanio_lista_exec = list_size(lista_exec);
        int tamanio_lista_blocked = list_size(lista_blocked);
        int tamanio_lista_ready = list_size(lista_ready);
        int index = 0;
        t_recurso_asignado *recurso_asignado_aux = NULL;
        t_proceso *aux = NULL;


        //Busco en exec
        //printf("Exec: %d \n", list_size(lista_exec));
        if(aux == NULL)
            aux = list_remove_by_condition(lista_exec, pedido_exit);

        //Si lo encontré en exec tengo que liberar el multiprocesamiento
        if(aux != NULL)
            sem_post(&liberar_multiprocesamiento);

        if(aux == NULL)
            aux = list_remove_by_condition(lista_new, pedido_exit);

        if(aux == NULL)
            aux = list_remove_by_condition(lista_ready, pedido_exit);

        if(aux == NULL)
            aux = list_remove_by_condition(lista_s_blocked, pedido_exit);
        
        if(aux == NULL)
            aux = list_remove_by_condition(lista_s_ready, pedido_exit);
        
        if(aux == NULL)
            aux = list_remove_by_condition(lista_blocked, pedido_exit);



        list_add(lista_exit, aux);
        aux->status = EXIT; 
        log_info(logger_kernel, "Terminando con proceso %d", aux->id);

        //Liberar recursos asignados
        eliminar_solicitud_de_sem(aux->id);

        while(index < list_size(lista_recursos_asignados)){
            recurso_asignado_aux = list_get(lista_recursos_asignados, index);
            if(recurso_asignado_aux->id_asignado == aux->id){
                
                list_remove(lista_recursos_asignados, index);
                printf("Posteando semaforo %d\n", aux->id);
                postear_semaforo(recurso_asignado_aux->nombre_recurso, aux->id);
            }
            index++;
        }

        
        
        sem_wait(&mutex_multiprogramacion);
        multiprogramacion_disponible = multiprogramacion_disponible + 1;
        sem_post(&mutex_multiprogramacion);

        //Aviso a todos los procesos, asi finaliza el hilo correspondiente
        avisar_cambio();

        sem_wait(&mutex_cant_procesos);
        cantidad_de_procesos = cantidad_de_procesos - 1;
        sem_post(&mutex_cant_procesos);
        
    }
    return NULL;
}

bool pedido_exit(void *elemento){
    t_proceso *proceso = elemento;
    if(proceso->salida_exit)
        return true;
    
    return false;
}

void enviar_confirmacion(int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
    paquete->codigo_operacion = OP_CONFIRMADA;

    enviar_paquete(paquete, socket);
    return;
}

void enviar_error(int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
    paquete->codigo_operacion = OP_ERROR;

    enviar_paquete(paquete, socket);

    
    return;
}

void eliminar_solicitud_de_sem(int id){
    int index = 0;
    int index2 = 0;
    t_semaforo *aux = NULL;
    int *aux_solicitante = NULL;

    while(index < list_size(lista_semaforos)){
        aux = list_get(lista_semaforos, index);
        while(index2 < list_size(aux->solicitantes)){
            aux_solicitante = list_get(aux->solicitantes, index2);
            if(*aux_solicitante == id){
                list_remove(aux->solicitantes, index2);
                aux->value = aux->value + 1;
            }
            index2++;
        }
        index++;
    }
}