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

struct Geometry
{
    Vec4 *vertices;
    int vertexCount;
    unsigned int *indices;
    int indexCount;
    Color *faceColors;
    int faceCount;
};

struct DrawBuffer
{
    int primitive = GL_TRIANGLES;

    Vec4 vertices[MAX_VERTICES] = {};
    unsigned int indices[MAX_INDICES] = {};
    Color faceColors[MAX_INDICES / 3] = {};
    int faceOffsets[MAX_OBJECTS] = {};
    TRS transforms[MAX_OBJECTS] = {};
    Mat4 models[MAX_OBJECTS] = {};
    DrawCommand commands[MAX_OBJECTS] = {};

    int vtxCount = 0;
    int idxCount = 0;
    int objCount = 1;

    GLuint vao = 0, vbo = 0, ebo = 0, commandBuffer = 0, modelBuffer = 0, faceColorBuffer = 0, faceOffsetBuffer = 0;

    Ref add(Geometry geo, TRS t)
    {
        int vtxOffset = vtxCount;
        int idxOffset = idxCount;

        memcpy(vertices + vtxCount, geo.vertices, geo.vertexCount * sizeof(Vec4));
        memcpy(indices + idxCount, geo.indices, geo.indexCount * sizeof(unsigned int));

        vtxCount += geo.vertexCount;
        idxCount += geo.indexCount;

        transforms[objCount] = t;
        models[objCount] = trs::compose(t);

        int faceSize = (primitive == GL_TRIANGLES) ? 3 : 2;
        faceOffsets[objCount] = idxOffset / faceSize;
        memcpy(faceColors + faceOffsets[objCount], geo.faceColors, geo.faceCount * sizeof(Color));

        commands[objCount] = {.indicesCount = (unsigned int)geo.indexCount,
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
        glCreateBuffers(1, &faceColorBuffer);
        glCreateBuffers(1, &faceOffsetBuffer);

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vec4));
        glVertexArrayElementBuffer(vao, ebo);

        glEnableVertexArrayAttrib(vao, idxVertex);
        glVertexArrayAttribFormat(vao, idxVertex, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, idxVertex, 0);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, faceColorBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, faceOffsetBuffer);
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

    void update_face_colors()
    {
        int faceSize = (primitive == GL_TRIANGLES) ? 3 : 2;
        int totalFaces = idxCount / faceSize;
        glNamedBufferData(faceColorBuffer, totalFaces * sizeof(Color), faceColors, GL_DYNAMIC_DRAW);
        glNamedBufferData(faceOffsetBuffer, objCount * sizeof(int), faceOffsets, GL_DYNAMIC_DRAW);
    }

    void set_face_color(Ref obj, int faceLocalIdx, Color c)
    {
        faceColors[faceOffsets[obj] + faceLocalIdx] = c;
    }

    void draw()
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, faceColorBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, faceOffsetBuffer);
        glBindVertexArray(vao);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
        glMultiDrawElementsIndirect(primitive, GL_UNSIGNED_INT, 0, objCount, 0);
    }

    void update()
    {
        update_geometry();
        update_models();
        update_face_colors();
        update_commands();
    }

    void free()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &commandBuffer);
        glDeleteBuffers(1, &modelBuffer);
        glDeleteBuffers(1, &faceColorBuffer);
        glDeleteBuffers(1, &faceOffsetBuffer);
        vao = vbo = ebo = commandBuffer = modelBuffer = faceColorBuffer = faceOffsetBuffer = 0;
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

namespace geo
{
inline Vec4 axisXVertices[] = {{0, 0, 0, 1}, {2, 0, 0, 1}};
inline unsigned int axisXIndices[] = {0, 1};
inline Color axisXColors[] = {{1, 0, 0, 1}};
inline Geometry axisX = {axisXVertices, 2, axisXIndices, 2, axisXColors, 1};

inline Vec4 axisYVertices[] = {{0, 0, 0, 1}, {0, 2, 0, 1}};
inline unsigned int axisYIndices[] = {0, 1};
inline Color axisYColors[] = {{0, 1, 0, 1}};
inline Geometry axisY = {axisYVertices, 2, axisYIndices, 2, axisYColors, 1};

inline Vec4 axisZVertices[] = {{0, 0, 0, 1}, {0, 0, 2, 1}};
inline unsigned int axisZIndices[] = {0, 1};
inline Color axisZColors[] = {{0, 0, 1, 1}};
inline Geometry axisZ = {axisZVertices, 2, axisZIndices, 2, axisZColors, 1};

inline Vec4 cubeVertices[] = {
    {1, -1, 1, 1},   //
    {1, 1, 1, 1},    //
    {-1, -1, 1, 1},  //
    {-1, 1, 1, 1},   //
    {1, -1, -1, 1},  //
    {1, 1, -1, 1},   //
    {-1, -1, -1, 1}, //
    {-1, 1, -1, 1},
};
inline unsigned int cubeIndices[] = {
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
inline Color cubeColors[] = {
    {1.0f, 0.2f, 0.2f, 1}, {1.0f, 0.2f, 0.2f, 1}, // front
    {0.2f, 1.0f, 0.2f, 1}, {0.2f, 1.0f, 0.2f, 1}, // back
    {0.2f, 0.6f, 1.0f, 1}, {0.2f, 0.6f, 1.0f, 1}, // top
    {1.0f, 1.0f, 0.2f, 1}, {1.0f, 1.0f, 0.2f, 1}, // bottom
    {1.0f, 0.4f, 0.0f, 1}, {1.0f, 0.4f, 0.0f, 1}, // right
    {0.8f, 0.2f, 1.0f, 1}, {0.8f, 0.2f, 1.0f, 1}, // left
};
inline Geometry cube = {cubeVertices, 8, cubeIndices, 36, cubeColors, 12};

} // namespace geo

#endif // GEOMETRY_H
