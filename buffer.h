#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "mat4.h"
#include "trs.h"
#include <GL/glew.h>
#include <cstring>

const int MAX_VERTICES = 8000;
const int MAX_INDICES = 24000;
const int MAX_OBJECTS = 2000;

struct Vec4
{
    float x, y, z, w;
};

struct Color
{
    float r, g, b, a;
};

struct DrawCommand
{
    unsigned int indicesCount;
    unsigned int copies = 1;
    unsigned int indexOffset;
    int vertexOffset;
    unsigned int baseInstance = 0;
};

typedef int Ref;

struct DrawBuffer
{
    int primitive = GL_TRIANGLES;

    Vec4 vertices[MAX_VERTICES] = {};
    unsigned int indices[MAX_INDICES] = {};
    TRS transforms[MAX_OBJECTS] = {};
    Mat4 models[MAX_OBJECTS] = {};
    Color colors[MAX_OBJECTS] = {};
    DrawCommand commands[MAX_OBJECTS] = {};

    int vtxCount = 0;
    int idxCount = 0;
    int objCount = 1;

    GLuint vao = 0, vbo = 0, ebo = 0, commandBuffer = 0, modelBuffer = 0, colorBuffer = 0;

    Ref add(Vec4 *vtx, int vtxSize, unsigned int *idx, int idxSize, TRS t, Color color)
    {
        int vtxOffset = vtxCount;
        int idxOffset = idxCount;

        memcpy(vertices + vtxCount, vtx, vtxSize * sizeof(Vec4));
        memcpy(indices + idxCount, idx, idxSize * sizeof(unsigned int));

        vtxCount += vtxSize;
        idxCount += idxSize;

        transforms[objCount] = t;
        models[objCount] = trs::compose(t);
        colors[objCount] = color;

        commands[objCount] = {.indicesCount = (unsigned int)idxSize,
                              .copies = 1,
                              .indexOffset = (unsigned int)idxOffset,
                              .vertexOffset = vtxOffset,
                              .baseInstance = 0};

        return objCount++;
    }

    void init(GLuint idxVertex)
    {
        glCreateVertexArrays(1, &vao);

        glCreateBuffers(1, &vbo);
        glCreateBuffers(1, &ebo);
        glCreateBuffers(1, &commandBuffer);
        glCreateBuffers(1, &modelBuffer);
        glCreateBuffers(1, &colorBuffer);

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vec4));
        glVertexArrayElementBuffer(vao, ebo);

        glEnableVertexArrayAttrib(vao, idxVertex);
        glVertexArrayAttribFormat(vao, idxVertex, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, idxVertex, 0);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorBuffer);
    }

    void update_geometry() const
    {
        glNamedBufferData(vbo, vtxCount * sizeof(Vec4), vertices, GL_DYNAMIC_DRAW);
        glNamedBufferData(ebo, idxCount * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);
    }

    void update_commands() const
    {
        glNamedBufferData(commandBuffer, objCount * sizeof(DrawCommand), commands, GL_DYNAMIC_DRAW);
    }

    void update_models() const
    {
        glNamedBufferData(modelBuffer, objCount * sizeof(Mat4), models, GL_DYNAMIC_DRAW);
    }

    void update_colors() const
    {
        glNamedBufferData(colorBuffer, objCount * sizeof(Color), colors, GL_DYNAMIC_DRAW);
    }

    void draw()
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorBuffer);
        glBindVertexArray(vao);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
        glMultiDrawElementsIndirect(primitive, GL_UNSIGNED_INT, 0, objCount, 0);
    }

    void update()
    {
        update_geometry();
        update_models();
        update_colors();
        update_commands();
    }

    void free()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &commandBuffer);
        glDeleteBuffers(1, &modelBuffer);
        glDeleteBuffers(1, &colorBuffer);
        vao = vbo = ebo = commandBuffer = modelBuffer = colorBuffer = 0;
    }

    void transform_faces(Ref obj, int *faces, int faceCount, Mat4 t)
    {
        if (faceCount == 0)
        {
            return;
        }

        DrawCommand &cmd = commands[obj];

        int touched[MAX_VERTICES];
        int touchedCount = 0;
        for (int f = 0; f < faceCount; f++)
        {
            int base = cmd.indexOffset + faces[f] * 3;
            for (int v = 0; v < 3; v++)
            {
                int vtx = cmd.vertexOffset + indices[base + v];
                bool found = false;
                for (int i = 0; i < touchedCount; i++)
                {
                    if (touched[i] == vtx)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    touched[touchedCount++] = vtx;
                }
            }
        }

        float cx = 0, cy = 0, cz = 0;
        for (int i = 0; i < touchedCount; i++)
        {
            cx += vertices[touched[i]].x;
            cy += vertices[touched[i]].y;
            cz += vertices[touched[i]].z;
        }
        cx /= touchedCount;
        cy /= touchedCount;
        cz /= touchedCount;

        const float *m = t.data();
        for (int i = 0; i < touchedCount; i++)
        {
            Vec4 &vx = vertices[touched[i]];
            float lx = vx.x - cx, ly = vx.y - cy, lz = vx.z - cz;
            vx.x = m[0] * lx + m[4] * ly + m[8] * lz + m[12] + cx;
            vx.y = m[1] * lx + m[5] * ly + m[9] * lz + m[13] + cy;
            vx.z = m[2] * lx + m[6] * ly + m[10] * lz + m[14] + cz;
        }

        update_geometry();
    }
};

struct AxisXGeometry
{
    Vec4 vertices[2] = {{0, 0, 0, 1}, {1, 0, 0, 1}};
    unsigned int indices[2] = {0, 1};
};

struct AxisYGeometry
{
    Vec4 vertices[2] = {{0, 0, 0, 1}, {0, 1, 0, 1}};
    unsigned int indices[2] = {0, 1};
};

struct AxisZGeometry
{
    Vec4 vertices[2] = {{0, 0, 0, 1}, {0, 0, 1, 1}};
    unsigned int indices[2] = {0, 1};
};

struct CubeGeometry
{
    Vec4 vertices[8] = {
        {1, -1, 1, 1},   //
        {1, 1, 1, 1},    //
        {-1, -1, 1, 1},  //
        {-1, 1, 1, 1},   //
        {1, -1, -1, 1},  //
        {1, 1, -1, 1},   //
        {-1, -1, -1, 1}, //
        {-1, 1, -1, 1},
    };
    unsigned int indices[36] = {
        0, 1, 2, //
        2, 1, 3, //
        6, 7, 4, //
        4, 7, 5, //
        1, 5, 3, //
        3, 5, 7, //
        0, 2, 4, //
        4, 2, 6, //
        0, 4, 1, //
        1, 4, 5, //
        2, 3, 6, //
        6, 3, 7,
    };
};

#endif // GEOMETRY_H
