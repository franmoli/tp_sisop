
#include "../include/planificador_corto.h"

void estimar(t_proceso *proceso);

void iniciar_planificador_corto(){

    printf("Inicio planificador CORTO \n");
    void *(*planificador)(void*);
    pthread_t hilo_planificador;
    pthread_t hilo_terminar_rafaga;
    int *multiprocesamiento = malloc(sizeof(int));
    *multiprocesamiento = config_kernel->GRADO_MULTIPROCESAMIENTO;

    if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "SJF")){

        planificador = planificador_corto_plazo_sjf;

    }else if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "HRRN")){

        planificador = planificador_corto_plazo_hrrn;

    }else{

        log_error(logger_kernel, "Planificador no soportado/no reconocido");
        return;
    }
     
    pthread_create(&hilo_planificador, NULL, planificador, (void *)multiprocesamiento);
    pthread_create(&hilo_terminar_rafaga, NULL, esperar_salida_exec, (void *)multiprocesamiento);
}

void *planificador_corto_plazo_sjf (void *multiprocesamiento_p){

    
    t_proceso *aux;
    int *multiprocesamiento = multiprocesamiento_p;
    while(1){

        //calcular estimaciones
        for(int i = 0; i < list_size(lista_ready); i++){

            aux = list_get(lista_ready, i);
            if(aux->estimar)
                estimar(aux);
        }
        
        if(*multiprocesamiento && list_size(lista_ready)){
            int index = -1;
            int estimacion_aux;

            //se busca la estimacion menor
            for(int i = 0; i < list_size(lista_ready); i++){
                aux = list_get(lista_ready, i);
                if(aux->estimacion < estimacion_aux || i == 0){
                    estimacion_aux = aux->estimacion;
                    index = i;
                }
            }

            //Se saca de ready y se pasa a exec
            mover_proceso_de_lista(lista_ready, lista_exec, index, EXEC);

            sem_wait(&mutex_multiprocesamiento);
            *multiprocesamiento = *multiprocesamiento - 1;
            sem_post(&mutex_multiprocesamiento);
        }
    }


    return NULL;
}

void *planificador_corto_plazo_hrrn (void *multiprocesamiento_p){
    
    return NULL;
}

void estimar(t_proceso *proceso){
    int alfa = config_kernel->ALFA;
    proceso->estimacion = (alfa * proceso->ejecucion_anterior) + (( 1 - alfa) * proceso->estimacion);
    proceso->estimar = false;
    return;
}

void *esperar_salida_exec(void *multiprocesamiento_p){

    int *multiprocesamiento = multiprocesamiento_p;

    while(1){

        sem_wait(&salida_exec);

        bool encontrado = false;
        int tamanio_lista_exec = list_size(lista_exec);
        int index = 0;

        while(!encontrado && (index < tamanio_lista_exec)){
            t_proceso *aux = list_get(lista_exec, index);
            if(aux->termino_rafaga){
                if(aux->block){
                    mover_proceso_de_lista(lista_exec, lista_blocked, index, BLOCKED);
                }else{
                    mover_proceso_de_lista(lista_exec, lista_ready, index, READY);
                }
                encontrado = true;
            }
            index ++;
        }
        sem_wait(&mutex_multiprocesamiento);
        *multiprocesamiento = *multiprocesamiento + 1;
        sem_post(&mutex_multiprocesamiento);
    }
}

void *esperar_salida_block(void *multiprocesamiento_p){

    

    while(1){

        sem_wait(&salida_block);

        bool encontrado = false;
        int tamanio_lista_blocked = list_size(lista_blocked);
        int index = 0;

        while(!encontrado && (index < tamanio_lista_blocked)){
            t_proceso *aux = list_get(lista_blocked, index);
            if(aux->termino_rafaga){
                    mover_proceso_de_lista(lista_blocked, lista_ready, index, READY);
                encontrado = true;
            }
            index ++;
        }
    }
}