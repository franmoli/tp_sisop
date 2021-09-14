#include "swap.h"

int main(int argc, char **argv)
{
    config_swap = generarConfigMemoria(leer_config_file("./cfg/swap.cfg"));
    return 1;
}