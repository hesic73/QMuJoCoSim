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
#include <QHBoxLayout>

#include "mujoco_opengl_window.hpp"
#include "my_window_container.hpp"
#include "control_panel.hpp"

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
        controlPanel = new ControlPanel(this);

        auto layout = new QHBoxLayout;
        layout->addWidget(controlPanel);
        layout->addWidget(new MyWindowContainer(muJoCoOpenGlWindow));

        auto widget = new QWidget(this);
        widget->setLayout(layout);
        setCentralWidget(widget);

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

        screenshotAction = new QAction("Screenshot", this);
        connect(screenshotAction, &QAction::triggered, [this]() {
            shootScreen();
        });

        saveXMLAction = new QAction("Save XML", this);
        connect(saveXMLAction, &QAction::triggered, [this]() {
            auto dirPath = QDir::currentPath();// to do: configure the output directory
            auto dir = QDir(dirPath);
            auto fullPath = dir.filePath("mjmodel.xml");
            qDebug() << QString("Attempting to save the model in XML format to the following path: '%1'").arg(fullPath);
            muJoCoOpenGlWindow->saveXML(fullPath);
        });


        saveMJBAction = new QAction("Save MJB", this);
        connect(saveMJBAction, &QAction::triggered, [this]() {
            auto dirPath = QDir::currentPath();// to do: configure the output directory
            auto dir = QDir(dirPath);
            auto fullPath = dir.filePath("mjmodel.mjb");
            qDebug() << QString("Attempting to save the model in MJB format to the following path: '%1'").arg(fullPath);
            muJoCoOpenGlWindow->saveMJB(fullPath);
        });


        auto *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction(openAction);
        fileMenu->addSeparator();
        fileMenu->addAction(screenshotAction);
        fileMenu->addAction(saveXMLAction);
        fileMenu->addAction(saveMJBAction);

        fileMenu->setStyleSheet("QMenu::item:disabled { background-color: #f0f0f0; color: #a0a0a0; }");


        // Simulation menu
        auto *simulationMenu = menuBar()->addMenu("&Simulation");
        simulationMenu->setStyleSheet("QMenu::item:disabled { background-color: #f0f0f0; color: #a0a0a0; }");

        // Pause action
        pauseAction = new QAction("&Pause", this);
        pauseAction->setCheckable(true);
        simulationMenu->addAction(pauseAction);
        connect(pauseAction, &QAction::triggered, muJoCoOpenGlWindow, [this]() {
            muJoCoOpenGlWindow->pauseSimulation(pauseAction->isChecked());
        });

        // Reset action
        resetAction = new QAction("&Reset", this);
        simulationMenu->addAction(resetAction);
        connect(resetAction, &QAction::triggered, muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::resetSimulation);\


        connect(muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::loadModelSuccess, [this]() {
            enableActions();
        });

        connect(muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::loadModelFailure, [this]() {
            disableActions();
        });

        disableActions();
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
    void disableActions() {
        screenshotAction->setEnabled(false);
        saveXMLAction->setEnabled(false);
        saveMJBAction->setEnabled(false);

        pauseAction->setEnabled(false);
        resetAction->setEnabled(false);
    }

    void enableActions() {
        screenshotAction->setEnabled(true);
        saveXMLAction->setEnabled(true);
        saveMJBAction->setEnabled(true);

        pauseAction->setEnabled(true);
        resetAction->setEnabled(true);
    }

    MuJoCoOpenGLWindow *muJoCoOpenGlWindow;
    ControlPanel *controlPanel;

    QAction *screenshotAction;
    QAction *saveXMLAction;
    QAction *saveMJBAction;

    QAction *pauseAction;
    QAction *resetAction;
};

#endif //MUJOCO_SIMULATION_QT_MAINWINDOW_H
