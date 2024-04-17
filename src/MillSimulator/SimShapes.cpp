#include "SimShapes.h"
#include <math.h>
#include <malloc.h>
#include <string.h>

#define PI 3.14159265 

float* sinTable = nullptr;
float* cosTable = nullptr;
int lastNumSlices = 0;
int lastNumSectionIndices = 0;
GLshort quadIndices[] = { 0, 2, 3, 0, 3, 1 };
GLshort quadIndicesReversed[] = { 0, 3, 2, 0, 1, 3 };
GLshort* sectionIndicesQuad = nullptr;
GLshort* sectionIndicesTri = nullptr;

bool GenerateSinTable(int nSlices)
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

bool GenerateSliceIndices(int nPoints)
{
    int numSectionIndices = nPoints * 3 * 3;
    if (numSectionIndices == lastNumSectionIndices)
        return sectionIndicesQuad;
    if (numSectionIndices > lastNumSectionIndices)
    {
        sectionIndicesQuad = (GLshort*)malloc(numSectionIndices * sizeof(GLshort));
        if (sectionIndicesQuad == nullptr)
            return false;
        sectionIndicesTri = &sectionIndicesQuad[nPoints * 2 * 3];
    }
    int qidx = 0;
    int tidx = 0;
    for (int i = 0; i < nPoints; i++)
    {
        int j = i + nPoints + 1;
        SET_TRIPLE(sectionIndicesQuad, qidx, i, j, j + 1);
        SET_TRIPLE(sectionIndicesQuad, qidx, i, j + 1, i + 1);
        SET_TRIPLE(sectionIndicesTri, tidx, i, j, j + 1);
    }
    return true;
}


void RotateProfile(float* profPoints, int nPoints, float distance, float deltaHeight, int nSlices, bool isHalfTurn)
{
    int half_nverts = (nSlices + 1) * 3;
    int nverts = half_nverts * 2;
    float* vertices = (float*)malloc(nverts * sizeof(float));
    if (vertices == nullptr)
        return;
    float* normals = (float*)malloc(nverts * sizeof(float));
    if (normals == nullptr)
    {
        free(vertices);
        return;
    }
    int nsinvals = nSlices;
    if (isHalfTurn)
        nsinvals *= 2;
    if (GenerateSinTable(nsinvals) == false)
        return;
    int nidxs = nSlices * 2 * 3;
    if (GenerateSliceIndices(nSlices) == false)
        return;
    if (fabsf(profPoints[nidxs - 2]) < 0.00001)
        nidxs -= 3; // if y value is too close to zero render only 1 triangle since all are merged to the same point

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);

    for (int i = 0; i < nPoints; i++)
    {
        int vidx = 0;
        int iidx = 0;
        int i2 = i * 2;
        float rad = fabsf(profPoints[i2]);
        float z = profPoints[i2 + 1];

        i2 = (i > 0 ? i - 1 : i) * 2;
        float diffy = profPoints[i2 + 2] - profPoints[i2];
        float diffz = profPoints[i2 + 3] - profPoints[i2 + 1];
        float len = sqrtf(diffy * diffy + diffz * diffz);
        float nz = diffy / len;

        for (int j = 0; j <= nSlices; j++)
        {
            // generate vertices
            float sx = sinTable[j];
            float sy = cosTable[j];
            float x = rad * sx + distance;
            float y = rad * sy;
            SET_TRIPLE(vertices, vidx, x, y, z);

            // generate normals
            float ny = -diffz / len;
            float nx = ny * sx;
            ny *= sy;
            SET_TRIPLE(normals, iidx, nx, ny, nz);
        }
        if (i > 0)
        {
            memcpy(&normals[half_nverts], &normals[0], half_nverts * sizeof(float));
            if (i == (nPoints - 1)) 
                glDrawElements(GL_TRIANGLES, nSlices * 3, GL_UNSIGNED_SHORT, sectionIndicesTri);
            else
                glDrawElements(GL_TRIANGLES, nSlices * 2 * 3, GL_UNSIGNED_SHORT, sectionIndicesQuad);
        }
 
        memcpy(&vertices[half_nverts], &vertices[0], half_nverts * sizeof(float));
    }
    free(vertices);
    free(normals);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // create top disc
    nverts = (nSlices + 2) * 3;
    vertices = (float*)malloc(nverts * sizeof(float));
    if (vertices == nullptr)
        return;
    int idx = 0;
    float y = profPoints[0];
    float z = profPoints[1];
    for (int j = nSlices; j >= 0; j--)
    {
        SET_TRIPLE(vertices, idx, distance + y * sinTable[j], y * cosTable[j], z);
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormal3f(0, 0, 1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nSlices + 1);
    glDisableClientState(GL_VERTEX_ARRAY);

    free(vertices);
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

void ExtrudeProfileRadial(float* profPoints, int nPoints, float radius, float angleRad, float deltaHeight, bool capStart, bool capEnd)
{
    int vidx = 0, vc1idx, vc2idx;
    int iidx = 0, ic1idx, ic2idx;
    int numVerts, numIndices;

    CalculateExtrudeBufferSizes(nPoints, capStart, capEnd, &numVerts, &numIndices, &vc1idx, &vc2idx, &ic1idx, &ic2idx);
    int vc1start = vc1idx / 6;
    int vc2start = vc2idx / 6;

    float* vbuffer = (float*)malloc(numVerts * 6 * sizeof(float));
    if (!vbuffer)
        return;
    GLushort* ibuffer = (GLushort*)malloc(numIndices * sizeof(float));
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
        SET_TRIPLE(ibuffer, iidx, vistart, vistart + 2, vistart + 3);
        SET_TRIPLE(ibuffer, iidx, vistart, vistart + 3, vistart + 1);

        if (capEnd)
        {
            SET_TRIPLE(vbuffer, vc2idx, x1, y1, z1);
            SET_TRIPLE(vbuffer, vc2idx, cosAng * dir, -sinAng, 0);
            if (i > 1)
                SET_TRIPLE(ibuffer, ic2idx, vc2start, vc2start + i + offs2, vc2start + i + offs1);
        }
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 24, vbuffer);
    glNormalPointer(GL_FLOAT, 24, vbuffer + 3);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, ibuffer);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    free(vbuffer);
    free(ibuffer);
}

void ExtrudeProfileLinear(float* profPoints, int nPoints, float fromX, float toX, float fromZ, float toZ, bool capStart, bool capEnd)
{
    int vidx = 0, vc1idx, vc2idx;
    int iidx = 0, ic1idx, ic2idx;
    int numVerts, numIndices;

    CalculateExtrudeBufferSizes(nPoints, capStart, capEnd, &numVerts, &numIndices, &vc1idx, &vc2idx, &ic1idx, &ic2idx);
    int vc1start = vc1idx / 6;
    int vc2start = vc2idx / 6;

    float* vbuffer = (float*)malloc(numVerts * 6 * sizeof(float));
    if (!vbuffer)
        return;
    GLushort* ibuffer = (GLushort*)malloc(numIndices * sizeof(float));
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

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 24, vbuffer);
    glNormalPointer(GL_FLOAT, 24, vbuffer + 3);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, ibuffer);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    free(vbuffer);
    free(ibuffer);
}

