#include "gl_util.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>

namespace glutil
{

GLuint compile_shader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        fprintf(stderr, "E: shader compile failed: %s\n", infoLog);
        glDeleteShader(shader);
        exit(1);
    }
    return shader;
}

GLuint make_program(const char *vertPath, const char *fragPath)
{
    char *vertSrc = file::read_file(vertPath);
    char *fragSrc = file::read_file(fragPath);
    if (!vertSrc || !fragSrc)
    {
        free(vertSrc);
        free(fragSrc);
        exit(1);
    }

    GLuint vert = compile_shader(vertSrc, GL_VERTEX_SHADER);
    GLuint frag = compile_shader(fragSrc, GL_FRAGMENT_SHADER);
    free(vertSrc);
    free(fragSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        fprintf(stderr, "E: program link failed: %s\n", infoLog);
        glDeleteProgram(program);
        glDeleteShader(vert);
        glDeleteShader(frag);
        exit(1);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

GLuint make_texture(int width, int height, const uint8_t *data, GLenum internalFormat, GLenum format)
{
    GLuint tex;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(tex, 1, internalFormat, width, height);
    glTextureSubImage2D(tex, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    return tex;
}

} // namespace glutil
