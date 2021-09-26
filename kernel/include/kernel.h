#ifndef KERNEL_H
#define KERNEL_H

#include "kernel-global.h"

void element_destroyer(void* elemento);
void liberar_memoria_y_finalizar(t_config_kernel *config_kernel, t_log *logger_kernel, t_config *config_file);
void print_inicializacion (t_config_kernel *config_kernel);
void atender_proceso (void* parametro );
void *iniciar_servidor_kernel(t_config_kernel *config_kernel, t_log *logger_kernel);

t_config *config_file;

#endif