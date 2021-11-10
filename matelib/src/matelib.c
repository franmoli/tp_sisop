#include "matelib.h"

int32_t obtenerIDRandom(){
    srand(0);
    return rand();
}

t_config_matelib* obtenerConfig(char* config){
    t_config *cfg;
    t_config_matelib* config_mate;
    cfg = leer_config_file(config);
    config_mate = generar_config_matelib(cfg);
    return config_mate;
}

int iniciar_conexion_con_modulos(mate_instance *lib_ref){
    socket_cliente = -1;
    socket_cliente = crear_conexion(lib_ref->config->IP_KERNEL,lib_ref->config->PUERTO_KERNEL);
    if(socket_cliente == -1){
        log_info(lib_ref->logger,"No se pudo conectar a kernel, probando conexion a memoria");
        socket_cliente = crear_conexion(lib_ref->config->IP_MEMORIA,lib_ref->config->PUERTO_MEMORIA);
    }
    if(socket_cliente == -1){
        log_error(lib_ref->logger,"No se pudo conectar a kernel ni a memoria");
    }
    return socket_cliente;
}

//-----------------------------------Instanciacion -----------------------------------

int mate_init(mate_instance *lib_ref, char *config){
    char *string = malloc(sizeof(char)*50);
    socket_cliente   = -1;

    lib_ref = malloc(sizeof(mate_instance));
    config_matelib = obtenerConfig(config);
    
    lib_ref->id             = obtenerIDRandom();
    lib_ref->config         = config_matelib;
    //Quizas se saca el group_info
    lib_ref->group_info = malloc(sizeof(mate_inner_structure));
    lib_ref->info_carpincho = malloc(sizeof(mate_inner_structure));

    sprintf(string,"./cfg/%d",lib_ref->id);
    strcat(string,".cfg");
    
    //TODO Fijarse como usar el log_level_debug para instanciarlo desde config (string to enum)
    lib_ref->logger = log_create(string,"MATELIB",0,LOG_LEVEL_DEBUG);
    log_info(lib_ref->logger,"Acabo de instanciarme");

    //Conexion con kernel y en caso que no exista, conexion con memoria
    socket_cliente = crear_conexion(config_matelib->IP_KERNEL, config_matelib->PUERTO_KERNEL);
    if(socket_cliente == -1){
        crear_conexion(config_matelib->IP_MEMORIA, config_matelib->PUERTO_MEMORIA);
    }

    // creo un paquete vacío para testear
    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = NUEVO_CARPINCHO;
    paquete->buffer = buffer;
    buffer->size = 0;

    if(socket_cliente == -1){
        log_error(lib_ref->logger,"No se pudo conectar a kernel ni a memoria");
        return 1;
    }
    
    enviar_paquete(paquete, socket_cliente);
    printf("init %d", socket_cliente);
    
    free(string);
    return 0;
}

int mate_close(mate_instance *lib_ref){
    free(lib_ref->group_info);
    free(lib_ref->info_carpincho);
    free(lib_ref);
    return 0;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    
    

    
    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    
    //----Crear paquete con nombre de semaforo y valor para que kernel haga el sem_init con el COD_OP correspondiente
    paquete->codigo_operacion = INIT_SEM;
    //serializar inputs (nombre y valor init)
    paquete->buffer = buffer;
    buffer->size = 0;
    enviar_paquete(paquete,socket_cliente);
    printf("Enviado %d\n", socket_cliente);
    //----esperar señal de inicializacion correcta
    free(paquete);    

    return 0;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    
    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    t_paquete *paquete = malloc(sizeof(t_paquete));
    
    //----Crear paquete con nombre de semaforo y valor para que kernel haga el sem_init con el COD_OP correspondiente
    
    paquete->codigo_operacion = SEM_WAIT;
    //serializar inputs (nombre y valor init)
    //agregar buffer al paquete
    enviar_paquete(paquete,socket_cliente);

    //----esperar señal de inicializacion correcta
    free(paquete); 

    return 1;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){

    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    return 1;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){

    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    return 1;
}

//-----------------------------------Funcion Entrada/Salida-----------------------------------

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){

    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    return 1;
}

//-----------------------------------Funciones Modulo Memoria -----------------------------------
/*
mate_pointer mate_memalloc(mate_instance *lib_ref, int size){

    int socket = iniciar_conexion_con_modulos(lib_ref);
    if (socket == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    mate_pointer p;

    return p;
}
*/
int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    return 1;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){

    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    return 1;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){

    socket_cliente = iniciar_conexion_con_modulos(lib_ref);
    if (socket_cliente == -1){
        log_error(lib_ref->logger,"No pudo establecerse una conexion con los otros modulos");
        return 1;
    }

    return 1;
}

int test(int num){
    return num + 1;
}