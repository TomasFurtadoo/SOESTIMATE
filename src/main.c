#include "memory.h"
#include "main.h"

void main_args(int argc, char *argv[], struct info_container *info){
    if(argc != 4){
        perror
    }
}

void create_dynamic_memory_structs(struct info_container *info, struct buffers *buffs){
    
}

void create_shared_memory_structs(struct info_container *info, struct buffers *buffs){

    info->terminate = create_dynamic_memory_structs("shm_terminate", sizeOf(int));

    info->sensors_pids = create_dynamic_memory_structs("shm_sensors_pids", info->n_sensors * sizeof(int));

}

void destroy_dynamic_memory_structs(struct info_container *info, struct buffers *buffs){

}

void destroy_shared_memory_structs(struct info_container *info, struct buffers *buffs){

}

void create_processes(struct info_container *info, struct buffers *buffs){

}

void user_interaction(struct info_container *info, struct buffers *buffs){

}

void write_final_statistics(struct info_container *info){
    
}

void end_execution(struct info_container *info, struct buffers *buffs){

}

void wait_processes(struct info_container *info){

}

void read_estimate(struct info_container *info, int m_id){

}

void print_stat(struct info_container *info){

}

void help(void){

}

int main(int argc, char *argv[]){
    return 0;
}