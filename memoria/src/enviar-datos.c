#include "enviar-datos.h"

void enviarDatos(t_paquete* paquete, t_destino destino){
    switch (destino)
    {
        case KERNEL:
        {
            log_info(logger_memoria,"Enviando paquete con datos a KERNEL");
            log_info(logger_memoria,"Paquete enviado a KERNEL");
        }
        case SWAMP:
        {
            log_info(logger_memoria,"Enviando paquete con datos a SWAMP");
            log_info(logger_memoria,"Paquete enviado a SWAMP");
        }
        case MATELIB:
        {
            log_info(logger_memoria,"Enviando paquete con datos a MATELIB");
            log_info(logger_memoria,"Paquete enviado a MATELIB");
        }
    }
}