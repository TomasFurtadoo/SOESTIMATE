#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "main.h"
#include "process.h"

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
    info->num_generated_measurements = (int*) create_shared_memory(ID_SHM_SENSORS_GENERATED, info->n_sensors * sizeof(int));
    info->num_invalid_measurements   = (int*) create_shared_memory(ID_SHM_SENSORS_INVALID, info->n_sensors * sizeof(int));
    info->num_estimates              = (int*) create_shared_memory(ID_SHM_SERVERS_PROCESSED, info->n_servers * sizeof(int));
    info->total_measurements         = (int*) create_shared_memory(ID_SHM_TOTAL_MEASUREMENTS, sizeof(int));
    info->terminate                  = (int*) create_shared_memory(ID_SHM_TERMINATE, sizeof(int));
    
    buffs->buff_main_sensors->ptrs   = (struct pointers*) create_shared_memory(ID_SHM_MAIN_SENSORS_PTR, sizeof(struct pointers));
    buffs->buff_main_sensors->buffer = (MeasurementInfo*) create_shared_memory(ID_SHM_MAIN_SENSORS_BUFFER, info->buffers_size * sizeof(MeasurementInfo));

    buffs->buff_sensors_controllers->ptrs   = (int*) create_shared_memory(ID_SHM_SENSORS_CONTROLLERS_PTR, info->buffers_size * sizeof(int));
    buffs->buff_sensors_controllers->buffer = (MeasurementInfo*) create_shared_memory(ID_SHM_SENSORS_CONTROLLERS_BUFFER, info->buffers_size * sizeof(MeasurementInfo));

    buffs->buff_controllers_servers->ptrs   = (int*) create_shared_memory(ID_SHM_CONTROLLERS_SERVERS_LATEST, info->buffers_size * sizeof(int));
    buffs->buff_controllers_servers->buffer = (MeasurementInfo*) create_shared_memory(ID_SHM_CONTROLLERS_SERVERS_VERSION, info->buffers_size * sizeof(MeasurementInfo));

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
    destroy_shared_memory(ID_SHM_SENSORS_GENERATED, info->num_generated_measurements, info->n_sensors * sizeof(int));
    destroy_shared_memory(ID_SHM_SENSORS_INVALID, info->num_invalid_measurements, info->n_sensors * sizeof(int));
    destroy_shared_memory(ID_SHM_SERVERS_PROCESSED, info->num_estimates, info->n_servers * sizeof(int));
    destroy_shared_memory(ID_SHM_TOTAL_MEASUREMENTS, info->total_measurements, sizeof(int));
    destroy_shared_memory(ID_SHM_TERMINATE, info->terminate, sizeof(int));

    destroy_shared_memory(ID_SHM_MAIN_SENSORS_PTR, buffs->buff_main_sensors->ptrs, sizeof(struct pointers));
    destroy_shared_memory(ID_SHM_MAIN_SENSORS_BUFFER, buffs->buff_main_sensors->buffer, info->buffers_size * sizeof(MeasurementInfo));

    destroy_shared_memory(ID_SHM_SENSORS_CONTROLLERS_PTR, buffs->buff_sensors_controllers->ptrs, info->buffers_size * sizeof(int));
    destroy_shared_memory(ID_SHM_SENSORS_CONTROLLERS_BUFFER, buffs->buff_sensors_controllers->buffer, info->buffers_size * sizeof(MeasurementInfo));

    destroy_shared_memory(ID_SHM_CONTROLLERS_SERVERS_LATEST, buffs->buff_controllers_servers->ptrs, info->buffers_size * sizeof(int));
    destroy_shared_memory(ID_SHM_CONTROLLERS_SERVERS_VERSION, buffs->buff_controllers_servers->buffer, info->buffers_size * sizeof(MeasurementInfo));

}

void create_processes(struct info_container *info, struct buffers *buffs){

    for(int i = 0; i < info->n_sensors; i++){
        info->sensors_pids[i] = launch_process(SENSOR_PROCESS, i, info, buffs);
        info->controllers_pids[i] = launch_process(CONTROLLER_PROCESS, i, info, buffs);
    }

    for(int i = 0; i < info->n_servers; i++){
        info->servers_pids[i] = launch_process(SERVER_PROCESS, i, info, buffs);
    }

}

void user_interaction(struct info_container *info, struct buffers *buffs){

    char input[256];
    char command[256];
    int arg;

    int current_m_id = 1; 

    while(!*info->terminate){
        printf("SOestimate> ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        int parsed_args = sscanf(input, "%255s %d", command, &arg);

        if (parsed_args >= 1) {

            if (strcmp(command, "measure") == 0) {
                MeasurementInfo req;
                req.state = REQUEST;
                req.m_id = current_m_id; 
                

                req.counter_sensors = info->n_sensors;
                req.counter_servers = info->n_servers;

                if (write_main_sensors_buffer(buffs->buff_main_sensors, info->buffers_size, &req)) {
                    printf("Ronda de medicao %d iniciada.\n", current_m_id);
                    current_m_id++; 
                } else {
                    printf("Erro: Buffer circular cheio.\n");
                }
            } 
            else if (strcmp(command, "read") == 0) {
                if(parsed_args == 2) {
                    read_estimate(info, arg);
                } else {
                    printf("Uso: read <m_id>\n");
                }
            } 
            else if (strcmp(command, "stat") == 0) {
                print_stat(info);
            } 
            else if (strcmp(command, "help") == 0) {
                help();
            } 

            else if (strcmp(command, "end") == 0) {
                *info->terminate = 1; 
            } 
            else {
                printf("Comando desconhecido. Use 'help'.\n");
            }
        }
    }

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