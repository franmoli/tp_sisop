#ifndef SERIALIZACIONES__H
#define SERIALIZACIONES__H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include "../../memoria/include/memoria-global.h"
#include "../../kernel/include/kernel-global.h"

int bytes_marco(t_marco marco);
int bytes_pagina(t_pagina pagina);
void *serializar_marco(t_marco marco, void **stream_inicial, int offset);
void *serializar_pagina(t_pagina pagina, void **stream_inicial, int offset);
t_marco deserializar_marco(void *stream, int offset);
t_pagina deserializar_pagina(void *stream, int offset);
t_paquete *serializar_mate_sem_init(uint32_t valor, char *nombre_sem);
t_mate_sem *deserializar_mate_sem_init(t_paquete *paquete);
t_paquete *serializar_mate_sem_resto(char *nombre_sem,op_code cod_op);
t_mate_sem *deserializar_mate_sem_resto(t_paquete *paquete);

#endif