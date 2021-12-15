
#include "../include/planificador_corto.h"

void estimar(t_proceso *proceso);

void iniciar_planificador_corto(){

    printf("Inicio planificador CORTO \n");
    void *(*planificador)(void*);
    pthread_t hilo_planificador;
    pthread_t hilo_esperar_bloqueo;
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
    pthread_create(&hilo_esperar_bloqueo, NULL, esperar_bloqueo, (void *)multiprocesamiento);
}

void *planificador_corto_plazo_sjf (void *_){

    
    t_proceso *aux;

    while(1){
        sem_wait(&cambio_de_listas_corto);
        sem_wait(&mutex_listas);
        //calcular estimaciones
        for(int i = 0; i < list_size(lista_ready); i++){

            aux = list_get(lista_ready, i);
            if(aux->estimar)
                estimar(aux);

        }
        if(multiprocesamiento && list_size(lista_ready)){
            int index = -1;
            int estimacion_aux = 0;
            //se busca la estimacion menor
            for(int i = 0; i < list_size(lista_ready); i++){
                aux = list_get(lista_ready, i);
                if(aux->estimacion < estimacion_aux || i == 0){
                    estimacion_aux = aux->estimacion;
                    index = i;
                }
            }
            
            //Se saca de ready y se pasa a exec
            int *carpincho = list_get(lista_ready,index);
            mover_proceso_de_lista(lista_ready, lista_exec, index, EXEC);

            
        }

        sem_post(&mutex_listas);
        }


    return NULL;
}

void *planificador_corto_plazo_hrrn (void *_){
    t_proceso *aux;


    while(1){
        
        sem_wait(&cambio_de_listas_corto);
        sem_wait(&mutex_listas);
        //calcular estimaciones
        for(int i = 0; i < list_size(lista_ready); i++){

            aux = list_get(lista_ready, i);
            if(aux->estimar)
                estimar(aux);

        }

        if(multiprocesamiento && list_size(lista_ready)){
            int index = -1;
            int response_ratio = 0;

            //se busca el response ratio mas alto
            for(int i = 0; i < list_size(lista_ready); i++){

                aux = list_get(lista_ready, i);
                
                if(calcular_response_ratio(aux) > response_ratio || i == 0){
                    response_ratio = calcular_response_ratio(aux);
                    index = i;
                }
            }

            //Se saca de ready y se pasa a exec
            mover_proceso_de_lista(lista_ready, lista_exec, index, EXEC);
            

        }
        sem_post(&mutex_listas);
    }


    return NULL;
}

void estimar(t_proceso *proceso){
    float alfa = config_kernel->ALFA;
    proceso->estimacion = (alfa * proceso->ejecucion_anterior) + (( 1 - alfa) * proceso->estimacion);
    proceso->estimar = false;
    //printf("Estimo que la proxima ejecucion del proceso %d: %d - ejecutó la vez pasada %d | | | alfa %f \n", proceso->id, proceso->estimacion, proceso->ejecucion_anterior, alfa);
    return;
}

float calcular_response_ratio(t_proceso *proceso){

    int tiempo_transcurrido = ((clock() - proceso->entrada_a_ready)/ CLOCKS_PER_SEC) * 1000;

    return (tiempo_transcurrido + proceso->estimacion)/proceso->estimacion;

}


void *esperar_bloqueo(void *multiprocesamiento_p){
    int *multiprocesamiento = multiprocesamiento_p;

    while(1){

        sem_wait(&solicitar_block);
        printf("Block solicitado\n");
        sem_wait(&mutex_listas);
        printf("Block Aceptado\n");
        procesos_esperando_bloqueo--;
        bool encontrado = false;
        int tamanio_lista_exec = list_size(lista_exec);
        int index = 0;

        while(!encontrado && (index < tamanio_lista_exec)){
            t_proceso *aux = list_get(lista_exec, index);
            
                if(aux->block){
                    printf("BloqueandoX %d \n", aux->id);
                    aux->block = false;
                    mover_proceso_de_lista(lista_exec, lista_blocked, index, BLOCKED);
                    printf("Ya bloqueado %d\n", aux->id);
                    encontrado = true;
                }
            index ++;
        }
        if(encontrado){

            sem_wait(&mutex_multiprocesamiento);
            *multiprocesamiento = *multiprocesamiento + 1;
            sem_post(&mutex_multiprocesamiento);
            index = 0;

        }
        sem_post(&mutex_listas);
    }
}