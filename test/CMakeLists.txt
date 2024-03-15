
add_executable(TEST_SIMULATION_WORKER test_simulation_worker.cpp)

target_compile_definitions(TEST_SIMULATION_WORKER PRIVATE
        "EXAMPLE_XML_PATH=\"${CMAKE_BINARY_DIR}/example.xml\"")

target_include_directories(TEST_SIMULATION_WORKER PRIVATE ${CMAKE_SOURCE_DIR}//src)

target_link_libraries(TEST_SIMULATION_WORKER PRIVATE
        ${MUJOCO_LIBRARY}
        Catch2::Catch2WithMain)
add_test(NAME TEST_SIMULATION_WORKER COMMAND TEST_SIMULATION_WORKER)

add_custom_command(
        TARGET TEST_SIMULATION_WORKER POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/assets/example.xml
        ${CMAKE_BINARY_DIR}/example.xml)