/*
Grupo Numero 24
Bernardo Palma - 63756
Dinis Raposo - 63776
Tomas Furtado - 63772
*/

#include "process.h"
#include "sensor.h"
#include "controller.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int launch_process(enum process_type process_type, int process_id, struct info_container *info, struct buffers *buffs) {

    pid_t pid = fork();

    if(pid < 0) {
        perror("Erro ao executar fork");
        return -1;
    }

    if(pid == 0) {
        int exit_status = 0;

        switch (process_type)  {
            case SENSOR_PROCESS:
                exit_status = execute_sensor(process_id, info, buffs);
                break;
            
            case CONTROLLER_PROCESS:
                exit_status = execute_controller(process_id, info, buffs);
                break;

            case SERVER_PROCESS:
                exit_status = execute_server(process_id, info, buffs);
                break;

            default:
                fprintf(stderr, "Tipo de processo desconhecido\n");
                exit(1);
        }

        exit(exit_status);

    }

    return (int) pid;
}


int wait_process(int process_id) {
    int status;

    if(waitpid(process_id, &status, 0) == -1) {
        perror("Erro ao esperar processo");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    

    return -1; 
}