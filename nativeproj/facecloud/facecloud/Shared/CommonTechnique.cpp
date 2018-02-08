
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
	printf("\nxxx start Init");

	if (!AddShader(GL_VERTEX_SHADER, (RES_PATH + string("shader/common.vs")).c_str())) {

		printf("\nCommonTechnique vs Finalize Failed");
		return false;
	}

	printf("\nxxxx start Init");
	if (!AddShader(GL_FRAGMENT_SHADER, (RES_PATH + string("shader/common.fs")).c_str())) {

		printf("\nCommonTechnique vs fs Failed");
		return false;
	}

	printf("\nxxxxx start Init");
	if (!Finalize()) {

		printf("\nCommonTechnique vs Finalize Failed");
		return false;
	}
	printf("\nxxxxxxx start Init");

	m_WVPLocation = GetUniformLocation("gWVP");
	InitSampler();

	printf("\nxxxff start Init");
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
