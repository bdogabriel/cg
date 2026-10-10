#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <GL/glew.h>

#include <stdint.h>

namespace glutil
{

GLuint compile_shader(const char *source, GLenum type);
GLuint make_program(const char *vertPath, const char *fragPath);
GLuint make_texture(int width, int height, const uint8_t *data, GLenum internalFormat, GLenum format);

} // namespace glutil

#endif // GL_UTIL_H
