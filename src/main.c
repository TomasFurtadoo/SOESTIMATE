/*
Grupo Numero 24
Bernardo Palma - 63756
Dinis Raposo - 63776
Tomas Furtado - 63772
*/

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "main.h"
#include "process.h"
#include <string.h>
#include <unistd.h>
#include "csettings.h"
#include "synchronization.h"
#include "csignal.h"
#include "cstats.h"
#include "clog.h"

struct info_container *info = NULL;
struct buffers *buffs = NULL;

void main_args(int argc, char *argv[], struct info_container *info){
    if(argc != 3){
        fprintf(stderr, "Argumentos invalidos.\n Uso: ./SOestimate <ficheiro_args> <ficheiro_settings>\n");
        exit(1);
    }
    
    read_arguments_file(argv[1], info);
    read_settings_file(argv[2], info);

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

    // Buffer Sensors->Controllers (RA: n_sensors * buffers_size)
    buffs->buff_sensors_controllers->ptrs   = (int*) create_shared_memory(ID_SHM_SENSORS_CONTROLLERS_PTR, info->n_sensors * sizeof(int));
    buffs->buff_sensors_controllers->buffer = (MeasurementInfo*) create_shared_memory(ID_SHM_SENSORS_CONTROLLERS_BUFFER, info->n_sensors * info->buffers_size * sizeof(MeasurementInfo));

    // Buffer Controllers->Servers (RA: n_sensors * buffers_size)
    buffs->buff_controllers_servers->ptrs   = (int*) create_shared_memory(ID_SHM_CONTROLLERS_SERVERS_LATEST, info->n_sensors * sizeof(int));
    buffs->buff_controllers_servers->buffer = (MeasurementInfo*) create_shared_memory(ID_SHM_CONTROLLERS_SERVERS_VERSION, info->n_sensors * info->buffers_size * sizeof(MeasurementInfo));

    info->last_logged_id = (int*) create_shared_memory(ID_SHM_LAST_LOGGED_ID, sizeof(int));

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

    destroy_shared_memory(ID_SHM_SENSORS_CONTROLLERS_PTR, buffs->buff_sensors_controllers->ptrs, info->n_sensors * sizeof(int));
    destroy_shared_memory(ID_SHM_SENSORS_CONTROLLERS_BUFFER, buffs->buff_sensors_controllers->buffer, info->n_sensors * info->buffers_size * sizeof(MeasurementInfo));

    destroy_shared_memory(ID_SHM_CONTROLLERS_SERVERS_LATEST, buffs->buff_controllers_servers->ptrs, info->n_sensors * sizeof(int));
    destroy_shared_memory(ID_SHM_CONTROLLERS_SERVERS_VERSION, buffs->buff_controllers_servers->buffer, info->n_sensors * info->buffers_size * sizeof(MeasurementInfo));

    destroy_shared_memory(ID_SHM_LAST_LOGGED_ID, info->last_logged_id, sizeof(int));
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

    int print_prompt = 1;

    while(!*info->terminate){
        if(print_prompt){
            printf("SOestimate> ");
            fflush(stdout);
        }
        
        if (fgets(input, sizeof(input), stdin) == NULL){
            if(! *info->terminate) {
                print_prompt = 0;
                continue;
            }
            else break;
        }
        print_prompt = 1;

        int parsed_args = sscanf(input, "%255s %d", command, &arg);

        if (parsed_args >= 1) {

            if (strcmp(command, "measure") == 0) {
                MeasurementInfo req;
                memset(&req, 0, sizeof(MeasurementInfo));
                req.state = REQUEST;
                req.m_id = (*info->total_measurements) + 1; 
                
                req.change_time.main_time = get_current_time();

                req.counter_sensors = info->n_sensors;
                req.counter_servers = info->n_servers;

                if (write_main_sensors_buffer(buffs->buff_main_sensors, info->buffers_size, &req)) {
                    printf("Ronda de medicao %d iniciada.\n", current_m_id);
                    (*info->total_measurements)++; 
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
    printf("[Main] Final SOestimate statistics:\n"); // [cite: 3]
    
    // Total global de medições solicitadas
    printf("[Main] Total generated measurements: %d\n", *info->total_measurements); // [cite: 3]

    // Estatísticas por Sensor
    for(int i = 0; i < info->n_sensors; i++) {
        printf("[Main] Sensor %d generated %d measurements (%d invalid after validation).\n", 
                i, info->num_generated_measurements[i], info->num_invalid_measurements[i]); // [cite: 3]
    }

    // Estatísticas por Servidor
    for(int i = 0; i < info->n_servers; i++) {
        printf("[Main] Server %d computed %d estimates.\n", i, info->num_estimates[i]); // [cite: 4]
    }
    
}

void end_execution(struct info_container *info, struct buffers *buffs){
    static int alread_ending = 0;
    if(alread_ending) return;
    alread_ending = 1;

    *info->terminate = 1;

    wakeup_processes(info);

    wait_processes(info);

    write_statistics(info);
}

void wait_processes(struct info_container *info){

    for(int i = 0; i < info->n_sensors; i++) {
        wait_process(info->sensors_pids[i]);
    }

    for(int i = 0; i < info->n_sensors; i++) {
        wait_process(info->controllers_pids[i]);
    }   

    for(int i = 0; i < info->n_servers; i++) {
        wait_process(info->servers_pids[i]);
    }
    printf("Todos os processos terminados com sucesso.\n");

}

void read_estimate(struct info_container *info, int m_id){
    double estimate = read_j(m_id, info);
    if(estimate != -1.0){
        printf("Ronda %d: estimativa = %.4f\n", m_id, estimate);
    }
    else{
        printf("A medição %d ainda nao foi concluida ou nao existe no log.\n", m_id);
    }
}

void print_stat(struct info_container *info){
    // Cabeçalho e Configuração baseados nas strings do SOestimate_profs
    printf("[Main] Current SOestimate state:\n");
    printf("[Main] n_sensors=%d, n_servers=%d, buffers_size=%d\n", 
            info->n_sensors, info->n_servers, info->buffers_size); 

    // Estado global
    printf("[Main] terminate=%d, total_measurements=%d\n", 
            *info->terminate, *info->total_measurements);

    // Secção de Sensores
    printf("[Main] Sensors:\n");
    for(int i = 0; i < info->n_sensors; i++) {
        printf("[Main]  sensor=%d pid=%d generated=%d\n",
                i, info->sensors_pids[i], info->num_generated_measurements[i]);
    }

    // Secção de Controladores (Apenas PIDs conforme as strings de referência)
    printf("[Main] Controllers:\n");
    for(int i = 0; i < info->n_sensors; i++) {
        printf("[Main]  controller=%d pid=%d\n",
                i, info->controllers_pids[i]);
    }

    // Secção de Servidores
    printf("[Main] Servers:\n");
    for(int i = 0; i < info->n_servers; i++) {
        printf("[Main]  server=%d pid=%d estimates=%d\n",
                i, info->servers_pids[i], info->num_estimates[i]);
    }

}

void help(void){
    printf("[Main] Available operations:\n"); // 
    printf("[Main]  measure - start a measurement cycle with one measurement per sensor.\n"); // [cite: 4]
    printf("[Main]  read <cycle_id> - read the estimate for a cycle.\n"); // [cite: 5]
    printf("[Main]  stat - print the current system state.\n"); // [cite: 5]
    printf("[Main]  help - show this menu.\n"); // 
    printf("[Main]  end - terminate SOestimate.\n"); //

}

int main(int argc, char *argv[]){


    info = (struct info_container*) allocate_dynamic_memory(sizeof(struct info_container));
    buffs = (struct buffers*) allocate_dynamic_memory(sizeof(struct buffers));


    main_args(argc, argv, info);
    clear_log(info);
    clear_statistics(info);

    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);

    info->sems = create_all_semaphores(info->buffers_size);

    create_processes(info, buffs);

    setup_sigint_handler();
    setup_alarm(info->period);

    user_interaction(info, buffs);

    if(*info->terminate == 0){
        end_execution(info, buffs);
    }

 
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);

    destroy_all_semaphores(info->sems);

    deallocate_dynamic_memory(info);
    deallocate_dynamic_memory(buffs);

    return 0;
}