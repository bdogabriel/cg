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

**Other**:
- `w` - toggle wireframe
- `ctrl+u` - undo
- `shift+r` - reset rotation
- `shift+t` - reset translation
- `shift+;` - open command prompt
- `esc` - cancel current command

## Commands

Open the prompt with `shift+;` and type commands:

- `save <name>` - save scene to `scenes/<name>/`
- `load <name>` - load scene from `scenes/<name>/`
- `save_obj <file>` - export selected object to `.obj`
- `load_obj <file>` - load a single `.obj` object
- `add_shape <name>` - add primitive (cube, pyramid, cylinder)
- `merge_coplanar <angle>` - merge coplanar faces (angle tolerance in degrees)
- `set_color <#RRGGBB>` - set current color
- `set_bg_color <#RRGGBB>` - set background color
- `list` - show all available commands

## Scene Format

Scenes are saved as directories under `scenes/<name>/` containing:
- `<name>.scene` - manifest with transforms and object references
- `<name>_<i>.obj` - per-object geometry with per-face colors
