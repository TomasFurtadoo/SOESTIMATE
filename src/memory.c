/*
Grupo Numero 24
Bernardo Palma - 63756
Dinis Raposo - 63776
Tomas Furtado - 63772
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "memory.h"


void *allocate_dynamic_memory(int size){
    return calloc(1,size);
}

void deallocate_dynamic_memory(void *ptr){
    free(ptr);
}

void *create_shared_memory(char *name, int size){
    //Cria o nome final para o shm_open
    char final_name[256];
    sprintf(final_name, "/%s_%d", name, getuid());
    
    //Cria o objeto de memória partilhada
    int fd = shm_open(final_name, O_CREAT | O_RDWR, 0600);
    if(fd == -1) {
        perror("Erro ao abrir memoria (shm_open)");
        return NULL;
    }
    
    //Define tamanho do objeto
    if(ftruncate(fd, size) == -1){
        perror("Erro no ftruncate");
        close(fd);
        return NULL;
    }
    
    //pega no ficheiro e retorna o ponteiro com memoria partilhada, leitura e escrita
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED) {
        perror("Erro no mmap");
        close(fd);
        return NULL;
    }
    //liberta o numero da tabela
    close(fd);
    
    //limpa dados antigos de outra execução
    memset(ptr, 0, size);
    
    return ptr;
}

void destroy_shared_memory(char *name, void *ptr, int size){
    //Cria o nome final para o shm_open
    char final_name[256];
    sprintf(final_name, "/%s_%d", name, getuid());

    //remove a ligacao entre o endereço de memoria do programa e a RAM
    if(munmap(ptr, size) == -1){
        perror("Erro no munmap");
    }

    if(shm_unlink(final_name) == -1){
        perror("Erro ao fechar memoria (shm_unlink)");
    }
}


int write_main_sensors_buffer(struct circ_buffer *buffer, int buffer_size, const MeasurementInfo *req){
    if((buffer->ptrs->in + 1) % buffer_size == buffer->ptrs->out){
        return 0;
    }

    buffer->buffer[buffer->ptrs->in] = *req;
    buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;
    return 1;
}


void read_main_sensors_buffer(struct circ_buffer *buffer, int buffer_size, int expected_m_id, MeasurementInfo *req){
    if(buffer->ptrs->in == buffer->ptrs->out || buffer->buffer[buffer->ptrs->out].m_id != expected_m_id){
        req->m_id = -1;
        return;
    }
    
    *req = buffer->buffer[buffer->ptrs->out];
    buffer->buffer[buffer->ptrs->out].counter_sensors--;
    
    if(buffer->buffer[buffer->ptrs->out].counter_sensors == 0){
        buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
    }
}

int write_sensor_controller_buffer(struct ra_buffer *buffer, int buffer_size, const MeasurementInfo *m){
    for(int i = 0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 0){
            buffer->buffer[i] = *m;
            buffer->ptrs[i] = 1;
            return 1;
        }       
    }
    return 0;
}

void read_sensor_controller_buffer(struct ra_buffer *buffer, int controller_id, int buffer_size, MeasurementInfo *m){
    for(int i = 0; i < buffer_size; i++ ){
        if(buffer->ptrs[i] == 1 && buffer->buffer[i].sensor_id == controller_id){
            *m = buffer->buffer[i];
            buffer->ptrs[i] = 0;
            return;
        }
    }
    m->m_id = -1;
}


int write_controller_servers_buffer(struct ra_buffer *buffer, int buffer_size, const MeasurementInfo *m){
    for(int i = 0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 0){
            buffer->buffer[i] = *m;
            buffer->ptrs[i] = 1;
            return 1;
        }
    }
    return 0;
}

void read_controller_servers_buffer(struct ra_buffer *buffer, int buffer_size, int expected_m_id, int expected_controller_id, MeasurementInfo *m){
    for(int i = 0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 1 && buffer->buffer[i].m_id == expected_m_id && buffer->buffer[i].controller_id == expected_controller_id){
            *m = buffer->buffer[i];
            buffer->buffer[i].counter_servers--;
            if(buffer->buffer[i].counter_servers == 0){
                buffer->ptrs[i] = 0;
            }
            return;
        }
    }
    m->m_id = -1;
}