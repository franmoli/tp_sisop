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
    
    lib_ref->info_carpincho = malloc(sizeof(mate_inner_structure));

    sprintf(string,"./cfg/%d",lib_ref->id);
    strcat(string,".cfg");
    
    //TODO Fijarse como usar el log_level_debug para instanciarlo desde config (string to enum)
    lib_ref->logger = log_create(string,"MATELIB",true,LOG_LEVEL_DEBUG);
    
    free(string);


    //test borrar
    /*t_paquete *test_p = serializar(1, 4, INT, 3, CHAR_PTR, "PRUEBA");
    int test_i = 0;
    char *test_c = NULL;
    deserializar(test_p, 4, INT, &test_i, CHAR_PTR, &test_c);
    printf("\n\n Cosas deserializadas int %d, str %s\n\n", test_i, test_c);
    */

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

    free(buffer);

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

int mate_close(mate_instance_pointer *instance_pointer){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = CLIENTE_DESCONECTADO;
    paquete->buffer = buffer;
    buffer->size = 0;

    enviar_paquete(paquete, socket_cliente);
    free(buffer);
    close(lib_ref->socket);
    
    //free(lib_ref->group_info);
    //printf("Group info freed\n");
    //free(lib_ref->info_carpincho);
    //free(lib_ref);    
    
    return 0;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance_pointer *instance_pointer, mate_sem_name sem, unsigned int value){
    
    
    mate_instance *lib_ref = instance_pointer->group_info;
    
    t_paquete *paquete = serializar(INIT_SEM, 4, U_INT, value, CHAR_PTR, sem);
    enviar_paquete(paquete, lib_ref->socket);
    sleep(1);

    //----esperar señal de inicializacion correcta
    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    
    if(paquete_recibido->codigo_operacion == OP_CONFIRMADA){
        log_info(lib_ref->logger,"La funcion SEM_INIT se ejecuto exitosamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"La funcion SEM_INIT no se ejecuto correctamente");
        return 1;
    }
}

int mate_sem_wait(mate_instance_pointer *instance_pointer, mate_sem_name sem){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(SEM_WAIT,2,CHAR_PTR,sem);
    enviar_paquete(paquete,lib_ref->socket);

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

int mate_sem_post(mate_instance_pointer *instance_pointer, mate_sem_name sem){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(SEM_POST,2,CHAR_PTR,sem);    
    enviar_paquete(paquete,lib_ref->socket);

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

int mate_sem_destroy(mate_instance_pointer *instance_pointer, mate_sem_name sem){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(SEM_DESTROY,2,CHAR_PTR,sem); 
    enviar_paquete(paquete,lib_ref->socket);

    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);
    if(paquete_recibido->codigo_operacion == SEM_DESTROY){
        log_info(lib_ref->logger,"La funcion SEM_DESTROY se ejecuto exitosamente");
        return 0;
    }else{
        log_error(lib_ref->logger,"La funcion SEM_DESTROY no se ejecuto correctamente");
        return 1;
    }
}

//-----------------------------------Funcion Entrada/Salida-----------------------------------

int mate_call_io(mate_instance_pointer *instance_pointer, mate_io_resource io, void *msg){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(CALLIO,4,CHAR_PTR,io,CHAR_PTR,msg);
    enviar_paquete(paquete,lib_ref->socket);

    log_info(lib_ref->logger,"Se llamo a la funcion mate_call_io");

//  HACER VERIFICACION CON EL RECIBIR PAQUETE

    return 0;
}

//-----------------------------------Funciones Modulo Memoria -----------------------------------

mate_pointer mate_memalloc(mate_instance_pointer *instance_pointer, int size){

    mate_pointer p = -1;
    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(MEMALLOC,2,INT,size);
    enviar_paquete(paquete,lib_ref->socket);

    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
    if(paquete_recibido->codigo_operacion == MEMALLOC){
        deserializar(paquete_recibido,2,INT,&p);
        log_info(lib_ref->logger,"La funcion MEM_ALLOC se ejecuto correctamente");
    
    }else{
        log_error(lib_ref->logger,"La funcion MEM_ALLOC no se ejecuto correctamente");
    }
    return p;
}

int mate_memfree(mate_instance_pointer *instance_pointer, mate_pointer addr){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(MEMFREE,2,INT,addr);
    enviar_paquete(paquete,lib_ref->socket);

    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
    if(paquete_recibido->codigo_operacion == MEMFREE){
        log_info(lib_ref->logger,"La funcion MEM_FREE se ejecuto correctamente");
        return 0;    
    }else{
        log_error(lib_ref->logger,"La funcion MEM_FREE no se ejecuto correctamente");
        return -1;
    }
}   

int mate_memread(mate_instance_pointer *instance_pointer, mate_pointer origin, void *dest, int size){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(MEMREAD,6,INT,origin,CHAR_PTR,dest,INT,size);
    enviar_paquete(paquete,lib_ref->socket);

    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
    if(paquete_recibido->codigo_operacion == MEMREAD){
        log_info(lib_ref->logger,"La funcion MEM_READ se ejecuto correctamente");
        return 0;    
    }else{
        log_error(lib_ref->logger,"La funcion MEM_READ no se ejecuto correctamente");
        return -1;
    }
}

int mate_memwrite(mate_instance_pointer *instance_pointer, void *origin, mate_pointer dest, int size){

    mate_instance *lib_ref = instance_pointer->group_info;

    t_paquete *paquete = serializar(MEMREAD,6,CHAR_PTR,origin,INT,dest,INT,size);
    enviar_paquete(paquete,lib_ref->socket);

    t_paquete *paquete_recibido = recibir_paquete(lib_ref->socket);    
    if(paquete_recibido->codigo_operacion == MEMWRITE){
        log_info(lib_ref->logger,"La funcion MEM_WRITE se ejecuto correctamente");
        return 0;    
    }else{
        log_error(lib_ref->logger,"La funcion MEM_WRITE no se ejecuto correctamente");
        return -1;
    }
}