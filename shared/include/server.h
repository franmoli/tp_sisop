#ifndef SERVER__H
#define SERVER__H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include "TAD.h"

typedef struct
{
    uint32_t size;
    void *stream;
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer *buffer;
} t_paquete;

void *recibir_buffer(int *, int, t_log *);

int iniciar_servidor(char *, char *, t_log *);
int esperar_cliente(int, t_log *);
t_list *recibir_paquete(int, t_log *);
t_paquete *recibir_mensaje(int socket_cliente);
int recibir_operacion(int, t_log *);
void enviarMensaje(t_paquete* paquete, int socket_cliente);
void* serializar_paquete(t_paquete* paquete, int *bytes);
#endif