#include "../include/kernel.h"

int main(int argc, char **argv) {
    //Inicio del programa
    system("clear");
    logger_kernel = log_create("./cfg/kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    log_info(logger_kernel, "Programa inicializado correctamente ");


    //Se carga la configuración
    log_info(logger_kernel, "Iniciando carga del archivo de configuración");
    config_file = leer_config_file("./cfg/kernel.cfg");
    config_kernel = generar_config_kernel(config_file);
    log_info(logger_kernel, "Configuración cargada correctamente");

    //Iniciar semaforos de uso general
    iniciar_semaforos_generales();

    //Iniciar listas de procesos
    iniciar_listas();
    
    //Iniciar planificador de largo plazo
    iniciar_planificador_largo();
    //Iniciar planificador de corto plazo
    iniciar_planificador_corto();
    //Iniciar planificador de mediano plazo
    iniciar_planificador_mediano();
    
    
    //Conectar a memoria (datos temporales hardcodeados)
    int socket_cliente_memoria = crear_conexion(config_kernel->IP_MEMORIA, config_kernel->PUERTO_MEMORIA);
    if(socket_cliente_memoria == -1){
        log_error(logger_kernel, "Fallo en la conexion a memoria");
    }
    while(1);
    //Fin del programa
    liberar_memoria_y_finalizar(config_kernel, logger_kernel, config_file);
    return 0;
}

void print_inicializacion (t_config_kernel *config_kernel){
    
    // se printea la informacion cargada
    printf("IP MEMORIA: %s\n", config_kernel->IP_MEMORIA);
    printf("PUERTO_MEMORIA: %s\n", config_kernel->PUERTO_MEMORIA);
    printf("ALGORITMO_PLANIFICACION: %s\n", config_kernel->ALGORITMO_PLANIFICACION);
    printf("RETARDO_CPU: %d\n", config_kernel->RETARDO_CPU);
    printf("GRADO_MULTIPROGRAMACION: %d\n", config_kernel->GRADO_MULTIPROGRAMACION);
    printf("GRADO_MULTIPROCESAMIENTO: %d\n", config_kernel->GRADO_MULTIPROCESAMIENTO);
    printf("ESTIMACION_INICIAL: %d\n", config_kernel->ESTIMACION_INICIAL);
    printf("ALFA: %d\n", config_kernel->ALFA);
    printf("DISPOSITIVOS IO:\n");

    t_list_iterator *io_devices_iterator = list_iterator_create(config_kernel->DISPOSITIVOS_IO);
    while(list_iterator_has_next(io_devices_iterator)) {
        printf("- %s\n", (char*)list_iterator_next(io_devices_iterator));
    }

    list_iterator_destroy(io_devices_iterator);
    printf("DURACIONES IO:\n");
    char* duraciones_itoa = string_itoa(list_size(config_kernel->DURACIONES_IO));
    log_info(logger_kernel, duraciones_itoa);
    t_list_iterator *io_durations_iterator = list_iterator_create(config_kernel->DURACIONES_IO);
    while(list_iterator_has_next(io_durations_iterator)) {
        printf("- %s\n", (char*) list_iterator_next(io_durations_iterator));
    }
    list_iterator_destroy(io_durations_iterator);
    free(duraciones_itoa);
}

void liberar_memoria_y_finalizar(t_config_kernel *config_kernel, t_log *logger_kernel, t_config *config_file){
    config_destroy(config_file);
    list_destroy_and_destroy_elements(config_kernel->DISPOSITIVOS_IO, element_destroyer);
    list_destroy_and_destroy_elements(config_kernel->DURACIONES_IO, element_destroyer);
    free(config_kernel);
    log_info(logger_kernel, "Programa finalizado con éxito");
    log_destroy(logger_kernel);
}

void element_destroyer(void* elemento){
    free(elemento);
}

void iniciar_listas(){
    
    lista_new = list_create();
    lista_ready = list_create();
    lista_exec = list_create();
    lista_blocked = list_create();
    lista_s_blocked = list_create();
    lista_s_ready = list_create();

    sem_init(&mutex_listas, 0, 1);
    
    cantidad_de_procesos = 0;
    log_info(logger_kernel, "Listas inicializadas correctamente");
    
    return;
}

void iniciar_semaforos_generales(){
    sem_init(&proceso_finalizo_o_suspended, 0, 0);
    sem_init(&salida_exec, 0, 0);
    sem_init(&actualizacion_de_listas_1, 0, 0);
    sem_init(&actualizacion_de_listas_2, 0, 0);
    sem_init(&actualizacion_de_listas_1_recibido, 0, 0);
    sem_init(&proceso_inicializado, 0, 0);
    sem_init(&libre_para_inicializar_proceso, 0, 1);
    sem_init(&mutex_multiprocesamiento, 0, 1);
    sem_init(&mutex_cant_procesos, 0, 1);
    return;
}

void mover_proceso_de_lista(t_list *origen, t_list *destino, int index, int status){
    t_proceso *aux;

    sem_wait(&mutex_listas);
        aux = list_remove(origen, index);
        aux->status =  status;
        aux->termino_rafaga = false;
        list_add(destino, aux);
    sem_post(&mutex_listas);

    avisar_cambio();
    return;
}

void avisar_cambio(){
    sem_wait(&mutex_cant_procesos);
    //Aviso que hubo un cambio de listas
    for(int i = 0; i < cantidad_de_procesos; i++){
        sem_post(&actualizacion_de_listas_1);
    }
    
    //Espero que todos los procesos hayan recibido el aviso y ejecutado
    for(int i = 0; i < cantidad_de_procesos; i++){
        sem_wait(&actualizacion_de_listas_1_recibido);
    }
    //Habilito que vuelvan a esperar una vez ya resuelto todo lo que tengan que hacer con su nuevo estado
    for(int i = 0; i < cantidad_de_procesos; i++){
        sem_post(&actualizacion_de_listas_2);
    }
    sem_post(&mutex_cant_procesos);
}
