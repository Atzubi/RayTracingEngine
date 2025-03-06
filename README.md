# RayTracingEngine
This project aims to provide a simple to use, yet powerful, ray tracing engine. 
The engine features a shader based pipeline structure similar to existing render pipelines.

Check the [examples](/examples/) for more details on how to use the engine.

The project is WIP, however the core functionality can already be used. Check
the setup section if you want to give it a try.

## Getting Started
This project requires cmake 3.20 or higher.\
If you wish to build the examples you also need SFML (Simple and Fast Multimedia Library). If you are using linux it can likely be installed via your distributions package manager.
For windows, download the correct version (2.x) of SFML and in cmake set the SFML_DIR to `path_to_sfml/lib/cmake/SFML`.\
*Note: MacOS may be able to build but it is currently not something that is actively tested.*

#### Installing SFML
```bash
# ubuntu
sudo apt install libsfml-dev
# Arch/Manjaro
sudo pacman -Syu sfml
# MacOS
brew install sfml
```

### Setup - Linux
1. Clone the project and create the build directory.
```bash
git clone https://github.com/Atzubi/RayTracingEngine.git
cd RayTracingEngine
mkdir build && cd build
```
2. Generate make files.
```bash
cmake .. -DATZUBI_RTENGINE_BUILD_EXAMPLES=1
# If you don't want examples built, omit:
# -DATUZBI_RTENGINE_BUILD_EXAMPLES=1 from the cmake command
```
3. Build and install.
```bash
make -j$(nproc)
sudo make install
```
*Note: GNU's default library install directory is not in the default path of many linux distrubutions so be sure to add it.*

4. Add to your project via cmake.
```cmake
target_link_libraries(YOUR_EXECUTABLE RayTraceEngine)
```
All done!\
Be sure to check out the [examples](/examples/) for more details on how to use the engine.