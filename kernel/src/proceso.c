#include "proceso.h"

void *proceso(void *self){
    
    t_proceso *proceso_struct = self;

    while(1){
        sem_wait(&actualizacion_de_listas_1);
        sem_post(&actualizacion_de_listas_1_recibido);
        switch (proceso_struct->status){
            case NEW:
                new();
                printf("Paso a new p: %d\n", proceso_struct->id);
                break;
            case READY:
                ready();
                printf("Paso a ready p: %d\n", proceso_struct->id);                
                break;
            case EXEC:
                exec();
                printf("Paso a exec p: %d\n", proceso_struct->id);
                break;
            case BLOCKED:
                blocked();
                printf("Paso a block p: %d\n", proceso_struct->id);
                break;
            case S_BLOCKED:
                blocked();
                break;
            case S_READY:
                blocked();
                break;
        }
        sem_wait(&actualizacion_de_listas_2);
    }


    return NULL;
}

void new(){

}
void ready(){

}
void exec(){

}
void blocked(){

}