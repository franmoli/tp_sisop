#include "proceso.h"

void *proceso(void *self){

    t_proceso *proceso_struct = self;
    sem_wait(&proceso_inicializado);
    int prev_status = -1;
    
    pthread_t hilo_ejecucion;

    while(1){
        sem_wait(&actualizacion_de_listas_1);
        if(prev_status != proceso_struct->status){
            switch (proceso_struct->status){
                case NEW:
                    //printf("New - p: %d ", proceso_struct->id);
                    new();
                    sleep(1);
                    break;
                case EXEC:
                    pthread_create(&hilo_ejecucion, NULL, exec, (void *)proceso_struct);
                    break;
                case BLOCKED:
                    //printf("Block - p: %d \n", proceso_struct->id);
                    break;
                case EXIT:
                    sem_post(&actualizacion_de_listas_1_recibido);
                    sem_wait(&actualizacion_de_listas_2);
                    return;
                default:
                    break;
                    //printf("Estoy en default y no hago nada %d - %d\n", proceso_struct->id, proceso_struct->status);
            }
            prev_status = proceso_struct->status;
        }
        sem_post(&actualizacion_de_listas_1_recibido);
        sem_wait(&actualizacion_de_listas_2);
        
    }


    return NULL;
}

void new(){
    printf("iniciando el carpincho - new\n");
    sleep(1);
}

void exec(t_proceso *self){

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
                    break;
                case CLIENTE_DESCONECTADO:
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
                    bloquear_f = solicitar_semaforo(semaforo_aux->nombre_semaforo, self->socket_carpincho);
                    break;

                case SEM_POST:
                    semaforo_aux = next_task->datos_tarea;
                    printf("Posteando semaforo %s\n", semaforo_aux->nombre_semaforo);
                    postear_semaforo(semaforo_aux->nombre_semaforo, self->id);
                    enviar_confirmacion(self->socket_carpincho);
                    break;

                case SEM_DESTROY:
                case OP_ERROR:
                    break;
                case CALLIO:
                    printf("Ejecutando la io\n");
                    io_recibida = next_task->datos_tarea;

                    while(index < list_size(config_kernel->DISPOSITIVOS_IO)){
                        io_device = list_get(config_kernel->DISPOSITIVOS_IO,index);
                        if(!strcmp(io_recibida->nombre, io_device)){
                            break;
                        }
                        index++;
                    }
                    io_recibida->proceso_solicitante = self;
                    //strtol(list_get(config_kernel->DURACIONES_IO, index), NULL, 10);
                    io_recibida->duracion = atoi(list_get(config_kernel->DURACIONES_IO, index));
                    pthread_create(&hilo_desbloquear_en, NULL, desbloquear_en, (void *)io_recibida);
                    index = 0;
                    bloquear_f = true;

                    break;
                case MEMALLOC:
                case MEMFREE:
                case MEMREAD:
                case MEMWRITE:
                    enviar_paquete(next_task->datos_tarea,socket_cliente_memoria);
                    paquete_recibido = recibir_paquete(socket_cliente_memoria);
                    enviar_paquete(paquete_recibido,self->socket_carpincho);
                    break;

            }
            
        }
    }

    //Solo por debug, borrar despues
    sleep(2);

    self->ejecucion_anterior = clock() - reloj_i;
    self->estimar = true;

    bloquear(self);
}

bool solicitar_semaforo(char *nombre_semaforo, int id){
    //TODO: AGREGAR MUTEX DE VALUE O LO QUE SEA
    // traer de la lista el semaforo
    printf("Se solicito el semadoro %s\n", nombre_semaforo);
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
        recurso_asignado->nombre_recurso = nombre_semaforo;
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
    
    if(semaforo_solicitado == NULL) return; //TODO: ENVIAR ERROR AL CARPINCHO
    
    if(semaforo_solicitado->value >= 0){        
        //si esta disponible sumarle uno a value
        semaforo_solicitado->value = semaforo_solicitado->value + 1;

    }else{
        //si no esta disponible: sumarle uno a value - sacarlo de la lista de solicitantes - enviar habilitacion de continuar a ese proceso 
        //TODO: hacer un mutex para estas listas de semaforos
        int *aux;
        semaforo_solicitado->value = semaforo_solicitado->value + 1;
        aux = list_remove(semaforo_solicitado->solicitantes, 0);

        //sumar a lista de recursos asignados
        t_recurso_asignado *recurso_asignado = malloc(sizeof(t_recurso_asignado));
        recurso_asignado->nombre_recurso = nombre_semaforo;
        recurso_asignado->id_asignado = *aux;
        list_add(lista_recursos_asignados, recurso_asignado);        

        desbloquear(traer_proceso_bloqueado(*aux));

    }
    
    devolver_recurso(id_del_chabon_que_postea);

}

t_proceso *traer_proceso_bloqueado(int id){

    t_proceso *encontrado = NULL;

    bool se_encontro(void *elemento){
        t_proceso *proceso = elemento;
        return proceso->id == id;
    }

    encontrado = list_find(lista_blocked, se_encontro);

    if(encontrado != NULL)
        return encontrado;
    
    encontrado = list_find(lista_s_blocked, se_encontro);

    if(encontrado != NULL)
        return encontrado;

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
    //printf("Bloqueando proceso: %d\n", self->id);
    self->block = true;
    sleep(1);
    sem_post(&solicitar_block);
    return;
}

void desbloquear(t_proceso *self){
    //printf("Desbloquear proceso: %d\n", self->id);
    self->salida_block = true;
    sleep(1);
    if(self->status == BLOCKED)
        enviar_confirmacion(self->id);
    sem_post(&salida_block);
    //sem_post(&pedir_salida_de_block);
    return;
}

void *desbloquear_en(void *param){
    t_io *io_recibida = param;
    log_info(logger_kernel, "Solicitando el dispositivo %s", io_recibida->nombre);
    sleep(io_recibida->duracion);
    log_info(logger_kernel, "IO ejecutada %s", io_recibida->mensaje);
    //printf("Desbloqueando X salida de io\n");
    desbloquear(io_recibida->proceso_solicitante);
    return NULL;
}

void devolver_recurso(int id){
    bool lo_encontre(void *elemento){
        t_recurso_asignado *recurso = elemento;
        if(recurso->id == id)
            return true;
        
        return false;
    }
    list_remove_by_condition(lista_recursos_asignados, lo_encontre);

    return;
}