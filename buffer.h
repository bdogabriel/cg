#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "mat4.h"
#include "trs.h"
#include <GL/glew.h>
#include <cmath>
#include <cstring>

// TODO: export and import .obj
// TODO: review architecture and separate logic into more files
// TODO: transform/extrude faces into their own local axis (normal as y)
// TODO: shift object vertices to the end when extruding to allow more than one object in the buffer
// TODO: use non-triangular faces for selecting/editing (selecting each triangle individually is a pain)
// TODO: cycle faces sorted by distance to the camera (select faces that the user is "looking at")
// TODO: "glue" two objects: parent/child or just concatenate

const int MAX_VERTICES = 8000;
const int MAX_INDICES = 24000;
const int MAX_OBJECTS = 2000;
const int MAX_UNDO = 32;

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

    int touched_vertices(const DrawCommand &cmd, int *faces, int faceCount, int *touched, int offset) const
    {
        int touchedCount = 0;
        for (int f = 0; f < faceCount; f++)
        {
            int base = cmd.indexOffset + faces[f] * 3;
            for (int v = 0; v < 3; v++)
            {
                int vtx = offset + indices[base + v];
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
        return touchedCount;
    }

    void transform_faces(Ref obj, int *faces, int faceCount, Mat4 t)
    {
        if (faceCount == 0)
        {
            return;
        }

        DrawCommand &cmd = commands[obj];

        int touched[MAX_VERTICES];
        int touchedCount = touched_vertices(cmd, faces, faceCount, touched, cmd.vertexOffset);

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

    void extrude_faces(Ref obj, int *faces, int faceCount)
    {
        if (faceCount == 0)
        {
            return;
        }

        DrawCommand &cmd = commands[obj];

        int touched[MAX_VERTICES];
        int touchedCount = touched_vertices(cmd, faces, faceCount, touched, 0);

        // averaged face normal
        float nx = 0, ny = 0, nz = 0;
        for (int f = 0; f < faceCount; f++)
        {
            int base = cmd.indexOffset + faces[f] * 3;
            Vec4 &a = vertices[cmd.vertexOffset + indices[base + 0]];
            Vec4 &b = vertices[cmd.vertexOffset + indices[base + 1]];
            Vec4 &c = vertices[cmd.vertexOffset + indices[base + 2]];
            float ex = b.x - a.x, ey = b.y - a.y, ez = b.z - a.z;
            float fx = c.x - a.x, fy = c.y - a.y, fz = c.z - a.z;
            nx += ey * fz - ez * fy;
            ny += ez * fx - ex * fz;
            nz += ex * fy - ey * fx;
        }
        float len = sqrtf(nx * nx + ny * ny + nz * nz);
        if (len > 0)
        {
            nx /= len;
            ny /= len;
            nz /= len;
        }

        // directed boundary edges (appear in exactly one selected face)
        struct Edge
        {
            int a, b, face;
        };
        Edge dirEdges[MAX_INDICES];
        int edgeCount = 0;
        int edgeHits[MAX_INDICES] = {};
        for (int f = 0; f < faceCount; f++)
        {
            int base = cmd.indexOffset + faces[f] * 3;
            for (int e = 0; e < 3; e++)
            {
                int va = indices[base + e];
                int vb = indices[base + (e + 1) % 3];
                int lo = va < vb ? va : vb;
                int hi = va < vb ? vb : va;
                bool found = false;
                for (int i = 0; i < edgeCount; i++)
                {
                    int elo = dirEdges[i].a < dirEdges[i].b ? dirEdges[i].a : dirEdges[i].b;
                    int ehi = dirEdges[i].a < dirEdges[i].b ? dirEdges[i].b : dirEdges[i].a;
                    if (elo == lo && ehi == hi)
                    {
                        edgeHits[i]++;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    dirEdges[edgeCount] = {va, vb, f};
                    edgeHits[edgeCount] = 1;
                    edgeCount++;
                }
            }
        }

        // duplicate touched vertices (append to global array)
        int newLocalBase = vtxCount - cmd.vertexOffset;
        for (int i = 0; i < touchedCount; i++)
        {
            vertices[vtxCount + i] = vertices[cmd.vertexOffset + touched[i]];
        }

        // remap selected face indices to new vertices
        for (int f = 0; f < faceCount; f++)
        {
            int base = cmd.indexOffset + faces[f] * 3;
            for (int v = 0; v < 3; v++)
            {
                int old = indices[base + v];
                for (int i = 0; i < touchedCount; i++)
                {
                    if (touched[i] == old)
                    {
                        indices[base + v] = newLocalBase + i;
                        break;
                    }
                }
            }
        }

        // append wall triangles and set colors (darker shade of the adjacent face)
        int existingFaceCount = cmd.indicesCount / 3;
        int wallFaceIdx = 0;
        for (int e = 0; e < edgeCount; e++)
        {
            if (edgeHits[e] != 1)
            {
                continue;
            }
            int va = dirEdges[e].a;
            int vb = dirEdges[e].b;
            int newA = newLocalBase, newB = newLocalBase;
            for (int i = 0; i < touchedCount; i++)
            {
                if (touched[i] == va)
                {
                    newA = newLocalBase + i;
                }
                if (touched[i] == vb)
                {
                    newB = newLocalBase + i;
                }
            }
            int wallBase = cmd.indexOffset + cmd.indicesCount + wallFaceIdx * 6;
            indices[wallBase + 0] = va;
            indices[wallBase + 1] = vb;
            indices[wallBase + 2] = newB;
            indices[wallBase + 3] = va;
            indices[wallBase + 4] = newB;
            indices[wallBase + 5] = newA;
            Color orig = faceColors[faceOffsets[obj] + faces[dirEdges[e].face]];
            Color wallColor = {orig.r * 0.6f, orig.g * 0.6f, orig.b * 0.6f, orig.a};
            faceColors[faceOffsets[obj] + existingFaceCount + wallFaceIdx * 2 + 0] = wallColor;
            faceColors[faceOffsets[obj] + existingFaceCount + wallFaceIdx * 2 + 1] = wallColor;
            wallFaceIdx++;
        }

        // displace new vertices slightly along the face normal
        float dist = 0.05f;
        for (int i = 0; i < touchedCount; i++)
        {
            vertices[vtxCount + i].x += nx * dist;
            vertices[vtxCount + i].y += ny * dist;
            vertices[vtxCount + i].z += nz * dist;
        }

        int added = wallFaceIdx * 6;
        vtxCount += touchedCount;
        idxCount += added;
        cmd.indicesCount += added;

        update_geometry();
        update_commands();
        update_face_colors();
    }
};

struct UndoStack
{
    Vec4 vertices[MAX_UNDO][MAX_VERTICES];
    Color faceColors[MAX_UNDO][MAX_INDICES / 3];
    TRS transforms[MAX_UNDO][MAX_OBJECTS];
    Mat4 models[MAX_UNDO][MAX_OBJECTS];
    unsigned int indices[MAX_UNDO][MAX_INDICES];
    DrawCommand commands[MAX_UNDO][MAX_OBJECTS];
    int faceOffsets[MAX_UNDO][MAX_OBJECTS];
    int vtxCounts[MAX_UNDO];
    int idxCounts[MAX_UNDO];
    int objCounts[MAX_UNDO];
    int head = 0;
    int count = 0;

    void push(const DrawBuffer &buf)
    {
        int i = (head + count) % MAX_UNDO;
        memcpy(vertices[i], buf.vertices, sizeof(vertices[i]));
        memcpy(faceColors[i], buf.faceColors, sizeof(faceColors[i]));
        memcpy(transforms[i], buf.transforms, sizeof(transforms[i]));
        memcpy(models[i], buf.models, sizeof(models[i]));
        memcpy(indices[i], buf.indices, sizeof(indices[i]));
        memcpy(commands[i], buf.commands, sizeof(commands[i]));
        memcpy(faceOffsets[i], buf.faceOffsets, sizeof(faceOffsets[i]));
        vtxCounts[i] = buf.vtxCount;
        idxCounts[i] = buf.idxCount;
        objCounts[i] = buf.objCount;
        if (count == MAX_UNDO)
        {
            head = (head + 1) % MAX_UNDO;
        }
        else
        {
            ++count;
        }
    }

    bool pop(DrawBuffer &buf)
    {
        if (count == 0)
        {
            return false;
        }
        --count;
        int i = (head + count) % MAX_UNDO;
        memcpy(buf.vertices, vertices[i], sizeof(vertices[i]));
        memcpy(buf.faceColors, faceColors[i], sizeof(faceColors[i]));
        memcpy(buf.transforms, transforms[i], sizeof(transforms[i]));
        memcpy(buf.models, models[i], sizeof(models[i]));
        memcpy(buf.indices, indices[i], sizeof(indices[i]));
        memcpy(buf.commands, commands[i], sizeof(commands[i]));
        memcpy(buf.faceOffsets, faceOffsets[i], sizeof(faceOffsets[i]));
        buf.vtxCount = vtxCounts[i];
        buf.idxCount = idxCounts[i];
        buf.objCount = objCounts[i];
        return true;
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
