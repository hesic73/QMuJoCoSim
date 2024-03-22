#ifndef MUJOCO_SIMULATION_QT_SIMULATION_WORKER_HPP
#define MUJOCO_SIMULATION_QT_SIMULATION_WORKER_HPP

#include <iostream>
#include <chrono>

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "mujoco/mujoco.h"

class SimulationWorker {
public:
    SimulationWorker(mjModel *model, mjData *data, int fps = 60)
            :
            isSimulationPaused(false),
            terminateRequested(false),
            m(model),
            d(data),
            FPS(fps),
            dt(std::chrono::milliseconds{1000 / fps}) {}

    ~SimulationWorker() {
        // Signal for simulation to terminate
        terminateSimulation();

        // Proceed with cleanup
        std::lock_guard<std::mutex> lock(mtx); // Ensure exclusive access during cleanup
        cleanup();
    }

    void startSimulationLoop() {
        std::cout << "startSimulationLoop begins." << std::endl;

        terminateRequested = false;

        std::chrono::time_point<std::chrono::high_resolution_clock> begin;

        while (!terminateRequested) {

            {
                std::unique_lock<std::mutex> uniqueLock(mtx);
                while (isSimulationPaused) {
                    cv_pause.wait(uniqueLock);
                }

                const mjtNum simStart = d->time;
                begin = std::chrono::high_resolution_clock::now();
                while (d->time - simStart < 1.0 / FPS) {
                    mj_step(m, d);
                }
            }

            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

            // std::cout << "Duration: " << duration.count() << "ms" << std::endl;

            if (duration < dt) {
                std::this_thread::sleep_for(dt - duration);
            } else {
                std::cout << "Simulation pace is lagging behind real-time progression." << std::endl;
            }

        }

        std::cout << "startSimulationLoop ends." << std::endl;
    }

    void terminateSimulation() {
        terminateRequested = true;
        setSimulationPaused(false);
    }

    void setSimulationPaused(bool pause) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        isSimulationPaused = pause;
        if (!pause) {
            cv_pause.notify_one();
        }
    }

    void resetSimulation() {
        std::lock_guard<std::mutex> lockGuard(mtx);
        mj_resetData(m, d);
    }

    void makeContext(mjrContext *con) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        mjr_makeContext(m, con, mjFONTSCALE_100);
    }

    void updateScene(mjvOption *opt, mjvCamera *cam, mjvScene *scn) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        mjv_updateScene(m, d, opt, nullptr, cam, mjCAT_ALL, scn);
    }

    bool isModelDataNull() {
        std::lock_guard<std::mutex> lockGuard(mtx);
        return m == nullptr || d == nullptr;
    }

    // While access to `isSimulationPaused` is not thread-safe,
    // its simple boolean nature minimizes the risk of serious concurrency issues.
    bool isPaused() const { return isSimulationPaused; }

    void replace(mjModel *newModel) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        cleanup();
        m = newModel;
        d = mj_makeData(m);
    }

    void close() {
        std::lock_guard<std::mutex> lockGuard(mtx);
        cleanup();
    }

    void save_xml(const std::string &filename) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        if (m == nullptr) {
            std::cout << "Skipping save operation: 'm' is not initialized (nullptr)." << std::endl;
            return;
        }
        char err[200] = {0};
        if (mj_saveLastXML(filename.c_str(), m, err, 200) == 0) {
            std::cout << "Save XML error: " << err << std::endl;
        }
    }

    void save_mjb(const std::string &filename) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        if (m == nullptr) {
            std::cout << "Skipping save operation: 'm' is not initialized (nullptr)." << std::endl;
            return;
        }
        mj_saveModel(m, filename.c_str(), nullptr, 0);
    }

private:
    void cleanup() {
        if (d) {
            mj_deleteData(d);
            d = nullptr;
        }
        if (m) {
            mj_deleteModel(m);
            m = nullptr;
        }
    }


    bool isSimulationPaused;
    std::atomic_bool terminateRequested;
    mjModel *m;
    mjData *d;

    const int FPS;
    const std::chrono::milliseconds dt;

    std::mutex mtx;
    std::condition_variable cv_pause;

};

#endif //MUJOCO_SIMULATION_QT_SIMULATION_WORKER_HPP
