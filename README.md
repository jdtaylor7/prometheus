[![Version][version-badge]](version-url)
[![MIT License][license-badge]](LICENSE.md)

# Drone Viewer

Viewing application for drone project.

### Building

The viewer is built with CMake via a build script:

```
./build.sh
```

### Running

The viewer can be run either with Linux or Cygwin, both with X11. Run the
following commands from the root project directory:

##### Linux (Arch/Manjaro)

`sudo ./build/main`

Root permission is currently required to access serial ports in Linux.

##### Cygwin

`startxwin ./build/main`

[version-badge]: https://img.shields.io/github/release/jdtaylor7/drone_viewer/all.svg
[version-url]: https://github.com/jdtaylor7/drone_viewer/releases/latest
[license-badge]: https://img.shields.io/badge/license-MIT-007EC7.svg
