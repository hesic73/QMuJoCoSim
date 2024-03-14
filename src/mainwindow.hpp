#ifndef MUJOCO_SIMULATION_QT_MAINWINDOW_H
#define MUJOCO_SIMULATION_QT_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

#include "mujocoopenglwidget.hpp"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {

        char error[1000] = "Could not load binary model";
        auto m = mj_loadXML("/home/admin1/CLionProjects/MuJoCoDemo/assets/hello.xml", nullptr, error, 1000);
        if (!m) {
            qCritical() << "Load model error:" << error;
            abort();
        }

        auto d = mj_makeData(m);

        mjvCamera cam; // MuJoCo camera
        mjvOption opt; // Visualization options
        mjvScene scn; // Scene for rendering
        mjrContext con; // Rendering context

        mjv_defaultFreeCamera(m, &cam);
        mjv_defaultOption(&opt);
        mjr_defaultContext(&con);
        mjv_makeScene(m, &scn, MAX_GEOM); // Allocate scene

        auto *widget = new MuJoCoOpenGLWidget(m, d, cam, opt, scn, con,
                                              this);
        setCentralWidget(widget);

        auto *openAction = new QAction("&Open", this);
        connect(openAction, &QAction::triggered, [widget,this](){
            QString fileName = QFileDialog::getOpenFileName(this, "Open Model File", "", "Model Files (*.xml *.mjb)");
            if (fileName.isEmpty()) {
                return;
            }
            widget->loadModel(fileName);
        });

        auto *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction(openAction);

        // Simulation menu
        auto *simulationMenu = menuBar()->addMenu("&Simulation");

        // Pause action
        auto *pauseAction = new QAction("&Pause", this);
        pauseAction->setCheckable(true);
        simulationMenu->addAction(pauseAction);
        connect(pauseAction, &QAction::triggered, widget, [widget, pauseAction]() {
            widget->pauseSimulation(pauseAction->isChecked());
        });

        // Reset action
        auto *resetAction = new QAction("&Reset", this);
        simulationMenu->addAction(resetAction);
        connect(resetAction, &QAction::triggered, widget, &MuJoCoOpenGLWidget::resetSimulation);
    }

};

#endif //MUJOCO_SIMULATION_QT_MAINWINDOW_H
