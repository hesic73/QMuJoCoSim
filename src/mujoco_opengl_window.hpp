#ifndef QMUJOCOSIM_MUJOCO_OPENGL_WINDOW_HPP
#define QMUJOCOSIM_MUJOCO_OPENGL_WINDOW_HPP

#include <QOpenGLWindow>
#include <mujoco/mujoco.h>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QThread>

#include <QWheelEvent>
#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <QtLogging>

#include <memory>
#include <cstring>
#include <algorithm>
#include <array>


#include "simulation_worker.hpp"

#include "utils.hpp"

static constexpr int MAX_GEOM = 2000;

class MuJoCoOpenGLWindow : public QOpenGLWindow {
Q_OBJECT

public:
    /**
     * All structs are trivially-copiable (although copying is a bit expensive).
     */
    explicit MuJoCoOpenGLWindow(mjvCamera cam, mjvOption opt, mjvPerturb pert, mjrContext con,
                                int fps = 60)
            : QOpenGLWindow(),
              simulationWorker(nullptr, nullptr, fps),
              cam(cam),
              opt(opt),
              pert(pert),
              con(con) {

        mjv_defaultPerturb(&pert);
        mjv_defaultScene(&scn);
        mjv_makeScene(nullptr, &scn, MAX_GEOM); // Allocate scene
        std::copy(scn.flags, scn.flags + mjtRndFlag::mjNRNDFLAG, renderingEffects);

        // Initialize the render timer
        renderTimer.setInterval(1000 / fps);
        connect(&renderTimer, &QTimer::timeout, this, [this]() {
            update();
        });
        renderTimer.start();

    }

    ~MuJoCoOpenGLWindow() override {
        simulationWorker.terminateSimulation();
        if (simulationThread.joinable()) {
            simulationThread.join();
        }

        mjv_freeScene(&scn);
        mjr_freeContext(&con);
    }

    bool getRenderingFlag(mjtRndFlag flag) const {
        return renderingEffects[flag];
    }

    bool getModelElementsFlag(mjtVisFlag flag) const {
        return opt.flags[flag];
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
            emit loadModelFailure(simulationWorker.isModelDataNull());
            return; // Return without changing the current model and data
        }
        load_error.clear();

        mjv_makeScene(newModel, &scn, MAX_GEOM); // Allocate scene
        std::copy(renderingEffects, renderingEffects + mjtRndFlag::mjNRNDFLAG, scn.flags);

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

    void closeModel() {
        simulationWorker.terminateSimulation();
        if (simulationThread.joinable()) {
            simulationThread.join();
        }
        simulationWorker.close();

    }

    void saveXML(const QString &filename) {
        simulationWorker.save_xml(filename.toStdString());
    }

    void saveMJB(const QString &filename) {
        simulationWorker.save_mjb(filename.toStdString());
    }

    void printModel(const QString &filename) {
        simulationWorker.print_model(filename.toStdString());
    }

    void printData(const QString &filename) {
        simulationWorker.print_data(filename.toStdString());
    }

    void setRenderingFlag(mjtRndFlag flag, bool value) {
        renderingEffects[flag] = value;
        scn.flags[flag] = value;
    }

    void setModelElement(mjtVisFlag flag, bool value) {
        opt.flags[flag] = value;
    }

    /**
     * change the slowdown level.
     * @param increment +1/-1
     */
    void changeSlowDown(int increment) {

        if (simulationWorker.isModelDataNull()) {
            return;
        }

        static constexpr int lengthOfPercentRealTime = static_cast<int>(percentRealTime.size());

        slowdown_index = std::clamp(slowdown_index + increment, 0, lengthOfPercentRealTime - 1);

        simulationWorker.setSlowdown(100 / percentRealTime[slowdown_index]);
    }

signals:

    void loadModelSuccess();

    void loadModelFailure(bool isNull);

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


        simulationWorker.updateScene(&opt, &pert, &cam, &scn);
        mjr_render(viewport, &scn, &con);

        if (simulationWorker.isPaused()) {
            mjr_overlay(mjFONT_BIG, mjGRID_TOP, viewport, "PAUSE", nullptr,
                        &con);
        }

        {
            float desiredRealtime = percentRealTime[slowdown_index];
            float actualRealtime = 100 / simulationWorker.getMeasuredSlowDown();

            // if running, check for misalignment of more than 10%
            float realtime_offset = mju_abs(actualRealtime - desiredRealtime);
            bool misaligned = (!simulationWorker.isPaused()) && realtime_offset > 0.1 * desiredRealtime;

            // make realtime overlay label
            char rtlabel[30] = {'\0'};
            if (desiredRealtime != 100.0 || misaligned) {
                // print desired realtime
                int labelsize = std::snprintf(rtlabel, sizeof(rtlabel), "%g%%", desiredRealtime);

                // if misaligned, append to label
                if (misaligned) {
                    std::snprintf(rtlabel + labelsize, sizeof(rtlabel) - labelsize, " (%-4.1f%%)", actualRealtime);
                }
            }

            // show real-time overlay
            if (rtlabel[0]) {
                mjr_overlay(mjFONT_BIG, mjGRID_TOPLEFT, viewport, rtlabel, nullptr,
                            &con);
            }
        }

    }


    void resizeGL(int w, int h) override {
        // Update viewport here if necessary
    }


    void wheelEvent(QWheelEvent *event) override {
        constexpr mjtNum zoom_increment = 0.02;
        auto delta = event->angleDelta().y() / 120;
        simulationWorker.moveCamera(mjtMouse::mjMOUSE_ZOOM, 0, delta * zoom_increment, &scn, &cam);
    }


    void mousePressEvent(QMouseEvent *event) override {
        // Start drag on mouse press
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton ||
            event->button() == Qt::MiddleButton) {
            dragging = true;
            dragStartPosition = event->pos();
            dragButton = event->button();
            modifiers = event->modifiers();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        dragging = false;
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (!dragging) return;

        // Calculate the distance the mouse has moved since the press event.
        auto delta = (event->pos() - dragStartPosition);
        int distance = delta.manhattanLength();
        if (distance < QApplication::startDragDistance()) {
            return;
        }

        auto action = get_mjtMouse(dragButton, modifiers);

        auto relX = 1.0 * delta.x() / width();
        auto relY = 1.0 * delta.y() / height();

        simulationWorker.moveCamera(action, relX, relY, &scn, &cam);

        dragStartPosition = event->pos();
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

    mjtByte renderingEffects[mjtRndFlag::mjNRNDFLAG];


    bool dragging = false;
    QPoint dragStartPosition;
    Qt::MouseButton dragButton;
    Qt::KeyboardModifiers modifiers;

    static constexpr std::array<float, 31> percentRealTime = {
            100, 80, 66, 50, 40, 33, 25, 20, 16, 13,
            10, 8, 6.6, 5.0, 4, 3.3, 2.5, 2, 1.6, 1.3,
            1, .8, .66, .5, .4, .33, .25, .2, .16, .13,
            .1
    };

    int slowdown_index = 0;
};

#endif //QMUJOCOSIM_MUJOCO_OPENGL_WINDOW_HPP
