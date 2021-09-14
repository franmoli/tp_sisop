#include "memoria.h"

int main(int argc, char **argv)
{
    config_memoria = generarConfigMemoria(leer_config_file("./cfg/memoria.cfg"));
    return 1;
}