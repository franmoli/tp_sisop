#include "proceso.h"

void *proceso(void *self){
    
    t_proceso *proceso_struct = self;
    int prev_status = -1;
    
    pthread_t hilo_ejecucion;


    while(!terminar_kernel){
        sem_wait(&actualizacion_de_listas_1);
        if(prev_status != proceso_struct->status){
            switch (proceso_struct->status){
                case NEW:
                    break;
                case EXEC:
                    pthread_create(&hilo_ejecucion, NULL, exec, (void *)proceso_struct);
                    break;
                case BLOCKED:
                    break;
                case EXIT:
                    //liberar semaforos tomados por este
                    sem_post(&actualizacion_de_listas_1_recibido);
                    sem_wait(&actualizacion_de_listas_2);
                    return NULL;
                case S_BLOCKED:
                    log_info(logger_kernel, "Suspendo al proceso %d", proceso_struct->id);
                default:
                    break;
            }
            prev_status = proceso_struct->status;
        }
        sem_post(&actualizacion_de_listas_1_recibido);
        sem_wait(&actualizacion_de_listas_2);
        
    }
    sem_post(&actualizacion_de_listas_1_recibido);
    sem_wait(&actualizacion_de_listas_2);
    return NULL;
}

void new(){
    //printf("iniciando el carpincho - new\n");
}

void exec(t_proceso *self){
    pthread_detach(pthread_self());
    log_info(logger_kernel,"Ejecutando el proceso %d",self->id);
    t_task *next_task = NULL;
    bool bloquear_f = false;
    int reloj_i = 0;
    reloj_i = clock();
    enviar_confirmacion(self->socket_carpincho);

    while(!bloquear_f){
        //Traer proxima operacion
        if(list_size(self->task_list)){
            next_task = list_remove(self->task_list, 0);
            t_paquete *paquete_recibido = NULL;
            t_semaforo *semaforo_aux = NULL;
            t_io *io_recibida = NULL;
            int index = 0;
            char *io_device = NULL;
            pthread_t hilo_desbloquear_en;
            

            switch (next_task->id){
                case INIT_SEM:
                    semaforo_aux = next_task->datos_tarea;
                    log_info(logger_kernel, "Iniciando semaforo: %s", semaforo_aux->nombre_semaforo);
                    iniciar_semaforo(next_task->datos_tarea);
                    enviar_confirmacion(self->socket_carpincho);
                    //free(semaforo_aux->nombre_semaforo);
                    break;
                case CLIENTE_DESCONECTADO:
                    free(next_task);
                    return;
                case CLIENTE_TEST:
                case NUEVO_CARPINCHO:
                case MATEINIT:
                case SWAPFREE:
                case SWAPSAVE:
                case OP_CONFIRMADA:
                case RECEPCION_PAGINA:
                    break;
                case SEM_WAIT:
                    semaforo_aux = next_task->datos_tarea;

                    sem_wait(&mutex_semaforos);
                    sem_wait(&mutex_recursos_asignados);

                    bloquear_f = solicitar_semaforo(semaforo_aux->nombre_semaforo, self->socket_carpincho);
                    free(semaforo_aux->nombre_semaforo);
                    free(semaforo_aux);
                    sem_post(&mutex_recursos_asignados);
                    sem_post(&mutex_semaforos);
                    break;

                case SEM_POST:
                    semaforo_aux = next_task->datos_tarea;

                    sem_wait(&mutex_semaforos);
                    sem_wait(&mutex_recursos_asignados);

                    printf("Carpincho %d posteando semaforo %s\n", self->socket_carpincho, semaforo_aux->nombre_semaforo);
                    postear_semaforo(semaforo_aux->nombre_semaforo, self->id);
                    free(semaforo_aux->nombre_semaforo);
                    free(semaforo_aux);
                    sem_post(&mutex_recursos_asignados);
                    sem_post(&mutex_semaforos);

                    enviar_confirmacion(self->socket_carpincho);
                    break;

                case SEM_DESTROY:
                    semaforo_aux = next_task->datos_tarea;

                    sem_wait(&mutex_semaforos);
                    sem_wait(&mutex_recursos_asignados);

                    printf("Carpincho %d eliminando semaforo %s\n", self->socket_carpincho, semaforo_aux->nombre_semaforo);
                    destruir_semaforo(semaforo_aux->nombre_semaforo);
                    enviar_confirmacion(self->socket_carpincho);
                    free(semaforo_aux->nombre_semaforo);
                    free(semaforo_aux);
                    sem_post(&mutex_recursos_asignados);
                    sem_post(&mutex_semaforos);

                    break;
                case OP_ERROR:
                    break;
                case CALLIO:
                    sleep(1);
                    io_recibida = next_task->datos_tarea;

                    while(index < list_size(config_kernel->DISPOSITIVOS_IO)){
                        io_device = list_get(config_kernel->DISPOSITIVOS_IO,index);
                        if(!strcmp(io_recibida->nombre, io_device)){
                            break;
                        }
                        index++;
                    }
                    io_recibida->proceso_solicitante = self;
                    io_recibida->id = index;
                    io_recibida->duracion = atoi(list_get(config_kernel->DURACIONES_IO, index));
                    pthread_create(&hilo_desbloquear_en, NULL, desbloquear_en, (void *)io_recibida);
                    index = 0;
                    bloquear_f = true;
                    break;
                case MEMALLOC:
                case MEMFREE:
                case MEMREAD:
                case MEMWRITE:
                    log_info(logger_kernel,"enviando paquete");
                    enviar_paquete(next_task->datos_tarea,socket_cliente_memoria);
                    paquete_recibido = recibir_paquete(socket_cliente_memoria);
                    enviar_paquete(paquete_recibido,self->socket_carpincho);
                    break;
            }
            free(next_task);
        }
    }

    //Solo por debug, borrar despues
    //sleep(1);
    self->ejecucion_anterior = ((clock() - reloj_i) / CLOCKS_PER_SEC) * 1000;
    self->estimar = true;

    bloquear(self);
    return;
}

