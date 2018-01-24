#include "FaceCloudLib.h"
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"


bool WriteTGA(char *file, short int width, short int height, unsigned char *outImage)
{
	// To save a screen shot is just like reading in a image. All you do
	// is the opposite. Istead of calling fread to read in data you call
	// fwrite to save it.

	FILE *pFile; // The file pointer.
	unsigned char uselessChar; // used for useless char.
	short int uselessInt; // used for useless int.
	unsigned char imageType; // Type of image we are saving.
	int index; // used with the for loop.
	unsigned char bits; // Bit depth.
	long Size; // Size of the picture.
	int colorMode;
	unsigned char tempColors;

	// Open file for output.
	pFile = fopen(file, "wb");

	// Check if the file opened or not.
	if (!pFile) { fclose(pFile); return false; }

	// Set the image type, the color mode, and the bit depth.
	imageType = 2; colorMode = 3; bits = 24;

	// Set these two to 0.
	uselessChar = 0; uselessInt = 0;

	// Write useless data.
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

	// Now image type.
	fwrite(&imageType, sizeof(unsigned char), 1, pFile);

	// Write useless data.
	fwrite(&uselessInt, sizeof(short int), 1, pFile);
	fwrite(&uselessInt, sizeof(short int), 1, pFile);
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
	fwrite(&uselessInt, sizeof(short int), 1, pFile);
	fwrite(&uselessInt, sizeof(short int), 1, pFile);

	// Write the size that you want.
	fwrite(&width, sizeof(short int), 1, pFile);
	fwrite(&height, sizeof(short int), 1, pFile);
	fwrite(&bits, sizeof(unsigned char), 1, pFile);

	// Write useless data.
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

	// Get image size.
	Size = width * height * colorMode;

	// Now switch image from RGB to BGR.
	for (index = 0; index < Size; index += colorMode)
	{
		tempColors = outImage[index];
		outImage[index] = outImage[index + 2];
		outImage[index + 2] = tempColors;
	}

	// Finally write the image.
	fwrite(outImage, sizeof(unsigned char), Size, pFile);

	// close the file.
	fclose(pFile);

	return true;
}


FaceCloudLib::FaceCloudLib()
{
	m_pGameCamera = NULL;
	m_pSkinningRenderer = NULL;
	m_FramebufferName = 0;

	m_Width = 1024;
	m_Height = 1024;
}
FaceCloudLib::~FaceCloudLib()
{

	SAFE_DELETE(m_pGameCamera);
	SAFE_DELETE(m_pSkinningRenderer);
	SAFE_DELETE(m_pCommonRenderer);

	for (map<string,SkinnedMesh*>::iterator iter = m_MeshMap.begin();iter != m_MeshMap.end();iter++)
	{
		SAFE_DELETE(iter->second);
	}
	m_MeshMap.clear();

	for (map<string, Texture*>::iterator iter = m_ColorTextureMap.begin(); iter != m_ColorTextureMap.end(); iter++)
	{
		SAFE_DELETE(iter->second);
	}
	m_ColorTextureMap.clear();
}
bool FaceCloudLib::Init()
{
	int argc = 0;
	char** argv = 0;
	GLUTBackendInit(argc, argv, true, false);

	if (!GLUTBackendCreateWindow(m_Width / 2, m_Height /2, false, "FaceCloudLib")) {
		return 1;
	}

	if (!InitCamera())
	{
		return false;
	}
	if (!InitMesh())
	{
		return false;
	}
	if (!InitJson())
	{
		return false;
	}
	
	CreateRenderTarget();
}
void FaceCloudLib::Calculate(string modelID, string photoPath, string jsonFace, string& photoPathOut, string& jsonModelOut)
{
	Texture* ptexture = new Texture(GL_TEXTURE_2D, photoPath);
	if (!ptexture->Load()) {
		return;
	}
	ptexture->Bind(GL_TEXTURE1);


	JsonFaceInfo jsonfaceinfo;
	jsonfaceinfo.LoadFromString(jsonFace);

	if (BeginRenterTexture())
	{
		Vector3f center;
		Vector2f uvsize;
		float yoffset;
		CalculateBone(modelID, jsonfaceinfo, photoPathOut, jsonModelOut,center,uvsize,yoffset);

		m_pSkinningRenderer->SetUVSize(uvsize);
		m_pSkinningRenderer->SetYOffset(yoffset);

		DrawOnce(modelID,center,uvsize);
		EndRenderTexture();
	}

	SaveTextureToFile(m_RenderTexture, m_Width, m_Width, photoPathOut);
	SAFE_DELETE(ptexture);
}



