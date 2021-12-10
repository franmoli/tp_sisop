#include "deadlock.h"

void iniciar_deadlock() {
    pthread_t hilo_deteccion_deadlock;
    pthread_create(&hilo_deteccion_deadlock, NULL, algoritmo_deteccion, (void *)NULL);
}

void* algoritmo_deteccion(void *_) {
    while(1) {
        sleep(config_kernel->TIEMPO_DEADLOCK);
        /* Implementar algoritmo */
    }
}