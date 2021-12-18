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



int mate_init(mate_instance_pointer *instance_pointer, char *config){

    char *string = malloc(sizeof(char)*50);
    socket_cliente   = -1;

    instance_pointer->group_info = malloc(sizeof(mate_instance));
    mate_instance *lib_ref = instance_pointer->group_info;
    
    config_matelib = obtenerConfig(config);
    
    lib_ref->id             = obtenerIDRandom();
    lib_ref->config         = config_matelib;
    lib_ref->desconectado = false;

    sprintf(string,"./cfg/%d",lib_ref->id);
    strcat(string,".log");
    
    //TODO Fijarse como usar el log_level_debug para instanciarlo desde config (string to enum)
    lib_ref->logger = log_create(string,"MATELIB",true,lib_ref->config->LOG_LEVEL);
    
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
    //Creo el paquete para enviar la señal a kernel o memoria.
    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = NUEVO_CARPINCHO;
    paquete->buffer = buffer;
    buffer->size = 0;
    
    enviar_paquete(paquete, lib_ref->socket);

    //Compruebo que la operacion fue exitosa
    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    int ret;
    if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
        log_info(lib_ref->logger,"Carpincho %d: Acabo de instanciarme correctamente",lib_ref->socket);
        ret = 0;
    }else{
        log_error(lib_ref->logger,"Carpincho %d: No me instancie correctamente",lib_ref->socket);
        ret = -1;
    }
    free(paquete_recibido->buffer);
    free(paquete_recibido);
    return ret;
}

int mate_close(mate_instance_pointer *instance_pointer){
    mate_instance *lib_ref = instance_pointer->group_info;
    if(!lib_ref->desconectado){
        log_info(lib_ref->logger,"Carpincho %d: Cerrando el MATE_INSTANCE",lib_ref->socket);

        t_paquete *paquete = malloc(sizeof(t_paquete));
        t_buffer *buffer = malloc(sizeof(t_buffer));
        paquete->codigo_operacion = CLIENTE_DESCONECTADO;
        paquete->buffer = buffer;
        buffer->size = 0;
        buffer->stream = NULL;

        enviar_paquete(paquete, lib_ref->socket);
        //printf("Cierro el socket %d\n", lib_ref->socket);
        close(lib_ref->socket);
        log_info(lib_ref->logger,"Carpincho %d: Se cerro el MATE_INSTANCE",lib_ref->socket);
        free(lib_ref->logger);

    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");
    }

    return 0;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance_pointer *instance_pointer, mate_sem_name sem, unsigned int value){
    
    
    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar SEMINIT",lib_ref->socket);
    if(!lib_ref->desconectado){
        t_paquete *paquete = serializar(INIT_SEM, 4, U_INT, value, CHAR_PTR, sem);
        enviar_paquete(paquete, lib_ref->socket);
        sleep(1);

        //----esperar señal de inicializacion correcta
        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
        
        if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
            log_info(lib_ref->logger,"Carpincho %d: La funcion SEM_INIT se ejecuto exitosamente",lib_ref->socket);
            ret = 0;
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion SEM_INIT no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = -1;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion sem init (Servidor desconectado)");     
        ret = -1;   
    }
    return ret;
}

int mate_sem_wait(mate_instance_pointer *instance_pointer, mate_sem_name sem){

    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar SEMWAIT",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(SEM_WAIT,2,CHAR_PTR,sem);
        enviar_paquete(paquete,lib_ref->socket);

        //----esperar señal de inicializacion correcta
        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
        
        if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
            log_info(lib_ref->logger,"Carpincho %d: La funcion SEM_WAIT se ejecuto exitosamente",lib_ref->socket);
            ret = 0;
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion SEM_WAIT no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = -1;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion sem wait (Servidor desconectado)");     
        ret = -1;   
    }
    return ret;
}

int mate_sem_post(mate_instance_pointer *instance_pointer, mate_sem_name sem){

    mate_instance *lib_ref = instance_pointer->group_info;  
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar SEMPOST",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(SEM_POST,2,CHAR_PTR,sem);    
        enviar_paquete(paquete,lib_ref->socket);

        //Esperar señal de inicializacion correcta
        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
        
        if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
            log_info(lib_ref->logger,"Carpincho %d: La funcion SEM_POST se ejecuto exitosamente",lib_ref->socket);
            ret = 0;
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion SEM_POST no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = -1;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion sem post (Servidor desconectado)");  
        ret = -1;      
    }
    return ret;

}

