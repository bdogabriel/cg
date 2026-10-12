# Handmade Mesh Editor

A mouseless lightweight OpenGL mesh editor for creating and manipulating 3D objects with support for n-gon faces, per-face colors, and scene management.

## Dependencies

- OpenGL
- GLEW
- GLFW

On Ubuntu/Debian:
```bash
sudo apt install libglew-dev libglfw3-dev
```

## Build

```bash
make
```

## Run

```bash
./exe
```

Or use `make run` to build and run.

## Controls

**Transform** (object/face mode):
- `t` - translate
- `r` - rotate
- `s` - scale
- `alt+s` - uniform scale
- `a` - shear
- `e` - extrude (face mode)
- `ctrl+d` - delete object

**Navigation**:
- `h/l` - move left/right on axis
- `j/k` - move down/up on axis
- `u/i` - move on Z axis
- `n/p` - cycle next/prev object or face

**Selection & Mode**:
- `o` - object mode
- `f` - face mode
- `space` - toggle face selection
- `c` - apply color
- `;` - toggle local/world transform lock

**Camera** (default mode):
- `h/l` - strafe left/right
- `j/k` - move backward/forward
- `u/d` - move up/down
- `;` - toggle move/look (while in camera mode)

**Other**:
- `w` - toggle wireframe
- `ctrl+u` - undo
- `shift+r` - reset rotation
- `shift+t` - reset translation
- `shift+s` - reset scale
- `shift+;` - open command prompt
- `esc` - return to camera mode

## Commands

Open the prompt with `shift+;` and type commands:

- `save_scene <name>` - save scene to `scenes/<name>/`
- `load_scene <name> [clear]` - load scene from `scenes/<name>/`
- `save_obj <file>` - export selected object to `.obj`
- `load_obj <file> [clear]` - load a single `.obj` object
- `add_pyramid [sides] [color]` - add a pyramid primitive
- `add_cylinder [segments] [color]` - add a cylinder primitive
- `add_polyhedron [faces] [color]` - add a polyhedron (4, 6, 8, 12, 20 faces; 6 = cube)
- `add_sphere [segments] [color]` - add a sphere primitive
- `camera <axis> <step>` - move/look the camera
- `merge_coplanar <angle>` - merge coplanar faces (angle tolerance in degrees)
- `set_color <#RRGGBB>` - set current color
- `set_bg_color <#RRGGBB>` - set background color
- `help [commands|bindings]` - show help on a topic (default: the `help` command itself)

## Scene Format

Scenes are saved as directories under `scenes/<name>/` containing:
- `<name>.scene` - manifest with transforms and object references
- `<name>_<i>.obj` - per-object geometry with per-face colors
