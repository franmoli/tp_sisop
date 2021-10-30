#include "proceso.h"

void *proceso(void *self){
    t_proceso *proceso_struct = self;
    sem_wait(&proceso_inicializado);
    int aux;
    aux = 0;
    while(1){
        printf("\n");
        sem_wait(&actualizacion_de_listas_1);

        switch (proceso_struct->status){
            case NEW:
                printf("N - p: %d ", proceso_struct->id);
                new();
                sleep(1);
                break;
            case READY:
                printf("R - p: %d ", proceso_struct->id);                
                ready();
                break;
            case EXEC:
                //TODO :proceso timestamp
                printf("E - p: %d ", proceso_struct->id);
                aux++;
                if(exec(aux)){
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
                }
                break;
            case BLOCKED:
                blocked();
                //printf("Paso a block p: %d\n", proceso_struct->id);
                break;
            case S_BLOCKED:
                blocked();
                break;
            case S_READY:
                blocked();
                break;
        }
        sem_post(&actualizacion_de_listas_1_recibido);
        sem_wait(&actualizacion_de_listas_2);
        if(aux > 1){
            printf("retorna %d---\n", proceso_struct->id);
            printf(".");
            return NULL;
        }
    }


    return NULL;
}

void new(){
    printf("iniciando el carpincho - new\n");
    sleep(1);
}
void ready(){

}
int exec(int aux){
    //Tarea mock
    printf("Haciendo la tarea - exec\n");
    sleep(2);

    if(aux > 1)
        return true;

    return false;

}
void blocked(){

}