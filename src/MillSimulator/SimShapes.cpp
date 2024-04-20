#include <GL/glew.h>
#include "SimShapes.h"
#include "Shader.h"
#include "GlUtils.h"
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <cstddef>

float* sinTable = nullptr;
float* cosTable = nullptr;
int lastNumSlices = 0;
int lastNumSectionIndices = 0;
GLshort quadIndices[] = { 0, 2, 3, 0, 3, 1 };
GLshort quadIndicesReversed[] = { 0, 3, 2, 0, 1, 3 };
GLshort* sectionIndicesQuad = nullptr;
GLshort* sectionIndicesTri = nullptr;

static bool GenerateSinTable(int nSlices)
{
    if (nSlices == lastNumSlices)
        return true;
    if (sinTable != nullptr)
        free(sinTable);
    if (cosTable != nullptr)
        free(cosTable);
    sinTable = cosTable = nullptr;

    float slice = (float)(2 * PI / nSlices);
    int nvals = nSlices + 1;
    sinTable = (float*)malloc(nvals * sizeof(float));
    if (sinTable == nullptr)
        return false;
    cosTable = (float*)malloc(nvals * sizeof(float));
    if (cosTable == nullptr)
    {
        free(sinTable);
        sinTable = nullptr;
        return false;
    }
    for (int i = 0; i < nvals; i++)
    {
        sinTable[i] = sinf(slice * i);
        cosTable[i] = cosf(slice * i);
    }
    lastNumSlices = nvals;
    return true;
}

static void GenerateModel(float* vbuffer, GLushort* ibuffer, int numVerts, int numIndices, Shape* retShape)
{
    GLuint vbo, ibo, vao;

    // vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex), vbuffer, GL_STATIC_DRAW);

    // index buffer
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), ibuffer, GL_STATIC_DRAW);

    //glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, nullptr);
    // vertex array
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));


    if (retShape)
    {
        retShape->vao = vao;
        retShape->ibo = ibo;
        retShape->vbo = vbo;
        retShape->numIndices = numIndices;
    }
}

void RotateProfile(float* profPoints, int nPoints, float distance, float deltaHeight, int nSlices, bool isHalfTurn, Shape* retShape)
{
    int vidx = 0;
    int iidx = 0;
    int numVerts, numIndices;
    int vstart;

    numVerts = nPoints * 2 * (nSlices + 1);
    numIndices = (nPoints - 1) * nSlices * 6;

    float* vbuffer = (float*)malloc(numVerts * sizeof(Vertex));
    if (vbuffer == nullptr)
        return;
    GLushort* ibuffer = (GLushort*)malloc(numIndices * sizeof(GLushort));
    if (ibuffer == nullptr)
    {
        free(vbuffer);
        return;
    }
    int nsinvals = nSlices;
    if (isHalfTurn)
        nsinvals *= 2;
    if (GenerateSinTable(nsinvals) == false)
    {
        free(vbuffer);
        free(ibuffer);
        return;
    }

    for (int i = 0; i < nPoints; i++)
    {
        int i2 = i * 2;

        float prevy = i > 0 ? profPoints[i2 - 2] : 0;
        float prevz = i > 0 ? profPoints[i2 - 1] : profPoints[i2 + 1];
        float prevrad = fabsf(prevy);
        float rad = fabsf(profPoints[i2]);
        float z2 = profPoints[i2 + 1];
        float diffy = profPoints[i2] - prevy;
        float diffz = z2 - prevz;
        float len = sqrtf(diffy * diffy + diffz * diffz);
        float nz = diffy / len;
        vstart = i * 2 * (nSlices + 1);

        for (int j = 0; j <= nSlices; j++)
        {
            // generate vertices
            float sx = sinTable[j];
            float sy = cosTable[j];
            float x1 = prevrad * sx + distance;
            float y1 = prevrad * sy;
            float x2 = rad * sx + distance;
            float y2 = rad * sy;

            // generate normals
            float ny = -diffz / len;
            float nx = ny * sx;
            ny *= sy;

            SET_TRIPLE(vbuffer, vidx, x1, y1, prevz);
            SET_TRIPLE(vbuffer, vidx, nx, ny, nz);
            SET_TRIPLE(vbuffer, vidx, x2, y2, z2);
            SET_TRIPLE(vbuffer, vidx, nx, ny, nz);

            if (j != nSlices)
            {
                // generate indices { 0, 3, 1, 0, 2, 3 }
                int pos = vstart + 2 * j;
                if (i < (nPoints - 1))
                    SET_TRIPLE(ibuffer, iidx, pos, pos + 3, pos + 1);
                if (i > 0)
                    SET_TRIPLE(ibuffer, iidx, pos, pos + 2, pos + 3);
            }
        }
    }

    GenerateModel(vbuffer, ibuffer, numVerts, numIndices, retShape);

    free(vbuffer);
    free(ibuffer);
}