bool FaceCloudLib::InitCamera()
{
	Vector3f Pos(0.0f, 175, 0);
	Vector3f Target(0.0f,0.0f, 1.0f);
	Vector3f Up(0.0, 1.0f, 0.0f);



	m_persProjInfo.FOV = 60.0f;
	m_persProjInfo.Height = m_Width;
	m_persProjInfo.Width = m_Height;
	m_persProjInfo.zNear = 0.1f;
	m_persProjInfo.zFar = 10000.0f;

	m_orthoProjInfo.b = -20;
	m_orthoProjInfo.t = 20;
	m_orthoProjInfo.l = -20;
	m_orthoProjInfo.r = 20;
	m_orthoProjInfo.n = -99999;
	m_orthoProjInfo.f = 99999;


	m_pGameCamera = new Camera(m_Width, m_Height, Pos, Target, Up);

	m_pSkinningRenderer = new UnlitSkinningTechnique(); 
	if (!m_pSkinningRenderer->Init()) {
		printf("Error initializing the UnlitSkinningTechnique\n");
		return false;
	}
	m_pSkinningRenderer->Enable();
	m_pSkinningRenderer->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
	m_pSkinningRenderer->SetDetailTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 1);

	m_pCommonRenderer = new CommonTechnique();
	if (!m_pCommonRenderer->Init()) {
		printf("Error initializing the CommonTechnique\n");
		return false;
	}
}
bool FaceCloudLib::InitMesh()
{
	vector<string> modelIDs;
	//modelIDs.push_back("10001");
	modelIDs.push_back("10002");

	for (vector<string>::iterator iter = modelIDs.begin();iter != modelIDs.end();iter++)
	{
		SkinnedMesh* pmesh = new SkinnedMesh;
		if (!pmesh->LoadMesh("data/facecloud/" + *iter + ".fbx")) {
			printf("Mesh load failed\n");
			return false;
		}

		m_MeshMap[*iter] = pmesh;

		Texture* ptexture = new Texture(GL_TEXTURE_2D, "data/facecloud/" + *iter + ".jpg");
		if (!ptexture->Load()) {
			return 1;
		}
		m_ColorTextureMap[*iter] = ptexture;
	}

	

}
bool FaceCloudLib::InitJson()
{
	m_BoneUtility.Init();
	m_JsonRoles.LoadFromFile("data/facecloud/model_offset.json");

	return true;
}




bool FaceCloudLib::CreateRenderTarget()
{
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	
	glGenFramebuffers(1, &m_FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferName);


	// The texture we're going to render to
	glGenTextures(1, &m_RenderTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);


	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	
	return true;
}

