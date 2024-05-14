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

#include "MillSimulation.h"
#include "OpenGlWrapper.h"
#include <vector>
#include <iostream>

namespace MillSim
{

MillSimulation::MillSimulation()
{
    mCurMotion = {eNop, -1, 0, 0, 0, 0, 0, 0};
    guiDisplay.SetMillSimulator(this);
}

MillSimulation::~MillSimulation()
{
    glDeleteFramebuffers(1, &mFbo);
}

void MillSimulation::ClearMillPathSegments()
{
    // for (std::vector<MillPathSegment*>::const_iterator i = MillPathSegments.begin(); i !=
    // MillPathSegments.end(); ++i) {
    //     MillSim::MillPathSegment* p = *i;
    //     delete p;
    // }
    for (int i = 0; i < MillPathSegments.size(); i++) {
        delete MillPathSegments[i];
    }
    MillPathSegments.clear();
}

void MillSimulation::Clear()
{
    mCodeParser.Operations.clear();
    for (int i = 0; i < mToolTable.size(); i++) {
        delete mToolTable[i];
    }
    mToolTable.clear();
    mCurStep = 0;
    mPathStep = -1;
    mNTotalSteps = 0;
}


void MillSimulation::SimNext()
{
    static int simDecim = 0;

    simDecim++;
    if (simDecim < 1) {
        return;
    }

    simDecim = 0;

    if (mCurStep < mNTotalSteps) {
        mCurStep += mSimSpeed;
        CalcSegmentPositions();
    }
}

void MillSimulation::InitSimulation(float quality)
{
    ClearMillPathSegments();

    mDestMotion = mZeroPos;
    // gDestPos = curMillOperation->startPos;
    mCurStep = 0;
    mPathStep = -1;
    mNTotalSteps = 0;
    MillPathSegment::SetQuality(quality, mMaxFar);
    int nOperations = (int)mCodeParser.Operations.size();
    ;
    for (int i = 0; i < nOperations; i++) {
        mCurMotion = mDestMotion;
        mDestMotion = mCodeParser.Operations[i];
        EndMill* tool = GetTool(mDestMotion.tool);
        if (tool != nullptr) {
            MillSim::MillPathSegment* segment =
                new MillSim::MillPathSegment(tool, &mCurMotion, &mDestMotion);
            segment->indexInArray = i;
            mNTotalSteps += segment->numSimSteps;
            MillPathSegments.push_back(segment);
        }
    }
    mNPathSteps = (int)MillPathSegments.size();
    InitDisplay(quality);
}

EndMill* MillSimulation::GetTool(int toolId)
{
    for (int i = 0; i < mToolTable.size(); i++) {
        if (mToolTable[i]->mToolId == toolId) {
            return mToolTable[i];
        }
    }
    return nullptr;
}

void MillSimulation::RemoveTool(int toolId)
{
    EndMill* tool;
    if ((tool = GetTool(toolId)) != nullptr) {
        auto it = std::find(mToolTable.begin(), mToolTable.end(), tool);
        if (it != mToolTable.end()) {
            mToolTable.erase(it);
        }
        delete tool;
    }
}

void MillSimulation::AddTool(EndMill* tool)
{
    // if we have another tool with same id, remove it
    RemoveTool(tool->mToolId);
    mToolTable.push_back(tool);
}

void MillSimulation::AddTool(const float* toolProfile, int numPoints, int toolid, float diameter)
{
    // if we have another tool with same id, remove it
    RemoveTool(toolid);
    EndMill* tool = new EndMill(toolProfile, numPoints, toolid, diameter);
    mToolTable.push_back(tool);
}

void MillSimulation::GlsimStart()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void MillSimulation::GlsimToolStep1(void)
{
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
    glDepthMask(GL_FALSE);
}


void MillSimulation::GlsimToolStep2(void)
{
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDepthFunc(GL_GREATER);
    glCullFace(GL_FRONT);
    glDepthMask(GL_TRUE);
}

void MillSimulation::GlsimClipBack(void)
{
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_ZERO);
    glDepthFunc(GL_LESS);
    glCullFace(GL_FRONT);
    glDepthMask(GL_FALSE);
}


void MillSimulation::GlsimRenderStock(void)
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDepthFunc(GL_EQUAL);
    glCullFace(GL_BACK);
}

void MillSimulation::GlsimRenderTools(void)
{
    glCullFace(GL_FRONT);
}

