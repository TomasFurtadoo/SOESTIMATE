/*
Grupo Numero 24
Bernardo Palma - 63756
Dinis Raposo - 63776
Tomas Furtado - 63772
*/

#include <stdio.h>
#include "sensor.h"
#include "memory.h"
#include "random_measurement.h"
#include <semaphore.h>


int execute_sensor(int sensor_id, struct info_container *info, struct buffers *buffs){
    seed_sensor_rng(sensor_id);
    int expected_m_id = 1;

    while(1){
        sem_wait(info->sems->terminate_mutex);
        if(*info->terminate) {
            sem_post(info->sems->terminate_mutex);
            break;
        }
        sem_post(info->sems->terminate_mutex);
    
        MeasurementInfo req;
        sensor_receive_request(&req, expected_m_id, info, buffs);

        if(req.m_id != -1){
            sensor_process_request(&req, sensor_id, info);
            sensor_send_measurement(&req, info, buffs);
            expected_m_id++;
        }
    }
    return 0;
}


void sensor_receive_request(MeasurementInfo *req, int expected_m_id, struct info_container *info, struct buffers *buffs){
    sem_wait(info->sems->main_sensors->unread);

    if(*info->terminate) return;

    sem_wait(info->sems->main_sensors->mutex);
    read_main_sensors_buffer(buffs->buff_main_sensors, info->buffers_size, expected_m_id, req);
    sem_post(info->sems->main_sensors->mutex);

    if(req->m_id != -1 && req->counter_sensors == 0) {
        sem_post(info->sems->main_sensors->free_space);
    }
}


void sensor_process_request(MeasurementInfo *m, int sensor_id, struct info_container *info){
    m->sensor_id = sensor_id;
    m->value = get_measurement();
    m->state = MEASURED;

    m->counter_servers = info->n_servers;

    info->num_generated_measurements[sensor_id]++;
}

void sensor_send_measurement(MeasurementInfo *m, struct info_container *info, struct buffers *buffs){
    sem_wait(info->sems->sensors_controllers->free_space);
    sem_wait(info->sems->sensors_controllers->mutex);
    write_sensor_controller_buffer(buffs->buff_sensors_controllers, info->buffers_size, m);
    sem_post(info->sems->sensors_controllers->mutex);
    sem_post(info->sems->sensors_controllers->unread);
}