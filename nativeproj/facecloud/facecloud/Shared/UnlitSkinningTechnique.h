#pragma once
#ifndef UNLIT_SKINNING_TECHNIQUE_H
#define	UNLIT_SKINNING_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"
class UnlitSkinningTechnique : public Technique {
public:

	static const uint MAX_POINT_LIGHTS = 2;
	static const uint MAX_SPOT_LIGHTS = 2;
	static const uint MAX_BONES = 254;

	UnlitSkinningTechnique();

	virtual bool Init();

	void SetWVP(const Matrix4f& WVP);
	void SetColorTextureUnit(uint TextureUnit);
	void SetBoneTransform(uint Index, const Matrix4f& Transform);
	void SetDetailTextureUnit(uint TextureUnit);
	void SetUVSize(Vector2f& uvsize);
	void SetYOffset(float yoffset);
private:

	GLuint m_WVPLocation;
	GLuint m_YOffsetLocation;
	GLuint m_UVSizeLocation;
	GLuint m_colorTextureLocation;
	GLuint m_detailTextureLocation;
	GLuint m_boneLocation[MAX_BONES];
};


#endif	/* UNLIT_SKINNING_TECHNIQUE_H */
