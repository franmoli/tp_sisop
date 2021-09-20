#include "../include/matelib.h"

//-----------------------------------Instanciacion -----------------------------------

int mate_init(mate_instance *lib_ref, char *config){

    return 1;
}

int mate_close(mate_instance *lib_ref){

    return 1;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){

    return 1;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){

    return 1;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){

    return 1;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){

    return 1;
}

//-----------------------------------Funcion Entrada/Salida-----------------------------------

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){

    return 1;
}

//-----------------------------------Funciones Modulo Memoria-----------------------------------

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){

    mate_pointer p;

    return p;
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

    return 1;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){

    return 1;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){

    return 1;
}