# QMuJoCoSim

![Static Badge](https://img.shields.io/badge/std-c%2B%2B20-blue)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/hesic73/PictureConverter/master/LICENSE)

## Controls Guide

| Function   | Key/Action           |
| ---------- | -------------------- |
| Open       | Ctrl + O             |
| Quit       | Ctrl + Q             |
| Play/Pause | Space                |
| Zoom       | Scroll / Middle drag |
| View Orbit | Left drag            |
| View Pan   | Shift + Right Drag   |

## Build Specifications

The following are the primary components and specific versions utilized for development (similar versions may also work):

- **CMake**: 3.22.1
- **C++ Compiler**: g++ 11.4.0 (C++20 standard)
- **Qt6**: 6.6.2
- **MuJoCo**: 3.1.2


## Compilation

```bash
mkdir -p build && cd build
# e.g. cmake ../ -DCMAKE_PREFIX_PATH:PATH="/opt/mujoco-3.1.2/;/home/admin1/Qt/6.6.2/gcc_64/"
cmake ../ -DCMAKE_PREFIX_PATH:PATH="path/to/mujoco;path/to/qt/library"
cmake --build . --config Release
```

## To-Do List

- [x] drag and drop
- [x] screenshot
- [x] toggling OpenGL Effects
- [x] free camera control
- [ ] free camera align
- [ ] toggling model elements
- [ ] headlight control

### Low priority

- [ ] Revise simulation and rendering synchronization: the simulation thread scheduled at 60 FPS, triggering the rendering process immediately upon each simulation cycle's completion.
- [ ] Implement options for users to specify directories for screenshots and model storage.

## Known Issues

- Currently, the rendering operates at a constant FPS, based on the presumption that **simulation speed surpasses real-time**. Should this not be the case, the program could become overloaded. Implementing buffer strategies might be a potential solution.

## Reference

- https://gist.github.com/JeanElsner/755d0feb49864ecadab4ef00fd49a22b

- https://github.com/google-deepmind/mujoco/tree/main/simulate

- https://stackoverflow.com/a/37119983/15687189