#ifndef QMUJOCOSIM_UTILS_H
#define QMUJOCOSIM_UTILS_H


#include "mujoco/mujoco.h"

void clearTimers(mjData *d);


void install_mjcb_time();

void check_mujoco_version();

#endif //QMUJOCOSIM_UTILS_H
