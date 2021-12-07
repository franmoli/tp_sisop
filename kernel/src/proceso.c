#include "proceso.h"

void *proceso(void *self){

    t_proceso *proceso_struct = self;
    sem_wait(&proceso_inicializado);
    int prev_status = -1;
    int reloj_i = 0;

    while(1){
        sem_wait(&actualizacion_de_listas_1);
        if(prev_status != proceso_struct->status){
            switch (proceso_struct->status){
                case NEW:
                    printf("N - p: %d ", proceso_struct->id);
                    new();
                    sleep(1);
                    break;
                case EXEC:

                    reloj_i = clock();
                    printf("E - p: %d ", proceso_struct->id);
                    exec(proceso_struct);
                    proceso_struct->ejecucion_anterior = clock() - reloj_i;
                    proceso_struct->estimar = true;

                    
                    /*if(exec(aux)){
                        proceso_struct->salida_exit = true;
                        printf("salida a exit \n");
                        sem_post(&salida_a_exit);
                        //sem_wait(&salida_a_exit_recibida);
                        
                    }else{
                        proceso_struct->termino_rafaga = true;
                        //TODO: proceso calcular ejecucion real Y y setear para estimar
                        printf("Salida a ready\n");
                        sem_post(&salida_exec);
                        //sem_wait(&salida_de_exec_recibida);
                    }*/
                    break;
                case BLOCKED:
                    printf("B - p: %d ", proceso_struct->id);
                    break;
                default:
                    printf("Estoy en default y no hago nada %d - %d\n", proceso_struct->id, proceso_struct->status);
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

    //Enviar exec disponible
    //El exec disponible se resolveria mandando el operacion recibida, asi solo se va a enviar cuando este en exec y empiece a procesar las solicitudes enviadas

    t_task *next_task = NULL;
    bool bloquear_f = false;

    while(!bloquear_f){
        //Traer proxima operacion
        if(list_size(self->task_list)){
            next_task = list_remove(self->task_list, 0);
            t_paquete *paquete_recibido = NULL;
            t_semaforo *semaforo_aux = NULL;

            switch (next_task->id){
                case INIT_SEM:
                    semaforo_aux = next_task->datos_tarea;
                    log_info(logger_kernel, "Iniciando semaforo: %s", semaforo_aux->nombre_semaforo);
                    iniciar_semaforo(next_task->datos_tarea);
                    break;
                case CLIENTE_DESCONECTADO:
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
                    printf("Posteando semadoro %s\n", semaforo_aux->nombre_semaforo);
                    postear_semaforo(semaforo_aux->nombre_semaforo);
                    //enviar_confirmacion(self->socket_carpincho);
                    break;

                case SEM_DESTROY:
                case OP_ERROR:
                    break;
                case CALLIO:
                        
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
            
            //bloquear_f = true;
        }
    }

    sleep(2);
    bloquear(self);
}

void bloquear(t_proceso *self){
    printf("Bloqueando proceso: %d\n", self->id);
    self->block = true;
    sleep(1);
    sem_post(&solicitar_block);
    return;
}

void desbloquear(t_proceso *self){
    printf("Desbloquear proceso: %d", self->id);
    self->salida_block = true;
    sleep(1);
    sem_post(&salida_block);
    return;
}

bool solicitar_semaforo(char *nombre_semaforo, int id){
    //TODO: AGREGAR MUTEX DE VALUE O LO QUE SEA
    // traer de la lista el semaforo
    t_semaforo *semaforo_solicitado = traer_semaforo(nombre_semaforo);
    
    if(semaforo_solicitado == NULL){
        enviar_error(id);
        return true;
    }

    if(semaforo_solicitado->value > 0){

        //si esta disponible restarle uno al value y enviar habilitacion para continuar
        semaforo_solicitado->value = semaforo_solicitado->value - 1;

        enviar_confirmacion(id);

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

void postear_semaforo(char *nombre_semaforo){

    // traer de la lista el semaforo
    t_semaforo *semaforo_solicitado = traer_semaforo(nombre_semaforo);
    
    if(semaforo_solicitado == NULL) return; //TODO: ENVIAR ERROR AL CARPINCHO

    printf("Se encontro el semaforo solicitado con value %d\n", semaforo_solicitado->value);
    
// TODO: ACÁ ROMPE ESTA PORONGA
    if(semaforo_solicitado->value >= 0){
        printf("Intenta entrar por el lado que no debería %d\n", semaforo_solicitado->value);        
        //si esta disponible sumarle uno a value
        semaforo_solicitado->value = semaforo_solicitado->value + 1;

    }else{
        printf("Intenta entrar por acá\n");
        //si no esta disponible: sumarle uno a value - sacarlo de la lista de solicitantes - enviar habilitacion de continuar a ese proceso 
        //TODO: hacer un mutex para estas listas de semaforos
        int *aux;
        semaforo_solicitado->value = semaforo_solicitado->value + 1;
        aux = list_remove(semaforo_solicitado->solicitantes, 0);
        printf("Intentando desbloquear %d\n", *aux);

        desbloquear(traer_proceso_bloqueado(*aux));

    }
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
