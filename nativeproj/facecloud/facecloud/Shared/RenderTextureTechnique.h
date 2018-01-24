#pragma once
#ifndef RENDERTEXTURE_TECHNIQUE_H
#define	RENDERTEXTURE_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

#include "ogldev_texture.h"
class RenderTextureTechnique : public Technique {
public:

	static const uint MAX_POINT_LIGHTS = 2;
	static const uint MAX_SPOT_LIGHTS = 2;
	static const uint MAX_BONES = 100;

	RenderTextureTechnique();

	virtual bool Init();

	void SetWVP(const Matrix4f& WVP);
	void SetSampler(Texture* pTexture);
	void InitSampler();

private:

	GLuint m_WVPLocation;
	GLuint m_Sampler;
};


#endif	/* RENDERTEXTURE_TECHNIQUE_H */
