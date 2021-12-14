#include "deadlock.h"

void iniciar_deadlock() {
    pthread_t hilo_deteccion_deadlock;
    pthread_create(&hilo_deteccion_deadlock, NULL, algoritmo_deteccion, (void *)NULL);
}

void* algoritmo_deteccion(void *_) {
    while(1) {
        sleep(config_kernel->TIEMPO_DEADLOCK/1000);
        log_info(logger_kernel, "Ejecutando algoritmo detecion de deadlock");
        
        int index = 0;
        int index2 = 0;
        int index3 = 0;
        bool deadlock = false;
        t_list *lista_recursos_en_deadlock = NULL;
        t_list *lista_de_procesos_en_deadlock = NULL;

        while(index < list_size(lista_semaforos) && !deadlock){
            t_semaforo *sem_aux = list_get(lista_semaforos, index);
            lista_recursos_en_deadlock = list_create();
            lista_de_procesos_en_deadlock = list_create();

            //log_info(logger_kernel, "Semaforo %s (value: %d - cantidad de solicitantes: %d - cantidad de r en dl: %d - cant de proc en dl: %d |: listo para analizar presencia de deadlock ", sem_aux->nombre_semaforo, sem_aux->value, list_size(sem_aux->solicitantes), list_size(lista_recursos_en_deadlock), list_size(lista_de_procesos_en_deadlock));

            if(sem_aux->value < 0 && list_size(sem_aux->solicitantes)){
                //log_info(logger_kernel, "Encuentro %d solicitantes en el semaforo", list_size(sem_aux->solicitantes));
                
                //Checkeo para todos los solicitantes de este semaforo si estan bien
                while(index2 < list_size(sem_aux->solicitantes) && !deadlock){
                    // -------- 1 ----------
                    int *solicitante = list_get(sem_aux->solicitantes, index2);
                    list_add(lista_de_procesos_en_deadlock, solicitante);
                    //printf("Listo para ckeckear al solicitante %d\n", *solicitante);

                    deadlock = proceso_en_deadlock(*solicitante, lista_recursos_en_deadlock, sem_aux->nombre_semaforo, lista_de_procesos_en_deadlock);
                    if(deadlock){
                        log_info(logger_kernel, "Deadlock encontrado");
                        print_lista_procesos_en_dl(lista_de_procesos_en_deadlock);
                        liberar_recursos_en_deadlock(lista_de_procesos_en_deadlock);
                    }

                    index2++;
                }
                index2 = 0;
            }
            index++;
        }
        lista_recursos_en_deadlock = NULL;
        lista_de_procesos_en_deadlock = NULL;
        deadlock = false;
    }
}





char *proceso_bloqueado_por_sem(int id){
    int index = 0;
    t_semaforo *sem_aux = NULL;
    int *id_encontrado = NULL;

    while(index < list_size(lista_semaforos)){

        sem_aux = list_get(lista_semaforos, index);

        bool encontre_el_solicitante(void *elemento){
            int *solicitante = elemento;
            if(*solicitante == id)
                return true;
            return false;
        }

        id_encontrado = list_find(sem_aux->solicitantes, encontre_el_solicitante);
        
        //Devuelvo el nombre del semáforo que bloqueó al id
        if(id_encontrado != NULL){
            return sem_aux->nombre_semaforo;
        }

        index++;
    }
    return NULL;
}

void agregar_recursos_a_lista(int id, t_list *lista){

    bool encontrar_asignado_a_id(void *elemento){
        t_recurso_asignado *recurso = elemento;
        if(recurso->id_asignado == id)
            return true;
        
        return false;
    };

    void *obtener_solamente_recurso(void *elemento){
        t_recurso_asignado *recurso = elemento;
        return recurso->nombre_recurso;
    };

    t_list *aux = list_filter(lista_recursos_asignados, encontrar_asignado_a_id);

    t_list *aux2 = list_map(aux, obtener_solamente_recurso);

    list_add_all(lista, aux2);

    return;
}

void print_lista_recursos_en_dl(t_list *lista){
    
    int index = 0;
    char *aux = NULL;
    while(index < list_size(lista)){
        aux = list_get(lista, index);
        printf("Recurso en posible dl - %s \n", aux);
        index++;
    }
}

void print_lista_procesos_en_dl(t_list *lista){
    
    int index = 0;
    int *aux = NULL;
    while(index < list_size(lista)){
        aux = list_get(lista, index);
        printf("Proceso en deadlock - %d \n", *aux);
        index++;
    }
}

