#ifndef SERIALIZACIONES__H
#define SERIALIZACIONES__H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include "../../memoria/include/memoria-global.h"

int bytes_pagina(t_pagina pagina);
void *serializar_pagina(t_pagina pagina, void **stream_inicial, int offset);
t_pagina deserializar_pagina(void *stream, int offset);

#endif