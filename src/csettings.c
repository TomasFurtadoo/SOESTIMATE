#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csettings.h"
#include "main.h"

void read_arguments_file(const char *filename, struct info_container *info){
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        fprintf(stderr, "[Erro] Não foi possivel abrir o ficheiro");
        exit(1);
    }

    int num_sensores, num_servers, buffer_size;

    if(fscanf(file, "%d%d%d", &num_sensores, &num_servers, &buffer_size) != 3){
        fprintf(stderr, "[Erro] O ficheiro nao tem o formato correto");
        fclose(file);
        exit(1);
    }

    if(num_sensores <= 0 || num_servers <= 0 || buffer_size <= 0) {
        fprintf(stderr, "[Erro] Os argumentos nao podem ter valores nulos os negativos");
        fclose(file);
        exit(1);
    }

    info->n_sensors = num_sensores;
    info->n_servers = num_servers;
    info->buffers_size = buffer_size;

    fclose(file);
}

void read_settings_file(const char *filename, struct info_container *info){
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        fprintf(stderr, "[ERRO] Não foi possivel abrir o ficheiro");
        exit(1);
    }
    
    if(fscanf(file, "%99s%99s%u", info->log_filename, info->statistics_filename, &info->period) != 3){
        fprintf(stderr, "[ERRO] O ficheiro não tem o formato correto");
        fclose(file);
        exit(1);
    }

    fclose(file);

}