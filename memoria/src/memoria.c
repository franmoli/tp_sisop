#include "memoria.h"

int main(int argc, char **argv)
{
    pthread_t hilo_client;
	t_config* config = leer_config_file("./cfg/memoria.cfg");
    config_memoria = generarConfigMemoria(config);
    tamanio_memoria = malloc(sizeof(config_memoria->TAMANIO));
    logger_memoria = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
	
	tabla_paginas = malloc(sizeof(t_tabla_paginas));
    tabla_paginas->paginas = list_create();
	tabla_paginas->paginas_totales_maximas =config_memoria->TAMANIO / config_memoria->TAMANIO_PAGINA;
    
    socket_server = iniciar_servidor("127.0.0.1", string_itoa(config_memoria->PUERTO), logger_memoria);
	log_info(logger_memoria, "Modulo Memoria iniciado socket:[%d] esperando clientes...", socket_server);
    while(1){

        socket_client = esperar_cliente(socket_server, logger_memoria);
		if (socket_client != -1)
		{
			log_info(logger_memoria, "Se conecto el cliente [%d]", socket_client);
			pthread_create(&hilo_client, NULL, (void *)ejecutar_operacion, (void *)socket_client);
		}
    }
    log_destroy(logger_memoria);
	liberar_config(config);
}
static void *ejecutar_operacion(int client)
{
	bool rec = true;
	while (rec)
	{
		t_paquete *paquete;
		paquete = recibir_mensaje(client);
        analizar_paquete(paquete);
		free(paquete);
	}
	close(client);
	log_info(logger_memoria, "Se desconecto el cliente [%d]", client);
}