#ifndef __shader_h__
#define __shader_h__

#include "linmath.h"

class CSShader
{
public:
	CSShader() {}

public:
	unsigned int shaderId = 0;
	void UpdateModelMat(mat4x4 transformMat, mat4x4 normalMat);
	void UpdateProjectionMat(mat4x4 mat);
	void UpdateViewMat(mat4x4 mat);
	void UpdateEnvColor(vec3 lightPos, vec3 lightColor, vec3 ambient);
	void UpdateObjColor(vec3 objColor);
	unsigned int CompileShader(char* vertShader, char* fragShader);
	void Activate();


protected:
	int modelPos = -1;
	int normalRotPos = -1;
	int projectionPos = -1;
	int viewPos = -1;
	int lightPosPos = -1;
	int lightColorPos = -1;
	int ambientPos = -1;
	int objectColorPos = -1;

	const char* vertShader = nullptr;
	const char* fragShader = nullptr;
};

extern CSShader *CurrentShader;

extern const char* FragShaderNorm;
extern const char* FragShaderFlat;
extern const char* VertShader3DNorm;
extern const char* VertShader3DInvNorm;

#endif // !__shader_h__

