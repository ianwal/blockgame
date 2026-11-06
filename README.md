# Block Game

Simple Minecraft clone in C++. Code quality can be improved.

This is a weekend project I did to learn OpenGL, SDL, and ImGui.

Also uses data-driven asset management.

## Interesting stuff

[World gen code](./src/chunk.cpp)
[World management code](./src/levelmanager.cpp)
[Main renderer](./src/render/renderer.cpp)

## Install

See [notes](notes.md)

## TODO

- Corner-relative block position instead of middle-relative
- Block states
- Leaf decay
- A separate model based block/item to texture mapping instead of directly using the item/block name
