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

Tutorial 38 - Skinning
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "texture.h"
#include "SkinningTechnique.h"
#include "CommonTechnique.h"
#include "RenderTextureTechnique.h"
#include "ogldev_glut_backend.h"
#include "ogldev_skinned_mesh.h"
#include "BoneUtility.h"


#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/core.hpp>
using namespace std;

#define WINDOW_WIDTH  400  
#define WINDOW_HEIGHT 300


struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex)
	{
		m_pos = pos;
		m_tex = tex;
	}
};


class SkinningApp : public ICallbacks, public OgldevApp
{
private:
	GLuint VBO;
	GLuint IBO;

	GLuint quad_vertexbuffer;
	SkinningTechnique * m_pEffect;

	CommonTechnique * m_pCommonEffect;
	RenderTextureTechnique * m_pRTEffect;
	Texture * m_pTextureColor;
	Texture * m_pTextureDetail;
	Camera* m_pGameCamera;
	DirectionalLight m_directionalLight;
	SkinnedMesh m_mesh;
	Vector3f m_position;
	PersProjInfo m_persProjInfo;
	OrthoProjInfo m_orthoProjInfo;

	GLuint m_RenderTexture;

	JsonFaceInfo jsonfaceinfo;
	jsonRoles jsonRoles;
	BoneUtility boneutility;
public:

	SkinningApp()
	{
		m_pGameCamera = NULL;
		m_pEffect = NULL;
		m_pCommonEffect = NULL;
		m_pRTEffect = NULL;
		m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
		m_directionalLight.AmbientIntensity = 0.55f;
		m_directionalLight.DiffuseIntensity = 0.9f;
		m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

		m_persProjInfo.FOV = 60.0f;
		m_persProjInfo.Height = WINDOW_HEIGHT;
		m_persProjInfo.Width = WINDOW_WIDTH;
		m_persProjInfo.zNear = 0.1f;
		m_persProjInfo.zFar = 10000.0f;


		m_orthoProjInfo.b = -15;
		m_orthoProjInfo.t = 15;
		m_orthoProjInfo.l = -20;
		m_orthoProjInfo.r = 20;
		m_orthoProjInfo.n = -1000;
		m_orthoProjInfo.f = 1000;

		m_position = Vector3f(0.0f, 0.0f, 6.0f);
	}

	~SkinningApp()
	{
		
		SAFE_DELETE(m_pEffect); 
		SAFE_DELETE(m_pRTEffect);
		SAFE_DELETE(m_pCommonEffect);
		SAFE_DELETE(m_pGameCamera);
	}

	bool Init()
	{
		/*Vector3f Pos(0.0f, 0, 0.0f);
		Vector3f Target(0.0f, 0.0f, 1.0f);
		Vector3f Up(0.0, 1.0f, 0.0f);*/

		Vector3f Pos(0.0f, 175, 10.0f);
			Vector3f Target(0.0f, 0.0f, -1.0f);
			Vector3f Up(0.0, 1.0f, 0.0f);

		/*Vector3f Pos(10.0f, 175, 0.0f);
		Vector3f Target(1.0f, 0.0f, .0f);*/


		m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);

		m_pEffect = new SkinningTechnique();
		m_pCommonEffect = new CommonTechnique();
		m_pRTEffect = new RenderTextureTechnique();

	
		if (!m_pEffect->Init()) {
			printf("Error initializing the lighting technique\n");
			return false;
		}

		if (!m_pRTEffect->Init()) {
			printf("Error initializing the RT technique\n");
			return false;
		}

		if (!m_pCommonEffect->Init()) {
			printf("Error initializing the common technique\n");
			return false;
		}
		
		m_pTextureColor = new Texture(GL_TEXTURE_2D);
		if (!m_pTextureColor->LoadFile("data/head_women.jpg")) {
			return 1;
		}
		m_pTextureDetail = new Texture(GL_TEXTURE_2D);
		if (!m_pTextureDetail->LoadFile("data/face/photoface.jpg")) {
			return 1;
		}
		

		m_pEffect->Enable();

