#include "matelib.h"

int main(int argc, char **argv){

//Inicializacion
    system("clear");
    logger_matelib = log_create("./cfg/matelib.log","MATELIB",true,LOG_LEVEL_DEBUG);
    log_info(logger_matelib,"Matelib inicializado correctamente");

//Cargar config 
    log_info(logger_matelib,"Cargando archivo de configuracion");
    arch_config = leer_config_file("./cfg/matelib.cfg");
    config_matelib = generar_config_matelib(arch_config);
    log_info(logger_matelib, "Configuración cargada correctamente");

//Iniciar servidor
    socket_servidor = iniciar_servidor(config_matelib->IP,string_itoa(config_matelib->PUERTO_MATELIB),logger_matelib);

//Esperar Conexiones de procesos(carpinchos)
    pthread_t hilo_cliente;
    log_info(logger_matelib,"Esperando conexiones...");
    while(1){

        socket_cliente = esperar_cliente(socket_servidor,logger_matelib);
        if(socket_cliente != -1){
        //    pthread_create(&hilo_cliente, NULL, (void*)funcionx, (void*)parametrosx);
        }
    //Terminar ejecucion del cliente cuando se desconecta
    }

//Terminar ejecucion    
    liberar_memoria();
    return 1;
}



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
/*
mate_pointer mate_memalloc(mate_instance *lib_ref, int size){

    mate_pointer p;

    return p;
}
*/
int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

    return 1;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){

    return 1;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){

    return 1;
}

void liberar_memoria(){

    config_destroy(arch_config);
    free(config_matelib);
    log_info(logger_matelib, "Finalizacion Matelib exitosa!");
    log_destroy(logger_matelib);    
}