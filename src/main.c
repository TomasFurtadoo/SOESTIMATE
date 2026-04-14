#include <stdlib.h>
#include "memory.h"
#include "main.h"

void main_args(int argc, char *argv[], struct info_container *info){
    if(argc != 4){
        fprintf(stderr, "Argumentos invalidos.\n Uso: ./SOestimate n_sensors n_servers buff_size\n");
        exit(1);
    }

    info->n_sensors = atoi(argv[1]);
    info->n_servers = atoi(argv[2]);
    info->buffers_size = atoi(argv[3]);

    if(info->n_sensors == 0 || info->n_servers == 0 || info->buffers_size)

}

void create_dynamic_memory_structs(struct info_container *info, struct buffers *buffs){
    info->sensors_pids = (int*) allocate_dynamic_memory(info->n_sensors * sizeof(int));
    info->controllers_pids = (int*) allocate_dynamic_memory(info->n_sensors * sizeof(int));
    info->servers_pids = (int*) allocate_dynamic_memory(info->n_servers * sizeof(int));

    buffs->buff_main_sensors = (struct circ_buffer*) allocate_dynamic_memory(sizeof(struct circ_buffer));
    buffs->buff_sensors_controllers = (struct ra_buffer*) allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_controllers_servers = (struct ra_buffer*) allocate_dynamic_memory(sizeof(struct ra_buffer));
}

void create_shared_memory_structs(struct info_container *info, struct buffers *buffs){

}

void destroy_dynamic_memory_structs(struct info_container *info, struct buffers *buffs){
    deallocate_dynamic_memory(info->sensors_pids);
    deallocate_dynamic_memory(info->controllers_pids);
    deallocate_dynamic_memory(info->servers_pids);
    
    deallocate_dynamic_memory(buffs->buff_controllers_servers);
    deallocate_dynamic_memory(buffs->buff_main_sensors);
    deallocate_dynamic_memory(buffs->buff_sensors_controllers);
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