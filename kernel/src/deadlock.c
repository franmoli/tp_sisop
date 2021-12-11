#include "deadlock.h"

void iniciar_deadlock() {
    pthread_t hilo_deteccion_deadlock;
    pthread_create(&hilo_deteccion_deadlock, NULL, algoritmo_deteccion, (void *)NULL);
}

void* algoritmo_deteccion(void *_) {
    while(1) {
        log_info(logger_kernel, "Esperando ejecución de algoritmo de deteccion de deadlock");
        sleep(config_kernel->TIEMPO_DEADLOCK);
        
        int index = 0;
        int index2 = 0;
        bool deadlock = false;

        while(index < list_size(lista_semaforos) && !deadlock){
            t_semaforo *sem_aux = list_get(lista_semaforos, index);
            t_list *lista_procesos_en_deadlock = list_create();

            log_info(logger_kernel, "Semaforo %s (value: %d - cantidad de solicitantes: %d): listo para analizar presencia de deadlock", sem_aux->nombre_semaforo, sem_aux->value, list_size(sem_aux->solicitantes));

            if(sem_aux->value > 0 && list_size(sem_aux->solicitantes)){
                log_info(logger_kernel, "Encuentro solicitantes en el semaforo");

                //Checkeo para todos los solicitantes de este semaforo si estan bien
                /*while(index2 < list_size(sem_aux->solicitantes) && !deadlock){
                    int *solicitante = list_get(sem_aux->solicitantes, index2);

                    //bloqueado esperando recurso, agrego como posible deadlock
                    list_add(lista_procesos_en_deadlock, solicitante);

                    //chequear si alguien esta reteniendo el recurso
                    deadlock = recurso_retenido(*solicitante, sem_aux->nombre_semaforo, lista_procesos_en_deadlock);
                    
                    index2++;
                    //Si llegue al final y ningun solicitante está en deadlock, saco al semaforo de la lista de posibles
                    if(index2 == list_size(sem_aux->solicitantes) && !deadlock){
                        list_remove(lista_procesos_en_deadlock, 0);
                    }
                }*/
                index2++;
            }

            index++;
        }
    }
}


bool recurso_retenido(int solicitante, char *semaforo_solicitado, t_list *lista_procesos_en_deadlock){

    t_semaforo *aux = NULL;
    t_recurso_asignado *asignado_aux = NULL;

    //ver si al semaforo solicitado lo esta reteniendo alguien que no sea el solicitante
    bool encontre_asignado(void *elemento){
        t_recurso_asignado *recurso = elemento;
        if(!strcmp(semaforo_solicitado, recurso->nombre_recurso) && solicitante != recurso->id_asignado)
            return true;
        
        return false;
    }

    asignado_aux = list_find(lista_recursos_asignados, encontre_asignado);

    if(asignado_aux != NULL){
        //alguien retiene este recurso

        //quien retiene este recurso está bloqueado?
        if(proceso_bloqueado(asignado_aux->id_asignado)){
            
            char *recurso_que_lo_bloquea = proceso_bloqueado_por_sem(asignado_aux->id_asignado);

            bool recurso_asignado_a_alguien_en_lista = false;
            
            int index = 0;
            int index2 = 0;
            
            
            // está bloqueado por recurso que tiene alguien de la lista ?
            while(index < list_size(lista_procesos_en_deadlock)){
                int *id_aux = list_get(lista_procesos_en_deadlock, index);

                while(index2 < list_size(lista_recursos_asignados)){
                    t_recurso_asignado *recurso_aux = list_get(lista_recursos_asignados, index2);

                    if(!strcmp(recurso_aux->nombre_recurso, recurso_que_lo_bloquea) && *id_aux == recurso_aux->id_asignado){
                        recurso_asignado_a_alguien_en_lista = true;
                        break;
                    }

                    index2++;
                }
                index++;
            }

            if(recurso_asignado_a_alguien_en_lista){
                //Esta bloqueado por alguien de la lista -  hay deadlock
                return true;

            }else{
                //Nuevo proceso en posible deadlock
                list_add(lista_procesos_en_deadlock, solicitante);

                //se repite la operacion con el siguiente proceso en posible deadlock
                return recurso_retenido(asignado_aux->id_asignado, recurso_que_lo_bloquea, lista_procesos_en_deadlock);
            }

            //Esta bloqueado por otro que no es de la lista - recursividad con el nuevo
            
            

        }else{
            //no está bloqueado - retornar false
            return false;
        }

    }else{
        return false;
    }

    
}


bool proceso_bloqueado(int id){
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
        
        //Se encontro al proceso en la lista de solicitantes de un semaforo entonces está bloqueado
        if(id_encontrado != NULL){
            return true;
        }

        index++;
    }
    return false;
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