t_list* quien_retiene_recurso(char *recurso, int id_solicitante){

    bool quien_lo_tiene(void *elemento){
        t_recurso_asignado *recurso_a_comparar = elemento;
        if(!strcmp(recurso_a_comparar->nombre_recurso, recurso) && recurso_a_comparar->id_asignado != id_solicitante)
            return true;
        
        return false;
    };

    t_list *aux = list_filter(lista_recursos_asignados, quien_lo_tiene);

    
    return aux;
}

bool proceso_en_deadlock(int solicitante, t_list *lista_recursos_en_deadlock, char *nombre_semaforo, t_list *lista_de_procesos_en_deadlock){
    int index3 = 0;
    //printf("Procesos en deadlock %d | recursos en deadlock %d---------\n", list_size(lista_de_procesos_en_deadlock), list_size(lista_recursos_en_deadlock));
    agregar_recursos_a_lista(solicitante, lista_recursos_en_deadlock);
    //+++++++++++ 1 +++++++++++

    //print_lista_recursos_en_dl(lista_recursos_en_deadlock);

    //----------- 2 -----------
    t_list *gente_reteniendo_mi_recurso = quien_retiene_recurso(nombre_semaforo, solicitante);
    //+++++++++++ 2 +++++++++++
    
    while(index3 < list_size(gente_reteniendo_mi_recurso)){
        //-------------- 3 -------------
        t_recurso_asignado *reteniendo_mi_recurso = list_get(gente_reteniendo_mi_recurso, index3);
        //++++++++++++++ 3 +++++++++++++

        //-------------- 4 -------------
        char *sem_que_lo_bloquea = NULL;

        sem_que_lo_bloquea = proceso_bloqueado_por_sem(reteniendo_mi_recurso->id_asignado);
        //++++++++++++++ 4 +++++++++++++

        //printf("El proceso %d que retiene mi recurso esta bloqueado por el semaforo => %s\n", reteniendo_mi_recurso->id_asignado, sem_que_lo_bloquea);

        if(sem_que_lo_bloquea != NULL){

            //-------------- 5 -------------
            char *aux_encontrado_en_lista = NULL;

            bool sem_esta_en_lista (void *elemento){
                char *sem_en_lista = elemento;
                if(!strcmp(sem_en_lista, sem_que_lo_bloquea))
                    return true;
                
                return false;
            };

            aux_encontrado_en_lista = list_find(lista_recursos_en_deadlock,sem_esta_en_lista);
            //++++++++++++++ 5 +++++++++++++

            int *aux_id_en_dl = malloc(sizeof(int));
            *aux_id_en_dl = reteniendo_mi_recurso->id_asignado;
            list_add(lista_de_procesos_en_deadlock, aux_id_en_dl);

            if(aux_encontrado_en_lista != NULL){
                //-------------- 6 -------------
                return true;
                //++++++++++++++ 6 +++++++++++++
            }else{
                //-------------- 6 -------------
                //printf("El semaforo que bloquea al que ocupo mi recurso no esta en la lista\n");
                return proceso_en_deadlock(reteniendo_mi_recurso->id_asignado, lista_recursos_en_deadlock, sem_que_lo_bloquea, lista_de_procesos_en_deadlock);
                //++++++++++++++ 6 +++++++++++++
            }
        }else{
            //-------------- 5 -------------
            return false;
            //++++++++++++++ 5 +++++++++++++
        }

        index3++;
    }

    //----------- 3 -----------
    return false;
    //+++++++++++ 3 +++++++++++
}

void liberar_recursos_en_deadlock(t_list *lista_de_recursos_en_deadlock){
    int index = 0;
    int *id_aux = NULL;
    int aux = 0;
    t_proceso *proceso_a_eliminar = NULL;

    //traer el id mayor en aux
    while(index < list_size(lista_de_recursos_en_deadlock)){
        id_aux = list_get(lista_de_recursos_en_deadlock, index);
        if(*id_aux > aux){
            aux = *id_aux;
        }
        index++;
    }

    bool proceso_encontrado(void *elemento){
        t_proceso *proceso_encontrado = elemento;
        if(proceso_encontrado->id == aux)
            return true;
        
        return false;
    }
    
    proceso_a_eliminar = list_find(lista_blocked, proceso_encontrado);

    if(proceso_a_eliminar == NULL){
        proceso_a_eliminar = list_find(lista_s_blocked, proceso_encontrado);
    }
    if(proceso_a_eliminar == NULL){
        print_lists();
        log_error(logger_kernel, "Error en la recuperacion de deadlock, no se encontró a %d bloqueado", aux);
        exit(EXIT_FAILURE);
    }
    proceso_a_eliminar->salida_exit = true;
    sem_post(&salida_a_exit);

    return;
}