#ifndef MATELIB_H
#define MATELIB_H

#include <stdint.h>
#include "matelib-global.h"
#include <time.h>

#define CANT_MAX_SEM 10

t_config_matelib *config_matelib;
int socket_cliente;


//-------------------Type Definitions----------------------/

typedef char *mate_sem_name;

typedef struct mate_instance
{
    int socket;
    int32_t id;
    t_config_matelib *config;
    void *group_info;
    t_log *logger;
} mate_instance;

typedef struct mate_instance_pointer{
    void *group_info;
}mate_instance_pointer;

typedef char *mate_io_resource;

typedef int32_t mate_pointer;

enum mate_errors {
    MATE_FREE_FAULT = -5,
    MATE_READ_FAULT = -6,
    MATE_WRITE_FAULT = -7
};

// TODO: Docstrings

int32_t obtenerIDRandom();
t_config_matelib* obtenerConfig(char* config);
int iniciar_conexion_con_modulos(mate_instance *lib_ref);

//------------------General Functions---------------------/
int mate_init(mate_instance_pointer *instance_pointer, char *config);

int mate_close(mate_instance_pointer *instance_pointer);

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance_pointer *instance_pointer, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance_pointer *instance_pointer, mate_sem_name sem);

int mate_sem_post(mate_instance_pointer *instance_pointer, mate_sem_name sem);

int mate_sem_destroy(mate_instance_pointer *instance_pointer, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance_pointer *instance_pointer, mate_io_resource io, void *msg);

//--------------Memory Module Functions-------------------/

//mate_pointer mate_memalloc(mate_instance_pointer *instance_pointer, int size);

int mate_memfree(mate_instance_pointer *instance_pointer, mate_pointer addr);

int mate_memread(mate_instance_pointer *instance_pointer, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance_pointer *instance_pointer, void *origin, mate_pointer dest, int size);

//-------------------Liberacion memoria------------------/

int test(int num);


#endif