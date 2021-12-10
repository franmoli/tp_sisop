#ifndef MEMORIA_H
#define MEMORIA_H

#include "memoria-global.h"
#include <pthread.h>
#include "paginacion.h"
#include <signal.h>
#include "recepcion-mensaje.h"
#include "mem-dump.h"
#include "enviar-datos.h"

static void *ejecutar_operacion(int client);
void imprimirMetricas();
void limpiarTlb();
void generarDump();
void recibirSignal(int signal);
void inicializarCarpincho(int socket_cliente);

char* cargarTextoTLb(t_list *paginas, int carpincho_id);

#endif