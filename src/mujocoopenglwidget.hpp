#ifndef MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP
#define MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP

#include <QOpenGLWidget>
#include <mujoco/mujoco.h>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>

static constexpr int MAX_GEOM = 2000;
static constexpr int FPS = 60;
static constexpr int MSEC_PER_FRAME = 1000 / FPS;

class MuJoCoOpenGLWidget : public QOpenGLWidget {
Q_OBJECT

public:
    /**
     * All structs are trivially-copiable (although it's a bit expensive).
     */
    MuJoCoOpenGLWidget(mjModel *m, mjData *d, mjvCamera cam, mjvOption opt, mjvScene scn, mjrContext con,
                       QWidget *parent = nullptr) : QOpenGLWidget(parent), m(m), d(d), cam(cam), opt(opt), scn(scn),
                                                    con(con) {
        renderTimer.setInterval(MSEC_PER_FRAME); // Aim for ~60 FPS for rendering
        connect(&renderTimer, &QTimer::timeout, this, [this]() {
            update(); // Schedule a new frame for rendering
        });
        renderTimer.start();
    }

    explicit MuJoCoOpenGLWidget(mjvCamera cam, mjvOption opt, mjvScene scn, mjrContext con, QWidget *parent = nullptr)
            : QOpenGLWidget(parent), m(nullptr),
              d(nullptr), cam(cam), opt(opt), scn(scn),
              con(con) {

        // Initialize the render timer
        renderTimer.setInterval(MSEC_PER_FRAME);
        connect(&renderTimer, &QTimer::timeout, this, [this]() {
            update();
        });
        renderTimer.start();
    }

    ~MuJoCoOpenGLWidget() override {
        QMutexLocker locker(&mutex);
        if (d) mj_deleteData(d);
        if (m) mj_deleteModel(m);
    }

public slots:

    void pauseSimulation(bool pause) {
        QMutexLocker locker(&mutex); // Ensure thread safety
        isSimulationPaused = pause;  // Directly use the provided boolean
    }

    void resetSimulation() {
        QMutexLocker locker(&mutex); // Lock the mutex for thread safety
        mj_resetData(m, d);
        update(); // Trigger a redraw to reflect the reset state
    }

    void loadModel(const QString &filename) {
        QMutexLocker locker(&mutex); // Lock the mutex to ensure thread safety

        // Attempt to load the new model first without altering the current state
        char error[1000] = "Could not load binary model";
        mjModel *newModel = mj_loadXML(filename.toStdString().c_str(), nullptr, error, sizeof(error));
        if (!newModel) {
            qCritical() << "Load model error:" << error;
            QMessageBox::critical(this, "Error", "Could not load the model.");
            return; // Return without changing the current model and data
        }

        // Delete the old model and data if a new model has been successfully loaded
        if (d) {
            mj_deleteData(d);
        }
        if (m) {
            mj_deleteModel(m);
        }

        // Use the newly loaded model and create new data for it
        m = newModel;
        d = mj_makeData(m);

        // Reinitialize the camera, options, and scene for the new model
        mjv_defaultFreeCamera(m, &cam);

        initializeGL();

        // Trigger a redraw to reflect the new model
        update();
    }


protected:
    void initializeGL() override {
        mjr_makeContext(m, &con, mjFONTSCALE_100); // Make context
    }

    void paintGL() override {
        QMutexLocker locker(&mutex);
        if (m == nullptr || d == nullptr) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            return;
        }

        runSimulationSteps();
        renderScene();
    }


    void resizeGL(int w, int h) override {
        // Update viewport here if necessary
    }

private:

    void runSimulationSteps() {

        if (isSimulationPaused) {
            return; // Skip simulation steps if paused
        }

        // Advance interactive simulation for 1/60 sec
        // Note: Assuming MuJoCo can simulate faster than real-time, which it usually can.
        mjtNum simstart = d->time;
        while (d->time - simstart < 1.0 / FPS) {
            mj_step(m, d);

            // Optionally, add a CPU time check here to avoid the simulation loop running too long
            // This would require starting a QElapsedTimer before entering the loop
            // and breaking out of the loop if too much real time has elapsed.
        }
    }

    void renderScene() {
        // Update scene and render
        mjv_updateScene(m, d, &opt, nullptr, &cam, mjCAT_ALL, &scn);
        mjrRect viewport = {0, 0, static_cast<int>(width() * devicePixelRatio()),
                            static_cast<int>(height() * devicePixelRatio())};
        mjr_render(viewport, &scn, &con);
    }

private:


    mjModel *m; // MuJoCo model
    mjData *d; // MuJoCo data

    mjvCamera cam; // MuJoCo camera
    mjvOption opt; // Visualization options
    mjvScene scn; // Scene for rendering
    mjrContext con; // Rendering context

    QTimer renderTimer; // Timer to ensure we attempt to render at 60 FPS
    bool isSimulationPaused = false; // Flag to pause simulation

    QMutex mutex; // Mutex for thread-safe operations
};

#endif //MUJOCO_SIMULATION_QT_MUJOCOOPENGLWIDGET_HPP
