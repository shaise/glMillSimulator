#include "SimShapes.h"
#include <math.h>

#define PI 3.14159265 

float* sinTable = nullptr;
float* cosTable = nullptr;
int lastNumSlices = 0;
int lastNumSectionIndices = 0;
GLshort quadIndices[] = { 0, 2, 3, 0, 3, 1 };
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
    nverts = (nSlices + 1) * 3;
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

// extrude profile parallel to its plane
void ExtrudeProfilePar(float* profPoints, int nPoints, float distance, float deltaHeight)
{
    float vertices[3 * 4];
    //float normals[3 * 4];

    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    //glNormalPointer(GL_FLOAT, 0, normals);
    nPoints *= 2;
    for (int i = 0; i < nPoints; i += 2)
    {
        int idx = 0;
        float y1 = profPoints[i];
        float z1 = profPoints[i + 1];
        int i2 = (i + 2) % nPoints;
        float y2 = profPoints[i2];
        float z2 = profPoints[i2 + 1];
        // verts
        SET_TRIPLE(vertices, idx, 0, y1, z1);
        SET_TRIPLE(vertices, idx, 0, y2, z2);
        SET_TRIPLE(vertices, idx, distance, y1, z1);
        SET_TRIPLE(vertices, idx, distance, y2, z2);

        // normals
        float ydiff = y2 - y1;
        float zdiff = z2 - z1;
        float len = sqrtf(ydiff * ydiff + zdiff * zdiff);
        float ny = -zdiff / len;
        float nz = ydiff / len;
        glNormal3f(0, ny, nz);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndices);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}


void FillNgon(float* vertices, int nPoints, float normX, float normY, float normZ)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormal3f(normX, normY, normZ);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nPoints);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// extrude profile radially 
void ExtrudeProfileRad(float* profPoints, int nPoints, float radius, float angleRad, float deltaHeight, bool capStart, bool capEnd)
{
    int nverts = nPoints * 3;
    float vertices[3 * 4];
    float* capStartVerts = nullptr, *capEndVerts = nullptr;
    if (capStart || capEnd)
    {
        capStartVerts = (float*)malloc(2 * nverts * sizeof(float));
        if (capStartVerts == nullptr)
            return;
    }
    if (capEnd)
        capEndVerts = capStartVerts + nverts;
    int capStartIdx = 0;
    int capEndIdx = nverts - 3;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    int nPointsD = nPoints * 2;
    float cosAng = cosf(angleRad);
    float sinAng = sinf(angleRad);
    for (int i = 0; i < nPointsD; i += 2)
    {
        int idx = 0;
        float y1 = profPoints[i] + radius;
        float z1 = profPoints[i + 1];
        int i2 = (i + 2) % nPointsD;
        float y2 = profPoints[i2] + radius;
        float z2 = profPoints[i2 + 1];

        // start verts
        SET_TRIPLE(vertices, idx, 0, y1, z1);
        SET_TRIPLE(vertices, idx, 0, y2, z2);
        if (capStart)
            SET_TRIPLE(capStartVerts, capStartIdx, 0, y1, z1);

        float x1 = y1 * sinAng;
        float x2 = y2 * sinAng;
        y1 *= cosAng;
        y2 *= cosAng;
        z1 += deltaHeight;
        z2 += deltaHeight;
        SET_TRIPLE(vertices, idx, x1, y1, z1);
        SET_TRIPLE(vertices, idx, x2, y2, z2);
        if (capEnd)
        {
            SET_TRIPLE(capEndVerts, capEndIdx, x1, y1, z1);
            capEndIdx -= 6; // end cap winding reversed so vert order is reversed
        }

        // normals
        float ydiff = y2 - y1;
        float zdiff = z2 - z1;
        float len = sqrtf(ydiff * ydiff + zdiff * zdiff);
        float ny = -zdiff / len;
        float nz = ydiff / len;
        float nx = -sinAng * ny;
        ny *= cosAng;
        glNormal3f(nx, ny, nz);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndices);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    if (capStart)
        FillNgon(capStartVerts, nPoints, -1, 0, 0);
    if (capEnd)
        FillNgon(capEndVerts, nPoints, cosAng, -sinAng, 0);
}

void TesselateProfile(float* profPoints, int nPoints, float distance, float deltaHeight)
{
    int nverts = nPoints * 3;
    float* vertices = (float*)malloc(nverts * sizeof(float));
    if (vertices == nullptr)
        return;

    int idx = 0;
    for (int i = 0; i < nPoints; i ++)
    {
        int i2 = i * 2;
        SET_TRIPLE(vertices, idx, distance, profPoints[i2], profPoints[i2 + 1]);
    }
    float normX = (distance > 0.0) ? 1 : -1;
    FillNgon(vertices, nPoints, normX, 0, 0);
    free(vertices);
}


void SolidCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks) {

    GLUquadricObj* qobj = gluNewQuadric();

    gluCylinder(qobj, radius, radius, height, slices, stacks);
    gluQuadricOrientation(qobj, GLU_INSIDE);
    gluDisk(qobj, 0.0, radius, slices, stacks);
    gluQuadricOrientation(qobj, GLU_OUTSIDE);
    glTranslatef(0.0f, 0.0f, static_cast<GLfloat>(height));
    gluDisk(qobj, 0.0, radius, slices, stacks);

    gluDeleteQuadric(qobj);
}
