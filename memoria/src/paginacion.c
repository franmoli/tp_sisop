#include "paginacion.h"

void guardarMemoria(t_paquete* paquete){

    int paginaAGuardar = 0;
    
    if(list_size(tabla_paginas->paginas) == 0){
        paginaAGuardar = getPaginaAGuardar(); 
        findAndSaveEnPagina(paginaAGuardar);
    }

    if(paginaAGuardar == 0){ // No encontro memoria para meter el contenido

    }
    //Deserializo
    //Me fijo si lo tengo 
}

int getPaginaAGuardar(){
    return 0;
}
void findAndSaveEnPagina(int pagina){
    //Busca en la tabla de paginas dicha pagina si esta mete ahi el contenido
    //Sino esta crea la pagina y guarda
}