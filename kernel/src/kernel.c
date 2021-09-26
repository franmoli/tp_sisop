#include "kernel.h"


int main(int argc, char **argv)
{
    config_kernel = generarConfigMemoria(leer_config_file("./cfg/kernel.cfg"));

    return 1;
}