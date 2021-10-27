#ifndef PROCESO_H
#define PROCESO_H

#include "kernel-global.h"

void *proceso(void *_);
void new();
void ready();
void exec();
void blocked();

#endif