#include "../include/carpincho.h"

int main(int argc, char **argv) {
    mate_instance mate_ref;
    mate_init(&mate_ref, "cfg/matelib.cfg");
    //mate_sem_init(&mate_ref,"sem1",1);
    return 0;
}