int mate_sem_destroy(mate_instance_pointer *instance_pointer, mate_sem_name sem){

    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar SEMDESTROY",lib_ref->socket);
    if(!lib_ref->desconectado){
        t_paquete *paquete = serializar(SEM_DESTROY,2,CHAR_PTR,sem); 
        enviar_paquete(paquete,lib_ref->socket);

        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
        if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
            log_info(lib_ref->logger,"Carpincho %d: La funcion SEM_DESTROY se ejecuto exitosamente", lib_ref->socket);
            ret = 0;
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion SEM_DESTROY no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = -1;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");
        ret = -1;        
    }
    return ret;
}

//-----------------------------------Funcion Entrada/Salida-----------------------------------

int mate_call_io(mate_instance_pointer *instance_pointer, mate_io_resource io, void *msg){

    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar CALLIO",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(CALLIO,4,CHAR_PTR,io,CHAR_PTR,msg);
        enviar_paquete(paquete,lib_ref->socket);

        log_info(lib_ref->logger,"Carpincho %d: Llamo a la io %s",lib_ref->socket,io);

        //Esperar señal de ejecucion
        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);

        if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
            log_info(lib_ref->logger,"Carpincho %d: La funcion CALL_IO se ejecuto exitosamente",lib_ref->socket);
            ret = 0;
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion CALL_IO no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = -1;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");
        ret = -1;
    }
    return ret;
}

//-----------------------------------Funciones Modulo Memoria -----------------------------------

mate_pointer mate_memalloc(mate_instance_pointer *instance_pointer, int size){

    mate_pointer p = 0;
    mate_instance *lib_ref = instance_pointer->group_info;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar MEMALLOC",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(MEMALLOC,4,INT,lib_ref->socket,INT,size);
        enviar_paquete(paquete,lib_ref->socket);

        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
        if(paquete_recibido->codigo_operacion == MEMALLOC){
            deserializar(paquete_recibido,2,INT,&p);
            log_info(lib_ref->logger,"Carpincho %d: La funcion MEM_ALLOC se ejecuto correctamente",lib_ref->socket);
            free(paquete_recibido->buffer->stream);        
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion MEM_ALLOC no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");       
    }
    return p;
}

int mate_memfree(mate_instance_pointer *instance_pointer, mate_pointer addr){

    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar MEMFREE",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(MEMFREE,4,INT,lib_ref->socket,U_INT,addr);
        enviar_paquete(paquete,lib_ref->socket);

        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
        if(paquete_recibido->codigo_operacion == MEMFREE){
            log_info(lib_ref->logger,"Carpincho %d: La funcion MEM_FREE se ejecuto correctamente",lib_ref->socket);
            ret = 0;    
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion MEM_FREE no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = MATE_FREE_FAULT;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");
        ret = MATE_FREE_FAULT;        
    }
    return ret;
}   

int mate_memread(mate_instance_pointer *instance_pointer, mate_pointer origin, void *dest, int size){

    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar MEMREAD",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(MEMREAD,8,INT,lib_ref->socket,INT,origin,CHAR_PTR,dest,INT,size);
        enviar_paquete(paquete,lib_ref->socket);

        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
        if(paquete_recibido->codigo_operacion == MEMREAD){
            log_info(lib_ref->logger,"Carpincho %d: La funcion MEM_READ se ejecuto correctamente",lib_ref->socket);
            ret = 0;    
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion MEM_READ no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = MATE_READ_FAULT;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");
        ret = MATE_READ_FAULT;        
    }
    return ret;
}

int mate_memwrite(mate_instance_pointer *instance_pointer, void *origin, mate_pointer dest, int size){

    mate_instance *lib_ref = instance_pointer->group_info;
    int ret;
    log_info(lib_ref->logger,"Carpincho %d: Quiero ejecutar MEMWRITE",lib_ref->socket);
    if(!lib_ref->desconectado){

        t_paquete *paquete = serializar(MEMWRITE,8,INT,lib_ref->socket,CHAR_PTR,origin,INT,dest,INT,size);
        enviar_paquete(paquete,lib_ref->socket);

        t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
        if(paquete_recibido->codigo_operacion == MEMWRITE){
            log_info(lib_ref->logger,"Carpincho %d: La funcion MEM_WRITE se ejecuto correctamente",lib_ref->socket);
            ret = 0;    
        }else{
            log_error(lib_ref->logger,"Carpincho %d: La funcion MEM_WRITE no se ejecuto correctamente",lib_ref->socket);
            lib_ref->desconectado = true;
            ret = MATE_WRITE_FAULT;
        }
        free(paquete_recibido->buffer);
        free(paquete_recibido);
    }else{
        log_error(lib_ref->logger, "No se pudo ejecutar la operacion mate close (Servidor desconectado)");
        ret = MATE_WRITE_FAULT;        
    }
    return ret;
}