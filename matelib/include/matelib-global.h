#ifndef MATELIB_GLOBAL_H
#define MATELIB_GLOBAL_H 

#include "server.h"
#include "config_utils.h"

//Config
t_config *arch_config;
//Log
t_log *logger_matelib;
//Conexiones
int socket_servidor;
int socket_cliente;

#endif