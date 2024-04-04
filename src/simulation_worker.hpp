#ifndef QMUJOCOSIM_SIMULATION_WORKER_HPP
#define QMUJOCOSIM_SIMULATION_WORKER_HPP

#include <iostream>
#include <chrono>

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "mujoco/mujoco.h"

// MuJoCo官方的simulate的实现是：while loop每次busy wait/ sleep for 1ms
// 然后根据时间判断是否需要step

constexpr double syncMisalign = 0.1;


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
        std::cout << "Simulation loop starts." << std::endl;

        // CPU-sim synchronization point
        auto syncCPU = std::chrono::high_resolution_clock::now();
        mjtNum syncSim = 0;


        while (!terminateRequested.load()) {
            double elapsedSim;

            // Check if the simulation should be paused and wait if so
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv_pause.wait(lock, [&] { return !isSimulationPaused; });

                // Record CPU time at the start of the iteration
                const auto startCPU = std::chrono::high_resolution_clock::now();

                // Elapsed CPU and simulation time since last sync
                const auto elapsedCPU = startCPU - syncCPU;
                elapsedSim = d->time - syncSim;

                // Calculate if misalignment condition is met
                bool misaligned =
                        std::abs(std::chrono::duration<double>(elapsedCPU).count() / slowdown - elapsedSim) >
                        syncMisalign;

                // Out-of-sync (for any reason): reset sync times, step
                if (elapsedSim < 0 || elapsedCPU.count() < 0 || syncCPU.time_since_epoch().count() == 0 || misaligned) {
                    // Re-sync
                    syncCPU = startCPU;
                    syncSim = d->time;

                    // Run single step
                    mj_step(m, d);
                } else {
                    bool firstStep = true;

                    // In-sync: step until ahead of CPU
                    while (std::chrono::duration<double>(elapsedCPU).count() / slowdown > elapsedSim) {
                        mj_step(m, d);

                        // Update elapsed simulation time
                        double newElapsedSim = d->time - syncSim;

                        // Measure slowdown on the first step if elapsed simulation time is non-zero
                        if (firstStep && elapsedSim > 0) {
                            measured_slowdown = std::chrono::duration<double>(elapsedCPU).count() / elapsedSim;
                            firstStep = false;
                        }

                        elapsedSim = newElapsedSim;
                    }
                }

            }

            // Sleep or yield to maintain pace with real-time
            if (busyWait) {
                std::this_thread::yield();
            } else { // If not busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Adjust as needed for your application
            }

        }

        std::cout << "Simulation loop ends." << std::endl;
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

    void updateScene(mjvOption *opt, mjvPerturb *pert, mjvCamera *cam, mjvScene *scn) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        mjv_updateScene(m, d, opt, pert, cam, mjCAT_ALL, scn);
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

    void print_model(const std::string &filename) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        if (m == nullptr) {
            std::cout << "Skipping print operation: 'm' is not initialized (nullptr)." << std::endl;
            return;
        }
        mj_printModel(m, filename.c_str());
    }

    void print_data(const std::string &filename) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        if (m == nullptr || d == nullptr) {
            std::cout << "Skipping print operation: 'm' is not initialized (nullptr)." << std::endl;
            return;
        }
        mj_printData(m, d, filename.c_str());
    }


    void moveCamera(mjtMouse action, mjtNum relative_delta_x, mjtNum relative_delta_y, mjvScene *scn, mjvCamera *cam) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        if (m == nullptr) {
            return;
        }
        mjv_moveCamera(m, action, relative_delta_x, relative_delta_y, scn, cam);
    }


    void setSlowdown(double slowdown) {
        this->slowdown = slowdown;
    }

    double getSlowDown() const {
        return slowdown;
    }

    double getMeasuredSlowDown() const {
        return measured_slowdown;
    }

    void setBusyWait(bool busyWait) {
        this->busyWait = busyWait;
    }


    bool getBusyWait() const {
        return busyWait;
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


    std::atomic<double> slowdown = 1.0;
    std::atomic<double> measured_slowdown = 1.0;
    std::atomic<bool> busyWait = false;
};

#endif //QMUJOCOSIM_SIMULATION_WORKER_HPP
