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

using namespace std;

#define WINDOW_WIDTH  1280  
#define WINDOW_HEIGHT 1024


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

		Vector3f Pos(0.0f, 10.0f, -50.0f);
		Vector3f Target(0.0f, 0.0f, 1.0f);
		Vector3f Up(0.0, 1.0f, 0.0f);

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
		
		m_pTextureColor = new Texture(GL_TEXTURE_2D, "data/head_women.jpg");
		if (!m_pTextureColor->Load()) {
			return 1;
		}
		m_pTextureDetail = new Texture(GL_TEXTURE_2D, "data/test.png");
		if (!m_pTextureDetail->Load()) {
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
		if (!m_mesh.LoadMesh("data/womenhead_split.FBX")) {
			printf("Mesh load failed\n");
//			return false;
		}


		JsonFaceInfo jsonfaceinfo;
		jsonfaceinfo.LoadFromFile("data/jsonfaceinfo.json");

		JsonRoleBone jsonrolebone;
		jsonrolebone.LoadFromFile("data/jsonrolebone.json");

		BoneUtility boneutility;
		boneutility.Init();
		//boneutility.CalculateFaceBone(&m_mesh, jsonrolebone, jsonfaceinfo);
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
		p.SetPerspectiveProj(m_persProjInfo);
		

		m_pCommonEffect->Enable();
		m_pCommonEffect->SetWVP(p.GetWVPTrans());
		DisplayGrid();

		Vector3f Pos(m_position);
		p.WorldPos(Pos);
		p.WorldPos(0.0f, 10, 0.0f);
		p.Rotate(0.0f, 180.0f, 0.0f);



		m_pCommonEffect->SetWVP(p.GetWVPTrans());
		//m_pCommonEffect->SetSampler(m_pTextureColor);


		//m_pTextureColor->Bind(GL_TEXTURE0);
		m_pTextureDetail->Bind(GL_TEXTURE1);


		glActiveTexture(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE0, m_RenderTexture);
		
		DrawTriangle();

		

		p.Scale(0.1f, 0.1f, 0.1f);
		p.Rotate(0.0f, 180.0f, 0.0f);

		m_pCommonEffect->SetWVP(p.GetWVPTrans());



		m_pEffect->Enable();

		vector<Matrix4f> Transforms;

		float RunningTime = GetRunningTime();

		m_mesh.BoneTransform(RunningTime, Transforms);

		for (uint i = 0; i < Transforms.size(); i++) {
			m_pEffect->SetBoneTransform(i, Transforms[i]);
		}

		m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());

	
		m_pEffect->SetWVP(p.GetWVPTrans());
		m_pEffect->SetWorldMatrix(p.GetWorldTrans());


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
	
		m_pCommonEffect->Enable();
		

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

			DisplayGrid();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
private:
	GLuint VBO;
	GLuint IBO;
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

	GLuint m_RenderTexture;
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

int main(int argc, char** argv)
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