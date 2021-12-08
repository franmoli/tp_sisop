#ifndef MEMORIA_H
#define MEMORIA_H

#include "memoria-global.h"
#include <pthread.h>
#include "paginacion.h"
#include <signal.h>
#include "recepcion-mensaje.h"
#include "mem-dump.h"
static void *ejecutar_operacion(int client);
void imprimirMetricas();
void limpiarTlb();
void generarDump();
void recibirSignal(int signal);
void inicializarCarpincho(t_paquete* paquete);

char* cargarTextoTLb(t_list *paginas, int carpincho_id);

#endif