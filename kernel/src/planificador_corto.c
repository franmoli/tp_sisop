
#include "../include/planificador_corto.h"

void estimar(t_proceso *proceso);

void iniciar_planificador_corto(){

    printf("Inicio planificador CORTO \n");
    void *(*planificador)(void*);

    if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "SJF")){

        planificador = planificador_corto_plazo_sjf;

    }else if(!strcmp(config_kernel->ALGORITMO_PLANIFICACION, "HRRN")){

        planificador = planificador_corto_plazo_hrrn;

    }else{
        log_error(logger_kernel, "Planificador no soportado/no reconocido");
        return;
    }
     
    pthread_create(&hilo_planificador, NULL, planificador, (void *)NULL);
    pthread_detach(hilo_planificador);
    pthread_create(&hilo_esperar_bloqueo, NULL, esperar_bloqueo, (void *)NULL);
    pthread_detach(hilo_esperar_bloqueo);
}

void *planificador_corto_plazo_sjf (void *_){

    
    t_proceso *aux;

    while(!terminar_kernel){
        sem_wait(&cambio_de_listas_corto);
        if(!terminar_kernel){
            
            sem_wait(&mutex_listas);
            //calcular estimaciones
            for(int i = 0; i < list_size(lista_ready); i++){

                aux = list_get(lista_ready, i);
                if(aux->estimar)
                    estimar(aux);

            }
            if(multiprocesamiento && list_size(lista_ready)){
                int index = -1;
                float estimacion_aux = 0;
                printf("Elijo de entre %d\n", list_size(lista_ready));
                sleep(1);
                //se busca la estimacion menor
                for(int i = 0; i < list_size(lista_ready); i++){
                    aux = list_get(lista_ready, i);
                    printf("Probando una estimacion %f\n", aux->estimacion);
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
    }


    return NULL;
}

void *planificador_corto_plazo_hrrn (void *_){
    t_proceso *aux;


    while(!terminar_kernel){
        
        sem_wait(&cambio_de_listas_corto);
        if(!terminar_kernel){

            sem_wait(&mutex_listas);
            //calcular estimaciones
            for(int i = 0; i < list_size(lista_ready); i++){

                aux = list_get(lista_ready, i);
                if(aux->estimar)
                    estimar(aux);

            }

            if(multiprocesamiento > 0 && list_size(lista_ready)){
                int index = -1;
                int response_ratio = 0;

                printf("Elijo de entre %d multiprocesamiento %d\n", list_size(lista_ready), multiprocesamiento);
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
    }


    return NULL;
}

void estimar(t_proceso *proceso){
    float alfa = config_kernel->ALFA;
    proceso->estimacion = (alfa * proceso->ejecucion_anterior) + (( 1 - alfa) * proceso->estimacion);
    proceso->estimar = false;
    printf("Estimo que la proxima ejecucion del proceso %d: %f - ejecutó la vez pasada %d | | | alfa %f \n", proceso->id, proceso->estimacion, proceso->ejecucion_anterior, alfa);
    return;
}

float calcular_response_ratio(t_proceso *proceso){

    int tiempo_transcurrido = ((clock() - proceso->entrada_a_ready)/ CLOCKS_PER_SEC) * 1000;

    return (tiempo_transcurrido + proceso->estimacion)/proceso->estimacion;

}


void *esperar_bloqueo(void *_){

    while(!terminar_kernel){

        sem_wait(&solicitar_block);
        if(!terminar_kernel){

            sem_wait(&mutex_listas);
            procesos_esperando_bloqueo--;
            bool encontrado = false;
            int tamanio_lista_exec = list_size(lista_exec);
            int index = 0;

            while(!encontrado && (index < tamanio_lista_exec)){
                t_proceso *aux = list_get(lista_exec, index);
                
                    if(aux->block){
                        aux->block = false;
                        mover_proceso_de_lista(lista_exec, lista_blocked, index, BLOCKED);
                        encontrado = true;
                    }
                index ++;
            }
            sem_post(&mutex_listas);

        }
    }

    printf("Lo cerro al coso este\n");
    return NULL;
}