#ifndef MEMORIA_GLOBAL_H
#define MEMORIA_GLOBAL_H

#include "config_utils.h"
#include "server.h"

t_config_memoria *config_memoria;
t_log *logger_memoria;

typedef struct{
    uint32_t numero_marco;
}t_tlb;

typedef struct{
    t_list *paginas;
    int paginas_totales_maximas;
}t_tabla_paginas;

typedef struct{
   uint32_t numero_marco;
   bool esta_vacia;
}t_pagina;

t_tabla_paginas* tabla_paginas;
void* tamanio_memoria;
int socket_server;
int socket_client;
#endif