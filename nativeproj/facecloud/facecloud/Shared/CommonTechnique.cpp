
#include <limits.h>
#include <string>


#include "CommonTechnique.h"
#include "ogldev_util.h"

using namespace std;

CommonTechnique::CommonTechnique()
{
}


bool CommonTechnique::Init()
{
	if (!Technique::Init()) {
		return false;
	}

	if (!AddShader(GL_VERTEX_SHADER, "data/shader/common.vs")) {
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, "data/shader/common.fs")) {
		return false;
	}

	if (!Finalize()) {
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
