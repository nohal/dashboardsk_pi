DashboardSK plugin for OpenCPN
==============================

This project provides an universal instrument dashboard display for [OpenCPN](https://opencpn.org) based on the [SignalK](https://signalk.org) data model.

[![Hosted By: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/27049/badge.svg)](https://scan.coverity.com/projects/nohal-dashboardsk_pi)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/nohal/dashboardsk_pi/main.svg)](https://results.pre-commit.ci/latest/github/nohal/dashboardsk_pi/main)

## Dependencies

The plugin uses wxWidgets 3.2 or newer (although building against wxWidgets 3.0 may still be possible until we start to hard-enforce use of wxBitmapBundle) and targets OpenCPN 5.7 or newer with plugin API 1.18.

## Compiling

To compile this plugin you have to be able to compile OpenCPN itself, refer to the [OpenCPN Developer Manual](https://opencpn-manuals.github.io/main/ocpn-dev-manual/0.1/index.html) for instructions on preparing the environment

### Linux/macOS

```bash
git clone --recurse-submodules https://github.com/nohal/dashboardsk_pi.git
cd dashboardsk_pi
mkdir build
cd build
cmake ..
make
```

### Windows

Run `x86 Native Tools Command Prompt for VS 2022` from the Start menu as Administrator

```
git clone https://github.com/nohal/dashboardsk_pi.git
git submodule update --init
cd dashboardsk_pi.git
ci\appveyor.bat
cd build
cmake -A Win32 ..
cmake --build .
```

Subsequent runs should not need running the command prompt as Administrator

### Obtaining the Git submodules

In case you already cloned the repository without the `--recurse-submodules` parameter, execute `git submodule update --init` in the root directory of the source tree.

## Contributing

### General

The project is developed in C++, specifically C++17.
Please format your code using `clang-format` before submitting pull requests, they are checked for compliance with the `.clang-format` file used by the project during the CI workflow and can't be accepted if the test is not passing.
To make this as simple as possible, a [pre-commit](https://pre-commit.com) configuration is provided in the project, which activates a Git pre-commit hook taking care of the formatting automatically. To use it run the following commands:

```bash
pip3 install pre-commit
cd <dashboard_sk source directory>
pre-commit install
```

#### pre-commit on Windows

Use [WSL](https://learn.microsoft.com/en-us/windows/wsl/) for pre-commit and save yourself the headache.

### Documentation

The code is documented using [Doxygen](https://www.doxygen.nl) style comments. Running `make doxygen-docs` in the build directory generates up to date HTML documentation under `docs`. Please completely document newly added code before submitting pull requests (We started with no warnings about undocumented code being emitted by doxygen and would like to keep it so for the future). Please use English both for naming the entities and documentation and try to name variables in a way describing their purpose. At the other hand, we are not writing a novel here, it is the technical aspect and accuracy of the information that is important.

You may also run `make asciidoxy-docs` to produce documentation formated using [asciidoxy](https://asciidoxy.org). The intermediate AsciiDoc product is also suitable to be included in the user manual (TODO).

To be able to do the above follow install the respective tools first...

The end user documentation integrates to the [OpenCPN plugin documentation framework](https://opencpn-manuals.github.io/plugins/opencpn-plugins/0.1/index.html), is written in [AsciiDoc](https://docs.asciidoctor.org/asciidoc/latest/) and processed using [Antora](https://antora.org) and [AsciiDoctor](https://asciidoctor.org). Please document newly added or changed functionality as soon as it is implemented and try to include the documentation in the same pull request as the code whenever feasible, unfortunately it really won't magically write itelf at some later moment.

### Images

Everything used from the code should be SVG, think at least twice before using a bitmap as the master source of the image. The only place where bitmaps make sense is the documentation.

- Process all the SVG images with [svgo](https://github.com/svg/svgo) - run `svgo --multipass --pretty <your>.svg` on it
- Process all PNG images with `oxipng`, `optipng` or similar tool (In case you use `pre-commit` hooks as configured in this repository, the PNG images are optimized automatically before being commited to the repository)
- Do not use JPEG or any other format using lossy compression for images (Why would you?)

### GUI

The prototypes for the forms `dashboardsk.fbp` and `dashboardsk_android.fbp` are designed using [wxFormBuilder](https://github.com/wxFormBuilder/wxFormBuilder) and the generated code resides in `include/dashboardskgui.h` and `src/dashboardskgui.cpp` for the desktop version and `include/dashboardskguiandroid.h` and `src/dashboardskguiandroid.cpp` for the Android. Never edit the generated files manually.
The actual implementation of the GUI functionality is in `include/dashboardskguiimpl.h` and `src/dashboardskguiimpl.cpp`.
To minimize te amount of conditionally compiled code always keep the two user interfaces as similar as possible and name all the widgets equally in both the versions.
If you consider it necessary to break the above and feel it is necessary to start writing the whole GUI code by hand, please think it twice and include a bulletproof justification in the description of the respective pull request, as the change will be hardly reversible and we all probably agree that writing GUI completely by hand tends to be little rewarding activity.

### Adding new instruments

All instruments inherit from the `Instrument` class and be named `<DescriptiveName>Instrument`. The class should be declared in `include/<lowercaseclassname>.h` header and if necessary implemented in `src/<lowercaseclassname.cpp>`. The tests for each respective class are implemented in `tests/XXX-ClassName.cpp`.
The easiest way to start implementing a new instrument is to copy a similar instrument header file (eg. `simplenumberinstrument.h`), rename the class and modify the code to implement the desired functionality.

Refer to `include/instrument.h` for documentation of the basic instrument interface.

Every new instrument must be added to the `INSTRUMENTS` table at [the beginning of](https://github.com/nohal/dashboardsk_pi/blob/main/include/dashboardsk.h#L54-L62) `include\dashboardsk.h` with a new ID and the exact name of the class implementing it. The respective code is then generated automagically by the preprocessor.
New source files have to be added to `HDR_DASHBOARD` and `SRC_DASHBOARD` lists in `Plugin.cmake`.

When adding a new instrument please also update the documentation under `manual/modules/ROOT` with the information relevant to the new functionality.

### Tests

It is not a completely bad idea to cover your code with tests where feasible (It is not very feasible for the GUI part, but the logic should usually be pretty well testable). The project uses the current branch of [Catch2](https://github.com/catchorg/Catch2) testing framework (Because we use C++17) and the testcases reside under `tests`.
Building the tests is enabled by default and may be disabled by running cmake `cmake` with `-DWITH_TESTS=OFF` parameter.
To execute the tests, simply run `ctest` in the build directory.

### Sanitizers support

To configure the build to enable sanitizer support, run cmake with `-DSANITIZE=<comma separated list of sanitizers>`, eg. `cmake -DSANITIZE=address ..` to enable the adderess sanitizer reporting memory leaks.

## Credits

- Thanks to Alec Leamas, Mike Rossiter, Kees Verruijt, Jon Gough and others from whose plugin related work this plugin reuses bits and pieces.
- Thanks to Dave for OpenCPN

## License

The Catch2 framework is licensed under terms of the Boost Software License 1.0, for details see [LICENSE.txt](https://github.com/catchorg/Catch2/blob/devel/LICENSE.txt)

wxWidgets is licensed under the terms of the wxWindows Library Licence, Version 3.1. For details see [licence.txt](https://github.com/wxWidgets/wxWidgets/blob/master/docs/licence.txt)

OpenCPN is licensed under the terms of the GPLv2+ license, for details see [LICENSING](https://github.com/OpenCPN/OpenCPN/blob/master/LICENSING)

The `dashboardsk_pi` code is licensed under the terms of the GPL v3, or, at your convenience, newer version of the GPL license. See [LICENSING](https://github.com/nohal/dashboardsk_pi/blob/main/LICENSING) for more details.
