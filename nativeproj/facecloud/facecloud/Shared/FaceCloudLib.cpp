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

		printf("\nGLUTBackendCreateWindow Failed");
		return false;
	}

	bool rt = InitCamera();
	if (rt == false)
	{
		printf("\nInitCamera Failed");
		return false;
	}
	rt = InitMesh();
	if (rt == false)
	{
		printf("\nInitMesh Failed");
		return false;
	}
	if (!InitJson())
	{
		printf("\nInitJson Failed");
		return false;
	}
	
	CreateRenderTarget();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	return true;
}
void FaceCloudLib::Calculate(string modelID, string photoPath, string jsonFace, string& photoPathOut, string& jsonModelOut)
{
	Texture* ptexture = new Texture(GL_TEXTURE_2D, photoPath);
	if (!ptexture->Load()) {
		return;
	}
	ptexture->Bind(GL_TEXTURE1);

	bool isman = true;
	if (modelID == "10001")
	{
		isman = true;
	}
	else if (modelID == "10002")
	{
		isman = false;
	}

	JsonFaceInfo jsonfaceinfo;
	jsonfaceinfo.LoadFromString(jsonFace,true);

	Texture* paftertex = m_BoneUtility.CalculateSkin(ptexture->GetTextureObj(), isman, m_JsonRoles.roles[modelID],jsonfaceinfo);
	m_pCurrentSkinTexture = paftertex;

	unsigned char* ptr;
	cv::Mat mat = GLTextureToMat(m_pCurrentSkinTexture->GetTextureObj(), ptr);
	//SaveTextureToFile(mat, GL_RGBA, "data/export/test.jpg");
	SAFE_DELETE(ptr);

	Vector3f center;
	Vector2f uvsize;
	float yoffset = 0;

	CalculateBone(modelID, jsonfaceinfo, photoPathOut, jsonModelOut, center, uvsize, yoffset);

	m_pSkinningRenderer->SetUVSize(uvsize);
	m_pSkinningRenderer->SetYOffset(yoffset);


	if (m_bRenderToTexture)
	{

		m_pGameCamera->SetPos(Vector3f(0, 0, 0));
		if (BeginRenterTexture())
		{
			DrawOnce(modelID, center, uvsize);
			EndRenderTexture();
		}
	}
	else
	{
		EndRenderTexture();
		DrawOnce(modelID, center, uvsize);
	}

	CombineTexture(m_RenderTexture, m_ColorTextureMap[modelID], m_pMaskTexture, photoPathOut);

	//SaveTextureToFile(ptexture->GetTextureObj(), m_Width, m_Width, "data/export/preskin.jpg");
	//SaveTextureToFile(paftertex->GetTextureObj(), m_Width, m_Width, "data/export/afterskin.jpg");

	if (m_bRenderToTexture)
	{
		SAFE_DELETE(ptexture);
		SAFE_DELETE(m_pCurrentSkinTexture);

	}
}

void FaceCloudLib::CombineTexture(GLuint FaceTexure, Texture* pWhole, Texture* pMask,string& photoPathOut)
{
	unsigned char* faceptr;
	unsigned char* maskptr;
	unsigned char* colorptr;

	cv::Mat facemat = GLTextureToMat(FaceTexure, faceptr);
	cv::Mat maskmat = GLTextureToMat(pMask->GetTextureObj(), maskptr);
	cv::Mat colormat = GLTextureToMat(pWhole->GetTextureObj(), colorptr);



	cv::Mat maskmat2;
	cv::resize(maskmat, maskmat2, cv::Size(m_Width, m_Height));
	cv::Mat colormat2;
	cv::resize(colormat, colormat2, cv::Size(m_Width, m_Height));



	cv::Mat facemat2;

	facemat.convertTo(facemat2, CV_16UC3);
	maskmat2.convertTo(maskmat2, CV_16UC3);
	colormat2.convertTo(colormat2, CV_16UC3);

	cv::flip(maskmat2, maskmat2, 0);
	cv::flip(colormat2, colormat2, 0);

	cv::cvtColor(maskmat2, maskmat2, CV_RGBA2RGB);
	cv::cvtColor(colormat2, colormat2, CV_RGBA2RGB);


	int type = facemat2.type();
	type = maskmat2.type();
	type = colormat2.type();
	facemat2 = 1.0f / 255 * ((facemat2.mul(cv::Scalar(255, 255, 255) - maskmat2)) +colormat2.mul( maskmat2));

	cv::flip(facemat2, facemat2, 0);
	facemat2 = facemat2(cv::Range(270/2, 1070/2), cv::Range(624 / 2, 1424 / 2));

	SaveTextureToFile(facemat2,GL_RGBA, photoPathOut, false);

	SAFE_DELETE(faceptr);
	SAFE_DELETE(maskptr);
	SAFE_DELETE(colorptr);
}


