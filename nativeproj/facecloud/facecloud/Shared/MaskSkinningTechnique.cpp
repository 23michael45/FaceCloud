/*

Copyright 2011 Etay Meiri

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <limits.h>
#include <string>


#include "MaskSkinningTechnique.h"
#include "ogldev_util.h"
#include "Predefined.h"
using namespace std;

MaskSkinningTechnique::MaskSkinningTechnique()
{
}


bool MaskSkinningTechnique::Init()
{
	if (!Technique::Init()) {

		printf("\nMaskSkinningTechnique Init Failed");
		return false;
	}

	if (!AddShader(GL_VERTEX_SHADER, (RES_PATH + string("shader/skinningunlit.vs")).c_str())) {

		printf("\nMaskSkinningTechnique vs Init Failed");
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, (RES_PATH + string("shader/skinningmask.fs")).c_str())) {

		printf("\nMaskSkinningTechnique fs Init Failed");
		return false;
	}

	if (!Finalize()) {

		printf("\nMaskSkinningTechnique vs Finalize Failed");
		return false;
	}

	m_WVPLocation = GetUniformLocation("gWVP");
	m_colorTextureLocation = GetUniformLocation("gColorMap");
	m_maskTextureLocation = GetUniformLocation("gMaskMap");


	m_detailTextureLocation = GetUniformLocation("gDetailMap");
	m_UVSizeLocation = GetUniformLocation("gUVSize");
	m_YOffsetLocation = GetUniformLocation("gYOffset");

	/*if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
		m_colorTextureLocation == INVALID_UNIFORM_LOCATION) {
		return false;
	}*/


	for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_boneLocation); i++) {
		char Name[128];
		memset(Name, 0, sizeof(Name));
		SNPRINTF(Name, sizeof(Name), "gBones[%d]", i);
		m_boneLocation[i] = GetUniformLocation(Name);
	}

	return true;
}

void MaskSkinningTechnique::SetWVP(const Matrix4f& WVP)
{
	glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP);
}


void MaskSkinningTechnique::SetColorTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(m_colorTextureLocation, TextureUnit);
}
void MaskSkinningTechnique::SetDetailTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(m_detailTextureLocation, TextureUnit);
}
void MaskSkinningTechnique::SetMaskTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(m_maskTextureLocation, TextureUnit);
}
void MaskSkinningTechnique::SetUVSize(Vector2f& uvsize)
{
	glUniform2fv(m_UVSizeLocation, 1,(const GLfloat*)&uvsize);
}
void MaskSkinningTechnique::SetYOffset(float yoffset)
{

	glUniform1f(m_YOffsetLocation, yoffset);
}


void MaskSkinningTechnique::SetBoneTransform(uint Index, const Matrix4f& Transform)
{
	assert(Index < MAX_BONES);
	//Transform.Print();
	glUniformMatrix4fv(m_boneLocation[Index], 1, GL_TRUE, (const GLfloat*)Transform);
}
