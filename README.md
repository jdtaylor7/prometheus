[![Version][version-badge]](version-url)
[![MIT License][license-badge]](LICENSE.md)

# Drone Viewer

Viewing application for drone project.

### Dependencies

Most library dependencies are included in the source tree under `include/`, but
there are a few additional dependencies.

### Building

The viewer can be built either with `make` or cmake:

```
./build.sh
cd build
```

### Running

The viewer can be run either with Linux or Cygwin, both with X11.

##### Linux (Arch/Manjaro)

`./main`

##### Cygwin

`startxwin ./main`

[version-badge]: https://img.shields.io/github/release/jdtaylor7/drone_viewer/all.svg
[version-url]: https://github.com/jdtaylor7/drone_viewer/releases/latest
[license-badge]: https://img.shields.io/badge/license-MIT-007EC7.svg
