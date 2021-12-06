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
            next_task = list_get(self->task_list, 0);
            t_paquete *paquete_recibido = NULL;
            switch (next_task->id){
                case INIT_SEM:
                    //log_info(logger_kernel, "Iniciando semaforo: %s", next_task->nombre_semaforo);
                    //iniciar_semaforo(next_task->nombre_semaforo, next_task->value);
                    printf("Ejecuto la tarea init sem\n");
                    break;
                case CLIENTE_DESCONECTADO:
                case CLIENTE_TEST:
                case NUEVO_CARPINCHO:
                case SEM_WAIT:
                case SEM_POST:
                case SEM_DESTROY:
                case OP_ERROR:
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
            
            bloquear_f = true;
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

void solicitar_semaforo(char *nombre_semaforo, int id){
    // traer de la lista el semaforo
    t_semaforo *semaforo_solicitado = traer_semaforo(nombre_semaforo);
    
    if(semaforo_solicitado == NULL) return; //TODO: ENVIAR ERROR AL CARPINCHO

    if(semaforo_solicitado->value > 0){

        //si esta disponible restarle uno al value y enviar habilitacion para continuar
        semaforo_solicitado->value = semaforo_solicitado->value - 1;
        enviar_sem_disponible(id);

    }else{

        //si no esta disponible, agregar a la lista de solicitantes
        int *aux = malloc(sizeof(int));
        *aux = id;
        semaforo_solicitado->value = semaforo_solicitado->value - 1;
        list_add(semaforo_solicitado->solicitantes, aux);

    }
    return ;
}

void iniciar_semaforo(char *nombre_semaforo, int valor){

    //crear el struct del semaforo
    t_semaforo *new_semaforo = malloc(sizeof(t_semaforo));
    new_semaforo->value = valor;
    new_semaforo->nombre_semaforo = string_new();
    string_append(&(new_semaforo->nombre_semaforo), nombre_semaforo);
    new_semaforo->solicitantes = list_create();

    //agergar a lista de semaforos
    list_add(lista_semaforos, new_semaforo);

}

void postear_semaforo(char *nombre_semaforo){

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
        enviar_sem_disponible(*aux);

    }
}

void enviar_sem_disponible(int id){
    //enviar al id el sem disponible
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