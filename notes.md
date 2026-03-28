# Notes

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
