#ifndef ACTION_H
#define ACTION_H

#include "editor.h"
#include "param.h"

using Action = void (*)(Editor &e, bool justPressed);

namespace cmd
{

void do_none(Editor &e, bool justPressed);
void do_prompt(Editor &e, bool justPressed);
void do_execute(Editor &e, bool justPressed);
void do_set_cmd(Editor &e, bool justPressed);
void do_set_args(Editor &e, bool justPressed);
void do_set_target(Editor &e, bool justPressed);
void do_toggle_lock(Editor &e, bool justPressed);
void do_set_color(Editor &e, bool justPressed);
void do_apply_color(Editor &e, bool justPressed);
void do_set_bg_color(Editor &e, bool justPressed);
void do_toggle_selection(Editor &e, bool justPressed);
void do_clear_selection(Editor &e, bool justPressed);
void do_wireframe_toggle(Editor &e, bool justPressed);
void do_cycle(Editor &e, bool justPressed);
void do_reset(Editor &e, bool justPressed);
void do_undo(Editor &e, bool justPressed);
void do_quit(Editor &e, bool justPressed);
void do_translate(Editor &e, bool justPressed);
void do_rotate(Editor &e, bool justPressed);
void do_rotate_degrees(Editor &e, bool justPressed);
void do_scale(Editor &e, bool justPressed);
void do_uniform_scale(Editor &e, bool justPressed);
void do_shear(Editor &e, bool justPressed);
void do_camera(Editor &e, bool justPressed);
void do_extrude(Editor &e, bool justPressed);
void do_delete(Editor &e, bool justPressed);
void do_merge(Editor &e, bool justPressed);
void do_merge_coplanar(Editor &e, bool justPressed);
void do_add_pyramid(Editor &e, bool justPressed);
void do_add_cylinder(Editor &e, bool justPressed);
void do_add_polyhedron(Editor &e, bool justPressed);
void do_add_sphere(Editor &e, bool justPressed);
void do_save_obj(Editor &e, bool justPressed);
void do_save_scene(Editor &e, bool justPressed);
void do_load_obj(Editor &e, bool justPressed);
void do_load_scene(Editor &e, bool justPressed);
void do_load_skybox(Editor &e, bool justPressed);
void do_help(Editor &e, bool justPressed);

extern const ParamSpec params_set_cmd[];
extern const ParamSpec params_axis_step[];
extern const ParamSpec params_axis_degrees[];
extern const ParamSpec params_uniform_scale[];
extern const ParamSpec params_set_target[];
extern const ParamSpec params_set_color[];
extern const ParamSpec params_cycle[];
extern const ParamSpec params_reset[];
extern const ParamSpec params_add_pyramid[];
extern const ParamSpec params_add_cylinder[];
extern const ParamSpec params_add_polyhedron[];
extern const ParamSpec params_add_sphere[];
extern const ParamSpec params_merge_coplanar[];
extern const ParamSpec params_save_obj[];
extern const ParamSpec params_save_scene[];
extern const ParamSpec params_load_obj[];
extern const ParamSpec params_load_scene[];
extern const ParamSpec params_load_skybox[];
extern const ParamSpec params_help[];

extern const char *const axis_choices[];
extern const char *const cmd_choices[];

AxisLock axis_lock(const char *s);

} // namespace cmd

#endif
