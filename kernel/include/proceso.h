#ifndef PROCESO_H
#define PROCESO_H

#include <unistd.h>

#include "kernel-global.h"

void *proceso(void *_);
void new();
void ready();
int exec(int mock);
void blocked();

#endif