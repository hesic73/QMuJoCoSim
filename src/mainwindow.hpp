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


        mjvCamera cam; // MuJoCo camera
        mjvOption opt; // Visualization options
        mjvScene scn; // Scene for rendering
        mjrContext con; // Rendering context

        mjv_defaultOption(&opt);
        mjr_defaultContext(&con);
        mjv_defaultScene(&scn);
        mjv_makeScene(nullptr, &scn, MAX_GEOM); // Allocate scene

        auto *widget = new MuJoCoOpenGLWidget(cam, opt, scn, con,
                                              this);
        setCentralWidget(widget);

        auto *openAction = new QAction("&Open", this);
        connect(openAction, &QAction::triggered, [widget, this]() {
            QString defaultDir = QDir::currentPath(); // Get the current working directory
            QString fileName = QFileDialog::getOpenFileName(this, "Open Model File", defaultDir, "Model Files (*.xml *.mjb)");
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
