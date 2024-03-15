#ifndef MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP
#define MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP

#include <QOpenGLWidget>
#include <mujoco/mujoco.h>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QThread>
#include <QList>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

#include <QtLogging>

#include <memory>


#include "simulation_worker.hpp"

static constexpr int MAX_GEOM = 2000;

class MuJoCoOpenGLWidget : public QOpenGLWidget {
Q_OBJECT

public:
    /**
     * All structs are trivially-copiable (although it's a bit expensive).
     */
    MuJoCoOpenGLWidget(mjModel *m, mjData *d, mjvCamera cam, mjvOption opt, mjvScene scn, mjrContext con,
                       QWidget *parent = nullptr, int fps = 60) : QOpenGLWidget(parent),
                                                                  simulationWorker(m, d, fps),
                                                                  cam(cam),
                                                                  opt(opt),
                                                                  scn(scn),
                                                                  con(con) {

        simulationThread = std::thread([&]() { simulationWorker.startSimulationLoop(); });
        renderTimer.setInterval(1000 / fps);
        connect(&renderTimer, &QTimer::timeout, this, [this]() {
            update(); // Schedule a new frame for rendering
        });
        renderTimer.start();

        setAcceptDrops(true);
    }

    explicit MuJoCoOpenGLWidget(mjvCamera cam, mjvOption opt, mjvScene scn, mjrContext con, QWidget *parent = nullptr,
                                int fps = 60)
            : QOpenGLWidget(parent),
              simulationWorker(nullptr, nullptr, fps),
              cam(cam),
              opt(opt),
              scn(scn),
              con(con) {

        // Initialize the render timer
        renderTimer.setInterval(1000 / fps);
        connect(&renderTimer, &QTimer::timeout, this, [this]() {
            update();
        });
        renderTimer.start();

        setAcceptDrops(true);
    }

    ~MuJoCoOpenGLWidget() override {
        simulationWorker.terminateSimulation();
        if (simulationThread.joinable()) {
            simulationThread.join();
        }
    }

public slots:

    void pauseSimulation(bool pause) {
        simulationWorker.setSimulationPaused(pause);
    }

    void resetSimulation() {
        simulationWorker.resetSimulation();
        update(); // Trigger a redraw to reflect the reset state
    }

    void loadModel(const QString &filename) {
        // Attempt to load the new model first without altering the current state
        char error[1000] = "Could not load binary model";
        mjModel *newModel = mj_loadXML(filename.toStdString().c_str(), nullptr, error, sizeof(error));
        if (!newModel) {
            qCritical() << "Load model error:" << error;
            QMessageBox::critical(this, "Error", "Could not load the model.");
            return; // Return without changing the current model and data
        }

        simulationWorker.replace(newModel, &cam);
        if (!simulationThread.joinable()) {
            simulationThread = std::thread([&]() { simulationWorker.startSimulationLoop(); });
        }

        initializeGL();

        // Trigger a redraw to reflect the new model
        update();
    }


protected:
    void initializeGL() override {
        simulationWorker.makeContext(&con);
    }

    void paintGL() override {
        if (simulationWorker.isModelDataNull()) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            return;
        }

        simulationWorker.updateScene(&opt, &cam, &scn);
        mjrRect viewport = {0, 0, static_cast<int>(width() * devicePixelRatio()),
                            static_cast<int>(height() * devicePixelRatio())};
        mjr_render(viewport, &scn, &con);
    }


    void resizeGL(int w, int h) override {
        // Update viewport here if necessary
    }

    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) {
            QList<QUrl> urls = event->mimeData()->urls();
            for (const QUrl &url: urls) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo = QFileInfo(filePath);
                if (fileInfo.suffix().compare("xml", Qt::CaseInsensitive) != 0) {
                    // Not an xml file, ignore it
                    event->ignore();
                    return;
                }
            }
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent *event) override {
        const QMimeData *mimeData = event->mimeData();

        if (mimeData->hasUrls()) {
            QUrl url = mimeData->urls().first();
            QString filePath = url.toLocalFile();
            loadModel(filePath);
            event->acceptProposedAction();
        }
    }

private:
    SimulationWorker simulationWorker;


    mjvCamera cam; // MuJoCo camera
    mjvOption opt; // Visualization options
    mjvScene scn; // Scene for rendering
    mjrContext con; // Rendering context

    QTimer renderTimer;

    std::thread simulationThread;
};

#endif //MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP
