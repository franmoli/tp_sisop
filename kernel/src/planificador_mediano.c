#include "planificador_mediano.h"

void iniciar_planificador_mediano(){
    printf("Inicio planificador MEDIANO \n");


    pthread_t hilo_planificador;
    pthread_create(&hilo_planificador, NULL, planificador_mediano_plazo, (void *)NULL);
    pthread_detach(hilo_planificador);
    pthread_t hilo_salida_de_block;
    pthread_create(&hilo_salida_de_block, NULL, salida_de_block, (void *)NULL);
    pthread_detach(hilo_salida_de_block);
    
}

void *planificador_mediano_plazo(void *_){
    while(!terminar_kernel){
        sem_wait(&cambio_de_listas_mediano);
        if(!terminar_kernel){
            int tamanio_block = list_size(lista_blocked);
            int tamanio_ready = list_size(lista_ready);
            int tamanio_new = list_size(lista_new);

            if(tamanio_block > 0 && tamanio_ready == 0 && tamanio_new > 0){
                
                
                mover_proceso_de_lista(lista_blocked, lista_s_blocked, tamanio_block - 1, S_BLOCKED);
                sem_wait(&mutex_multiprogramacion);
                multiprogramacion_disponible = multiprogramacion_disponible + 1;
                sem_post(&mutex_multiprogramacion);
                sem_post(&cambio_de_listas_largo);
            }

        }

    }
}

void *salida_de_block(void *_){
    
    while(!terminar_kernel){
        //sem_wait(&pedir_salida_de_block);
        sem_wait(&salida_block);
        if(!terminar_kernel){

            sem_wait(&mutex_listas);

            bool encontrado = false;
            int tamanio_lista_blocked = list_size(lista_blocked);
            int tamanio_lista_s_blocked = list_size(lista_s_blocked);
            int index = 0;

            //Busco en block   
            while(!encontrado && (index < tamanio_lista_blocked)){
                
                t_proceso *aux = list_get(lista_blocked, index);
                if(aux->salida_block){
                    aux->salida_block = false;
                    printf("salió de block el %d\n", aux->id);
                    mover_proceso_de_lista(lista_blocked, lista_ready, index, READY);

                    encontrado = true;
                }
                index ++;
            }

            index = 0;

            //Busco en suspended block
            while(!encontrado && (index < tamanio_lista_s_blocked)){

                t_proceso *aux = list_get(lista_s_blocked, index);
                
                if(aux->salida_block){
                    aux->salida_block = false;                
                    mover_proceso_de_lista(lista_s_blocked, lista_s_ready, 0, READY);
                    encontrado = true;
                    
                }
                index ++;
            }

            sem_post(&mutex_listas);        
        }
    }
    return NULL;
}