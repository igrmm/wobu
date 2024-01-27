# wobu - world builder

clone with:

```
git clone --recurse-submodules https://github.com/igrmm/wobu.git
```

Building on Windows
===================================================================

Install [msys2](https://www.msys2.org/) and update:
```
pacman -Syu
```

Install c compiler and cmake
```
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
```

Install sdl
```
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_ttf
```

Install git and clone repo:
```
pacman -S git
```

Build and run with:
```
mkdir -p build
cd build
cmake ..
cmake --build .
./wobu
```
Don't forget to put tileset.png in the same directory with wobu binary.
