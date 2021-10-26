#include "paginacion.h"

void guardarMemoria(t_paquete* paquete){

    int paginaAGuardar = getPaginaAGuardar(2); 
    int posicionEnPagina = 0;
    if(paginaAGuardar >= 0){
        posicionEnPagina = getPosicionPagina(paginaAGuardar);
    }
    else{// No encontro memoria para meter el contenido
        //Swap Time
    }
}

int getPosicionPagina(int pagina){
    return 0;
}
int getPaginaAGuardar(int size){
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int numeroPagina= -1;
    bool pagainaFueEncontrada = false;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada){
		t_pagina *paginaLeida = list_iterator_next(list_iterator);
		int a = (config_memoria->TAMANIO_PAGINA - paginaLeida->tamanio_ocupado - size);
		if (a >= 0)
		{
			pagainaFueEncontrada = true;
			numeroPagina = paginaLeida->numero_pagina;
		}
	}
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}
void findAndSaveEnPagina(int pagina){
    //Busca en la tabla de paginas dicha pagina si esta mete ahi el contenido
    //Sino esta crea la pagina y guarda
}