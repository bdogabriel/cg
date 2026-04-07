#ifndef SCENE_H
#define SCENE_H

#include "buffer.h"
#include "editor.h"
#include <filesystem>

namespace scene
{
bool load_models(DrawBuffer &buf, EditorState &state, const std::filesystem::path &modelsDir);
} // namespace scene

#endif // SCENE_H
