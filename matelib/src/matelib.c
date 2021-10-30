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

int encontrar_lugar_libre(mate_instance *lib_ref){

    for(int i = 0;i < CANT_MAX_SEM;i++){

        if(!(lib_ref->info_carpincho->lista_sem[i].ocupado)){
            return i;
        }
    }
    return -1;
}

void inicializar_lista_sem(mate_instance *lib_ref){


}

//-----------------------------------Instanciacion -----------------------------------

int mate_init(mate_instance *lib_ref, char *config){
    char *string = malloc(sizeof(char)*50);
    int socket = -1;

    lib_ref = malloc(sizeof(mate_instance));
    config_matelib = obtenerConfig(config);
    
    lib_ref->id             = obtenerIDRandom();
    lib_ref->config         = config_matelib;
    //Quizas se saca el group_info
    lib_ref->group_info = malloc(sizeof(mate_inner_structure));
    lib_ref->info_carpincho = malloc(sizeof(mate_inner_structure));
    //hacer malloc en la estructura del semaforo minimamente, sino Seg Fault.

    sprintf(string,"./cfg/%d",lib_ref->id);
    strcat(string,".cfg");
    
    //TODO Fijarse como usar el log_level_debug para instanciarlo desde config (string to enum)
    lib_ref->logger = log_create(string,"MATELIB",0,LOG_LEVEL_DEBUG);
    log_info(lib_ref->logger,"Acabo de instanciarme");

    //Conexion con kernel y en caso que no exista, conexion con memoria
    socket = crear_conexion(config_matelib->IP_KERNEL, config_matelib->PUERTO_KERNEL);
    if(socket == -1){
        crear_conexion(config_matelib->IP_MEMORIA, config_matelib->PUERTO_MEMORIA);
    }

    // creo un paquete vacío para testear
    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = NUEVO_CARPINCHO;
    paquete->buffer = buffer;
    buffer->size = 0;

    if(socket == -1){
        log_error(lib_ref->logger,"No se pudo conectar a kernel ni a memoria");
        return 1;
    }
    
    enviar_paquete(paquete, socket);
    free(string);
    return 0;
}

int mate_close(mate_instance *lib_ref){
    free(lib_ref->group_info);
    free(lib_ref);
    return 0;
}

//-----------------------------------Semaforos-----------------------------------

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){

    int posicion = encontrar_lugar_libre(lib_ref);
    if(posicion == -1){
        log_info(lib_ref->logger,"No hay lugar disponible para inicializar el semaforo");
        return -1;
    }
    lib_ref->info_carpincho->lista_sem[posicion].nombre = malloc(sizeof(char)*20);
    lib_ref->info_carpincho->lista_sem[posicion].semaforo = malloc(sizeof(sem_t));
    strcpy(lib_ref->info_carpincho->lista_sem[posicion].nombre,sem);
    sem_init(lib_ref->info_carpincho->lista_sem[posicion].semaforo,0,value);
    lib_ref->info_carpincho->lista_sem[posicion].ocupado = true;
    log_info(lib_ref->logger,"Semaforo instanciado correctamente");

    return 0;
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

//-----------------------------------Funciones Modulo Memoria -----------------------------------
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

int test(int num){
    return num + 1;
}