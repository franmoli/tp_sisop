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
    socket_servidor = iniciar_servidor(config_matelib->IP,config_matelib->PUERTO_MATELIB,logger_matelib);

//Crear conexion con kernel
    socket_kernel = crear_conexion(config_matelib->IP,config_matelib->PUERTO_KERNEL);

//Esperar Conexiones de procesos(carpinchos)
    pthread_t hilo_cliente;
    t_paquete *paquete;
    log_info(logger_matelib,"Esperando conexiones...");
    while(1){

        socket_cliente = esperar_cliente(socket_servidor,logger_matelib);
        log_info(logger_matelib,"Se conecto un carpincho");
        paquete = recibir_paquete(socket_cliente);
        log_info(logger_matelib,"Recibi un paquete a procesar");

        if(socket_cliente != -1){
            pthread_create(&hilo_cliente, NULL, (void*)realizar_operacion, (void*) paquete);
        }
    //Terminar ejecucion del cliente cuando se desconecta
    }

//Libero conexion con kernel
    close(socket_kernel);

//Terminar ejecucion    
    liberar_memoria();
    return 1;
}

static void *realizar_operacion(t_paquete* paquete){
    t_paquete* paquete_a_enviar;
	/*  Armar switch para analizar dependiendo del codigo_operacion  */
    enviar_paquete(paquete_a_enviar,socket_kernel);
    log_info(logger_matelib,"Se envio el paquete a kernel");
    return NULL;
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

    liberar_config(arch_config);
    free(config_matelib);
    log_info(logger_matelib, "Finalizacion Matelib exitosa!");
    log_destroy(logger_matelib);    
}