#define _POSIX_C_SOURCE 200809L
#include "main.h"
#include "csignal.h"
#include <signal.h>

static void sigint_handler(int signum){
    end_execution(info, buffs);
}

void setup_sigint_handler(void){
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void ignore_signals(void){
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void alarm_handler(int signum){
    
}

void setup_alarm(int period){
    if(period > 0){
        struct sigaction sa;
        sa.sa_handler = alarm_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGALRM, &sa, NULL);
        alarm(period);
    }
}