void CalculateExtrudeBufferSizes(int nProfilePoints, bool capStart, bool capEnd,
    int* numVerts, int* numIndices, int* vc1idx, int* vc2idx, int* ic1idx, int* ic2idx)
{
    *numVerts = nProfilePoints * 4; // one face per profile point times 4 vertex per face
    *numIndices = nProfilePoints * 2 * 3; // 2 triangles per face times 3 indices per triangle
    if (capStart)
    {
        *vc1idx = *numVerts * 6;
        *numVerts += nProfilePoints;
        *ic1idx = *numIndices;
        *numIndices += (nProfilePoints - 2) * 3;
    }
    if (capEnd)
    {
        *vc2idx = *numVerts * 6;
        *numVerts += nProfilePoints;
        *ic2idx = *numIndices;
        *numIndices += (nProfilePoints - 2) * 3;
    }
}

void ExtrudeProfileRadial(float* profPoints, int nPoints, float radius, float angleRad, float deltaHeight, bool capStart, bool capEnd, Shape* retShape)
{
    int vidx = 0, vc1idx, vc2idx;
    int iidx = 0, ic1idx, ic2idx;
    int numVerts, numIndices;

    CalculateExtrudeBufferSizes(nPoints, capStart, capEnd, &numVerts, &numIndices, &vc1idx, &vc2idx, &ic1idx, &ic2idx);
    int vc1start = vc1idx / 6;
    int vc2start = vc2idx / 6;

    float* vbuffer = (float*)malloc(numVerts * sizeof(Vertex));
    if (!vbuffer)
        return;
    GLushort* ibuffer = (GLushort*)malloc(numIndices * sizeof(GLushort));
    if (!ibuffer)
    {
        free(vbuffer);
        return;
    }

    bool is_clockwise = angleRad > 0;
    angleRad = (float)fabs(angleRad);
    float dir = is_clockwise ? 1.0f : -1.0f;
    int offs1 = is_clockwise ? -1 : 0;
    int offs2 = is_clockwise ? 0 : -1;

    float cosAng = cosf(angleRad);
    float sinAng = sinf(angleRad);
    for (int i = 0; i < nPoints; i++)
    {
        int p1 = i * 2;
        float y1 = profPoints[p1] + radius;
        float z1 = profPoints[p1 + 1];
        int p2 = (p1 + 2) % (nPoints * 2);
        float y2 = profPoints[p2] + radius;
        float z2 = profPoints[p2 + 1];

        // normals
        float ydiff = y2 - y1;
        float zdiff = z2 - z1;
        float len = sqrtf(ydiff * ydiff + zdiff * zdiff);
        float ny = -zdiff / len;
        float nz = ydiff / len;
        float nx = -sinAng * ny;
        ny *= cosAng;

        // start verts
        SET_TRIPLE(vbuffer, vidx, 0, y1, z1);
        SET_TRIPLE(vbuffer, vidx, nx, ny, nz);
        SET_TRIPLE(vbuffer, vidx, 0, y2, z2);
        SET_TRIPLE(vbuffer, vidx, nx, ny, nz);

        if (capStart) {
            SET_TRIPLE(vbuffer, vc1idx, 0, y1, z1);
            SET_TRIPLE(vbuffer, vc1idx, -1 * dir, 0, 0);
            if (i > 1)
                SET_TRIPLE(ibuffer, ic1idx, vc1start, vc1start + i + offs1, vc1start + i + offs2);
        }

        float x1 = y1 * sinAng * dir;
        float x2 = y2 * sinAng * dir;
        y1 *= cosAng;
        y2 *= cosAng;
        z1 += deltaHeight;
        z2 += deltaHeight;
        SET_TRIPLE(vbuffer, vidx, x1, y1, z1);
        SET_TRIPLE(vbuffer, vidx, nx, ny, nz);
        SET_TRIPLE(vbuffer, vidx, x2, y2, z2);
        SET_TRIPLE(vbuffer, vidx, nx, ny, nz);

        // face have 2 triangles { 0, 2, 3, 0, 3, 1 };
        GLushort vistart = i * 4;
        if (is_clockwise)
        {
            SET_TRIPLE(ibuffer, iidx, vistart, vistart + 2, vistart + 3);
            SET_TRIPLE(ibuffer, iidx, vistart, vistart + 3, vistart + 1);
        }
        else
        {
            SET_TRIPLE(ibuffer, iidx, vistart, vistart + 3, vistart + 2);
            SET_TRIPLE(ibuffer, iidx, vistart, vistart + 1, vistart + 3);
        }

        if (capEnd)
        {
            SET_TRIPLE(vbuffer, vc2idx, x1, y1, z1);
            SET_TRIPLE(vbuffer, vc2idx, cosAng * dir, -sinAng, 0);
            if (i > 1)
                SET_TRIPLE(ibuffer, ic2idx, vc2start, vc2start + i + offs2, vc2start + i + offs1);
        }
    }

    GenerateModel(vbuffer, ibuffer, numVerts, numIndices, retShape);

    free(vbuffer);
    free(ibuffer);
}

