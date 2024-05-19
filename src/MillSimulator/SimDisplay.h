/***************************************************************************
 *   Copyright (c) 2024 Shai Seger <shaise at gmail>                       *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __simdisplay_h__
#define __simdisplay_h__

#include "GlUtils.h"
#include "Shader.h"
#include "StockObject.h"


namespace MillSim
{

class SimDisplay
{
public:
    ~SimDisplay();
    void InitGL();
    void PrepareDisplay(vec3 objCenter);
    void StartDepthPass();
    void StartGeometryPass(vec3 objColor, bool invertNormals);
    void RenderLightObject();
    void ScaleViewToStock(StockObject* obj);
    void StartInvertedGeometryPass();
    void RenderResult();
    void TiltEye(float tiltStep);
    void RotateEye(float rotStep);
    void MoveEye(float x, float z);
    void UpdateEyeFactor(float factor);

    void UpdateProjection();
    float GetEyeFactor()
    {
        return mEyeDistFactor;
    }

public:
    bool applySSAO = false;
    bool updateDisplay = true;
    float maxFar = 100;

protected:
    void InitShaders();
    void CreateDisplayFbos();
    void CreateFboQuad();

protected:
    // shaders
    Shader shader3D, shaderInv3D, shaderFlat, shaderSimFbo;
    Shader shaderGeom, shaderSSAO, shaderLighting, shaderSSAOLighting, shaderSSAOBlur;
    vec3 lightColor = {0.8f, 0.9f, 1.0f};
    vec3 lightPos = {20.0f, 20.0f, 10.0f};
    vec3 ambientCol = {0.3f, 0.3f, 0.5f};

    vec3 eye = {0, 100, 40};
    vec3 target = {0, 0, -10};
    vec3 upvec = {0, 0, 1};

    mat4x4 mMatLookAt;
    StockObject mlightObject;


    float mEyeDistance = 30;
    float mEyeRoration = 0;
    float mEyeInclination = PI / 6;  // 30 degree
    float mEyeStep = PI / 36;        // 5 degree

    float mMaxStockDim = 100;
    float mEyeDistFactor = 0.0f;
    float mEyeXZFactor = 0.01f;
    float mEyeXZScale = 0;
    float mEyeX = 0.0f;
    float mEyeZ = 0.0f;

    // frame buffer
    unsigned int mFbo;
    unsigned int mFboColTexture;
    unsigned int mFboPosTexture;
    unsigned int mFboNormTexture;
    unsigned int mRboDepthStencil;
    unsigned int mFboQuadVAO, mFboQuadVBO;
};

}  // namespace MillSim
#endif  // !__simdisplay_h__