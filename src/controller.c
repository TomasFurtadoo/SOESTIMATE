/*
Grupo Numero 24
Bernardo Palma - 63756
Dinis Raposo - 63776
Tomas Furtado - 63772
*/

#include <stdio.h>
#include "memory.h"
#include "main.h"
#include "controller.h"
#include "random_measurement.h"
#include <semaphore.h>
#include <unistd.h>

int execute_controller(int controller_id, struct info_container *info, struct buffers *buffs){

    int expected_m_id = 1;

    while(1){
        sem_wait(info->sems->terminate_mutex);
        if(*info->terminate) {
            sem_post(info->sems->terminate_mutex);
            break;
        }
        sem_post(info->sems->terminate_mutex);

        MeasurementInfo req;
        controller_receive_measurement(&req, controller_id, info, buffs);
        if(req.m_id == expected_m_id){
            controller_process_measurement(&req, controller_id, info);
            controller_send_measurement(&req, info, buffs);
            expected_m_id++;
        }
    }
    return 0;
}

void controller_receive_measurement(MeasurementInfo *m, int controller_id, struct info_container *info, struct buffers *buffs){
    m->m_id = -1;
    sem_wait(info->sems->sensors_controllers->unread);
    
    if(*info->terminate) return;

    sem_wait(info->sems->sensors_controllers->mutex);
    read_sensor_controller_buffer(buffs->buff_sensors_controllers, controller_id, info->buffers_size, m);
    sem_post(info->sems->sensors_controllers->mutex);

    if(m->m_id == -1){
        sem_post(info->sems->sensors_controllers->unread);
        usleep(1000);
    }
    else{
        sem_post(info->sems->sensors_controllers->free_space);
    }
}

void controller_process_measurement(MeasurementInfo *m, int controller_id, struct info_container *info){
    m->controller_id = controller_id;
    if(is_valid_measurement(m->value)){
        m->state = VALID;
    } else {
        m->state = INVALID;
        info->num_invalid_measurements[controller_id]++;
    } 
}

void controller_send_measurement(MeasurementInfo *m, struct info_container *info, struct buffers *buffs){
    sem_wait(info->sems->controllers_servers->free_space);
    sem_wait(info->sems->controllers_servers->mutex);
    write_controller_servers_buffer(buffs->buff_controllers_servers, info->buffers_size, m);
    sem_post(info->sems->controllers_servers->mutex);
    sem_post(info->sems->controllers_servers->unread);

    for(int i = 0; i < info->n_servers; i++){
        sem_post(info->sems->controllers_servers->unread);
    }

}