bool FaceCloudLib::InitCamera()
{
	Vector3f Pos(0.0f, 0, 0);
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

	printf("\nStart UnlitSkinningTechnique init\n");
	m_pSkinningRenderer = new UnlitSkinningTechnique(); 
	if (!m_pSkinningRenderer->Init()) {
		printf("Error initializing the UnlitSkinningTechnique\n");
		return false;
	}
	m_pSkinningRenderer->Enable();
	m_pSkinningRenderer->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
	m_pSkinningRenderer->SetDetailTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 1);
	m_pSkinningRenderer->SetMaskTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 2);


	printf("\nStart CommonTechnique init\n");
	m_pCommonRenderer = new CommonTechnique();
	if (!m_pCommonRenderer->Init()) {
		printf("Error initializing the CommonTechnique\n");
		return false;
	}

	return true;
}
bool FaceCloudLib::InitMesh()
{
	vector<string> modelIDs;
	modelIDs.push_back("10001");
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
			return false;
		}
		m_ColorTextureMap[*iter] = ptexture;
	}
	m_pMaskTexture = new Texture(GL_TEXTURE_2D, "data/facecloud/mask.jpg");
	if (!m_pMaskTexture->Load()) {
		return false;
	}
	

	return true;
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

	glBindTexture(GL_TEXTURE_2D, 0);
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
	m_Lastcenter = center;
	m_Lastuvsize = uvsize;

	m_pGameCamera->OnRender();
	// Always check that our framebuffer is ok

	Pipeline p;
	p.WorldPos(0.0f, 0, 0.0f);
	p.Rotate(0.0f, 180.0f, 0.0f);
	p.Scale(1, 1, 1);
	p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());

	m_orthoProjInfo.b = -uvsize.x/2;
	m_orthoProjInfo.t = uvsize.x / 2;
	m_orthoProjInfo.l = -uvsize.y / 2;
	m_orthoProjInfo.r = uvsize.y / 2;

	p.SetOrthographicProj(m_orthoProjInfo);
	p.SetPerspectiveProj(m_persProjInfo);
	m_pSkinningRenderer->Enable();

	if (m_bRenderToTexture)
	{
		m_pSkinningRenderer->SetWVP(p.GetWVOrthoPTrans());
	}
	else
	{
		m_pSkinningRenderer->SetWVP(p.GetWVPTrans());

	}



	/*m_pCommonRenderer->Enable();
	m_pCommonRenderer->SetWVP(p.GetWVPTrans());*/
	if (m_MeshMap.find(modelID) != m_MeshMap.end())
	{
		if (m_ColorTextureMap.find(modelID) != m_ColorTextureMap.end())
		{
			m_ColorTextureMap[modelID]->Bind(GL_TEXTURE0);
			//m_ColorTextureMap[modelID]->Bind(GL_TEXTURE1);
			if (m_pCurrentSkinTexture != NULL)
			{
				m_pCurrentSkinTexture->Bind(GL_TEXTURE1);
			}
			if (m_pMaskTexture != NULL)
			{
				m_pMaskTexture->Bind(GL_TEXTURE2);
			}

			SkinnedMesh* pmesh = m_MeshMap[modelID];
			vector<Matrix4f> Transforms;
			pmesh->BoneTransform(0, Transforms);
			for (uint i = 0; i < Transforms.size(); i++) {
				m_pSkinningRenderer->SetBoneTransform(i, Transforms[i]);
			}
			pmesh->Render();
		}
	}

	return true;

}

cv::Mat FaceCloudLib::GLTextureToMat(GLuint texture, unsigned char*& outimagptr)
{
	glBindTexture(GL_TEXTURE_2D, texture);

	GLint wtex, htex, comp, rs, gs, bs, as;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &wtex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &htex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &comp);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &rs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &gs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &bs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &as);

	long size = 0;

	if (comp == GL_RGB)
	{

		size = wtex * htex * 3;
	}
	else if (comp == GL_RGBA)
	{
		size = wtex * htex * 4;
	}

	outimagptr = new unsigned char[size];

	glGetTexImage(GL_TEXTURE_2D, 0, comp, GL_UNSIGNED_BYTE, outimagptr);


	GLenum error = glGetError();
	const GLubyte * eb = gluErrorString(error);
	string errorstring((char*)eb);

	cv::Mat  img;

	if (comp == GL_RGB)
	{
		img = cv::Mat(wtex, htex, CV_8UC3, (unsigned*)outimagptr);
	}
	else if (comp == GL_RGBA)
	{
		img = cv::Mat(wtex, htex, CV_8UC4, (unsigned*)outimagptr);
	}


	return img;
}
void FaceCloudLib::SaveTextureToFile(cv::Mat imag, int format,string path,bool flip)
{
	//WriteTGA((char*)"data/export/rendertexture.tga", wtex, htex, output_image);

	cv::Mat  bgra;

	if (format == GL_RGB)
	{
		cv::cvtColor(imag, bgra, cv::COLOR_RGB2BGRA);
	}
	else if (format == GL_RGBA)
	{
		cv::cvtColor(imag, bgra, cv::COLOR_RGBA2BGRA);
	}

	if (flip)
	{
		cv::Mat flipimg;
		/*	flipCode	Anno
		1	ˮƽ��ת
		0	��ֱ��ת
		- 1	ˮƽ��ֱ��ת*/
		cv::flip(bgra, flipimg, 0);
		cv::imwrite(path, flipimg);
	}
	else
	{

		cv::imwrite(path, bgra);
	}

}

void FaceCloudLib::DisplayGrid()
{
	Pipeline p;
	p.WorldPos(0.0f, 0, 0.0f);
	p.Rotate(0.0f, 0.0f, 0.0f);
	p.Scale(1, 1, 1);
	p.SetOrthographicProj(m_orthoProjInfo);
	p.SetPerspectiveProj(m_persProjInfo);
	p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
	m_pCommonRenderer->Enable();
	m_pCommonRenderer->SetWVP(p.GetWVPTrans());


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
