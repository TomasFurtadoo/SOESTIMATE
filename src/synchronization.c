#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "synchronization.h"
#include "main.h"

sem_t *create_semaphore(char *name, int value){
    sem_unlink(name);
    sem_t *sem = sem_open(name, O_CREAT, 0644, value);
    if(sem == SEM_FAILED){
        perror("[Sincronização] Erro ao criar semáforo (create_semaphore())");
        return NULL;
    }
    return sem;
}

void destroy_semaphore(char *name, sem_t *sem){
    if (sem_close(sem) == -1){
        perror("[Sincronização] Erro ao fechar semáforo (destroy_semaphore())");
    }

    if(sem_unlink(name) == -1){
        perror("[Sincronização] Erro ao dar unlink ao semáforo (destroy_semaphore())");
    }
}


struct buffer_semaphores *create_buffer_semaphores(int value, char *unread_name, char *free_name, char *mutex_name){
    struct buffer_semaphores *sems = malloc(sizeof(struct buffer_semaphores));
    sems->unread = create_semaphore(unread_name, 0);
    sems->free_space = create_semaphore(free_name, value);
    sems->mutex = create_semaphore(mutex_name, 1);

    if (sems->unread == NULL || sems->free_space == NULL || sems->mutex == NULL) {
        printf("[Erro] Falha ao criar um dos semáforos do buffer.\n"); //
        destroy_buffer_semaphores(sems, unread_name, free_name, mutex_name); 
        return NULL; 
    }

    return sems;
}


void destroy_buffer_semaphores(struct buffer_semaphores *sems, char *unread_name, char *free_name, char *mutex_name){
    destroy_semaphore(unread_name, sems->unread);
    destroy_semaphore(free_name, sems->free_space);
    destroy_semaphore(mutex_name, sems->mutex);
    free(sems);
}


struct semaphores *create_all_semaphores(unsigned value) {
    struct semaphores *sems = malloc(sizeof(struct semaphores));
    if (!sems) return NULL;

    sems->main_sensors = create_buffer_semaphores(value, 
        SEM_MAIN_SENSORS_UNREAD, SEM_MAIN_SENSORS_FREE, SEM_MAIN_SENSORS_MUTEX);

    sems->sensors_controllers = create_buffer_semaphores(value, 
        SEM_SENSORS_CONTROLLERS_UNREAD, SEM_SENSORS_CONTROLLERS_FREE, SEM_SENSORS_CONTROLLERS_MUTEX);

    sems->controllers_servers = create_buffer_semaphores(value, 
        SEM_CONTROLLERS_SERVERS_UNREAD, SEM_CONTROLLERS_SERVERS_FREE, SEM_CONTROLLERS_SERVERS_MUTEX);
    
    sems->terminate_mutex = create_semaphore(SEM_TERMINATE_MUTEX, 1);
    sems->log_mutex = create_semaphore(SEM_LOG_MUTEX, 1);

    if (!sems->main_sensors || !sems->sensors_controllers || !sems->controllers_servers || 
        !sems->terminate_mutex || !sems->log_mutex) {
        
        fprintf(stderr, "[Erro Crítico] Falha na inicialização da estrutura de sincronização.\n"); //
        destroy_all_semaphores(sems);
        return NULL;
    }

    return sems;
}

void destroy_all_semaphores(struct semaphores *sems) {
    if (!sems) return;

    destroy_buffer_semaphores(sems->main_sensors, 
        SEM_MAIN_SENSORS_UNREAD, SEM_MAIN_SENSORS_FREE, SEM_MAIN_SENSORS_MUTEX);
    
    destroy_buffer_semaphores(sems->sensors_controllers, 
        SEM_SENSORS_CONTROLLERS_UNREAD, SEM_SENSORS_CONTROLLERS_FREE, SEM_SENSORS_CONTROLLERS_MUTEX);
    
    destroy_buffer_semaphores(sems->controllers_servers, 
        SEM_CONTROLLERS_SERVERS_UNREAD, SEM_CONTROLLERS_SERVERS_FREE, SEM_CONTROLLERS_SERVERS_MUTEX);
    
    destroy_semaphore(SEM_TERMINATE_MUTEX, sems->terminate_mutex);
    destroy_semaphore(SEM_LOG_MUTEX, sems->log_mutex);

    free(sems);
}

void print_all_semaphores(struct semaphores *sems){
    int val;

    printf("[Main] --- Estado do Semáforo ---\n");

    //main sensors
    sem_getvalue(sems->main_sensors->unread, &val);
    printf("[Main] Main-Sensors UNREAD: %d\n", val);

    sem_getvalue(sems->main_sensors->free_space, &val);
    printf("[Main]  Main-Sensors FREE:   %d\n", val);
    
    sem_getvalue(sems->main_sensors->mutex, &val);
    printf("[Main]  Main-Sensors MUTEX:  %d\n", val);

    //sensors controllers
    sem_getvalue(sems->sensors_controllers->unread, &val);
    printf("[Main] Sensors-Controllers UNREAD: %d\n", val);

    sem_getvalue(sems->sensors_controllers->free_space, &val);
    printf("[Main]  Sensors-Controllers FREE:   %d\n", val);
    
    sem_getvalue(sems->sensors_controllers->mutex, &val);
    printf("[Main]  Sensors-Controllers MUTEX:  %d\n", val);

    //controllers servers
    sem_getvalue(sems->controllers_servers->unread, &val);
    printf("[Main] Controllers-Servers UNREAD: %d\n", val);

    sem_getvalue(sems->controllers_servers->free_space, &val);
    printf("[Main]  Controllers-Servers FREE:   %d\n", val);
    
    sem_getvalue(sems->controllers_servers->mutex, &val);
    printf("[Main]  Controllers-Servers MUTEX:  %d\n", val);

    //mutexs
    sem_getvalue(sems->terminate_mutex, &val);
    printf("[Main]  Terminate Mutex:     %d\n", val);

    sem_getvalue(sems->log_mutex, &val);
    printf("[Main]  Log Mutex:           %d\n", val);
    
    printf("[Main] ------------------------\n");
}

void wakeup_processes(struct info_container *info){
    //Main sensors
    for (int i = 0; i < info->n_sensors; i++) {
        sem_post(info->sems->main_sensors->unread);
        sem_post(info->sems->main_sensors->free_space);
    }

    //sensors controllers
    for (int i = 0; i < info->n_sensors; i++) {
        sem_post(info->sems->sensors_controllers->unread);
        sem_post(info->sems->sensors_controllers->free_space);
    }

    //controllers servers
    int max_procs = (info->n_sensors > info->n_servers) ? info->n_sensors : info->n_servers;
    for (int i = 0; i < max_procs; i++) {
        sem_post(info->sems->controllers_servers->unread);
        sem_post(info->sems->controllers_servers->free_space);
    }

}