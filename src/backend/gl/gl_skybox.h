#ifndef GL_SKYBOX_H
#define GL_SKYBOX_H

namespace glskybox
{

struct Skybox
{
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int sampler = 0;
};

void init(Skybox &s);
void draw(const Skybox &s, unsigned int texture);
void free(Skybox &s);

} // namespace glskybox

#endif // GL_SKYBOX_H
