DashboardSK plugin for OpenCPN
==============================

This project provides an universal instrument dashboard display for [OpenCPN](https://opencpn.org) based on the [SignalK](https://signalk.org) data model.

## Dependencies

The plugin requires wxWidgets 3.0 (3.1+ very strongly encouraged, support for 3.0 will be completely dropped as soon as 3.2 makes it to the major Linux distributions) or newer and targets OpenCPN 5.6 or newer.

## Compiling

To compile this plugin you have to be able to compile OpenCPN itself, refer to the [OpenCPN Developer Manual](https://opencpn-manuals.github.io/main/ocpn-dev-manual/intro-AboutThisManual.html) for instructions on preparing the environment

```bash
git clone --recurse-submodules git://github.com/nohal/dashboardsk_pi.git
cd dashboardsk_pi
mkdir build
cd build
cmake ..
make
```

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

The protoypes for the forms are designed using [wxFormBuilder](https://github.com/wxFormBuilder/wxFormBuilder) and the generated code resides in `include/dashboardskgui.h` and `src/dashboardskgui.cpp`. Never edit the generated files manually.
The actual implementation of the GUI functionality is in `include/dashboardskguiimpl.h` and `src/dashboardskguiimpl.cpp`.
If you consider it necessary to break the above and feel it is necessary to start writing the whole GUI code by hand, please think it twice and include a bulletproof justification in the description of the respective pull request, as the change will be hardly reversible and we all probably agree that writing GUI completely by hand tends to be little rewarding activity.

### Adding new instruments

All instruments inherit from the `Instrument` class and be named `<DescriptiveName>Instrument`. The class should be declared in `include/<lowercaseclassname>.h` header and if necessary implemented in `src/<lowercaseclassname.cpp>`. The tests for each respective class are implemented in `tests/XXX-ClassName.cpp`.
The easiest way to start implementing a new instrument is to copy a similar instrument header file (eg. `simplenumberinstrument.h`), rename the class and modify the code to implement the desired functionality.

Refer to `include/instrument.h` for documentation of the basic instrument interface.

Every new instrument must be added to the `INSTRUMENTS` table at [the beginning of](https://github.com/nohal/dashboardsk_pi/blob/main/include/dashboardsk.h#L54-L62) `include\dashboardsk.h` with a new ID and the exact name of the class implementing it. The respective code is then generated automagically by the preprocessor.

When adding a new instrument please also update the documentation under `manual/modules/ROOT` with the information relevant to the new functionality.

### Tests

It is not a completely bad idea to cover your code with tests where feasible (It is not very feasible for the GUI part, but the logic should usually be pretty well testable). The project uses the current branch of [Catch2](https://github.com/catchorg/Catch2) testing framework (Because we use C++17) and the testcases reside under `tests`.
Building the tests is enabled by default and may be disabled by running cmake `cmake` with `-DWITH_TESTS=OFF` parameter.
To execute the tests, simply run `ctest` in the build directory.
