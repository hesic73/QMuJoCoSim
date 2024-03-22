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
#include <QApplication>
#include <QIcon>
#include <QScrollArea>

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


        muJoCoOpenGlWindow = new MuJoCoOpenGLWindow(cam, opt, pert, con);


        auto scrollArea = new QScrollArea(this);
        controlPanel = new ControlPanel(scrollArea);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFixedWidth(250);
        scrollArea->setWidget(controlPanel);

        auto layout = new QHBoxLayout;
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(scrollArea);
        layout->addWidget(new MyWindowContainer(muJoCoOpenGlWindow));

        initializeRenderingEffectsButtonsChecked();

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
        openAction->setShortcut(QKeySequence("Ctrl+O"));

        closeAction = new QAction("Close", this);
        connect(closeAction, &QAction::triggered, [this]() {
            muJoCoOpenGlWindow->closeModel();
            actionSetEnabledWhenModelIsNull();
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

        auto quitAction = new QAction("Quit", this);
        connect(quitAction, &QAction::triggered, []() {
            QApplication::quit();
        });
        quitAction->setShortcut(QKeySequence("Ctrl+Q"));

        auto *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction(openAction);
        fileMenu->addAction(closeAction);
        fileMenu->addSeparator();
        fileMenu->addAction(screenshotAction);
        fileMenu->addSeparator();
        fileMenu->addAction(saveXMLAction);
        fileMenu->addAction(saveMJBAction);
        fileMenu->addSeparator();
        fileMenu->addAction(quitAction);


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
        pauseAction->setShortcut(QKeySequence(Qt::Key_Space));

        // Reset action
        resetAction = new QAction("&Reset", this);
        simulationMenu->addAction(resetAction);
        connect(resetAction, &QAction::triggered, muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::resetSimulation);

        resetAction->setShortcut(QKeySequence("Ctrl+R"));


        connect(muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::loadModelSuccess, [this]() {
            actionSetEnabledWhenModelIsNotNull();
        });

        connect(muJoCoOpenGlWindow, &MuJoCoOpenGLWindow::loadModelFailure, [this](bool isNull) {
            if (isNull)
                actionSetEnabledWhenModelIsNull();
            else
                actionSetEnabledWhenModelIsNotNull();
        });

        actionSetEnabledWhenModelIsNull();
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
    void initializeRenderingEffectsButtonsChecked() {
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_SHADOW,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_SHADOW));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_WIREFRAME,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_WIREFRAME));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_REFLECTION,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_REFLECTION));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_ADDITIVE,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_ADDITIVE));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_SKYBOX,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_SKYBOX));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_FOG,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_FOG));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_HAZE,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_HAZE));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_SEGMENT,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_SEGMENT));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_IDCOLOR,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_IDCOLOR));
        controlPanel->renderingSection->setRenderingEffectsButtonChecked(mjtRndFlag::mjRND_CULL_FACE,
                                                                         muJoCoOpenGlWindow->getRenderingFlag(
                                                                                 mjtRndFlag::mjRND_CULL_FACE));


        connect(controlPanel->renderingSection, &RenderingSection::updateRenderingFlag,
                [this](mjtRndFlag flag, bool value) {
                    muJoCoOpenGlWindow->setRenderingFlag(flag, value);
                });
    }

    void actionSetEnabledWhenModelIsNull() {
        closeAction->setEnabled(false);

        screenshotAction->setEnabled(false);
        saveXMLAction->setEnabled(false);
        saveMJBAction->setEnabled(false);

        pauseAction->setEnabled(false);
        resetAction->setEnabled(false);
    }

    void actionSetEnabledWhenModelIsNotNull() {
        closeAction->setEnabled(true);

        screenshotAction->setEnabled(true);
        saveXMLAction->setEnabled(true);
        saveMJBAction->setEnabled(true);

        pauseAction->setEnabled(true);
        resetAction->setEnabled(true);
    }

    MuJoCoOpenGLWindow *muJoCoOpenGlWindow;
    ControlPanel *controlPanel;

    QAction *closeAction;
    QAction *screenshotAction;
    QAction *saveXMLAction;
    QAction *saveMJBAction;

    QAction *pauseAction;
    QAction *resetAction;
};

#endif //MUJOCO_SIMULATION_QT_MAINWINDOW_H
