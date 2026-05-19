#ifndef CSIGNAL_H_GUARD
#define CSIGNAL_H_GUARD
#include "main.h"

void setup_sigint_handler(void);

void ignore_signals(void);

void alarm_handler(int signum);

void setup_alarm(int period);

#endif