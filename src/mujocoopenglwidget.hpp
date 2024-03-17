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
     * All structs are trivially-copiable (although copying is a bit expensive).
     */
    explicit MuJoCoOpenGLWidget(mjvCamera cam, mjvOption opt, mjvPerturb pert, mjrContext con,
                                QWidget *parent = nullptr,
                                int fps = 60)
            : QOpenGLWidget(parent),
              simulationWorker(nullptr, nullptr, fps),
              cam(cam),
              opt(opt),
              pert(pert),
              con(con) {

        mjv_defaultScene(&scn);
        mjv_makeScene(nullptr, &scn, MAX_GEOM); // Allocate scene


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
        isLoading = true;

        // std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        // Attempt to load the new model first without altering the current state
        char error[1000] = "Could not load binary model";
        mjModel *newModel = mj_loadXML(filename.toStdString().c_str(), nullptr, error, sizeof(error));
        if (!newModel) {
            load_error = error;
            qCritical() << "Load model error:" << error;
            isLoading = false;
            return; // Return without changing the current model and data
        }
        load_error.clear();

        mjv_makeScene(newModel, &scn, MAX_GEOM); // Allocate scene


        simulationWorker.replace(newModel);
        if (!simulationThread.joinable()) {
            simulationThread = std::thread([&]() { simulationWorker.startSimulationLoop(); });
        }

        mjv_defaultCamera(&cam);


        isLoading = false;

        initializeGL();



        // Trigger a redraw to reflect the new model
        update();
    }


protected:
    void initializeGL() override {
        simulationWorker.makeContext(&con);
    }

    void paintGL() override {
        mjrRect viewport = {0, 0, static_cast<int>(width() * devicePixelRatio()),
                            static_cast<int>(height() * devicePixelRatio())};


        if (simulationWorker.isModelDataNull()) {
            mjr_rectangle(viewport, 0.2f, 0.3f, 0.4f, 1);

            // Currently, `loadModel` executes in the main loop, rendering the loading flag unused.
            // Moving `loadModel` to a separate thread triggers an error: "ERROR: Could not allocate offscreen framebuffer".
            if (isLoading) {
                mjr_overlay(mjFONT_BIG, mjGRID_TOP, viewport, "LOADING...", nullptr,
                            &con);
            } else {
                QString info = QString("MuJoCo version %1\nDrag-and-drop model file here").arg(mj_versionString());
                mjr_overlay(mjFONT_NORMAL, mjGRID_TOPLEFT, viewport, info.toStdString().c_str(), nullptr,
                            &con);
            }

            if (!load_error.isEmpty()) {
                mjr_overlay(mjFONT_NORMAL, mjGRID_BOTTOMLEFT, viewport, load_error.toStdString().c_str(), nullptr,
                            &con);
            }

            return;
        }

        simulationWorker.updateScene(&opt, &cam, &scn);
        mjr_render(viewport, &scn, &con);

        if (simulationWorker.isPaused()) {
            mjr_overlay(mjFONT_BIG, mjGRID_TOP, viewport, "PAUSE", nullptr,
                        &con);
        }
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
    mjvPerturb pert;
    mjvScene scn; // Scene for rendering
    mjrContext con; // Rendering context

    QTimer renderTimer;

    std::thread simulationThread;


    std::atomic_bool isLoading = false;
    QString load_error;
};

#endif //MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP
