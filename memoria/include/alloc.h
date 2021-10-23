#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>

void alloc_mem(int size);
bool memoria_disponible(int size);
#endif