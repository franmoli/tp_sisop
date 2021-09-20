#include "recepcion-mensaje.h"

void analizar_paquete(t_paquete *paquete)
{
	switch (paquete->codigo_operacion){
        default:
		log_error(logger_memoria, "Codigo de operacion desconocido");
		break;
    }
}