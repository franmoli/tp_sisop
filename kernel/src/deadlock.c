#include "deadlock.h"

void iniciar_deadlock() {
    pthread_t hilo_deteccion_deadlock;
    pthread_create(&hilo_deteccion_deadlock, NULL, algoritmo_deteccion, (void *)NULL);
}

void* algoritmo_deteccion(void *_) {
    while(1) {
        sleep(config_kernel->TIEMPO_DEADLOCK);
        /* Implementar algoritmo */
        // 1. Revisar en lista de semaforos bloqueados los procesos con los cuales se bloquearon
        // 2. Relacionar que procesos bloqueados dependen de otros procesos bloqueados
        // 3. Finalizar el proceso que corresponda
    }
}