void MillSimulation::GlsimEnd(void)
{
    glCullFace(GL_BACK);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void MillSimulation::renderSegmentForward(int iSeg)
{
    MillSim::MillPathSegment* p = MillPathSegments.at(iSeg);
    int step = iSeg == mPathStep ? mSubStep : p->numSimSteps;
    int start = p->isMultyPart ? 1 : step;
    for (int i = start; i <= step; i++) {
        GlsimToolStep1();
        p->render(i);
        GlsimToolStep2();
        p->render(i);
    }
}

void MillSimulation::renderSegmentReversed(int iSeg)
{
    MillSim::MillPathSegment* p = MillPathSegments.at(iSeg);
    int step = iSeg == mPathStep ? mSubStep : p->numSimSteps;
    int end = p->isMultyPart ? 1 : step;
    for (int i = step; i >= end; i--) {
        GlsimToolStep1();
        p->render(i);
        GlsimToolStep2();
        p->render(i);
    }
}

void MillSimulation::CalcSegmentPositions()
{
    mSubStep = mCurStep;
    for (mPathStep = 0; mPathStep < mNPathSteps; mPathStep++) {
        MillSim::MillPathSegment* p = MillPathSegments[mPathStep];
        if (mSubStep < p->numSimSteps) {
            break;
        }
        mSubStep -= p->numSimSteps;
    }
    if (mPathStep >= mNPathSteps) {
        mPathStep = mNPathSteps - 1;
        mSubStep = MillPathSegments[mPathStep]->numSimSteps;
    }
    else {
        mSubStep++;
    }
}

void MillSimulation::RenderSimulation()
{
    shaderFlat.Activate();
    shaderFlat.UpdateViewMat(mMatLookAt);

    GlsimStart();
    mStockObject.render();

    GlsimToolStep2();

    for (int i = 0; i <= mPathStep; i++) {
        renderSegmentForward(i);
    }

    for (int i = mPathStep; i >= 0; i--) {
        renderSegmentForward(i);
    }

    for (int i = 0; i < mPathStep; i++) {
        renderSegmentReversed(i);
    }

    for (int i = mPathStep; i >= 0; i--) {
        renderSegmentReversed(i);
    }

    GlsimClipBack();
    mStockObject.render();

    // start coloring
    shader3D.Activate();
    shader3D.UpdateViewMat(mMatLookAt);
    shader3D.UpdateObjColor(stockColor);
    GlsimRenderStock();
    mStockObject.render();
    GlsimRenderTools();

    // render cuts (back faces of tools)

    shaderInv3D.Activate();
    shaderInv3D.UpdateViewMat(mMatLookAt);
    shaderInv3D.UpdateObjColor(cutColor);
    for (int i = 0; i <= mPathStep; i++) {
        MillSim::MillPathSegment* p = MillPathSegments.at(i);
        int step = (i == mPathStep) ? mSubStep : p->numSimSteps;
        int start = p->isMultyPart ? 1 : step;
        for (int j = start; j <= step; j++) {
            MillPathSegments.at(i)->render(j);
        }
    }

    GlsimEnd();

    glEnable(GL_CULL_FACE);

    if (mPathStep >= 0) {
        vec3 toolPos;
        MotionPosToVec(toolPos, &mDestMotion);
        MillSim::MillPathSegment* p = MillPathSegments.at(mPathStep);
        p->GetHeadPosition(toolPos);
        mat4x4 tmat;
        mat4x4_translate(tmat, toolPos[0], toolPos[1], toolPos[2]);
        // mat4x4_translate(tmat, toolPos.x, toolPos.y, toolPos.z);
        shader3D.Activate();
        shader3D.UpdateObjColor(toolColor);
        p->mEndmill->mToolShape.Render(tmat, identityMat);
    }

    shaderFlat.Activate();
    shaderFlat.UpdateObjColor(lightColor);
    mlightObject.render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MillSimulation::Render()
{
    mat4x4_look_at(mMatLookAt, eye, target, upvec);

    // set background
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    mat4x4_translate_in_place(mMatLookAt, mEyeX * mEyeXZFactor, 0, mEyeZ * mEyeXZFactor);
    mat4x4_rotate_X(mMatLookAt, mMatLookAt, mEyeInclination);
    mat4x4_rotate_Z(mMatLookAt, mMatLookAt, mEyeRoration);
    mat4x4_translate_in_place(mMatLookAt,
                              -mStockObject.mCenter[0],
                              -mStockObject.mCenter[1],
                              -mStockObject.mCenter[2]);

     // render the simulation offscreen in an FBO
    if (mSimRefresh) {
        RenderSimulation();
        mSimRefresh = false;
    }

     // display the sim result within the FBO
    shaderSimFbo.Activate();
    glBindVertexArray(mFboQuadVAO);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFboTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (mDebug > 0) {
        mat4x4 test;
        mat4x4_identity(test);
        mat4x4_translate_in_place(test, 20, 20, 3);
        mat4x4_rotate_Z(test, test, 30.f * 3.14f / 180.f);
        int dpos = mNPathSteps - mDebug2;
        MillSim::MillPathSegment* p = MillPathSegments.at(dpos);
        if (mDebug > p->numSimSteps) {
            mDebug = 1;
        }
        p->render(mDebug);
    }
    float progress = (float)mCurStep / mNTotalSteps;
    guiDisplay.Render(progress);
}

void MillSimulation::ProcessSim(unsigned int time_ms)
{

    static int ancient = 0;
    static int last = 0;
    static int msec = 0;
    static int fps = 0;
    static int renderTime = 0;

    last = msec;
    msec = time_ms;
    if (mIsRotate) {
        RotateEye((msec - last) / 4600.0f);
    }

    if (last / 1000 != msec / 1000) {
        float calcFps = 1000.0f * fps / (msec - ancient);
        mFpsStream.str("");
        mFpsStream << "fps: " << calcFps << "    rendertime:" << renderTime
                   << "    zpos:" << mDestMotion.z << std::ends;
        ancient = msec;
        fps = 0;
    }

    if (mSimPlaying || mSingleStep) {
        SimNext();
        mSingleStep = false;
    }

    Render();

    ++fps;
}

void MillSimulation::HandleKeyPress(int key)
{
    switch (key) {
        case ' ':
            mIsRotate = !mIsRotate;
            break;

        case 'S':
            mSimPlaying = true;
            break;

        case 'P':
            mSimPlaying = false;
            break;

        case 'T':
            mSimPlaying = false;
            mSingleStep = true;
            break;

        case 'D':
            mDebug++;
            break;

        case 'K':
            mDebug2++;
            gDebug = mNPathSteps - mDebug2;
            break;

        case 'F':
            if (mSimSpeed == 1) {
                mSimSpeed = 10;
            }
            else if (mSimSpeed == 10) {
                mSimSpeed = 40;
            }
            else {
                mSimSpeed = 1;
            }
            guiDisplay.UpdateSimSpeed(mSimSpeed);
            break;

        default:
            if (key >= '1' && key <= '9') {
                mSimSpeed = key - '0';
            }
            break;
    }
    guiDisplay.UpdatePlayState(mSimPlaying);
}

void MillSimulation::UpdateEyeFactor(float factor)
{
    mEyeDistFactor = factor;
    mEyeXZFactor = factor * mMaxFar * 0.005f;
    eye[1] = -factor * mMaxFar;
}

void MillSimulation::TiltEye(float tiltStep)
{
    mEyeInclination += tiltStep;
    if (mEyeInclination > PI / 2) {
        mEyeInclination = PI / 2;
    }
    else if (mEyeInclination < -PI / 2) {
        mEyeInclination = -PI / 2;
    }
}

void MillSimulation::RotateEye(float rotStep)
{
    mEyeRoration += rotStep;
    if (mEyeRoration > PI2) {
        mEyeRoration -= PI2;
    }
    else if (mEyeRoration < 0) {
        mEyeRoration += PI2;
    }
    mSimRefresh = true;
}

void MillSimulation::MoveEye(float x, float z)
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
    mSimRefresh = true;
}

void MillSimulation::UpdateProjection()
{
    // Setup projection
    mat4x4 projmat;
    mat4x4_perspective(projmat, 0.7f, 4.0f / 3.0f, 1.0f, mMaxFar);
    // mat4x4_perspective(projmat, 0.7f, 4.0f / 3.0f, 1, 100);
    shader3D.Activate();
    shader3D.UpdateProjectionMat(projmat);
    shaderInv3D.Activate();
    shaderInv3D.UpdateProjectionMat(projmat);
    shaderFlat.Activate();
    shaderFlat.UpdateProjectionMat(projmat);
}

void MillSimulation::CreateFboQuad()
{
    float quadVertices[] = {
        // a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

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

void MillSimulation::CreateSimulationFbo()
{
    // setup frame buffer for simulation
    glGenFramebuffers(1, &mFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

    // a texture for the frame buffer
    glGenTextures(1, &mFboTexture);
    glBindTexture(GL_TEXTURE_2D, mFboTexture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 WINDSIZE_W,
                 WINDSIZE_H,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFboTexture, 0);

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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return;
    }

    CreateFboQuad();

}

void MillSimulation::InitDisplay(float quality)
{
    // use shaders
    //   standard diffuse shader
    shader3D.CompileShader(VertShader3DNorm, FragShaderNorm);
    shader3D.UpdateEnvColor(lightPos, lightColor, ambientCol);

    //   invarted normal diffuse shader for inner mesh
    shaderInv3D.CompileShader(VertShader3DInvNorm, FragShaderNorm);
    shaderInv3D.UpdateEnvColor(lightPos, lightColor, ambientCol);

    //   null shader to calculate meshes only (simulation stage)
    shaderFlat.CompileShader(VertShader3DNorm, FragShaderFlat);

    //   texture shader to render Simulator FBO
    shaderSimFbo.CompileShader(VertShader2DFbo, FragShader2dFbo);
    shaderSimFbo.UpdateTextureSlot(0);

    UpdateProjection();

    // setup light object and generate tools
    mlightObject.GenerateBoxStock(-0.5f, -0.5f, -0.5f, 1, 1, 1);
    for (int i = 0; i < mToolTable.size(); i++) {
        mToolTable[i]->GenerateDisplayLists(quality);
    }

    // separate frame buffer for simulation
    CreateSimulationFbo();
 
    // init gui elements
    guiDisplay.InutGui();
}

void MillSimulation::SetBoxStock(float x, float y, float z, float l, float w, float h)
{
    mStockObject.GenerateBoxStock(x, y, z, l, w, h);
    mMaxStockDim = fmaxf(w, l);
    mMaxFar = mMaxStockDim * 4;
    UpdateProjection();
    vec3_set(eye, 0, 0, 0);
    UpdateEyeFactor(0.4f);
    vec3_set(lightPos, x, y, h + mMaxStockDim / 3);
    mlightObject.SetPosition(lightPos);
}

void MillSimulation::MouseDrag(int buttons, int dx, int dy)
{
    if (buttons == (MS_MOUSE_MID | MS_MOUSE_LEFT)) {
        TiltEye((float)dy / 100.0f);
        RotateEye((float)dx / 100.0f);
    }
    else if (buttons == MS_MOUSE_MID) {
        MoveEye(dx, -dy);
    }
    guiDisplay.MouseDrag(buttons, dx, dy);
}

void MillSimulation::MouseMove(int px, int py)
{
    if (mMouseButtonState > 0) {
        int dx = px - mLastMouseX;
        int dy = py - mLastMouseY;
        if (dx != 0 || dy != 0) {
            MouseDrag(mMouseButtonState, dx, dy);
            mLastMouseX = px;
            mLastMouseY = py;
        }
    }
    else {
        MouseHover(px, py);
    }
}

void MillSimulation::MouseScroll(float dy)
{
    float f = mEyeDistFactor;
    f += 0.05f * dy;
    if (f > 0.6f) {
        f = 0.6f;
    }
    else if (f < 0.05f) {
        f = 0.05f;
    }
    UpdateEyeFactor(f);
}


void MillSimulation::MouseHover(int px, int py)
{
    guiDisplay.MouseCursorPos(px, py);
}

void MillSimulation::MousePress(int button, bool isPressed, int px, int py)
{
    if (isPressed) {
        mMouseButtonState |= button;
    }
    else {
        mMouseButtonState &= ~button;
    }

    if (mMouseButtonState > 0) {
        mLastMouseX = px;
        mLastMouseY = py;
    }
    guiDisplay.MousePressed(button, isPressed, mSimPlaying);
}


bool MillSimulation::LoadGCodeFile(const char* fileName)
{
    if (mCodeParser.Parse(fileName)) {
        std::cout << "GCode file loaded successfuly" << std::endl;
        return true;
    }
    return false;
}

bool MillSimulation::AddGcodeLine(const char* line)
{
    return mCodeParser.AddLine(line);
}

void MillSimulation::SetSimulationStage(float stage)
{
    int newStep = (int)((float)mNTotalSteps * stage);
    if (newStep == mCurStep) {
        return;
    }
    mCurStep = newStep;
    mSimRefresh = true;
    mSingleStep = true;
    CalcSegmentPositions();
}

}  // namespace MillSim