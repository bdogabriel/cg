#ifndef RENDER_RENDERER_H
#define RENDER_RENDERER_H

#include "color.h"

#include <stdint.h>

struct Camera;
struct LineBatch;
struct MeshBatch;
struct ModelTextureCache;
struct QuadBatch;
struct Window;

namespace render
{

using TextureHandle = uint32_t; // 0 = invalid

enum class TextureFormat
{
    RGBA8,
    R8
};

struct TextureDesc
{
    int width;
    int height;
    TextureFormat format;
    const void *pixels;
};

enum class DrawMode
{
    SHADED,
    WIREFRAME
};

enum class QuadPipeline
{
    SOLID,
    TEXT
};

struct LineStyle
{
    float width = 1.0f;
    bool depthEqual = false;
};

// lifecycle
bool init(Window &window);
void shutdown();
bool needs_graphics_context();

// frame
void begin_frame(int fbWidth, int fbHeight, Color clear);
void set_camera(const Camera &camera, float aspect);
void sync_mesh(MeshBatch &batch); // uploads dirty data, clears flags
void draw_meshes(const MeshBatch &batch, const ModelTextureCache &textures, DrawMode mode);
void draw_lines(const LineBatch &batch, LineStyle style);
void draw_quads(const QuadBatch &batch, QuadPipeline pipeline, TextureHandle texture);
void draw_skybox(TextureHandle texture);
void end_frame(); // presents

// textures
TextureHandle create_texture(const TextureDesc &desc);
void destroy_texture(TextureHandle handle);

} // namespace render

#endif // RENDER_RENDERER_H
