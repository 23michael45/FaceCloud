
#include <limits.h>
#include <string>


#include "RenderTextureTechnique.h"
#include "ogldev_util.h"

using namespace std;

RenderTextureTechnique::RenderTextureTechnique()
{
}


bool RenderTextureTechnique::Init()
{
	if (!Technique::Init()) {
		return false;
	}

	if (!AddShader(GL_VERTEX_SHADER, "data/shader/Passthrough.vertexshader")) {
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, "data/shader/WobblyTexture.fragmentshader")) {
		return false;
	}


	/*if (!AddShader(GL_VERTEX_SHADER, "data/shader/c.vs")) {
		return false;
	}
	if (!AddShader(GL_FRAGMENT_SHADER, "data/shader/c.fs")) {
		return false;
	}*/

	if (!Finalize()) {
		return false;
	}

	
	return true;
}

void RenderTextureTechnique::SetWVP(const Matrix4f& WVP)
{
	glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP);
}

void RenderTextureTechnique::SetSampler(Texture* pTexture)
{

}
void RenderTextureTechnique::InitSampler()
{
	m_Sampler = glGetUniformLocation(m_shaderProg, "gSampler");
	glUniform1i(m_Sampler, 0);

}
