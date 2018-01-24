#pragma once
#ifndef FACE_CLOUD_LIB_H
#define	FACE_CLOUD_LIB_H
#include <stdio.h>
#include <string>
#include <map>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "texture.h"
#include "CommonTechnique.h"
#include "SkinningTechnique.h"
#include "UnlitSkinningTechnique.h"
#include "RenderTextureTechnique.h"
#include "ogldev_glut_backend.h"
#include "ogldev_skinned_mesh.h"
#include "BoneUtility.h"

using namespace std;
class FaceCloudLib{

public:
	FaceCloudLib(); 
	~FaceCloudLib();
	bool Init();

	void Calculate(string modelID, string photoPath, string jsonFace, string& photoPathOut, string& jsonModelOut);


	void CalculateBone(string modelID, JsonFaceInfo jsonfaceinfo, string& photoPathOut, string& jsonModelOut, Vector3f& centerpos, Vector2f& uvsize, float& yOffset);
	bool DrawOnce(string modelID, Vector3f& center, Vector2f& uvsize);
private:
	Camera * m_pGameCamera;
	PersProjInfo m_persProjInfo;
	OrthoProjInfo m_orthoProjInfo;
	UnlitSkinningTechnique * m_pSkinningRenderer;
	CommonTechnique * m_pCommonRenderer;


	map<string, SkinnedMesh*> m_MeshMap;
	map<string, Texture*> m_ColorTextureMap;

	jsonRoles m_JsonRoles;
	BoneUtility m_BoneUtility;

	GLuint m_RenderTexture; 
	GLuint m_FramebufferName;

	int m_Width;
	int m_Height;

	bool InitCamera();
	bool InitMesh();
	bool InitJson();


	bool CreateRenderTarget();
	bool BeginRenterTexture();
	void EndRenderTexture();
	void SaveTextureToFile(GLuint texture, int width, int height,string path);

	void DisplayGrid();
};


#endif	/* FACE_CLOUD_LIB_H */
