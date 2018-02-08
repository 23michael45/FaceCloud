
#include <limits.h>
#include <string>


#include "CommonTechnique.h"
#include "ogldev_util.h"
#include "Predefined.h"
using namespace std;

CommonTechnique::CommonTechnique()
{
}


bool CommonTechnique::Init()
{
	printf("\nCommonTechnique start Init");
	if (!Technique::Init()) {

		printf("\nCommonTechnique vs Init Failed");
		return false;
	}

	if (!AddShader(GL_VERTEX_SHADER, (RES_PATH + string("shader/common.vs")).c_str())) {

		printf("\nCommonTechnique vs Finalize Failed");
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, (RES_PATH + string("shader/common.fs")).c_str())) {

		printf("\nCommonTechnique vs fs Failed");
		return false;
	}

	if (!Finalize()) {

		printf("\nCommonTechnique vs Finalize Failed");
		return false;
	}

	m_WVPLocation = GetUniformLocation("gWVP");
	InitSampler();

	return true;
}

void CommonTechnique::SetWVP(const Matrix4f& WVP)
{
	glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP);
}

void CommonTechnique::SetSampler(Texture* pTexture)
{

}
void CommonTechnique::InitSampler()
{
	m_Sampler = glGetUniformLocation(m_shaderProg, "gSampler");
	glUniform1i(m_Sampler, 0);

}