bool solicitar_semaforo(char *nombre_semaforo, int id){
    //TODO: AGREGAR MUTEX DE VALUE O LO QUE SEA
    // traer de la lista el semaforo
    printf("Carpincho %d - solicité el semaforo %s\n", id, nombre_semaforo);
    t_semaforo *semaforo_solicitado = traer_semaforo(nombre_semaforo);
    
    if(semaforo_solicitado == NULL){
        enviar_error(id);
        return true;
    }

    if(semaforo_solicitado->value > 0){

        //si esta disponible restarle uno al value y enviar habilitacion para continuar
        semaforo_solicitado->value = semaforo_solicitado->value - 1;
        enviar_confirmacion(id);

        // sumar a lista de recursos asignados 
        t_recurso_asignado *recurso_asignado = malloc(sizeof(t_recurso_asignado));
        recurso_asignado->nombre_recurso = semaforo_solicitado->nombre_semaforo;
        recurso_asignado->id_asignado = id;
        list_add(lista_recursos_asignados, recurso_asignado);

        //Esta disponible, no bloquear
        return false;
    }else{

        //si no esta disponible, agregar a la lista de solicitantes
        int *aux = malloc(sizeof(int));
        *aux = id;

        semaforo_solicitado->value = semaforo_solicitado->value - 1;
        list_add(semaforo_solicitado->solicitantes, aux);

        //no esta disponible, bloquear
        return true;
    }

}

void iniciar_semaforo(t_semaforo *semaforo){

    //crear el struct del semaforo
    semaforo->solicitantes = list_create();
    char *nombre_semaforo = semaforo->nombre_semaforo;

    bool esta_el_semaforo(void *semaforo_param){
        t_semaforo *semaforo_aux = semaforo_param;
        if(string_contains(semaforo_aux->nombre_semaforo, nombre_semaforo))
            return true;
        
        return false;
    }

    //Si no existia el semaforo antes se agrega a las lista de semaforos
    if(!list_any_satisfy(lista_semaforos, esta_el_semaforo))
        list_add(lista_semaforos, semaforo);

}

void postear_semaforo(char *nombre_semaforo, int id_del_chabon_que_postea ){

    // traer de la lista el semaforo
    t_semaforo *semaforo_solicitado = traer_semaforo(nombre_semaforo);
        //printf("\nPass %p\n\n", semaforo_solicitado);
    
    if(semaforo_solicitado == NULL) return; //TODO: ENVIAR ERROR AL CARPINCHO
    
    if(semaforo_solicitado->value >= 0){        
        //si esta disponible sumarle uno a value
        semaforo_solicitado->value = semaforo_solicitado->value + 1;

        //printf("Posteando %d\n", semaforo_solicitado->value);

    }else{
        //si no esta disponible: sumarle uno a value - sacarlo de la lista de solicitantes - enviar habilitacion de continuar a ese proceso 
        //TODO: hacer un mutex para estas listas de semaforos
        
        int *aux;
        semaforo_solicitado->value = semaforo_solicitado->value + 1;
        aux = list_remove(semaforo_solicitado->solicitantes, 0);

        printf("Posteando al solicitante %d\n", *aux);
        //sumar a lista de recursos asignados
        t_recurso_asignado *recurso_asignado = malloc(sizeof(t_recurso_asignado));
        recurso_asignado->nombre_recurso = nombre_semaforo;
        recurso_asignado->id_asignado = *aux;

        list_add(lista_recursos_asignados, recurso_asignado);        

        //printf("Pass %d\n", *aux);
        printf("Quiero desbloquear a %d\n", *aux);

        while(procesos_esperando_bloqueo);

        desbloquear(traer_proceso_bloqueado(*aux));

    }
    //printf("Pass 2\n");
    devolver_recurso(id_del_chabon_que_postea, nombre_semaforo);


}

