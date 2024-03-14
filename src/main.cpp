#include <QApplication>
#include <QSurfaceFormat>
#include "mainwindow.hpp"


static void set_surface_format() {
    QSurfaceFormat format;
    format.setDepthBufferSize(24);

    format.setStencilBufferSize(8);
    format.setSamples(4);
    format.setSwapInterval(1);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setVersion(2, 0);

    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    set_surface_format();

    QApplication app(argc, argv);


    MainWindow w;
    w.resize(800, 600);
    w.show();

    return app.exec();
}