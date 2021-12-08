#include "planificador_mediano.h"

void iniciar_planificador_mediano(){
    printf("Inicio planificador MEDIANO \n");


    pthread_t hilo_planificador;
    pthread_create(&hilo_planificador, NULL, planificador_mediano_plazo, (void *)NULL);

    pthread_t hilo_salida_de_block;
    pthread_create(&hilo_salida_de_block, NULL, salida_de_block, (void *)NULL);
}


void *planificador_mediano_plazo(void *_){
    while(1){
        sem_wait(&cambio_de_listas);
        printf("Entre en el planificador mediano plazo y pase el sem_cambio_de_listas\n");
        int tamanio_block = list_size(lista_blocked);
        int tamanio_ready = list_size(lista_ready);
        int tamanio_new = list_size(lista_new);

        printf("tamanio blocked = %d -- tamanio ready = %d -- tamanio new = %d\n",tamanio_block,tamanio_ready,tamanio_new);

        if(tamanio_block > 0 && tamanio_ready == 0 && tamanio_new > 0){
            
            mover_proceso_de_lista(lista_blocked, lista_ready, tamanio_block - 1, READY);

            sem_wait(&mutex_multiprogramacion);
            multiprogramacion_disponible = multiprogramacion_disponible - 1;
            sem_post(&mutex_multiprogramacion);
        }
    }
}

void *salida_de_block(void *_){
    
    while(1){
        //sem_wait(&pedir_salida_de_block);
        sem_wait(&salida_block);

        bool encontrado = false;
        int tamanio_lista_blocked = list_size(lista_blocked);
        int tamanio_lista_s_blocked = list_size(lista_s_blocked);
        int index = 0;

        printf("Estoy en salida blocked\n tamanio blocked = %d\n",tamanio_lista_blocked);
        //Busco en block   
        while(!encontrado && (index < tamanio_lista_blocked)){
            
            t_proceso *aux = list_get(lista_blocked, index);
            printf("Estoy en salida de block dentro del while\n");
            if(aux->salida_block){

                mover_proceso_de_lista(lista_blocked, lista_ready, 0, READY);

                encontrado = true;
            }
            index ++;
        }

        index = 0;

        //Busco en suspended block
        while(!encontrado && (index < tamanio_lista_s_blocked)){

            t_proceso *aux = list_get(lista_s_blocked, index);
            
            if(aux->salida_block){
                
                mover_proceso_de_lista(lista_s_blocked, lista_s_ready, 0, READY);
                encontrado = true;
                
            }
            index ++;
        }
        
    }
    return NULL;
}