[![CircleCI][circle-badge]][circle-url]
[![Version][version-badge]](version-url)
[![MIT License][license-badge]](LICENSE.md)

<p align="center">
    <img alt="Prometheus logo" src="assets/logos/logo_with_text.png">
</p>

Prometheus is a real-time drone monitoring application. Built with OpenGL, it
allows one to easily visualize the telemetry from a drone system. It runs under
X11 and is supported on Arch Linux and Ubuntu Focal.

### Dependencies

Some dependencies are included in the source directory, but a few must be
installed locally. Those which must be installed locally are:

* CMake
* OpenGL
* GLFW
* Assimp

##### Arch Linux

The above dependencies can be installed via the following packages from the
official Arch Linux repositories:

```
sudo pacman -Syu cmake glfw-x11 assimp
```

##### Ubuntu Focal (20.04LTS)

The following packages are required in Ubuntu:

```
sudo apt install libgl1-mesa-dev libglfw3 libglfw3-dev libassimp-dev
```

### Building

Prometheus is built with [CMake](https://cmake.org/) via a build script:

```
./build.sh -e prometheus
```

### Running

Run the binary from the root project directory:

```
./build/prometheus
```

At the moment some file pathnames are relative so running from the build
directory itself will not work.

[circle-badge]: https://circleci.com/gh/jdtaylor7/prometheus.svg?style=svg
[circle-url]: https://circleci.com/gh/jdtaylor7/prometheus
[version-badge]: https://img.shields.io/github/release/jdtaylor7/drone_viewer/all.svg
[version-url]: https://github.com/jdtaylor7/drone_viewer/releases/latest
[license-badge]: https://img.shields.io/badge/license-MIT-007EC7.svg
