#ifndef SKYBOX_H
#define SKYBOX_H

struct Skybox
{
    unsigned int texture = 0; // GL_TEXTURE_2D (equirectangular); 0 = no sky
    unsigned int vao = 0;     // unit cube geometry
    unsigned int vbo = 0;
    char path[256] = {}; // source path; used by scene save
};

namespace skybox
{
void init(Skybox &s);                   // create cube VAO/VBO
bool load(Skybox &s, const char *path); // load equirect texture; set path
void draw(const Skybox &s);             // bind VAO + texture; draw 36 verts
void reset(Skybox &s);                  // delete texture only (scene clear)
void free(Skybox &s);                   // delete texture + VAO + VBO
} // namespace skybox

#endif
