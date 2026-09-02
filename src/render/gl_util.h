#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <GL/glew.h>

namespace glutil
{

GLuint compile_shader(const char *source, GLenum type);
GLuint make_program(const char *vertPath, const char *fragPath);

} // namespace glutil

#endif // GL_UTIL_H
