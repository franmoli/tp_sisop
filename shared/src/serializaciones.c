#include "../include/serializaciones.h"

/**************************** Memoria <---> Swap ****************************/
//Cálculos de bytes
int bytes_marco(t_marco marco) {
    int bytes_marco = 0;

    bytes_marco += sizeof(marco.numero_marco);

    return bytes_marco;
}

int bytes_pagina(t_pagina pagina) {
    int bytes_pagina = 0;

    bytes_pagina += sizeof(pagina.numero_pagina);
    bytes_pagina += bytes_marco(*pagina.marco);
    bytes_pagina += sizeof(pagina.esta_vacia);

    return bytes_pagina;
}

//Serializaciones
void *serializar_marco(t_marco marco, void **stream, int offset) {
    memcpy(*stream + offset, &(marco.numero_marco), sizeof(marco.numero_marco));
	offset += sizeof(marco.numero_marco);

    return *stream;
}

void *serializar_pagina(t_pagina pagina, void **stream, int offset) {    
    memcpy(*stream + offset, &(pagina.numero_pagina), sizeof(pagina.numero_pagina));
	offset += sizeof(pagina.numero_pagina);
    serializar_marco(*pagina.marco, &(*stream), offset);
    offset += bytes_marco(*pagina.marco);
    memcpy(*stream + offset, &(pagina.esta_vacia), sizeof(pagina.esta_vacia));
	offset += sizeof(pagina.esta_vacia);
    return *stream;
}

//Deserializaciones
t_marco deserializar_marco(void *stream, int offset) {
    t_marco marco;

    memcpy(&marco.numero_marco, stream + offset, sizeof(marco.numero_marco));
	offset += sizeof(marco.numero_marco);

    return marco;
}

t_pagina deserializar_pagina(void *stream, int offset) {
    t_pagina pagina;

    memcpy(&pagina.numero_pagina, stream + offset, sizeof(pagina.numero_pagina));
	offset += sizeof(pagina.numero_pagina);
    t_marco marco_pagina = deserializar_marco(stream, offset);
    offset += bytes_marco(marco_pagina);
    memcpy(&pagina.esta_vacia, stream + offset, sizeof(pagina.esta_vacia));
	offset += sizeof(pagina.esta_vacia);
    
    pagina.marco = malloc(bytes_marco(marco_pagina));
    pagina.marco->numero_marco = marco_pagina.numero_marco;

    return pagina;
}