# mujoco_simulation_qt

![Static Badge](https://img.shields.io/badge/std-c%2B%2B20-blue)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/hesic73/PictureConverter/master/LICENSE)

A simple mujoco simulation application in Qt.

## Requirements

- CMake
- Modern C++ compiler
- Qt6
- MuJoCo

## Compilation

```bash
mkdir -p build && cd build
cmake ../ -DCMAKE_PREFIX_PATH:PATH="path/to/mujoco;path/to/qt/library"
cmake --build . --config Release
```

## To-Do List

- [x] drag and drop
- [x] screenshot
- [ ] add text when paused
- [ ] shortcuts
- [ ] camera control
- [ ] OpenGL Effects

## Known Issues

- Currently, the rendering operates at a constant FPS, based on the presumption that **the simulation outpaces real-time**. Should this not be the case, the program could become overloaded. Implementing buffer strategies might be a potential solution.

## Reference

https://gist.github.com/JeanElsner/755d0feb49864ecadab4ef00fd49a22b

https://github.com/google-deepmind/mujoco/tree/main/simulate
