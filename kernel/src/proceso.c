#include "proceso.h"

void *proceso(void *self){
    t_proceso *proceso_struct = self;
    sem_wait(&proceso_inicializado);
    int index = 0;
    int prev_status = -1;
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
                    //TODO :proceso timestamp
                    printf("E - p: %d ", proceso_struct->id);
                    exec(proceso_struct);
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
            switch (next_task->id){
                case INIT_SEM:
                    log_info(logger_kernel, "Iniciando semaforo: %s", next_task->nombre_semaforo);
                    iniciar_semaforo(next_task->nombre_semaforo, next_task->value);
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

void *solicitar_semaforo(char *nombre_semaforo, int id){
    return NULL;
}

void iniciar_semaforo(char *nombre_semaforo, int valor){
    //agergar a lista de semaforos
    t_semaforo *new_semaforo = malloc(sizeof(t_semaforo));
    new_semaforo->value = valor;
    new_semaforo->nombre_semaforo = string_new();
    string_append(&(new_semaforo->nombre_semaforo), nombre_semaforo);
    new_semaforo->solicitantes = list_create();

    //printf("%s", new_semaforo->nombre_semaforo);
    list_add(lista_semaforos, new_semaforo);

}

void postear_semaforo(char *nombre_semaforo){

}