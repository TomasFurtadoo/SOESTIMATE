#include "main.h"
#include "csignal.h"
#include <signal.h>



static void sigint_handler(int signum){
    end_execution(info, buffs);
}

void ignore_signals(void){
    
}






void setup_alarm(int period){
    if(period > 0){
    }
}













