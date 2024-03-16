#ifndef MUJOCO_SIMULATION_QT_MAINWINDOW_H
#define MUJOCO_SIMULATION_QT_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QPixmap>
#include <QStandardPaths>
#include <QImageWriter>

#include "mujocoopenglwidget.hpp"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {


        mjvCamera cam; // MuJoCo camera
        mjvOption opt; // Visualization options
        mjvScene scn; // Scene for rendering
        mjrContext con; // Rendering context

        mjv_defaultOption(&opt);
        mjr_defaultContext(&con);
        mjv_defaultScene(&scn);
        mjv_makeScene(nullptr, &scn, MAX_GEOM); // Allocate scene

        muJoCoOpenGlWidget = new MuJoCoOpenGLWidget(cam, opt, scn, con,
                                                    this);
        setCentralWidget(muJoCoOpenGlWidget);

        auto *openAction = new QAction("&Open", this);
        connect(openAction, &QAction::triggered, [this]() {
            QString defaultDir = QDir::currentPath(); // Get the current working directory
            QString fileName = QFileDialog::getOpenFileName(this, "Open Model File", defaultDir,
                                                            "Model Files (*.xml *.mjb)");
            if (fileName.isEmpty()) {
                return;
            }
            muJoCoOpenGlWidget->loadModel(fileName);
        });

        auto *screenshotAction = new QAction("Screenshot", this);
        connect(screenshotAction, &QAction::triggered, [this]() {
            shootScreen();
        });


        auto *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction(openAction);
        fileMenu->addSeparator();
        fileMenu->addAction(screenshotAction);

        // Simulation menu
        auto *simulationMenu = menuBar()->addMenu("&Simulation");

        // Pause action
        auto *pauseAction = new QAction("&Pause", this);
        pauseAction->setCheckable(true);
        simulationMenu->addAction(pauseAction);
        connect(pauseAction, &QAction::triggered, muJoCoOpenGlWidget, [this, pauseAction]() {
            muJoCoOpenGlWidget->pauseSimulation(pauseAction->isChecked());
        });

        // Reset action
        auto *resetAction = new QAction("&Reset", this);
        simulationMenu->addAction(resetAction);
        connect(resetAction, &QAction::triggered, muJoCoOpenGlWidget, &MuJoCoOpenGLWidget::resetSimulation);
    }

private slots:

    void shootScreen() {
        auto pixmap = muJoCoOpenGlWidget->grab();
        QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (path.isEmpty())
            path = QDir::currentPath();

        // Get current date and time
        QDateTime now = QDateTime::currentDateTime();
        // Format date and time to string with desired format
        QString dateTimeString = now.toString("yyyy_MM_dd_hh_mm_ss_zzz");

        // Append formatted date and time to filename
        path += QString("/screenshot_%1.png").arg(dateTimeString);
        if (!pixmap.save(path)) {
            QMessageBox::warning(this, tr("Save Error"), tr("The image could not be saved to \"%1\".")
                    .arg(QDir::toNativeSeparators(path)));
        } else {
            qDebug() << "Screenshot saved to" << QDir::toNativeSeparators(path);
        }
    };

private:
    MuJoCoOpenGLWidget *muJoCoOpenGlWidget;
};

#endif //MUJOCO_SIMULATION_QT_MAINWINDOW_H
