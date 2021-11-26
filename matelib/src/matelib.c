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
    lib_ref->logger = log_create(string,"MATELIB",true,LOG_LEVEL_DEBUG);
    
    free(string);

    //Conexion con kernel y en caso que no exista, conexion con memoria
    socket_cliente = crear_conexion(config_matelib->IP_KERNEL, config_matelib->PUERTO_KERNEL);
    log_info(lib_ref->logger,"Conectando a kernel ip - %s | puerto - %s  ...", config_matelib->IP_KERNEL, config_matelib->PUERTO_KERNEL);
    if(socket_cliente == -1){
        socket_cliente = crear_conexion(config_matelib->IP_MEMORIA, config_matelib->PUERTO_MEMORIA);
        log_info(lib_ref->logger,"No pudo conectarse a kernel. Conectando a memoria...");
    }
    if(socket_cliente == -1){
        log_error(lib_ref->logger,"No se pudo conectar a ningun modulo");
        exit(EXIT_FAILURE);
    }
    lib_ref->socket = socket_cliente;
    printf("Socket cliente%d\n", socket_cliente);
    //Creo el paquete para enviar la señal a kernel o memoria.
    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = NUEVO_CARPINCHO;
    paquete->buffer = buffer;
    buffer->size = 0;
    
    enviar_paquete(paquete, lib_ref->socket);
    printf("init %d\n", lib_ref->socket);
    free(paquete);

    //Compruebo que la operacion fue exitosa
    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);

    if(paquete_recibido->codigo_operacion == NUEVO_CARPINCHO){
        log_info(lib_ref->logger,"Acabo de instanciarme correctamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"No me instancie correctamente");
        return 1;
    }
}

int mate_close(mate_instance *lib_ref){

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = CLIENTE_DESCONECTADO;
    paquete->buffer = buffer;
    buffer->size = 0;

    enviar_paquete(paquete, socket_cliente);
    
    close(lib_ref->socket);
    
    //free(lib_ref->group_info);
    //printf("Group info freed\n");
    //free(lib_ref->info_carpincho);
    //free(lib_ref);    
    
    return 0;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    
    //----Crear paquete con nombre de semaforo y valor para que kernel haga el sem_init con el COD_OP correspondiente
    //serializar inputs (nombre y valor init)
    printf("Socket %d\n", lib_ref->socket);
    t_paquete *paquete = malloc(sizeof(t_paquete)); 
    t_buffer *buffer = malloc(sizeof(t_buffer));

    unsigned long stream_size = sizeof(uint32_t) + sizeof(char)*strlen(sem) +1;
    paquete->buffer = buffer;
    buffer->stream = malloc(stream_size);
    buffer->size = stream_size;
    paquete->codigo_operacion = INIT_SEM;
    serializar_mate_sem_init(buffer->stream, value, sem);
    enviar_paquete(paquete, lib_ref->socket);
    sleep(1);

    free(paquete);
    //----esperar señal de inicializacion correcta
    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    if(paquete_recibido->codigo_operacion == INIT_SEM){
        log_info(lib_ref->logger,"La funcion SEM_INIT se ejecuto exitosamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"La funcion SEM_INIT no se ejecuto correctamente");
        return 1;
    }
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    
    t_paquete *paquete = malloc(sizeof(t_paquete));
    
    //----Crear paquete con nombre de semaforo y valor para que kernel haga el sem_wait con el COD_OP correspondiente
    
    paquete->codigo_operacion = SEM_WAIT;
    //serializar inputs (nombre y valor init)
    
    //agregar buffer al paquete
    enviar_paquete(paquete,lib_ref->socket);
    free(paquete); 

    //----esperar señal de inicializacion correcta
    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    
    if(paquete_recibido->codigo_operacion == SEM_WAIT){
        log_info(lib_ref->logger,"La funcion SEM_WAIT se ejecuto exitosamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"La funcion SEM_WAIT no se ejecuto correctamente");
        return 1;
    }
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){

     t_paquete *paquete = malloc(sizeof(t_paquete));
    
    //----Crear paquete con nombre de semaforo y valor para que kernel haga el sem_post con el COD_OP correspondiente
    
    paquete->codigo_operacion = SEM_POST;
    //serializar inputs (nombre y valor init)
    
    //agregar buffer al paquete
    
    enviar_paquete(paquete,lib_ref->socket);
    free(paquete); 

    //Esperar señal de inicializacion correcta
    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    
    if(paquete_recibido->codigo_operacion == SEM_POST){
        log_info(lib_ref->logger,"La funcion SEM_POST se ejecuto exitosamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"La funcion SEM_POST se ejecuto correctamente");
        return 1;
    }
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = SEM_DESTROY;
    buffer->size = 0;
    paquete->buffer = buffer;

    enviar_paquete(paquete,lib_ref->socket);
    free(paquete);
    free(buffer);

    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    if(paquete_recibido->codigo_operacion == SEM_DESTROY){
        log_info(lib_ref->logger,"La funcion SEM_DESTROY se ejecuto exitosamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"La funcion SEM_DESTROY se ejecuto correctamente");
        return 1;
    }
}

//-----------------------------------Funcion Entrada/Salida-----------------------------------

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){
    

    return 1;
}

//-----------------------------------Funciones Modulo Memoria -----------------------------------

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){

    mate_pointer p = 0;

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

int test(int num){
    return num + 1;
}