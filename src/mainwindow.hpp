#ifndef QMUJOCOSIM_MAINWINDOW_H
#define QMUJOCOSIM_MAINWINDOW_H

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
        initializeModeElementsButtonsChecked();

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

            updateControlPanelWhenModelIsNull();
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


        printModelAction = new QAction("Print Model", this);
        connect(printModelAction, &QAction::triggered, [this]() {
            auto dirPath = QDir::currentPath();// to do: configure the output directory
            auto dir = QDir(dirPath);
            auto fullPath = dir.filePath("MJMODEL.TXT");
            muJoCoOpenGlWindow->printModel(fullPath);
        });

        printDataAction = new QAction("Print Data", this);
        connect(printDataAction, &QAction::triggered, [this]() {
            auto dirPath = QDir::currentPath();// to do: configure the output directory
            auto dir = QDir(dirPath);
            auto fullPath = dir.filePath("MJDATA.TXT");
            muJoCoOpenGlWindow->printData(fullPath);
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
        fileMenu->addAction(printModelAction);
        fileMenu->addAction(printDataAction);
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
            updateControlPanelWhenModelIsNotNull();
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

        for (int i = 0; i < mjtRndFlag::mjNRNDFLAG; i++) {
            controlPanel->renderingSection->setRenderingEffectsButtonChecked(static_cast<mjtRndFlag>(i),
                                                                             muJoCoOpenGlWindow->getRenderingFlag(
                                                                                     static_cast<mjtRndFlag>(i)));
        }

        connect(controlPanel->renderingSection, &RenderingSection::updateRenderingFlag,
                [this](mjtRndFlag flag, bool value) {
                    muJoCoOpenGlWindow->setRenderingFlag(flag, value);
                });
    }


    void initializeModeElementsButtonsChecked() {

        for (int i = 0; i < mjtVisFlag::mjNVISFLAG; i++) {
            controlPanel->renderingSection->setModelElementsButtonChecked(static_cast<mjtVisFlag>(i),
                                                                          muJoCoOpenGlWindow->getModelElementsFlag(
                                                                                  static_cast<mjtVisFlag>(i)));
        }

        connect(controlPanel->renderingSection, &RenderingSection::updateModelElementsFlag,
                [this](mjtVisFlag flag, bool value) {
                    muJoCoOpenGlWindow->setModelElement(flag, value);
                });
    }

    void updateControlPanelWhenModelIsNull() {
        controlPanel->simulationSection->resetWhenModelIsNull();
        controlPanel->renderingSection->hide();
    }

    void updateControlPanelWhenModelIsNotNull() {
        controlPanel->renderingSection->show();
        controlPanel->simulationSection->resetWhenModelIsNotNull(muJoCoOpenGlWindow->getSimulationHistoryBufferSize(),
                                                                 [](int value) {
                                                                     qDebug() << value;
                                                                 });
    }

    void actionSetEnabledWhenModelIsNull() {
        closeAction->setEnabled(false);

        screenshotAction->setEnabled(false);
        saveXMLAction->setEnabled(false);
        saveMJBAction->setEnabled(false);

        printModelAction->setEnabled(false);
        printDataAction->setEnabled(false);

        pauseAction->setEnabled(false);
        resetAction->setEnabled(false);
    }

    void actionSetEnabledWhenModelIsNotNull() {
        closeAction->setEnabled(true);

        screenshotAction->setEnabled(true);
        saveXMLAction->setEnabled(true);
        saveMJBAction->setEnabled(true);

        printModelAction->setEnabled(true);
        printDataAction->setEnabled(true);

        pauseAction->setEnabled(true);
        resetAction->setEnabled(true);
    }

    MuJoCoOpenGLWindow *muJoCoOpenGlWindow;
    ControlPanel *controlPanel;

    QAction *closeAction;
    QAction *screenshotAction;
    QAction *saveXMLAction;
    QAction *saveMJBAction;

    QAction *printModelAction;
    QAction *printDataAction;

    QAction *pauseAction;
    QAction *resetAction;
};

#endif //QMUJOCOSIM_MAINWINDOW_H
