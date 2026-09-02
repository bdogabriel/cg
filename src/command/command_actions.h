#ifndef COMMAND_INTERNAL_H
#define COMMAND_INTERNAL_H

#include "command.h"
#include "editor.h"

struct Editor;
struct Session;

namespace cmd
{

void do_none(Editor &, Session &, const Args &, bool);
void do_prompt(Editor &, Session &, const Args &, bool);
void do_execute(Editor &, Session &, const Args &, bool);
void do_set_cmd(Editor &, Session &, const Args &, bool);
void do_set_args(Editor &, Session &, const Args &, bool);
void do_set_target(Editor &, Session &, const Args &, bool);
void do_toggle_lock(Editor &, Session &, const Args &, bool);
void do_set_color(Editor &, Session &, const Args &, bool);
void do_apply_color(Editor &, Session &, const Args &, bool);
void do_set_bg_color(Editor &, Session &, const Args &, bool);
void do_toggle_selection(Editor &, Session &, const Args &, bool);
void do_clear_selection(Editor &, Session &, const Args &, bool);
void do_wireframe_toggle(Editor &, Session &, const Args &, bool);
void do_cycle(Editor &, Session &, const Args &, bool);
void do_reset(Editor &, Session &, const Args &, bool);
void do_undo(Editor &, Session &, const Args &, bool);
void do_quit(Editor &, Session &, const Args &, bool);
void do_translate(Editor &, Session &, const Args &, bool);
void do_rotate(Editor &, Session &, const Args &, bool);
void do_scale(Editor &, Session &, const Args &, bool);
void do_shear(Editor &, Session &, const Args &, bool);
void do_extrude(Editor &, Session &, const Args &, bool);
void do_del(Editor &, Session &, const Args &, bool);
void do_merge(Editor &, Session &, const Args &, bool);
void do_merge_coplanar(Editor &, Session &, const Args &, bool);
void do_add_shape(Editor &, Session &, const Args &, bool);
void do_save_obj(Editor &, Session &, const Args &, bool);
void do_save(Editor &, Session &, const Args &, bool);
void do_load_obj(Editor &, Session &, const Args &, bool);
void do_load(Editor &, Session &, const Args &, bool);
void do_list(Editor &, Session &, const Args &, bool);

extern const ParamSpec params_set_cmd[];
extern const ParamSpec params_axis_step[];
extern const ParamSpec params_set_target[];
extern const ParamSpec params_set_color[];
extern const ParamSpec params_cycle[];
extern const ParamSpec params_reset[];
extern const ParamSpec params_add_shape[];
extern const ParamSpec params_merge_coplanar[];
extern const ParamSpec params_save_obj[];
extern const ParamSpec params_save[];
extern const ParamSpec params_load_obj[];
extern const ParamSpec params_load[];

extern const char *const axis_choices[];
extern const char *const cmd_choices[];

AxisLock axis_lock(const char *s);

} // namespace cmd

#endif
