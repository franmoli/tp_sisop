#include "paginacion.h"

void guardarMemoria(t_paquete *paquete)
{
    int espacioAguardar = deserializar_alloc(paquete);
    memAlloc(espacioAguardar);
}

int getPaginaByDireccion(uint32_t direccion){
    uint32_t inicio = tamanio_memoria;
    uint32_t resta =  direccion - inicio;
    return resta/config_memoria->TAMANIO_PAGINA;
}
//////////////////////////////////////////////////////////////////////////////

// Mem Read: Dada una direccion de memoria busco el contenido que se encuentra alli
t_heap_metadata* memRead(uint32_t direccion) {

    //TODO: Tener en cuenta que la direccion pertenezca a una pagina de la tabla de este proceso

    t_heap_metadata* alloc = traerAllocDeMemoria(direccion);

    log_info(logger_memoria, "Traje alloc de memoria");
    return alloc;

}