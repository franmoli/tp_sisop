#include "recepcion-mensaje.h"

void receptor(){
	while(1){
		t_paquete* paquete;

		switch (paquete->codigo_operacion)
        {
			case RECEPCION_PAGINA:{
				
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