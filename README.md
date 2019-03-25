Mandelbrot
==========

Renders the Mandelbrot fractal in a fragment shader.

Building from source
--------------------

### Linux

Install the development dependencies

```
        sudo apt-get install build-essential libxmu-dev libxi-dev libgl-dev
```

To build third-party libraries, from the project root, run

```
        mkdir -p vendor_src/build/linux
        cd vendor_src/build/linux
        cmake -G "Unix Makefiles" ../..
        make -j4
```

To build the app, from project root, run

```
        mkdir -p build/linux
        cd build/linux
        cmake -G "Unix Makefiles" ../..
        make -j4
```

### OS X

To build third-party libraries, from the project root, run

```
        mkdir -p vendor_src/build/linux
        cd vendor_src/build/linux
        CXX=/usr/local/opt/llvm/bin/clang++ cmake -G "Unix Makefiles" ../..
        make -j4
```

To build the app, from project root, run

```
        mkdir -p build/linux
        cd build/linux
        CXX=/usr/local/opt/llvm/bin/clang++ cmake -G "Unix Makefiles" ../..
        make -j4
		make install
```

This will create an app bundle.

### Windows

To build third-party libraries, from the project root, run

```
        mkdir vendor_src\build\linux
        cd vendor_src\build\linux
        cmake -G "Visual Studio 15 2017 Win64" ..\..
```

Open the solution, select the release configuration, and build the ALL_BUILD
target.

To build the app, from project root, run

```
        mkdir build\linux
        cd build\linux
        cmake -G "Visual Studio 15 2017 Win64" ..\..
```

Open the solution, select the release configuration, and build the ALL_BUILD
target.
