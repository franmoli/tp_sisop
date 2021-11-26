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

typedef struct mate_inner_structure
{
    void *memory;
    int32_t cod_op;
} mate_inner_structure;

typedef struct mate_instance
{
    int socket;
    int32_t id;
    mate_inner_structure *info_carpincho;
    t_config_matelib *config;
    void *group_info;
    t_log *logger;
} mate_instance;

typedef struct mate_instance_pointer{
    void *group_info;
}mate_instance_pointer;

typedef char *mate_io_resource;

typedef int32_t mate_pointer;

// TODO: Docstrings

int32_t obtenerIDRandom();
t_config_matelib* obtenerConfig(char* config);
int iniciar_conexion_con_modulos(mate_instance *lib_ref);

//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config);

int mate_close(mate_instance *lib_ref);

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg);

//--------------Memory Module Functions-------------------/

//mate_pointer mate_memalloc(mate_instance *lib_ref, int size);

int mate_memfree(mate_instance *lib_ref, mate_pointer addr);

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size);

//-------------------Liberacion memoria------------------/

int test(int num);


#endif