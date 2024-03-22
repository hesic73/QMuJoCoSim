#ifndef MUJOCO_SIMULATION_QT_UTILS_HPP
#define MUJOCO_SIMULATION_QT_UTILS_HPP

#include "mujoco/mujoco.h"

#include <QString>

inline QString renderingEffectsToString(mjvScene *scn) {
    QString s =
            "Shadow: %1, Wireframe: %2, Reflection: %3, Additive: %4, Skybox: %5, Fog: %6, Haze: %7, Segment: %8, Id Color: %9, Cull Face: %10";

    return s.arg(scn->flags[mjtRndFlag::mjRND_SHADOW])
            .arg(scn->flags[mjtRndFlag::mjRND_WIREFRAME])
            .arg(scn->flags[mjtRndFlag::mjRND_REFLECTION])
            .arg(scn->flags[mjtRndFlag::mjRND_ADDITIVE])
            .arg(scn->flags[mjtRndFlag::mjRND_SKYBOX])
            .arg(scn->flags[mjtRndFlag::mjRND_FOG])
            .arg(scn->flags[mjtRndFlag::mjRND_HAZE])
            .arg(scn->flags[mjtRndFlag::mjRND_SEGMENT])
            .arg(scn->flags[mjtRndFlag::mjRND_IDCOLOR])
            .arg(scn->flags[mjtRndFlag::mjRND_CULL_FACE]);
}

#endif //MUJOCO_SIMULATION_QT_UTILS_HPP
