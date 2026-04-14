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

    iinfo->num_generated_measurements = create_shared_memory("shm_gen", info->n_sensors * sizeof(int));
    info->num_invalid_measurements = create_shared_memory("shm_inv", info->n_sensors * sizeof(int));
    info->num_estimates = create_shared_memory("shm_est", info->n_servers * sizeof(int));
    info->total_measurements = create_shared_memory("shm_total", sizeof(int));
    info->terminate = create_shared_memory("shm_terminate", sizeof(int));
    
    buffs->main_sensor_buffer = create_shared_memory("shm_main_buff", sizeof(struct circ_buffer));
    buffs->main_sensor_buffer->buffer = create_shared_memory("shm_main_data", info->buffers_size * sizeof(MeasurementInfo));
    buffs->main_sensor_buffer->ptrs = create_shared_memory("shm_main_ptrs", sizeof(struct shared_pointers));

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