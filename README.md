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
cmake ../ -DCMAKE_PREFIX_PATH:PATH="path/to/mujoco;path/to/qt"
cmake --build . --config Release
```

## Reference

https://gist.github.com/JeanElsner/755d0feb49864ecadab4ef00fd49a22b
