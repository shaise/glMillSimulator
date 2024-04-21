#include "MillSimulation.h"
#include "GL/glew.h"
#include "GlUtils.h"
#include <vector>
#include <iostream>

namespace MillSim {

    MillSimulation::MillSimulation()
    {
    }

    void MillSimulation::ClearMillPathSegments() {
        for (std::vector<MillPathSegment*>::const_iterator i = MillPathSegments.begin(); i != MillPathSegments.end(); ++i) {
            MillSim::MillPathSegment* p = *i;
            delete p;
        }
        MillPathSegments.clear();
    }

    void MillSimulation::SimNext()
    {
        static int simDecim = 0;

        simDecim++;
        if (simDecim < 1)
            return;

        simDecim = 0;

        mCurStep += mSimSpeed;
        while (mCurStep > mNSteps)
        {
            if (mPathStep >= mNPathSteps)
            {
                mCurStep = mNSteps;
                return;
            }

            mCurMotion = mDestMotion;
            mDestMotion = mCodeParser.Operations[mPathStep];
            if (mDestMotion.tool != mLastToolId)
            {
                SetTool(mDestMotion.tool);
            }

            if (curTool != nullptr)
            {
                MillSim::MillPathSegment* segment = new MillSim::MillPathSegment(curTool, &mCurMotion, &mDestMotion);
                mCurStep -= mNSteps;
                mNSteps = segment->numSimSteps;
                MillPathSegments.push_back(segment);
            }

            mPathStep++;
        }
    }

    void MillSimulation::InitSimulation()
    {
        ClearMillPathSegments();
        curTool = nullptr;

        mDestMotion = mZeroPos;
        //gDestPos = curMillOperation->startPos;
        mCurStep = 0;
        mNSteps = -1;
        mLastToolId = -1;
        mNPathSteps = (int)mCodeParser.Operations.size();;
        mPathStep = 0;
    }

    void MillSimulation::SetTool(int tool) {
        //curMillOpIx = 0;
        curTool = nullptr;
        for (int i = 0; i < mToolTable.size(); i++)
        {
            if (mToolTable[i]->mToolId == tool)
            {
                curTool = mToolTable[i];
            }
        }            
        mLastToolId = tool;
    }

    void MillSimulation::AddTool(EndMill* tool)
    {
        mToolTable.push_back(tool);
    }

    void MillSimulation::GlsimStart()
    {
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
        int step = iSeg == (MillPathSegments.size() - 1) ? mCurStep : p->numSimSteps;
        int start = p->isMultyPart ? 1 : step;
        for (int i = start; i <= step; i++)
        {
            GlsimToolStep1();
            p->render(i);
            GlsimToolStep2();
            p->render(i);
        }
    }

    void MillSimulation::renderSegmentReversed(int iSeg)
    {
        MillSim::MillPathSegment* p = MillPathSegments.at(iSeg);
        int step = iSeg == (MillPathSegments.size() - 1) ? mCurStep : p->numSimSteps;
        int end = p->isMultyPart ? 1 : step;
        for (int i = step; i >= end; i--)
        {
            GlsimToolStep1();
            p->render(i);
            GlsimToolStep2();
            p->render(i);
        }
    }

    void MillSimulation::Render()
    {
        mat4x4 matLookAt, model;
        mat4x4_identity(model);
        eye[2] = mEyeHeight;
        mat4x4_look_at(matLookAt, eye, target, upvec);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        mat4x4_rotate_Z(matLookAt, matLookAt, mEyeRoration);

        shaderFlat.Activate();
        shaderFlat.UpdateViewMat(matLookAt);

        GlsimStart();
        gStockObject->render();

        int len = (int)MillPathSegments.size();

        GlsimToolStep2();

        for (int i = 0; i < len; i++)
            renderSegmentForward(i);

        for (int i = len - 1; i >= 0; i--)
            renderSegmentForward(i);

        for (int i = 0; i < len; i++)
            renderSegmentReversed(i);

        for (int i = len - 1; i >= 0; i--)
            renderSegmentReversed(i);

        GlsimClipBack();
        gStockObject->render();

        // start coloring
        shader3D.Activate();
        shader3D.UpdateViewMat(matLookAt);
        shader3D.UpdateObjColor(stockColor);
        GlsimRenderStock();
        gStockObject->render();
        GlsimRenderTools();

        // render cuts (back faces of tools)

        shaderInv3D.Activate();
        shaderInv3D.UpdateViewMat(matLookAt);
        shaderInv3D.UpdateObjColor(cutColor);
        for (int i = 0; i < len; i++)
        {
            MillSim::MillPathSegment* p = MillPathSegments.at(i);
            int step = (i == (len - 1)) ? mCurStep : p->numSimSteps;
            int start = p->isMultyPart ? 1 : step;
            for (int j = start; j <= step; j++)
                MillPathSegments.at(i)->render(j);
        }

        GlsimEnd();

        glEnable(GL_CULL_FACE);

        if (curTool)
        {
            vec3 toolPos;
            //Vector3 toolPos;
            MotionPosToVec(toolPos, &mDestMotion);
            //toolPos.FromMillMotion(&gDestMotion);
            if (len > 0)
            {
                MillSim::MillPathSegment* p = MillPathSegments.at(len - 1);
                p->GetHeadPosition(toolPos);
                //toolPos = *p->GetHeadPosition();
            }
            mat4x4 tmat;
            mat4x4_translate(tmat, toolPos[0], toolPos[1], toolPos[2]);
            //mat4x4_translate(tmat, toolPos.x, toolPos.y, toolPos.z);
            shader3D.Activate();
            shader3D.UpdateObjColor(toolColor);
            curTool->mToolShape.Render(tmat, identityMat);
        }

        shaderFlat.Activate();
        shaderFlat.UpdateObjColor(lightColor);
        glightObject->render();

        if (mDebug > 0)
        {
            mat4x4 test;
            mat4x4_dup(test, model);
            mat4x4_translate_in_place(test, 20, 20, 3);
            mat4x4_rotate_Z(test, test, 30.f * 3.14f / 180.f);
            if (mDebug >= MillPathSegments.size())
                mDebug = 1;
            MillSim::MillPathSegment* p = MillPathSegments.at(mDebug);
            p->render(1);
        }
    }

