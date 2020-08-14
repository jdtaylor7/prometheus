[![Version][version-badge]](version-url)
[![MIT License][license-badge]](LICENSE.md)

<img alt="Prometheus Logo" style="float: center;" src="assets/logos/logo_with_text.png">

Prometheus is a real-time drone monitoring application. Built from with OpenGL,
it allows one to easily visualize the telemetry from a drone system. It runs in
Linux under X11.

### Dependencies

Some dependencies are included in the source directory, but a few must be
installed locally. Those which must be installed locally are:

* OpenGL
* GLFW
* Assimp

##### Arch/Manjaro (XFCE 64 bit version)

The above dependencies can be installed via the following packages from the
official Arch Linux repositories:

* [glfw-x11](https://www.archlinux.org/packages/community/x86_64/glfw-x11/)
* [assimp](https://www.archlinux.org/packages/extra/x86_64/assimp/)
* Others

##### Ubuntu Focal (20.04LTS)

For Ubuntu, the following packages should be installed:

* [libgl1-mesa-dev](https://packages.ubuntu.com/focal/libgl1-mesa-dev)
* [libglfw3](https://packages.ubuntu.com/focal/libglfw3)
* [libglfw3-dev](https://packages.ubuntu.com/focal/libglfw3-dev)
* [libassimp-dev](https://packages.ubuntu.com/xenial/libassimp-dev)

### Building

The viewer is built with [CMake](https://cmake.org/) via a build script:

```
./build.sh -t prometheus
```

### Running

Run the binary from the root project directory:

```
./build/prometheus
```

Some file pathnames are relative so running from the build directory itself will
not work.

[version-badge]: https://img.shields.io/github/release/jdtaylor7/drone_viewer/all.svg
[version-url]: https://github.com/jdtaylor7/drone_viewer/releases/latest
[license-badge]: https://img.shields.io/badge/license-MIT-007EC7.svg
