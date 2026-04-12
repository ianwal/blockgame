# Notes

## Setup

## Linux

Install everything in ubuntu-requirements.txt.

Init submodules:

```sh
git submodule update --init --recursive
```

## Windows

Install MSYS2.

In MSYS2 UCRT64:

```sh
pacman -Syyu
# Build dependencies
pacman -S mingw-w64-ucrt-x86_64-gcc # Select all if prompted
pacman -S mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja git
# Game dependencies
pacman -S mingw-w64-ucrt-x86_64-freetype mingw-w64-ucrt-x86_64-libjpeg-turbo mingw-w64-x86_64-glew mingw-w64-ucrt-x86_64-harfbuzz mingw-w64-ucrt-x86_64-mesa mingw-w64-ucrt-x86_64-opusfile mingw-w64-ucrt-x86_64-opus mingw-w64-ucrt-x86_64-libogg
```

Init submodules:

```sh
# Fix github URLs to avoid generating SSH key
git config --global url."https://github.com/".insteadOf "git@github.com:"
git submodule update --init --recursive
```

Build:

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . .build-win64
cmake --build .build-win64 --parallel
```

## SDL3_ttf

Font paths are relative to the game executable.

If the executable is in `foo/` then to load the font with just `font.ttf` then `font.ttf` would need to be in `foo/`.

## OpenGL Model/View/Projection

https://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/#the-model-view-and-projection-matrices

https://glusoft.com/sdl3-tutorials/opengl-3d-cube-sdl3/

https://antongerdelan.net/opengl/raycasting.html

https://learnopengl.com/Getting-Started/Camera

All vertices are at first relative to the center of their model (if there is a model) which is called Model Space. Once you
apply the model matrix to the vertices, they will be put into World Space

Then, to move things relative to a camera, all vertices are translated relative to the camera position. This is called Camera Space.

Then, the projection matrix is used to project the vertices onto the projection . This transforms the camera coordinates to homogeneous coordinates.

## Coordinate System

Blocks positions are integers, and the integer is the center of the block in world position. This should be changed to corner-centered.
