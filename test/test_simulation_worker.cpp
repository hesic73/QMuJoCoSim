#include <catch2/catch_test_macros.hpp>
#include "simulation_worker.hpp"
#include "mujoco/mujoco.h"

#ifndef EXAMPLE_XML_PATH
#define EXAMPLE_XML_PATH ""
#endif

char error[1000];

TEST_CASE("Simulation worker", "[basic]") {
    mjModel *m = mj_loadXML(EXAMPLE_XML_PATH, nullptr, error, 1000);
    REQUIRE(m != nullptr);
    mjData *d = mj_makeData(m);
    REQUIRE(d != nullptr);

    SimulationWorker simulationWorker(m, d, 60);

    std::thread simulationThread([&]() { simulationWorker.startSimulationLoop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_NOTHROW(simulationWorker.setSimulationPaused(true));

    // Give some time for the simulation to pause
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_NOTHROW(simulationWorker.setSimulationPaused(false));

    // Ensure the simulation can be terminated cleanly
    REQUIRE_NOTHROW(simulationWorker.terminateSimulation());

    // Wait for the simulation thread to finish before proceeding
    if (simulationThread.joinable()) {
        simulationThread.join();
    }

}