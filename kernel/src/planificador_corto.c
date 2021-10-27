
#include "../include/planificador_corto.h"

void estimar(t_proceso *proceso);

void iniciar_planificador_corto(){

    printf("Inicio planificador CORTO \n");
    void *(*planificador)(void*);
    pthread_t hilo_planificador;

    if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "SJF")){

        planificador = planificador_corto_plazo_sjf;

    }else if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "HRRN")){

        planificador = planificador_corto_plazo_hrrn;

    }else{

        log_error(logger_kernel, "Planificador no soportado/no reconocido");
        return;
    }
     
    pthread_create(&hilo_planificador, NULL, planificador, (void *)NULL);
}

void *planificador_corto_plazo_sjf (void *_){

    int multiprocesamiento = config_kernel->GRADO_MULTIPROCESAMIENTO;
    t_proceso *aux;

    while(1){

        //calcular estimaciones
        for(int i = 0; i < list_size(lista_ready); i++){

            aux = list_get(lista_ready, i);
            if(aux->estimar)
                estimar(aux);

        }
        
        if(multiprocesamiento && list_size(lista_ready)){

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

            multiprocesamiento--;

        }else if(!multiprocesamiento){
            printf("Procesos en EXEC:\n");
            for(int i = 0; i< list_size(lista_exec); i++){
                t_proceso *aux = list_get(lista_exec, i);
                printf("P: %d | estimacion: %d\n", aux->id, aux->estimacion);
            }
            sem_wait(&salida_exec);
            multiprocesamiento++;
        }
    }


    return NULL;
}

void *planificador_corto_plazo_hrrn (void *_){
    
    return NULL;
}

void estimar(t_proceso *proceso){
    int alfa = config_kernel->ALFA;
    proceso->estimacion = (alfa * proceso->ejecucion_anterior) + (( 1 - alfa) * proceso->estimacion);
    proceso->estimar = false;
    return;
}