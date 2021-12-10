#include "recepcion-mensaje.h"

void receptor(){
	while(1){
		t_paquete* paquete;
		pthread_mutex_unlock(&mutex_envio_pagina);
		paquete = recibir_paquete(socket_server);
		switch (paquete->codigo_operacion)
        {
			case RECEPCION_PAGINA:{
				//t_pagina_swap swap = deserializar_pagina(paquete);
				break;
			}
			default:{
				log_error(logger_memoria, "Codigo de operacion desconocido");
				break;
			}
		}
		free(paquete);
	}
}
void analizar_paquete(t_paquete *paquete){
	switch (paquete->codigo_operacion){
        default:
		log_error(logger_memoria, "Codigo de operacion desconocido");
		break;
    }
}