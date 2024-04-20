#include "MillOperation.h"
#include "SimShapes.h"


void MillOperation::RenderTool(mat4x4 transformMat, mat4x4 normalMat)
{
	endmill->mToolShape.Render(transformMat, normalMat);
}