    void MillSimulation::ProcessSim(unsigned int time_ms) {

        static int ancient = 0;
        static int last = 0;
        static int msec = 0;
        static int fps = 0;
        static int renderTime = 0;

        last = msec;
        msec = time_ms;
        if (mIsRotate) {
            mEyeRoration += (msec - last) / 4600.0f;
            while (mEyeRoration >= PI2)
                mEyeRoration -= PI2;
        }

        if (last / 1000 != msec / 1000) {
            float calcFps = 1000.0f * fps / (msec - ancient);
            mFpsStream.str("");
            mFpsStream << "fps: " << calcFps << "    rendertime:" << renderTime << "    zpos:" << mDestMotion.z << std::ends;
            ancient = msec;
            fps = 0;
        }

        if (mSimPlaying || mSingleStep)
        {
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
            mSimPlaying = !mSimPlaying;
            break;
        case 'T':
            mSimPlaying = false;
            mSingleStep = true;
            break;
        case'D':
            mDebug++;
            break;
        default:
            if (key >= '1' && key <= '9')
                mSimSpeed = key - '0';
            break;
        }
    }

    void MillSimulation::TiltEye(float tiltStep)
    {
        mEyeHeight += tiltStep;
    }

    void MillSimulation::InitDisplay()
    {
        // gray background
        GL(glClearColor(0.7f, 0.7f, 0.4f, 1.0f));

        // Setup projection
        mat4x4 projmat;
        mat4x4_perspective(projmat, 0.7f, 4.0f / 3.0f, 1.0f, 200.0f);

        // use shaders
        //   standard diffuse shader
        shader3D.CompileShader((char*)VertShader3DNorm, (char*)FragShaderNorm);
        shader3D.UpdateEnvColor(lightPos, lightColor, ambientCol);
        shader3D.UpdateProjectionMat(projmat);

        //   invarted normal diffuse shader for inner mesh
        shaderInv3D.CompileShader((char*)VertShader3DInvNorm, (char*)FragShaderNorm);
        shaderInv3D.UpdateEnvColor(lightPos, lightColor, ambientCol);
        shaderInv3D.UpdateProjectionMat(projmat);

        //   null shader to calculate meshes only (simulation stage)
        shaderFlat.CompileShader((char*)VertShader3DNorm, (char*)FragShaderFlat);
        shaderFlat.UpdateProjectionMat(projmat);

        glMatrixMode(GL_MODELVIEW);

        // setup tools ans stock
        //MillSim::resolution = 0.1;
        gStockObject = new MillSim::StockObject(0, 0, -8.7f, 50, 50, 8.7f);
        glightObject = new MillSim::StockObject(-0.5f, -0.5f, -0.5f, 1, 1, 1);
        glightObject->SetPosition(lightPos);
        for (int i = 0; i < mToolTable.size(); i++)
            mToolTable[i]->GenerateDisplayLists();
    }

    bool MillSimulation::LoadGCodeFile(const char* fileName)
    {
        if (mCodeParser.Parse(fileName))
        {
            std::cout << "GCode file loaded successfuly" << std::endl;
            return true;
        }
        return false;
    }








