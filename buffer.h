#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "mat4.h"
#include "trs.h"
#include <GL/glew.h>
#include <cmath>
#include <cstring>

// TODO: review architecture and separate logic into more files
// TODO: transform/extrude faces into their own local axis (normal as y)
// TODO: shift object vertices to the end when extruding to allow more than one object in the buffer
// TODO: use non-triangular faces for selecting/editing (selecting each triangle individually is a pain)
// TODO: cycle faces sorted by distance to the camera (select faces that the user is "looking at")
// TODO: review code that creates temporary buffers with MAX_VERTICES, MAX_INDICES etc
// TODO: review face transform and extrude (AI slop)
// TODO: use intrusive list (used slots, next vertex offset etc)
// TODO: refactor merge_obj to allow arbitrary selection
// TODO: remove color hacks when lighting is implemented

const int MAX_VERTICES = 8000;
const int MAX_INDICES = 24000;
const int MAX_OBJECTS = 2000;
const int MAX_UNDO = 32;

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

// color palette for distinguishing shapes
namespace colorutil
{
inline Color palette[] = {
    {1.0f, 0.2f, 0.2f, 1.0f}, // red
    {0.2f, 1.0f, 0.2f, 1.0f}, // green
    {0.2f, 0.6f, 1.0f, 1.0f}, // blue
    {1.0f, 1.0f, 0.2f, 1.0f}, // yellow
    {1.0f, 0.4f, 0.0f, 1.0f}, // orange
    {0.8f, 0.2f, 1.0f, 1.0f}, // purple
    {0.2f, 1.0f, 1.0f, 1.0f}, // cyan
    {1.0f, 0.5f, 0.7f, 1.0f}, // pink
};
inline int paletteSize = sizeof(palette) / sizeof(palette[0]);

inline Color getNextColor(Color current)
{
    for (int i = 0; i < paletteSize; i++)
    {
        if (palette[i].r == current.r && palette[i].g == current.g && palette[i].b == current.b)
        {
            return palette[(i + 1) % paletteSize];
        }
    }
    return palette[0];
}
} // namespace colorutil

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
    bool usedSlots[MAX_OBJECTS] = {};

    int vtxCount = 0;
    int idxCount = 0;
    int objCount = 1;

    GLuint vao = 0, vbo = 0, ebo = 0, commandBuffer = 0, modelBuffer = 0, faceColorBuffer = 0, faceOffsetBuffer = 0;

    void reset()
    {
        vtxCount = 0;
        idxCount = 0;
        objCount = 1;
        memset(usedSlots, 0, sizeof(usedSlots));
    }

    Ref add(Geometry geo, TRS t)
    {
        int vtxOffset = vtxCount;
        int idxOffset = idxCount;

        memcpy(vertices + vtxCount, geo.vertices, geo.vertexCount * sizeof(Vec4));
        memcpy(indices + idxCount, geo.indices, geo.indexCount * sizeof(unsigned int));

        vtxCount += geo.vertexCount;
        idxCount += geo.indexCount;

        // find first unused slot (slot 0 is permanently reserved)
        int slot = 1;
        while (slot < MAX_OBJECTS && usedSlots[slot])
        {
            slot++;
        }
        usedSlots[slot] = true;
        if (slot >= objCount)
        {
            objCount = slot + 1;
        }

        transforms[slot] = t;
        models[slot] = trs::compose(t);

        int faceSize = (primitive == GL_TRIANGLES) ? 3 : 2;
        faceOffsets[slot] = idxOffset / faceSize;
        memcpy(faceColors + faceOffsets[slot], geo.faceColors, geo.faceCount * sizeof(Color));

        commands[slot] = {.indicesCount = (unsigned int)geo.indexCount,
                          .copies = 1,
                          .indexOffset = (unsigned int)idxOffset,
                          .vertexOffset = vtxOffset,
                          .baseInstance = 0};

        return slot;
    }

    void free_slot(Ref obj)
    {
        usedSlots[obj] = false;
        commands[obj].indicesCount = 0;
        while (objCount > 1 && !usedSlots[objCount - 1])
        {
            objCount--;
        }
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

    void add_face_highlights(DrawBuffer &highlight, Ref obj, int face, Color color, Mat4 model)
    {
        DrawCommand &cmd = commands[obj];
        int base = cmd.indexOffset + face * 3;
        Vec4 v[3] = {
            vertices[cmd.vertexOffset + indices[base + 0]],
            vertices[cmd.vertexOffset + indices[base + 1]],
            vertices[cmd.vertexOffset + indices[base + 2]],
        };
        unsigned int idx[6] = {0, 1, 1, 2, 2, 0};
        Color c[3] = {color, color, color};
        Ref r = highlight.add({v, 3, idx, 6, c, 3}, TRS{});
        highlight.models[r] = model;
    }

    void move_obj_to_end(Ref obj)
    {
        DrawCommand &cmd = commands[obj];
        if (cmd.indexOffset + cmd.indicesCount == (unsigned int)idxCount)
        {
            return;
        }

        // find smallest vertexOffset among other objects that is > cmd.vertexOffset
        int origVtxOffset = cmd.vertexOffset;
        int origIdxOffset = cmd.indexOffset;
        int origFaceOffset = faceOffsets[obj];

        int nextVtxOffset = vtxCount;
        for (int j = 1; j < objCount; j++)
        {
            if (!usedSlots[j] || j == obj)
            {
                continue;
            }
            if (commands[j].vertexOffset > origVtxOffset && commands[j].vertexOffset < nextVtxOffset)
            {
                nextVtxOffset = commands[j].vertexOffset;
            }
        }

        int movedVtxCount = nextVtxOffset - origVtxOffset;
        int movedIdxCount = (int)cmd.indicesCount;
        int movedFaceCount = movedIdxCount / 3;

        static Vec4 savedVerts[MAX_VERTICES];
        static unsigned int savedIndices[MAX_INDICES];
        static Color savedColors[MAX_INDICES / 3];

        memcpy(savedVerts, vertices + origVtxOffset, movedVtxCount * sizeof(Vec4));
        memcpy(savedIndices, indices + origIdxOffset, movedIdxCount * sizeof(unsigned int));
        memcpy(savedColors, faceColors + origFaceOffset, movedFaceCount * sizeof(Color));

        // shift left
        memmove(vertices + origVtxOffset, vertices + origVtxOffset + movedVtxCount,
                (vtxCount - origVtxOffset - movedVtxCount) * sizeof(Vec4));
        memmove(indices + origIdxOffset, indices + origIdxOffset + movedIdxCount,
                (idxCount - origIdxOffset - movedIdxCount) * sizeof(unsigned int));
        memmove(faceColors + origFaceOffset, faceColors + origFaceOffset + movedFaceCount,
                (idxCount / 3 - origFaceOffset - movedFaceCount) * sizeof(Color));

        // paste at tail
        int newVtxOffset = vtxCount - movedVtxCount;
        int newIdxOffset = idxCount - movedIdxCount;
        int newFaceOffset = idxCount / 3 - movedFaceCount;

        memcpy(vertices + newVtxOffset, savedVerts, movedVtxCount * sizeof(Vec4));
        memcpy(indices + newIdxOffset, savedIndices, movedIdxCount * sizeof(unsigned int));
        memcpy(faceColors + newFaceOffset, savedColors, movedFaceCount * sizeof(Color));

        // update offsets
        cmd.vertexOffset = newVtxOffset;
        cmd.indexOffset = newIdxOffset;
        faceOffsets[obj] = newFaceOffset;

        for (int j = 1; j < objCount; j++)
        {
            if (!usedSlots[j] || j == obj)
            {
                continue;
            }
            if (commands[j].vertexOffset > origVtxOffset)
            {
                commands[j].vertexOffset -= movedVtxCount;
            }
            if ((int)commands[j].indexOffset > origIdxOffset)
            {
                commands[j].indexOffset -= movedIdxCount;
            }
            if (faceOffsets[j] > origFaceOffset)
            {
                faceOffsets[j] -= movedFaceCount;
            }
        }

        update_geometry();
        update_commands();
        update_face_colors();
        update_models();
    }

    Ref merge_obj(Ref dst, Ref src)
    {
        move_obj_to_end(dst);
        move_obj_to_end(src);

        // transform src vertices into dst's local space
        Mat4 xform = mat4::inverse(models[dst]) * models[src];
        int srcVtxStart = commands[src].vertexOffset;
        int srcVtxCount = vtxCount - srcVtxStart;
        for (int i = 0; i < srcVtxCount; i++)
        {
            vertices[srcVtxStart + i] = xform * vertices[srcVtxStart + i];
        }

        // move src indices
        int vtxRelOffset = commands[src].vertexOffset - commands[dst].vertexOffset;
        for (unsigned int i = 0; i < commands[src].indicesCount; i++)
        {
            indices[commands[src].indexOffset + i] += (unsigned int)vtxRelOffset;
        }

        // expand dst's command
        commands[dst].indicesCount += commands[src].indicesCount;

        free_slot(src);

        update_geometry();
        update_commands();
        update_face_colors();
        update_models();

        return dst;
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

        Vec4 centroid = {};
        for (int i = 0; i < touchedCount; i++)
        {
            centroid += vertices[touched[i]];
        }
        centroid *= (1.0f / touchedCount);

        for (int i = 0; i < touchedCount; i++)
        {
            Vec4 &vx = vertices[touched[i]];
            Vec4 local = {vx.x - centroid.x, vx.y - centroid.y, vx.z - centroid.z, 1.0f};
            Vec4 result = t * local;
            vx.x = result.x + centroid.x;
            vx.y = result.y + centroid.y;
            vx.z = result.z + centroid.z;
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
        Vec4 normal = {};
        for (int f = 0; f < faceCount; f++)
        {
            int base = cmd.indexOffset + faces[f] * 3;
            Vec4 &a = vertices[cmd.vertexOffset + indices[base + 0]];
            Vec4 &b = vertices[cmd.vertexOffset + indices[base + 1]];
            Vec4 &c = vertices[cmd.vertexOffset + indices[base + 2]];
            normal += vec4::cross(b - a, c - a);
        }
        normal = vec4::normalize(normal);

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

        // change extruded faces color
        Color currentColor = faceColors[faceOffsets[obj] + faces[0]];
        Color nextColor = colorutil::getNextColor(currentColor);
        for (int f = 0; f < faceCount; f++)
        {
            faceColors[faceOffsets[obj] + faces[f]] = nextColor;
        }

        // append wall triangles with colors from palette
        int existingFaceCount = cmd.indicesCount / 3;
        int wallFaceIdx = 0;
        int wallColorIdx = 0;
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

            // cycle through palette skipping the extruded face color
            Color wallColor = colorutil::palette[wallColorIdx % colorutil::paletteSize];
            while (wallColor.r == nextColor.r && wallColor.g == nextColor.g && wallColor.b == nextColor.b)
            {
                wallColorIdx++;
                wallColor = colorutil::palette[wallColorIdx % colorutil::paletteSize];
            }

            faceColors[faceOffsets[obj] + existingFaceCount + wallFaceIdx * 2 + 0] = wallColor;
            faceColors[faceOffsets[obj] + existingFaceCount + wallFaceIdx * 2 + 1] = wallColor;
            wallColorIdx++;
            wallFaceIdx++;
        }

        // displace new vertices slightly along the face normal
        Vec4 offset = normal * 0.05f;
        for (int i = 0; i < touchedCount; i++)
        {
            vertices[vtxCount + i] += offset;
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
    bool usedSlots[MAX_UNDO][MAX_OBJECTS];
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
        memcpy(usedSlots[i], buf.usedSlots, sizeof(usedSlots[i]));
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
        memcpy(buf.usedSlots, usedSlots[i], sizeof(usedSlots[i]));
        buf.vtxCount = vtxCounts[i];
        buf.idxCount = idxCounts[i];
        buf.objCount = objCounts[i];
        return true;
    }
};

namespace geo
{
inline Vec4 axisXVertices[] = {{0, 0, 0, 1}, {1, 0, 0, 1}};
inline unsigned int axisXIndices[] = {0, 1};
inline Color axisXColors[] = {colorutil::palette[0]}; // red
inline Geometry axisX = {axisXVertices, 2, axisXIndices, 2, axisXColors, 1};

inline Vec4 axisYVertices[] = {{0, 0, 0, 1}, {0, 1, 0, 1}};
inline unsigned int axisYIndices[] = {0, 1};
inline Color axisYColors[] = {colorutil::palette[1]}; // green
inline Geometry axisY = {axisYVertices, 2, axisYIndices, 2, axisYColors, 1};

inline Vec4 axisZVertices[] = {{0, 0, 0, 1}, {0, 0, 1, 1}};
inline unsigned int axisZIndices[] = {0, 1};
inline Color axisZColors[] = {colorutil::palette[2]}; // blue
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
    colorutil::palette[0], colorutil::palette[0], // front (red)
    colorutil::palette[1], colorutil::palette[1], // back (green)
    colorutil::palette[2], colorutil::palette[2], // top (blue)
    colorutil::palette[3], colorutil::palette[3], // bottom (yellow)
    colorutil::palette[4], colorutil::palette[4], // right (orange)
    colorutil::palette[5], colorutil::palette[5], // left (purple)
};
inline Geometry cube = {cubeVertices, 8, cubeIndices, 36, cubeColors, 12};

} // namespace geo

#endif // GEOMETRY_H