bool FaceCloudLib::BeginRenterTexture()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferName);
		glViewport(0, 0, m_Width, m_Height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

											 // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
	return false;
}
void FaceCloudLib::EndRenderTexture()
{

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glutSwapBuffers();
}
void FaceCloudLib::CalculateBone(string modelID, JsonFaceInfo jsonfaceinfo, string& photoPathOut, string& jsonModelOut, Vector3f& centerpos, Vector2f& uvsize,float& yOffset)
{
	SkinnedMesh* pmesh = m_MeshMap[modelID];
	m_BoneUtility.CalculateFaceBone(pmesh, m_JsonRoles.roles[modelID], jsonfaceinfo, jsonModelOut, centerpos,uvsize,yOffset);
}
bool FaceCloudLib::DrawOnce(string modelID,Vector3f& center,Vector2f& uvsize)
{
	// Always check that our framebuffer is ok

	m_pGameCamera->OnRender();

	Pipeline p;
	p.WorldPos(0.0f, 0, 0.0f);
	p.Rotate(0.0f, 180.0f, 0.0f);
	p.Scale(1, 1, 1);
	p.SetCamera(center, m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());

	m_orthoProjInfo.b = -uvsize.x/2;
	m_orthoProjInfo.t = uvsize.x / 2;
	m_orthoProjInfo.l = -uvsize.y / 2;
	m_orthoProjInfo.r = uvsize.y / 2;

	p.SetOrthographicProj(m_orthoProjInfo);
	//p.SetPerspectiveProj(m_persProjInfo);
	m_pSkinningRenderer->Enable();

	m_pSkinningRenderer->SetWVP(p.GetWVOrthoPTrans());


	if (m_MeshMap.find(modelID) != m_MeshMap.end())
	{
		if (m_ColorTextureMap.find(modelID) != m_ColorTextureMap.end())
		{
			m_ColorTextureMap[modelID]->Bind(GL_TEXTURE0);
			//m_ColorTextureMap[modelID]->Bind(GL_TEXTURE1);


			SkinnedMesh* pmesh = m_MeshMap[modelID];
			vector<Matrix4f> Transforms;
			pmesh->BoneTransform(0, Transforms);
			for (uint i = 0; i < Transforms.size(); i++) {
				m_pSkinningRenderer->SetBoneTransform(i, Transforms[i]);
			}
			pmesh->Render();
		}
	}


	/*p.WorldPos(0.0f, 175, 0.0f);
	m_pCommonRenderer->Enable();
	m_pCommonRenderer->SetWVP(p.GetWVOrthoPTrans());*/

	DisplayGrid();


	return true;

}
void FaceCloudLib::SaveTextureToFile(GLuint texture, int width, int height,string path)
{
	width = 2048;
	height = 2048;
	long size = width * height * 3;
	unsigned char* output_image = new unsigned char[size];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	//m_pTextureColor->Bind(GL_TEXTURE0);

	GLint wtex, htex, comp, rs, gs, bs, as;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &wtex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &htex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &comp);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &rs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &gs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &bs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &as);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, output_image);

	GLenum error = glGetError();
	const GLubyte * eb = gluErrorString(error);
	string errorstring((char*)eb);



	//WriteTGA((char*)"data/export/rendertexture.tga", wtex, htex, output_image);


	cv::Mat  img = cv::Mat(wtex, htex, CV_8UC3, (unsigned*)output_image);
	cv::Mat  bgra;
	cv::cvtColor(img, bgra, cv::COLOR_RGB2BGRA);

	cv::Mat flipimg;

	//cv::imwrite(path, img);

	/*	flipCode	Anno
	1	水平翻转
	0	垂直翻转
	- 1	水平垂直翻转*/
	cv::flip(bgra, flipimg, 0);
	cv::imwrite(path, flipimg);

	delete output_image;

}

void FaceCloudLib::DisplayGrid()
{
	glPushMatrix();
	//glMultMatrixd(pTransform);

	// Draw a grid 500*500
	glColor3f(0.3f, 0.3f, 0.3f);
	glLineWidth(1.0);
	const int hw = 500;
	const int step = 20;
	const int bigstep = 100;
	int       i;

	// Draw Grid
	for (i = -hw; i <= hw; i += step) {

		if (i % bigstep == 0) {
			glLineWidth(2.0);
		}
		else {
			glLineWidth(1.0);
		}
		glBegin(GL_LINES);
		glVertex3i(i, 0, -hw);
		glVertex3i(i, 0, hw);
		glEnd();
		glBegin(GL_LINES);
		glVertex3i(-hw, 0, i);
		glVertex3i(hw, 0, i);
		glEnd();

	}
	glPopMatrix();

}