void ExtrudeProfileLinear(float* profPoints, int nPoints, float fromX, float toX, float fromZ, float toZ, bool capStart, bool capEnd, Shape* retShape)
{
    int vidx = 0, vc1idx, vc2idx;
    int iidx = 0, ic1idx, ic2idx;
    int numVerts, numIndices;

    CalculateExtrudeBufferSizes(nPoints, capStart, capEnd, &numVerts, &numIndices, &vc1idx, &vc2idx, &ic1idx, &ic2idx);
    int vc1start = vc1idx / 6;
    int vc2start = vc2idx / 6;

    float* vbuffer = (float*)malloc(numVerts * sizeof(Vertex));
    if (!vbuffer)
        return;
    GLushort* ibuffer = (GLushort*)malloc(numIndices * sizeof(GLushort));
    if (!ibuffer)
    {
        free(vbuffer);
        return;
    }

    for (int i = 0; i < nPoints; i++)
    {
        // hollow pipe verts
        int p1 = i * 2;
        float y1 = profPoints[p1];
        float z1 = profPoints[p1 + 1];
        int p2 = (p1 + 2) % (nPoints * 2);
        float y2 = profPoints[p2];
        float z2 = profPoints[p2 + 1];

        // nornal
        float ydiff = y2 - y1;
        float zdiff = z2 - z1;
        float len = sqrtf(ydiff * ydiff + zdiff * zdiff);
        float ny = -zdiff / len;
        float nz = ydiff / len;

        SET_TRIPLE(vbuffer, vidx, fromX, y1, z1 + fromZ);
        SET_TRIPLE(vbuffer, vidx, 0, ny, nz);
        SET_TRIPLE(vbuffer, vidx, fromX, y2, z2 + fromZ);
        SET_TRIPLE(vbuffer, vidx, 0, ny, nz);
        SET_TRIPLE(vbuffer, vidx, toX, y1, z1 + toZ);
        SET_TRIPLE(vbuffer, vidx, 0, ny, nz);
        SET_TRIPLE(vbuffer, vidx, toX, y2, z2 + toZ);
        SET_TRIPLE(vbuffer, vidx, 0, ny, nz);

        // face have 2 triangles { 0, 2, 3, 0, 3, 1 };
        GLushort vistart = i * 4;
        SET_TRIPLE(ibuffer, iidx, vistart, vistart + 2, vistart + 3);
        SET_TRIPLE(ibuffer, iidx, vistart, vistart + 3, vistart + 1);

        if (capStart)
        {
            SET_TRIPLE(vbuffer, vc1idx, fromX, profPoints[p1], profPoints[p1 + 1] + fromZ);
            SET_TRIPLE(vbuffer, vc1idx, -1, 0, 0);
            if (i > 1)
                SET_TRIPLE(ibuffer, ic1idx, vc1start, vc1start + i - 1, vc1start + i);
        }
        if (capEnd)
        {
            SET_TRIPLE(vbuffer, vc2idx, toX, profPoints[p1], profPoints[p1 + 1] + toZ);
            SET_TRIPLE(vbuffer, vc2idx, 1, 0, 0);
            if (i > 1)
                SET_TRIPLE(ibuffer, ic2idx, vc2start, vc2start + i, vc2start + i - 1);
        }
    }

    GenerateModel(vbuffer, ibuffer, numVerts, numIndices, retShape);

    free(vbuffer);
    free(ibuffer);
}

void Shape::Render()
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, nullptr);
}

void Shape::Render(mat4x4 modelMat, mat4x4 normallMat) // normals are rotated only
{
    CurrentShader->UpdateModelMat(modelMat, normallMat);
    Render();
}

void Shape::FreeResources()
{
    if (vao > 0)
    {
        glBindVertexArray(vao);
        if (vbo > 0) glDeleteBuffers(1, &vbo);
        if (ibo > 0) glDeleteBuffers(1, &ibo);
        glDeleteVertexArrays(1, &vao);
    }
}
