#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "main.h"
#include "process.h"
#include <string.h>
#include <unistd.h>

void main_args(int argc, char *argv[], struct info_container *info){
    if(argc != 4){
        fprintf(stderr, "Argumentos invalidos.\n Uso: ./SOestimate n_sensors n_servers buff_size\n");
        exit(1);
    }

    info->n_sensors = atoi(argv[1]);
    info->n_servers = atoi(argv[2]);
    info->buffers_size = atoi(argv[3]);

    if(info->n_sensors <= 0 || info->n_servers <= 0 || info->buffers_size <= 0){
        fprintf(stderr, "Argumentos invalidos. n_sensors, n_servers e buff_size devem ser inteiros positivos.\n");
        exit(1);

    }

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
                req.m_id = (*info->total_measurements) + 1; 
                

                req.counter_sensors = info->n_sensors;
                req.counter_servers = info->n_servers;

                if (write_main_sensors_buffer(buffs->buff_main_sensors, info->buffers_size, &req)) {
                    printf("Ronda de medicao %d iniciada.\n", current_m_id);
                    (*info->total_measurements)++; 
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
    printf("\n--- ESTATISTICAS FINAIS SOestimate ---\n");
    printf("Configuracao: %d Sensores | %d Servidores | Buffer: %d\n", 
            info->n_sensors, info->n_servers, info->buffers_size);
    
    printf("\nSensores:\n");
    for(int i = 0; i < info->n_sensors; i++) {
        printf("  Sensor %d (PID %d): %d geradas, %d invalidas\n", 
                i, info->sensors_pids[i], 
                info->num_generated_measurements[i], 
                info->num_invalid_measurements[i]);
    }

    printf("\nServidores:\n");
    for(int i = 0; i < info->n_servers; i++) {
        printf("  Server %d (PID %d): %d estimativas calculadas\n", 
                i, info->servers_pids[i], info->num_estimates[i]);
    }

    printf("\nTotal de rondas solicitadas: %d\n", *info->total_measurements);
    printf("---------------------------------------\n");
    
}

void end_execution(struct info_container *info, struct buffers *buffs){
    *info->terminate = 1;

    wait_processes(info);

    write_final_statistics(info);

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
  
    if (m_id <= 0 || m_id > *info->total_measurements) {
        printf("Erro: Identificador de ronda %d e invalido ou ainda nao foi processado.\n", m_id); 
        return;
    }

    printf("Operacao 'read %d': A leitura do ficheiro de log sera implementada apenas na 2a Fase.\n", m_id); 

}

void print_stat(struct info_container *info){

    printf("\n======= ESTADO ATUAL DO SISTEMA (STAT) =======\n");
    printf("Configuracao:\n");
    printf("  N. Sensores: %d | N. Servidores: %d | Tamanho Buffers: %d\n", 
            info->n_sensors, info->n_servers, info->buffers_size); 

    printf("\nProcessos (PIDs):\n");
    printf("  Main PID: %d\n", getpid()); 
    
    printf("  Sensores:     ");
    for(int i = 0; i < info->n_sensors; i++) printf("%d ", info->sensors_pids[i]);
    
    printf("\n  Controladores:");
    for(int i = 0; i < info->n_sensors; i++) printf("%d ", info->controllers_pids[i]);
    
    printf("\n  Servidores:   ");
    for(int i = 0; i < info->n_servers; i++) printf("%d ", info->servers_pids[i]); 

    printf("\n\nContadores em Memoria Partilhada:\n");
    printf("  Rondas de medicao solicitadas: %d\n", *info->total_measurements); 
    printf("  Flag Terminate: %d\n", *info->terminate);
    
    printf("\nEstatisticas por Entidade:\n");
    for(int i = 0; i < info->n_sensors; i++) {
        printf("  [S%d] Geradas: %d | Invalidas: %d\n", 
                i, info->num_generated_measurements[i], info->num_invalid_measurements[i]);
    }
    for(int i = 0; i < info->n_servers; i++) {
        printf("  [SRV%d] Estimativas: %d\n", i, info->num_estimates[i]);
    }
    printf("==============================================\n");

}

void help(void){
    printf("\nComandos disponiveis:\n");
    printf("  measure  - Solicita uma nova ronda de medicao (mid++) \n");
    printf("  read <j> - Le a estimativa da ronda j (Fase 2) \n");
    printf("  stat     - Mostra o estado atual do sistema \n");
    printf("  help     - Mostra este menu de ajuda \n");
    printf("  end      - Termina a execucao do sistema \n\n");

}

int main(int argc, char *argv[]){


    struct info_container* info = (struct info_container*) allocate_dynamic_memory(sizeof(struct info_container));
    struct buffers* buffs = (struct buffers*) allocate_dynamic_memory(sizeof(struct buffers));


    main_args(argc, argv, info);


    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);


    create_processes(info, buffs);


    user_interaction(info, buffs);


    end_execution(info, buffs);

 
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);

    deallocate_dynamic_memory(info);
    deallocate_dynamic_memory(buffs);

    return 0;
}