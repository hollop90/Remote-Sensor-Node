/**
 * @file lmicJobs.h
 * @author Ugochukwu Uzoukwu
 * @brief Fucntion and variable declarations
 * @version 0.9
 * @date 2023-02-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef UGO_LMIC_JOBS
#define UGO_LMIC_JOBS

#include <lmic.h>

void onEvent (ev_t ev);
void initFunc(osjob_t* j);
void wakeUp();
void readSensor(osjob_t* j);
void logData(osjob_t* j);
void do_send(osjob_t* j);
void sleep(osjob_t* j);
void setupChannelsEU868();
double readBattVoltage();

extern osjob_t initJob;
extern osjob_t wakeJob;
extern osjob_t readJob;
extern osjob_t logJob;
extern osjob_t sendjob;
extern osjob_t sleepJob;

#endif // UGO_LMIC_JOBS