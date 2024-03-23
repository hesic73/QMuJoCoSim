#ifndef QMUJOCOSIM_UTILS_HPP
#define QMUJOCOSIM_UTILS_HPP

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

inline mjtMouse get_mjtMouse(Qt::MouseButton dragButton, Qt::KeyboardModifiers modifiers) {
    if (dragButton == Qt::LeftButton && (modifiers & Qt::ShiftModifier)) {
        return mjtMouse::mjMOUSE_ROTATE_H;
    } else if (dragButton == Qt::LeftButton && !(modifiers & Qt::ShiftModifier)) {
        return mjtMouse::mjMOUSE_ROTATE_V;
    } else if (dragButton == Qt::RightButton && (modifiers & Qt::ShiftModifier)) {
        return mjtMouse::mjMOUSE_MOVE_H;
    } else if (dragButton == Qt::RightButton && !(modifiers & Qt::ShiftModifier)) {
        return mjtMouse::mjMOUSE_MOVE_V;
    } else if (dragButton == Qt::MiddleButton) {
        return mjtMouse::mjMOUSE_ZOOM;
    }
    mju_warning("Unknown combination of dragButton and modifiers");
    return mjtMouse::mjMOUSE_NONE;
}

#endif //QMUJOCOSIM_UTILS_HPP
