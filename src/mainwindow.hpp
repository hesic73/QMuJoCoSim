#ifndef MUJOCO_SIMULATION_QT_MAINWINDOW_H
#define MUJOCO_SIMULATION_QT_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QPixmap>
#include <QStandardPaths>
#include <QImageWriter>
#include <QGuiApplication>
#include <QScreen>

#include "mujoco_opengl_window.hpp"
#include "my_window_container.hpp"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {


        mjvCamera cam;
        mjvOption opt;
        mjvPerturb pert;
        mjrContext con; // Rendering context

        mjv_defaultCamera(&cam);
        mjv_defaultOption(&opt);
        mjv_defaultPerturb(&pert);

        // it's crucial to initialize the context before we create the widget (but I don't know why).
        mjr_defaultContext(&con);


        muJoCoOpenGlWindow = new MuJoCoOpenGLWindow(cam, opt, pert, con,
                                                    this);
        setCentralWidget(new MyWindowContainer(muJoCoOpenGlWindow));

        auto *openAction = new QAction("&Open", this);
        connect(openAction, &QAction::triggered, [this]() {
            QString defaultDir = QDir::currentPath(); // Get the current working directory
            QString fileName = QFileDialog::getOpenFileName(this, "Open Model File", defaultDir,
                                                            "Model Files (*.xml *.mjb)");
            if (fileName.isEmpty()) {
                return;
            }
            muJoCoOpenGlWindow->loadModel(fileName);
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
        connect(pauseAction, &QAction::triggered, muJoCoOpenGlWindow, [this, pauseAction]() {
            muJoCoOpenGlWindow->pauseSimulation(pauseAction->isChecked());
        });

        // Reset action
        auto *resetAction = new QAction("&Reset", this);
        simulationMenu->addAction(resetAction);
        connect(resetAction, &QAction::triggered, muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::resetSimulation);\

        setAcceptDrops(true);
    }

private slots:

    void shootScreen() {
        QScreen *screen = QGuiApplication::primaryScreen();
        QPixmap pixmap = screen->grabWindow(muJoCoOpenGlWindow->winId());
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


protected:

private:
    MuJoCoOpenGLWindow *muJoCoOpenGlWindow;
};

#endif //MUJOCO_SIMULATION_QT_MAINWINDOW_H
