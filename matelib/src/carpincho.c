#include "carpincho.h"

int main(int argc, char **argv) {
    /*
    mate_instance mate_ref;
    mate_init(&mate_ref, "cfg/matelib.cfg");
    mate_sem_init(&mate_ref,"sem1",1);
    mate_close(&mate_ref);
    return 0;
    */
    
    char *sem = "holanico";
    unsigned int value = 1;
    
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete = serializar_mate_sem_init(value,sem);
    
    
    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem = deserializar_mate_sem_init(paquete);
    
    int comp1 = strncmp(sem,datos_sem->nombre,15);
    bool comp2 = (value == datos_sem->value);

    if(comp1 == 0 && comp2){
        printf("los datos estan correctos \n");
    }else{
        printf("los datos estan mal pasados \n");
        printf("dato base: %s, %d\n",sem,value);
        printf("dato serializado: %s, %d\n",datos_sem->nombre,datos_sem->value);
    }
    return 0;
}