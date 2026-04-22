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

int execute_controller(int controller_id, struct info_container *info, struct buffers *buffs){

    int expected_m_id = 1;

    while(!*info->terminate){
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
    read_sensor_controller_buffer(buffs->buff_sensors_controllers, controller_id, info->buffers_size, m);
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
    while(!write_controller_servers_buffer(buffs->buff_controllers_servers, info->buffers_size, m))
        if(*info->terminate){
            return;
        }

}