#include <QApplication>
#include <QCoreApplication>
#include <QSurfaceFormat>
#include <QColorSpace>

#include <mujoco/mujoco.h>


#include <iostream>


#include "mainwindow.hpp"


#include "utils.h"


static void set_surface_format() {
    QSurfaceFormat format;
    format.setDepthBufferSize(24);

    format.setStencilBufferSize(8);
    format.setSamples(4);
    format.setSwapInterval(1);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setVersion(2, 0);

    format.setColorSpace(QColorSpace::SRgb);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    check_mujoco_version();
    set_surface_format();

    install_mjcb_time();

    QCoreApplication::setApplicationName("QMuJoCoSim");

    QApplication app(argc, argv);

    MainWindow w;
    QIcon icon(":/assets/icon.png");
    w.setWindowIcon(icon);

    w.resize(1600, 1200);
    w.show();

    return app.exec();
}