#include <QApplication>
#include <QSurfaceFormat>
#include <QColorSpace>
#include "mainwindow.hpp"

#include "mujoco/mujoco.h"
#include <iostream>

static void check_mujoco_version() {
    std::cout << "MuJoCo version " << mj_versionString() << std::endl;
    if (mjVERSION_HEADER != mj_version()) {
        mju_error("Headers and library have different versions");
    }
}


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

    QApplication app(argc, argv);

    MainWindow w;
    QIcon icon(":/assets/icon.png");
    w.setWindowIcon(icon);

    w.resize(1600, 1200);
    w.show();

    return app.exec();
}