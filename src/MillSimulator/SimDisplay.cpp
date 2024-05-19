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

#include "SimDisplay.h"
#include "linmath.h"
#include "OpenGlWrapper.h"

#define GL_UBYTE GL_UNSIGNED_BYTE

namespace MillSim
{

void SimDisplay::InitShaders()
{
    // use shaders
    //   standard diffuse shader
    shader3D.CompileShader(VertShader3DNorm, FragShaderNorm);
    shader3D.UpdateEnvColor(lightPos, lightColor, ambientCol, 0.0f);

    //   invarted normal diffuse shader for inner mesh
    shaderInv3D.CompileShader(VertShader3DInvNorm, FragShaderNorm);
    shaderInv3D.UpdateEnvColor(lightPos, lightColor, ambientCol, 0.0f);

    //   null shader to calculate meshes only (simulation stage)
    shaderFlat.CompileShader(VertShader3DNorm, FragShaderFlat);

    //   texture shader to render Simulator FBO
    shaderSimFbo.CompileShader(VertShader2DFbo, FragShader2dFbo);
    shaderSimFbo.UpdateTextureSlot(0);

    // geometric shader - generate texture with all geometric info for further processing
    shaderGeom.CompileShader(VertShaderGeom, FragShaderGeom);
    shaderInv3D.UpdateEnvColor(lightPos, lightColor, ambientCol, 0.0f);

    // ligthing shader - apply standard ligting based on geometric buffers
    shaderLighting.CompileShader(VertShader2DFbo, FragShaderStdLighting);
    shaderLighting.UpdateMultiTexSlots(0, 1, 2);
    shaderLighting.UpdateEnvColor(lightPos, lightColor, ambientCol, 0.0f);

    // SSAO shader - generate SSAO info and embed in texture buffer
    shaderSSAO.CompileShader(VertShader2DFbo, FragShaderSSAO);

    // SSAO blur shader - smooth generated SSAO texture
    shaderSSAOBlur.CompileShader(VertShader2DFbo, FragShaderSSAOBlur);

    // SSAO lighting shader - apply lightig modified by SSAO calculations
    shaderSSAOLighting.CompileShader(VertShader2DFbo, FragShaderSSAOLighting);
    shaderSSAOLighting.UpdateMultiTexSlots(0, 1, 2);
    shaderSSAOLighting.UpdateEnvColor(lightPos, lightColor, ambientCol, 0.0f);
}

void SimDisplay::CreateFboQuad()
{
    float quadVertices[] = {// a quad that fills the entire screen in Normalized Device Coordinates.
                            // positions   // texCoords
                            -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                            1.0f,  -1.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                            1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};

    glGenVertexArrays(1, &mFboQuadVAO);
    glGenBuffers(1, &mFboQuadVBO);
    glBindVertexArray(mFboQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mFboQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void SimDisplay::CreateDisplayFbos()
{
    // setup frame buffer for simulation
    glGenFramebuffers(1, &mFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

    // a color texture for the frame buffer
    glGenTextures(1, &mFboColTexture);
    glBindTexture(GL_TEXTURE_2D, mFboColTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WINDSIZE_W, WINDSIZE_H, 0, GL_RGBA, GL_UBYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFboColTexture, 0);

    // a position texture for the frame buffer
    glGenTextures(1, &mFboPosTexture);
    glBindTexture(GL_TEXTURE_2D, mFboPosTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDSIZE_W, WINDSIZE_H, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mFboPosTexture, 0);

    // a normal texture for the frame buffer
    glGenTextures(1, &mFboNormTexture);
    glBindTexture(GL_TEXTURE_2D, mFboNormTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDSIZE_W, WINDSIZE_H, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mFboNormTexture, 0);



    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0,
                                   GL_COLOR_ATTACHMENT1,
                                   GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &mRboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, mRboDepthStencil);
    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        WINDSIZE_W,
        WINDSIZE_H);  // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              mRboDepthStencil);  // now actually attach it

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffers(3, attachments);


    CreateFboQuad();
}


SimDisplay::~SimDisplay()
{
    glDeleteFramebuffers(1, &mFbo);
}

void SimDisplay::InitGL()
{
    // setup light object
    mlightObject.GenerateBoxStock(-0.5f, -0.5f, -0.5f, 1, 1, 1);

    InitShaders();
    CreateDisplayFbos();
    UpdateProjection();
}

void SimDisplay::PrepareDisplay(vec3 objCenter)
{
    mat4x4_look_at(mMatLookAt, eye, target, upvec);
    mat4x4_translate_in_place(mMatLookAt, mEyeX * mEyeXZFactor, 0, mEyeZ * mEyeXZFactor);
    mat4x4_rotate_X(mMatLookAt, mMatLookAt, mEyeInclination);
    mat4x4_rotate_Z(mMatLookAt, mMatLookAt, mEyeRoration);
    mat4x4_translate_in_place(mMatLookAt, -objCenter[0], -objCenter[1], -objCenter[2]);
}

void SimDisplay::StartDepthPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    shaderFlat.Activate();
    shaderFlat.UpdateViewMat(mMatLookAt);
}

void SimDisplay::StartGeometryPass(vec3 objColor, bool invertNormals)
{
    shaderGeom.Activate();
    shaderGeom.UpdateNormalState(invertNormals);
    shaderGeom.UpdateViewMat(mMatLookAt);
    shaderGeom.UpdateObjColor(objColor);

    //if (invertNormals) {
    //    shaderInv3D.Activate();
    //    shaderInv3D.UpdateViewMat(mMatLookAt);
    //    shaderInv3D.UpdateObjColor(objColor);
    //}
    //else {
    //    shader3D.Activate();
    //    shader3D.UpdateViewMat(mMatLookAt);
    //    shader3D.UpdateObjColor(objColor);
    //}
}

void SimDisplay::RenderLightObject()
{
    shaderFlat.Activate();
    shaderFlat.UpdateObjColor(lightColor);
    mlightObject.render();
}

void SimDisplay::ScaleViewToStock(StockObject* obj)
{
    mMaxStockDim = fmaxf(obj->size[0], obj->size[1]);
    maxFar = mMaxStockDim * 4;
    UpdateProjection();
    vec3_set(eye, 0, 0, 0);
    UpdateEyeFactor(0.4f);
    vec3_set(lightPos, obj->position[0], obj->position[1], obj->position[2] + mMaxStockDim / 3);
    mlightObject.SetPosition(lightPos);
}

void SimDisplay::RenderResult()
{
    // set default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // display the sim result within the FBO
    shaderLighting.Activate();
    //shaderSimFbo.Activate();
    glBindVertexArray(mFboQuadVAO);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFboColTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mFboPosTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mFboNormTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(glGetUniformLocation(shaderLighting.shaderId, "gAlbedo"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SimDisplay::TiltEye(float tiltStep)
{
    mEyeInclination += tiltStep;
    if (mEyeInclination > PI / 2) {
        mEyeInclination = PI / 2;
    }
    else if (mEyeInclination < -PI / 2) {
        mEyeInclination = -PI / 2;
    }
}

void SimDisplay::RotateEye(float rotStep)
{
    mEyeRoration += rotStep;
    if (mEyeRoration > PI2) {
        mEyeRoration -= PI2;
    }
    else if (mEyeRoration < 0) {
        mEyeRoration += PI2;
    }
    updateDisplay = true;
}

void SimDisplay::MoveEye(float x, float z)
{
    mEyeX += x;
    if (mEyeX > 100) {
        mEyeX = 100;
    }
    else if (mEyeX < -100) {
        mEyeX = -100;
    }
    mEyeZ += z;
    if (mEyeZ > 100) {
        mEyeZ = 100;
    }
    else if (mEyeZ < -100) {
        mEyeZ = -100;
    }
    updateDisplay = true;
}

void SimDisplay::UpdateEyeFactor(float factor)
{
    if (mEyeDistFactor == factor) {
        return;
    }
    updateDisplay = true;
    mEyeDistFactor = factor;
    mEyeXZFactor = factor * maxFar * 0.005f;
    eye[1] = -factor * maxFar;
}

void SimDisplay::UpdateProjection()
{
    // Setup projection
    mat4x4 projmat;
    mat4x4_perspective(projmat, 0.7f, 4.0f / 3.0f, 1.0f, maxFar);
    // mat4x4_perspective(projmat, 0.7f, 4.0f / 3.0f, 1, 100);
    shader3D.Activate();
    shader3D.UpdateProjectionMat(projmat);
    shaderInv3D.Activate();
    shaderInv3D.UpdateProjectionMat(projmat);
    shaderFlat.Activate();
    shaderFlat.UpdateProjectionMat(projmat);
    shaderGeom.Activate();
    shaderGeom.UpdateProjectionMat(projmat);
}


}  // namespace MillSim