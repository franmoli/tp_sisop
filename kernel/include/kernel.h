#ifndef KERNEL_H
#define KERNEL_H
// Acá se están todas las declaraciones de funciones y librerias necesitadas Exclusivamente por kernel.c

#include "kernel-global.h"

void element_destroyer(void* elemento);
void liberar_memoria_y_finalizar(t_config_kernel *config_kernel, t_log *logger_kernel, t_config *config_file);
void print_inicializacion (t_config_kernel *config_kernel);
void iniciar_listas();

t_config *config_file;

#endif