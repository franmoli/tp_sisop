#ifndef MEM_DUMP_H
#define MEM_DUMP_H

#include "memoria-global.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int createFile(char *path);
void memdump();
char *dumpPaginacion();
char* cargarTexto(t_list *paginas, int carpincho_id);
#endif