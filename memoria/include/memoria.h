#ifndef MEMORIA_H
#define MEMORIA_H

#include "memoria-global.h"
#include <pthread.h>
#include "paginacion.h"
#include <signal.h>
#include "recepcion-mensaje.h"
#include "mem-dump.h"
#include "enviar-datos.h"

static void *ejecutar_operacion();
void imprimirMetricas();
void limpiarTlb();
void generarDump();
void inicializarCarpincho(int socket_cliente);
void metricas();
char* cargarTextoTLb(t_list *paginas, int carpincho_id);

#endif