		m_pEffect->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
		m_pEffect->SetDetailTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 1);
		m_pEffect->SetDirectionalLight(m_directionalLight);
		m_pEffect->SetMatSpecularIntensity(0.0f);
		m_pEffect->SetMatSpecularPower(0);


		/*if (!m_mesh.LoadMesh("../Content/boblampclean.md5mesh")) {
		printf("Mesh load failed\n");
		return false;
		} */
		if (!m_mesh.LoadMesh("data/face/women_head_fix.FBX")) {
			printf("Mesh load failed\n");
//			return false;
		}


		jsonfaceinfo.LoadFromFile("data/face/mytest1.info");

		jsonRoles.LoadFromFile("data/face/a_role.bytes");

		boneutility.Init();

		

#ifndef WIN32
		if (!m_fontRenderer.InitFontRenderer()) {
			return false;
		}
#endif        	
		return true;
	}

	void Run()
	{
		//CreateRenderTarget();
		GLUTBackendRun(this);
	}
	void DrawQuad()
	{
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset

		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

		glDisableVertexAttribArray(0);
	}
	virtual void DrawTriangle()
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

	}
	virtual void RenderSceneCB()
	{
		CalcFPS();

		m_pGameCamera->OnRender();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		Pipeline p;
		p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
		//p.SetPerspectiveProj(m_persProjInfo);
		p.SetOrthographicProj(m_orthoProjInfo);
		m_pCommonEffect->Enable();
		m_pCommonEffect->SetWVP(p.GetWVOrthoPTrans());


		m_pTextureColor->Bind(GL_TEXTURE0);
		DisplayGrid();

		/*glutSwapBuffers();
		return;*/
		Vector3f Pos(m_position);
		p.WorldPos(Pos);
		p.WorldPos(0.0f, 10, 0.0f);
		p.Rotate(0.0f, 180.0f, 0.0f);

		

		m_pCommonEffect->SetWVP(p.GetWVOrthoPTrans());
		//m_pCommonEffect->SetWVP(p.GetWVPTrans());
		//m_pCommonEffect->SetSampler(m_pTextureColor);


		m_pTextureColor->Bind(GL_TEXTURE0);
		m_pTextureDetail->Bind(GL_TEXTURE1);


		
		DrawTriangle();
		

		p.WorldPos(5.0f, 10, 0.0f);

		m_pCommonEffect->SetWVP(p.GetWVPTrans());

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_RenderTexture);


		DrawQuad();

		p.WorldPos(0.0f, 0, 0.0f);
		p.Scale(1.0f, 1.0f, 1.0f);
		p.Rotate(0.0f, 0, 0.0f);

		m_pCommonEffect->SetWVP(p.GetWVPTrans());



		m_pEffect->Enable();


		vector<Matrix4f> Transforms;

		float RunningTime = GetRunningTime();

	
		m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());
		//m_pEffect->SetWVP(p.GetWVPTrans());

		m_pEffect->SetWVP(p.GetWVOrthoPTrans());
		m_pEffect->SetWorldMatrix(p.GetWorldTrans());

		

		string outJson;

		static bool hasdone = false;
		if (!hasdone)
		{
			hasdone = true;
			Vector3f center;
			Vector2f uvsize;
			float yoffset;
			boneutility.CalculateFaceBone(&m_mesh, jsonRoles.roles["10002"], jsonfaceinfo, outJson,center,uvsize,yoffset);

		}

		m_mesh.BoneTransform(RunningTime, Transforms);


		for (uint i = 0; i < Transforms.size(); i++) {
			m_pEffect->SetBoneTransform(i, Transforms[i]);
		}

		m_mesh.Render();

		RenderFPS();

		glutSwapBuffers();
	}


	virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
	{
		switch (OgldevKey) {
		case OGLDEV_KEY_ESCAPE:
		case OGLDEV_KEY_q:
			GLUTBackendLeaveMainLoop();
			break;
		default:
			m_pGameCamera->OnKeyboard(OgldevKey);
		}
	}


	virtual void PassiveMouseCB(int x, int y)
	{
		m_pGameCamera->OnMouse(x, y);
	}

	void DisplayGrid()
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
	
	}  //optional
	void CreateVertexBuffer()
	{
		Vertex Vertices[4] = { Vertex(Vector3f(-1.0f, -1.0f, 0.5773f), Vector2f(0.0f, 0.0f)),
			Vertex(Vector3f(0.0f, -1.0f, -1.15475f), Vector2f(0.5f, 0.0f)),
			Vertex(Vector3f(1.0f, -1.0f, 0.5773f),  Vector2f(1.0f, 0.0f)),
			Vertex(Vector3f(0.0f, 1.0f, 0.0f),      Vector2f(0.5f, 1.0f)) };

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

		unsigned int Indices[] = { 0, 3, 1,
			1, 3, 2,
			2, 3, 0,
			0, 1, 2 };

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);


		// The fullscreen quad's FBO

		static const GLfloat g_quad_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
		};



		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);


	}



	bool CreateRenderTarget()
	{
		// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
		GLuint FramebufferName = 0;
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);


		// The texture we're going to render to
		glGenTextures(1, &m_RenderTexture);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, m_RenderTexture);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		// The depth buffer
		GLuint depthrenderbuffer;
		glGenRenderbuffers(1, &depthrenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);


		// Set "renderedTexture" as our colour attachement #0
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTexture, 0);

		// Set the list of draw buffers.
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

									   // Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		{
			// Render to our framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

										  // Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			m_pGameCamera->OnRender();
			Pipeline p;
			p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
			p.SetPerspectiveProj(m_persProjInfo);

			
			m_pCommonEffect->Enable();
			m_pCommonEffect->SetWVP(p.GetWVPTrans());
			
			m_pTextureDetail->Bind(GL_TEXTURE0);
			DrawTriangle();
			DisplayGrid();
			glutSwapBuffers();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			SaveTextureToFile(m_RenderTexture,1024,1024);

			return true;
		}
		return false;
	}

	int SaveTexture(GLuint texture)
	{
		unsigned char *pixels;
		FILE *image;

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		pixels = new unsigned char[viewport[2] * viewport[3] * 3];

		glReadPixels(0, 0, viewport[2], viewport[3], GL_BGR,
			GL_UNSIGNED_BYTE, pixels);

		char tempstring[50];
		sprintf(tempstring, "data/savetexture%i.tga", 1);
		if ((image = fopen(tempstring, "wb")) == NULL) return 1;

		unsigned char TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };

		unsigned char header[6] = { ((int)(viewport[2] % 256)),
			((int)(viewport[2] / 256)),
			((int)(viewport[3] % 256)),
			((int)(viewport[3] / 256)),24,0 };

		// TGA header schreiben
		fwrite(TGAheader, sizeof(unsigned char), 12, image);
		// Header schreiben
		fwrite(header, sizeof(unsigned char), 6, image);

		fwrite(pixels, sizeof(unsigned char),
			viewport[2] * viewport[3] * 3, image);

		fclose(image);
		delete[] pixels;

	}

	void SaveTextureToFile(GLuint texture,int width,int height)
	{
		width = 2048;
		height = 2048;
		long size = width * height * 3;
		unsigned char* output_image = new unsigned char[size];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		//m_pTextureColor->Bind(GL_TEXTURE0);

		GLint wtex, htex, comp, rs, gs, bs, as;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &wtex);
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

		/*ofstream rawfile;
		rawfile.open("data/rendertexture.raw");
		rawfile.write((const char*)output_image, size);
		rawfile.close();*/
		WriteTGA((char*)"data/rendertexture.tga", wtex, htex, output_image);
		
		cv::Mat  img = cv::Mat(wtex ,htex , CV_8UC3, (unsigned*)output_image);
		cv::Mat flipimg;

		/*	flipCode	Anno
				1	水平翻转
				0	垂直翻转
				- 1	水平垂直翻转*/
		cv::flip(img, flipimg, 0);
		cv::imwrite("data/rendertexture.jpg", flipimg);


		
		delete output_image;

	}

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

};

SkinningApp* pApp;
GLuint VBOtemp;
static void RenderSceneCB()
{
	if (pApp != NULL)
	{
		pApp->DrawTriangle();
	}
}

int maina(int argc, char** argv)
{
	JsonModelFormat jsonmodel;
	jsonmodel.LoadFromFile("data/women_head_split.JD");



	GLUTBackendInit(argc, argv, true, false);

	if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "SkinningApp")) {
		return 1;
	}
	pApp = new SkinningApp();

	SRANDOM;
	

	if (!pApp->Init()) {
		return 1;
	}
	pApp->CreateVertexBuffer();
	pApp->Run();

	delete pApp;

	return 0;
}