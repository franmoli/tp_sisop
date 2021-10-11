#ifndef SWAPL_GLOBAL_H
#define SWAP_GLOBAL_H

#include "config_utils.h"
#include "server.h"
#include "serializaciones.h"

//Configuración
t_config_swap *config_swap;
//Logs
t_log *logger_swap;
//Conexiones
int socket_server;
int socket_client;

#endif