#include "memoria.h"

int main(int argc, char **argv)
{
    system("clear");
    logger_memoria = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    log_info(logger_memoria, "Programa inicializado correctamente");
    pthread_t hilo_client;
    t_config *config = leer_config_file("./cfg/memoria.cfg");
    config_memoria = generarConfigMemoria(config);
    log_info(logger_memoria, "Configuración cargada correctamente");
    tamanio_memoria = malloc(sizeof(config_memoria->TAMANIO));

    socket_server = iniciar_servidor(config_memoria->IP, string_itoa(config_memoria->PUERTO), logger_memoria);
    socket_cliente_swap = crear_conexion(config_memoria->IP, "5002");
    if (socket_cliente_swap == -1) {
        log_info(logger_memoria, "Fallo en la conexion a swap");
    }

        tabla_tlb = malloc(sizeof(t_tabla_tlb));
        tabla_tlb->tlb = list_create();
        tabla_tlb->hit_totales = 0;
        tabla_tlb->miss_totales = 0;

        tlb_LRU = list_create();
        tlb_FIFO = queue_create();
        entradas_tlb = config_memoria->CANTIDAD_ENTRADAS_TLB;

        if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") != 0){
            if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0) {
                reemplazo_LRU = list_create();
            }else 
            {
                reemplazo_CLOCK = list_create();
            }
        }

        tabla_marcos_memoria = malloc(sizeof(t_tabla_marcos));
        tabla_marcos_memoria->marcos = list_create();

        tabla_procesos = list_create();

        int i = 0;
        uint32_t cant_marcos = config_memoria->TAMANIO / config_memoria->TAMANIO_PAGINA;;
        while (i < cant_marcos)
        {
            t_marco *tlb = malloc(sizeof(t_marco));
            tlb->numero_marco = i;
            list_add(tabla_marcos_memoria->marcos, tlb);
            i++;
        }
        //Conectar a swap
        socket_cliente_swap = crear_conexion(config_memoria->IP, "5002");
        if (socket_cliente_swap == -1) {
            log_info(logger_memoria, "Fallo en la conexion a swap");
        }

        signal(SIGINT, imprimirMetricas);
        signal(SIGUSR1, generarDump);
        signal(SIGUSR2, limpiarTlb);

    while (1)
    {
        socket_client = esperar_cliente(socket_server, logger_memoria);
        if (socket_client != -1)
        {
            inicializarCarpincho(socket_client);
            pthread_create(&hilo_client, NULL, (void *)ejecutar_operacion, (void *)socket_client);
        }
    }
    log_info(logger_memoria, "Programa finalizado con éxito");
    log_destroy(logger_memoria);
    liberar_config(config);
}

static void *ejecutar_operacion(int client)
{
    while (1)
    {
        t_paquete *paquete = recibir_paquete(client);

        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        switch (paquete->codigo_operacion)
        {
        case CLIENTE_TEST:
            log_info(logger_memoria, "Mensaje de prueba recibido correctamente por el cliente %d", client);
            break;
        case MEMALLOC:
            log_info(logger_memoria, "recibi orden de memalloc del cliente %d", client);
            int dire_logica =memAlloc(paquete);
            
            t_paquete* paquete_enviar = serializar(MEMALLOC,2,INT,dire_logica);
            //dire_logica = deserializar(paquete);
            log_info(logger_memoria,"Enviando paquete con direccion logica");
            enviar_paquete(paquete_enviar,socket_client);
            log_info(logger_memoria,"Paquete enviado");
            break;
        case MEMFREE:
            log_info(logger_memoria, "recibi orden de memfree del cliente %d", client);
            freeAlloc(paquete);

            break;

        case MEMWRITE:
            log_info(logger_memoria, "recibi orden de guardar en memoria del cliente %d", client);
            memAlloc(paquete);
            break;
        case MEMREAD:
            log_info(logger_memoria, "recibi orden de leer memoria del cliente %d", client);
            t_heap_metadata *data = memRead(paquete);
            break;
        case MATEINIT:
            log_info(logger_memoria, "recibi un nuevo carpincho para inicializar del cliente %d", client);
            //inicializarCarpincho(paquete);
            break;
        default:
            log_error(logger_memoria, "Codigo de operacion desconocido");
            break;
        }

        //Libero la memoria ocupada por el paquete
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
    }
    close(client);
    log_info(logger_memoria, "Se desconecto el cliente [%d]", client);
    return NULL;
}
void recibirSignal(int signal)
{
    if (signal == SIGINT)
    {
        //memdump();
        imprimirMetricas();
    }
    if (signal == SIGUSR1)
    {
        generarDump();
    }
    if (signal == SIGUSR2)
    {
        limpiarTlb();
    }
}
void limpiarTlb()
{
    log_info(logger_memoria, "SEÑAL RECIBIDA LIMPIANDO TLB");
    list_clean(tabla_tlb->tlb);
    log_info(logger_memoria, "TLB VACIA");
}
void generarDump()
{
   memdump();
}
void imprimirMetricas()
{
    metricas();
    log_info(logger_memoria, "SEÑAL RECIBIDA");
    exit(EXIT_SUCCESS);
}

void metricas(){
    char *texto = string_new();
    t_list_iterator *list_iterator = list_iterator_create(tabla_procesos);
    t_tabla_paginas *tablas;

    log_info("HITS TOTALES: %d \n MISS TOTALES: %d \n", tabla_tlb->hit_totales, tabla_tlb->miss_totales);
    while (list_iterator_has_next(list_iterator))
    {
        tablas = list_iterator_next(list_iterator);
        log_info("CARPINCHO: %d HITS POR CARPINCHO: %d MISS POR CARPINCHO: %d \n", tablas->pid, tablas->hit, tablas->miss);
    }
}

void inicializarCarpincho(int socket_cliente)
{

    //t_mateinit_serializado *mateInit_deserializado = deserializar_mate_init(paquete);
    uint32_t id = socket_cliente;

    t_tabla_paginas *nuevaTabla = malloc(sizeof(t_tabla_paginas));

    nuevaTabla->pid = id;
    nuevaTabla->paginas = list_create();
    nuevaTabla->Lru = list_create();
    nuevaTabla->Clock = list_create();
    nuevaTabla->paginas_en_memoria = 0;
    nuevaTabla->paginas_totales_maximas = config_memoria->TAMANIO / config_memoria->TAMANIO_PAGINA;
    nuevaTabla->hit = 0;
    nuevaTabla->miss = 0;
    list_add(tabla_procesos, nuevaTabla);

    return;
}