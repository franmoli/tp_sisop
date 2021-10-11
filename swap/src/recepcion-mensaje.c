#include "recepcion-mensaje.h"

void analizar_paquete(t_paquete *paquete) {
	switch (paquete->codigo_operacion){
		case CLIENTE_TEST:
			log_info(logger_swap, "Mensaje de prueba recibido correctamente");
			break;
        default:
			log_error(logger_swap, "Codigo de operacion desconocido");
			break;
    }
}