t_proceso *traer_proceso_bloqueado(int id){
    printf("trayendo proceso bloqueado %d \n", id);

    t_proceso *encontrado = NULL;
    int index_aux = 20;

    bool se_encontro(void *elemento){
        t_proceso *proceso = elemento;
        return proceso->id == id;
    };

    while(index_aux){
        encontrado = list_find(lista_blocked, se_encontro);

        if(encontrado != NULL)
            return encontrado;
        
        encontrado = list_find(lista_s_blocked, se_encontro);

        if(encontrado != NULL)
            return encontrado;

        index_aux--;
        printf("Sleeping\n");
    }

    print_lists();
    log_error(logger_kernel, "Error en traer bloqueado");
    exit(1);

}

t_semaforo *traer_semaforo(char *nombre_solicitado){

    t_semaforo *semaforo_solicitado;
    int index = 0;

    while(index < list_size(lista_semaforos)){

        semaforo_solicitado = list_get(lista_semaforos, index);
        if(!strcmp(semaforo_solicitado->nombre_semaforo, nombre_solicitado)) break;
        index++;

    }

    if(index == list_size(lista_semaforos)) return NULL;
    

    return semaforo_solicitado;

}

void bloquear(t_proceso *self){
    printf("Bloqueando proceso: %d\n", self->id);
    self->block = true;
    procesos_esperando_bloqueo++;
    sem_post(&solicitar_block);
    return;
}

void desbloquear(t_proceso *self){
    printf("Desbloquear proceso: %d %d\n", self->id, procesos_esperando_bloqueo);
    self->salida_block = true;
    sem_post(&salida_block);
    return;
}

void *desbloquear_en(void *param){
    pthread_detach(pthread_self());
    t_io *io_recibida = param;
    sem_wait(&io_libre[io_recibida->id]);

    sleep(io_recibida->duracion/1000);
    log_info(logger_kernel, "IO ejecutada %s", io_recibida->mensaje);
    //printf("Desbloqueando X salida de io\n");
    desbloquear(io_recibida->proceso_solicitante);
    
    sem_post(&io_libre[io_recibida->id]);

    free(io_recibida->nombre);
    free(io_recibida->mensaje);
    free(io_recibida);

    return NULL;
}

void devolver_recurso(int id, char *sem_devuelto){
    t_recurso_asignado *aux = NULL;
    bool lo_encontre(void *elemento){
        t_recurso_asignado *recurso = elemento;
        if(recurso->id_asignado == id && !strcmp(sem_devuelto, recurso->nombre_recurso))
            return true;
        
        return false;
    }
    aux =list_remove_by_condition(lista_recursos_asignados, lo_encontre);
    if(aux != NULL){
        free(aux);
    }
    return;
}

void destruir_semaforo(char *nombre_semaforo){
    t_semaforo *semaforo = traer_semaforo(nombre_semaforo);
    int cantidad_de_solicitantes = list_size(semaforo->solicitantes);
    t_recurso_asignado *aux_asignado = NULL;

    //Desbloquear todos los solicitantes bloqueados por este
    for(int i = 0; i < cantidad_de_solicitantes; i++){
        printf("Tenía solicitantes, posteandolo\n");
        postear_semaforo(nombre_semaforo, 0);
    }

    //eliminar los que lo tengan como recurso asignado
    bool recurso_encontrado(void *elemento){
        t_recurso_asignado *a_testear = elemento;
        if(!strcmp(a_testear->nombre_recurso, nombre_semaforo))
            return true;
        
        return false;
    };

    aux_asignado = list_remove_by_condition(lista_recursos_asignados, recurso_encontrado);
    while(aux_asignado != NULL){
        aux_asignado = NULL;
        aux_asignado = list_remove_by_condition(lista_recursos_asignados, recurso_encontrado);
    }

    //Eliminar de la lista de semaforos 
    bool semaforo_encontrado(void *elemento){
        t_semaforo *sem_a_testear = elemento;
        if(!strcmp(sem_a_testear->nombre_semaforo, nombre_semaforo))
            return true;

        return false;
    };

    list_remove_by_condition(lista_semaforos, semaforo_encontrado);
    list_destroy(semaforo->solicitantes);
    free(semaforo->nombre_semaforo);
    free(semaforo);

    print_semaforos();
    return;
}