    MillMotion DemoSimulation[] = {
    {eMoveLiner,  1, -0.7f, -0.7f, 10},
    {eMoveLiner,  1, -0.7f, -0.7f, 1},
    {eMoveLiner,  1, 0.7f, 0.7f, 1, 0.7f, 0.7f, 0},
    {eMoveLiner,  1, 0.7f, 0.7f, 10},

    {eMoveLiner,  1, -3, -3, 10},
    {eMoveLiner,  1, -3, -3, 0.5f},
    {eMoveLiner,  1, 3, 3, 0.5f, 3, 3, -1},
    {eMoveLiner,  1, 3, 3, 10},

    {eMoveLiner,  1, 15, 15, 10},
    {eMoveLiner,  1,  15, 15, 1.5f},
    {eMoveLiner,  1, 15, -15, 1.5f},
    {eMoveLiner,  1, -15, -15, 1.5f},
    {eMoveLiner,  1, -15, 15, 1.5f},
    {eMoveLiner,  1, 15, 15, 1.5f},

    {eMoveLiner,  1, 15, 15, 1},
    {eMoveLiner,  1, 15, -15, 1},
    {eMoveLiner,  1, -15, -15, 1},
    {eMoveLiner,  1, -15, 15, 1},
    {eMoveLiner,  1, 15, 15, 1},

    {eMoveLiner,  1, 15, 15, 0.5f},
    {eMoveLiner,  1, 15, -15, 0.5f},
    {eMoveLiner,  1, -15, -15, 0.5f},
    {eMoveLiner,  1, -15, 15, 0.5f},
    {eMoveLiner,  1, 15, 15, 0.5f},

    {eMoveLiner,  1, 15, 15, 0},
    {eMoveLiner,  1, 15, -15, 0},
    {eMoveLiner,  1, -15, -15, 0},
    {eMoveLiner,  1, -15, 15, 0},
    {eMoveLiner,  1, 15, 15, 0},

    {eMoveLiner,  1, 15, 15, 10},

    {eMoveLiner,  1, 8, 8, 10},
    {eMoveLiner,  1, 8, 8, 1.5f},
    {eMoveLiner,  1, 8, -8, 1.5f},
    {eMoveLiner,  1, 6.1f, -8, 1.5f},
    {eMoveLiner,  1, 6.1f, 8, 1.5f},
    {eMoveLiner,  1, 4.2f, 8, 1.5f},
    {eMoveLiner,  1, 4.2f, -8, 1.5f},
    {eMoveLiner,  1, 2.3f, -8, 1.5f},
    {eMoveLiner,  1, 2.3f, 8, 1.5f},
    {eMoveLiner,  1, 0.4f, 8, 1.5f},
    {eMoveLiner,  1, 0.4f, -8, 1.5f},
    {eMoveLiner,  1, -1.5f, -8, 1.5f},
    {eMoveLiner,  1, -1.5f, 8, 1.5f},
    {eMoveLiner,  1, -3.4f, 8, 1.5f},
    {eMoveLiner,  1, -3.4f, -8, 1.5f},
    {eMoveLiner,  1, -5.3f, -8, 1.5f},
    {eMoveLiner,  1, -5.3f, 8, 1.5f},
    {eMoveLiner,  1, -7.2f, 8, 1.5f},
    {eMoveLiner,  1, -7.2f, -8, 1.5f},
    {eMoveLiner,  1, -8,  -8, 1.5f},
    {eMoveLiner,  1, -8,  8, 1.5f},
    {eMoveLiner,  1,  8,  8, 1.5f},
    {eMoveLiner,  1,  8,  -8, 1.5f},
    {eMoveLiner,  1, -8,  -8, 1.5f},

    {eMoveLiner,  1, -8,  -8, 10},

    // taper mill motion
    {eMoveLiner,  3, 14.2f, 14.2f, 10},
    {eMoveLiner,  3, 14.2f, 14.2f, 1.5f},
    {eMoveLiner,  3, 14.2f, -14.2f, 1.5f},
    {eMoveLiner,  3, -14.2f, -14.2f, 1.5f},
    {eMoveLiner,  3, -14.2f, 14.2f, 1.5f},
    {eMoveLiner,  3, 14.2f, 14.2f, 1.5f},
    {eMoveLiner,  3, 14.2f, 14.2f, 10},
    {eMoveLiner,  3, 0, 0, 10},

    // ball mill motion
    {eMoveLiner,  2, 12, 12, 10},
    {eMoveLiner,  2, 12, 12, 1.5f},
    {eMoveLiner,  2, 12, -12, 2.5f},
    {eMoveLiner,  2, -12, -12, 1.5f},
    {eMoveLiner,  2, -12, 12, 2.5f},
    {eMoveLiner,  2, 12, 12, 1.5f},
    {eMoveLiner,  2, 12, 12, 10},
    {eMoveLiner,  2, 0, 0, 10},
    };

#define NUM_DEMO_MOTIONS (sizeof(DemoSimulation) / sizeof(MillMotion))


}