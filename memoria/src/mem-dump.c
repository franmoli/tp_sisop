#include "mem-dump.h"

int createFile(char *path)
{
    /*
        1: fue creado con exito
        0: no fue creado porque existe
    */
    int status = 0;
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        //No existe
        file = fopen(path, "w+");
        status = 1;
    }
    return status;
}
void memdump()
{
    char *path_folder_dump = string_new();
    char *timestamp = temporal_get_string_time( "Dump_%y%m%d%H%M%S.dmp");
    char* pathIntermedio = string_new();
    string_append_with_format(&pathIntermedio, "%s/%s", config_memoria->PATH_DUMP_TLB,timestamp);
    string_append_with_format(&path_folder_dump, "%s", pathIntermedio);

    createFile(path_folder_dump);
    int fd = open(path_folder_dump, O_RDWR | (O_APPEND | O_CREAT), S_IRUSR | S_IWUSR);

   char *contenido = string_new();
   string_append(&contenido, dumpPaginacion());
   write(fd, contenido, string_length(contenido));
   close(fd);
}


char *dumpPaginacion()
{
    char *texto = string_new();
    char *timestamp = temporal_get_string_time("%d/%m/%y %H:%M:%S");
    string_append_with_format(&texto, "Dump: %s", timestamp);


    

    t_list_iterator *list_iterator = list_iterator_create(tabla_tlb->tlb);
    t_tlb *tlb;
    int i = 0;
    char* estado;
    if(list_size(tabla_tlb->tlb) == config_memoria->CANTIDAD_ENTRADAS_TLB){
        while (list_iterator_has_next(list_iterator))
        {
            tlb = list_iterator_next(list_iterator);
            estado ="Ocupado";
            string_append_with_format(&texto, "Entrada:%d	Estado:%s	Carpincho: %d	Pagina: %d	Marco: %d \n",i, estado, tlb->pid,tlb->numero_pagina, tlb->numero_marco);
            i++;
        }
    }
    else{
        int restantes = config_memoria->CANTIDAD_ENTRADAS_TLB - i;
        while (list_iterator_has_next(list_iterator))
        {
            tlb = list_iterator_next(list_iterator);
            estado ="Ocupado";
            string_append_with_format(&texto, "Entrada:%d	Estado:%s	Carpincho: %d	Pagina: %d	Marco: %d \n",i, estado, tlb->pid,tlb->numero_pagina, tlb->numero_marco);
            i++;
        }
        int p = i;
        for(int k = 0; k < restantes; k++){
            string_append_with_format(&texto, "Entrada:%d	Estado:Libre	Carpincho: -	Pagina: -	Marco: - \n",p);
            p++;
        }
    }
    
    return texto;
}
char* cargarTexto(t_list *paginas, int carpincho_id){

    t_list_iterator *list_iterator = list_iterator_create(paginas);
    t_pagina *pagina;
    char* texto = string_new();
    char* estado= string_new();
    int i = 0;
    while (list_iterator_has_next(list_iterator))
    {
        pagina = list_iterator_next(list_iterator);
        if(pagina->tamanio_ocupado == 0){
            estado = "Libre";
        }else{
            estado="Ocupado";
        }
        string_append_with_format(&texto, "Entrada:%d	Estado:%s	Carpincho: %d	Pagina: %d	Marco: %d \n",i,estado, carpincho_id,pagina->numero_pagina, pagina->marco_asignado);
        i++;
    }
    return texto;
}