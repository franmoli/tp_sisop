#include "proceso.h"

void *proceso(void *self){
    t_proceso *proceso_struct = self;
    sem_wait(&proceso_inicializado);
    int index = 0;
    while(1){
        printf("esperando actual ed listax\n");
        sem_wait(&actualizacion_de_listas_1);

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
                printf("Estoy bloqueado y no hago nada %d\n", proceso_struct->id);
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

void *exec(t_proceso *self){

    bool bloquear_f = false;
    //Tarea mock
    printf("Haciendo la tarea - exec\n");

    while(!bloquear_f){
        sleep(2);
        bloquear_f = true;
    }

    bloquear(self);
    return NULL;
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