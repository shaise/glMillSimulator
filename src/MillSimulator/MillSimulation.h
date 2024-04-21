#ifndef __millsimulation__h__
#define __millsimulation__h__

#include "MillMotion.h"
#include "GCodeParser.h"
#include "Shader.h"
#include "linmath.h"
#include "StockObject.h"
#include "MillPathSegment.h"
#include "EndMillFlat.h"
#include "EndMillBall.h"
#include "EndMillTaper.h"
#include <sstream>

namespace MillSim {

	class MillSimulation
	{
	public:
		MillSimulation();
		void ClearMillPathSegments();
		void SimNext();
		void InitSimulation();
		void SetTool(int tool);
		void AddTool(EndMill* tool);
		void Render();
		void ProcessSim(unsigned int time_ms);
		void HandleKeyPress(int key);
		void TiltEye(float tiltStep);

		void InitDisplay();

		bool LoadGCodeFile(const char* fileName);


	protected:
		void GlsimStart();
		void GlsimToolStep1(void);
		void GlsimToolStep2(void);
		void GlsimClipBack(void);
		void GlsimRenderStock(void);
		void GlsimRenderTools(void);
		void GlsimEnd(void);
		void renderSegmentForward(int iSeg);
		void renderSegmentReversed(int iSeg);


	protected:
		EndMill* curTool = nullptr;
		std::vector<EndMill*> mToolTable;
		CSShader shader3D, shaderInv3D, shaderFlat;
		GCodeParser mCodeParser;
		std::vector<MillPathSegment*> MillPathSegments;
		std::ostringstream mFpsStream;

		MillMotion mZeroPos = { eNop, -1, 0, 0,  10 };
		MillMotion mCurMotion;
		MillMotion mDestMotion;

		StockObject* gStockObject;
		StockObject* glightObject;

		vec3 lightColor = { 1.0f, 1.0f, 0.9f };
		vec3 lightPos = { 20.0f, 20.0f, 10.0f };
		vec3 ambientCol = { 0.3f, 0.3f, 0.1f };

		vec3 eye = { 0, 100, 50 };
		vec3 target = { 0, 0, 0 };
		vec3 upvec = { 0, 0, 1 };

		vec3 stockColor = { 0.7f, 0.7f, 0.7f };
		vec3 cutColor = { 0.4f, 0.7f, 0.4f };
		vec3 toolColor = { 0.4f, 0.4f, 0.7f };

		float mEyeHeight = 30;
		float mEyeRoration = 0;


		int mLastToolId = -1;
		int mCurStep = 0;
		int mNSteps = 0;
		int mPathStep = 0;
		int mNPathSteps = 0;
		int mDebug = 0;
		int mSimSpeed = 1;

		bool mIsInStock = false;
		bool mIsRotate = true;
		bool mSimPlaying = false;
		bool mSingleStep = false;
	};
}
#endif