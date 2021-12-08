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

//Estructuras
typedef enum {
	CLIENTE_DESCONECTADO = 1,
	CLIENTE_TEST = 2,
    NUEVO_CARPINCHO = 3,
    SEM_WAIT = 4,
    SEM_POST = 5,
    INIT_SEM = 6,
    SEM_DESTROY = 7,
    OP_ERROR = 8,
    MEMALLOC = 9,
    MEMFREE = 10,
    MEMREAD = 11,
    MEMWRITE = 12,
    MATEINIT = 13,
    SWAPFREE = 14,
    SWAPSAVE = 15,
    CALLIO = 16,
    OP_CONFIRMADA = 17,
    RECEPCION_PAGINA = 18,
    DIRECCION_LOGICA = 19,
} op_code;

typedef struct {
    uint32_t size;
    void *stream;
} t_buffer;

typedef struct {
    op_code codigo_operacion;
    t_buffer *buffer;
} t_paquete;

//Funciones
int iniciar_servidor(char *ip, char *puerto, t_log *logger);
int crear_conexion(char *ip, char* puerto);
int esperar_cliente(int socket_servidor, t_log *logger);
void* serializar_paquete(t_paquete *paquete, int *bytes);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
t_paquete *recibir_paquete(int socket_cliente);

#endif