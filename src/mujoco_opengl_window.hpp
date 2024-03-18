#ifndef MUJOCO_SIMULATION_QT_MUJOCO_OPENGL_WINDOW_HPP
#define MUJOCO_SIMULATION_QT_MUJOCO_OPENGL_WINDOW_HPP

#include <QOpenGLWindow>
#include <mujoco/mujoco.h>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QThread>


#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <QtLogging>

#include <memory>
#include <cstring>


#include "simulation_worker.hpp"

static constexpr int MAX_GEOM = 2000;

class MuJoCoOpenGLWindow : public QOpenGLWindow {
Q_OBJECT

public:
    /**
     * All structs are trivially-copiable (although copying is a bit expensive).
     */
    explicit MuJoCoOpenGLWindow(mjvCamera cam, mjvOption opt, mjvPerturb pert, mjrContext con,
                                QWidget *parent = nullptr,
                                int fps = 60)
            : QOpenGLWindow(),
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

        // setAcceptDrops(true);
    }

    ~MuJoCoOpenGLWindow() override {
        simulationWorker.terminateSimulation();
        if (simulationThread.joinable()) {
            simulationThread.join();
        }

        mjv_freeScene(&scn);
        mjr_freeContext(&con);
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
        mjModel *newModel = nullptr;

        isLoading = true;

        // std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        // Attempt to load the new model first without altering the current state
        char error[1000] = "Could not load binary model";
        QFileInfo fileInfo = QFileInfo(filename);
        if (fileInfo.suffix().compare("xml", Qt::CaseInsensitive) == 0) {
            newModel = mj_loadXML(filename.toStdString().c_str(), nullptr, error, sizeof(error));
        } else if (fileInfo.suffix().compare("mjb", Qt::CaseInsensitive) == 0) {
            newModel = mj_loadModel(filename.toStdString().c_str(), nullptr);
        } else {
            qWarning()
                    << "loadModel method supports only '.xml' and '.mjb' file formats. The provided file path does not match these formats.";

            std::strcpy(error,
                        "loadModel method supports only '.xml' and '.mjb' file formats. The provided file path does not match these formats.");
        }

        if (!newModel) {
            load_error = error;
            qCritical() << "Load model error:" << error;
            isLoading = false;
            emit loadModelFailure();
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


        emit loadModelSuccess();

        // Trigger a redraw to reflect the new model
        update();

        return;
    }

    void saveXML(const QString &filename) {
        simulationWorker.save_xml(filename.toStdString());
    }

    void saveMJB(const QString &filename) {
        simulationWorker.save_mjb(filename.toStdString());
    }

signals:

    void loadModelSuccess();

    void loadModelFailure();

protected:
    void initializeGL() override {
        simulationWorker.makeContext(&con);
    }

    void paintGL() override {

        auto f = QOpenGLContext::currentContext()->functions();
        Q_ASSERT(f->hasOpenGLFeature(QOpenGLFunctions::OpenGLFeature::FixedFunctionPipeline));


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

#endif //MUJOCO_SIMULATION_QT_MUJOCO_OPENGL_WINDOW_HPP
