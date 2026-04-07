# Computer Graphics - Project 1

Student: Gabriel Barbosa de Oliveira - 12543415
Course: SCC0250 - Computer Graphics (2026)
Professor: Jean Roberto Ponciano

## Description

Interactive RPG equipment showcase displaying 5 hand-modeled 3D objects
with keyboard-controlled transformations.

## Objects

All objects created from scratch in this editor using extrusion modeling (no imports):

  1. Bow
  2. Shield
  3. Sword
  4. Grimoire
  5. Staff

## Requirements Fulfillment

[1] Five or more objects, different colors, at least 2 3D
    - 5 3D multi-colored (many colors used to distinguish shape) objects

[2] Composed from primitives
    - Created via face extrusion from basic shapes (cubes, cylinders, ...) using this customized editor built from scratch
    - No external tools used for modeling

[3] Individual transformation matrices
    - Each object has TRS struct composed to 4x4 model matrix

[4] Translation, rotation, scale applied
    - All objects have all degrees of freedom and can be translated, rotated and scaled freely
    - Note: there are other transformations supported by the editor that are beyond the scope of the assignment, including shear, face extrusion and per-face transformations. These features were used for modeling

[5-7] Keyboard controls for transformations
    - Select object with n/p
    - Switch modes with t/r/s/esc
    - Apply transformations with h/j/k/l (directions)
    - Lock transformation to an axis with x/y/z

[8] Well-defined objective
    - RPG equipment showcase

[9] Wireframe toggle
    - W key toggles wireframe

[10] No textures/camera/lighting
    - None used

## Editor & Code Structure

The project consists of a 3D modeling editor built from scratch with OpenGL.
Models were created using this editor and saved as .obj/.mtl files.

Editor features:
  - Face-level extrusion and manipulation
  - Object and face transformation modes
  - OBJ/MTL import/export
  - Use basic primitives as starting point for modeling

Code organization:
  - main.cpp: Window setup and render loop
  - scene.cpp/h: Project 1 scene initialization
  - editor.cpp/h: Transformation controls and input handling
  - buffer.h: GPU buffer management and geometry storage
  - trs.cpp/h: TRS (Translation/Rotation/Scale) system
  - mat4.h: 4x4 matrix math with SIMD optimization

Scene loading:
  1. obj_load() parses .obj and .mtl files
  2. load_and_position() loads each model and sets initial transform
  3. TRS transforms composed into 4x4 model matrices
  4. Buffers uploaded to GPU
  5. Render loop updates only the selected object's matrix

## Controls

Navigation:
  n - Next object
  p - Previous object

Transformation modes:
  t - Translation mode
  r - Rotation mode
  s - Scale mode

Apply transformations (inside a mode):
  h/j/k/l - Transform along axes

Lock to axis (inside a mode):
  x/y/z - Lock to specific axis

Other:
  w - Toggle wireframe
  Esc - Back to normal mode (no transformation selected)

## Build & Run

  make run

## Tech Stack

- OpenGL 4.6 + GLEW + GLFW 3
- C++ with SIMD-optimized matrix math
