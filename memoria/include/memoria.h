#ifndef MEMORIA_H
#define MEMORIA_H

#include "memoria-global.h"
#include <pthread.h>
#include "paginacion.h"
#include <signal.h>

static void *ejecutar_operacion(int client);
void imprimirMetricas(int signal);
void limpiarTlb(int signal);
void generarDump(int signal);
#endif