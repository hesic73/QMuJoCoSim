#include "utils.h"

#include <chrono>
#include <iostream>


void clearTimers(mjData *d) {
    for (int i = 0; i < mjNTIMER; i++) {
        d->timer[i].duration = 0;
        d->timer[i].number = 0;
    }
}


void install_mjcb_time() {
    mjcb_time = []() {
        return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count();
    };
}

void check_mujoco_version() {
    std::cout << "MuJoCo version " << mj_versionString() << std::endl;
    if (mjVERSION_HEADER != mj_version()) {
        mju_error("Headers and library have different versions");
    }
}