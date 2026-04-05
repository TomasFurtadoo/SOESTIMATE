#include <stdio.h>
#include "sensor.h"
#include "memory.h"
#include "random_measurement.h"


int execute_sensor(int sensor_id, struct info_container *info, struct buffers *buffs){
    seed_sensor_rng(sensor_id);
    int expected_m_id = 1;
    while(!*info->terminate){
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
    read_main_sensors_buffer(buffs->buff_main_sensors, info->buffers_size, expected_m_id, req);
}


void sensor_process_request(MeasurementInfo *m, int sensor_id, struct info_container *info){
    m->sensor_id = sensor_id;
    m->value = get_measurement();
    m->state = MEASURED;

    info->num_generated_measurements[sensor_id]++;
}

void sensor_send_measurement(MeasurementInfo *m, struct info_container *info, struct buffers *buffs){
    while(!write_sensor_controller_buffer(buffs->buff_sensors_controllers, info->buffers_size, m)){
        if(*info->terminate){
            return;
        }
    }
}