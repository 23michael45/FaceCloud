
#include <stdio.h>
#include <string.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_util.h"
#include "ogldev_glut_backend.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"

#include <json/json.h>
#include <fstream>
#include "BoneUtility.h"

#include "ogldev_texture.h"
#include "FBXImportUtility.h"

#include "FBXLoader/FBXLoader.h"
#include "ogldev_skinned_mesh.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WindowTitle  "OpenGLˮ�浹Ӱ"  

GLuint VBO;
	GLuint IBO;

	//�����������������  
	GLuint shanghai;
	GLuint water;
	Json::Value jsonRoot;

	Camera* pGameCamera = NULL;
	PersProjInfo gPersProjInfo;
	const char* pVSFileName = "data/shader.vs";
	const char* pFSFileName = "data/shader.fs";
	GLuint gWVPLocation;

	JsonModelFormat jsonModel;

	GLuint gSampler;

	//globals
	//FBXLoader * gSceneContext;
	Texture* pTexture = NULL;
#define BMP_Header_Length 54  //ͼ���������ڴ���е�ƫ����  


	// ����power_of_two�����ж�һ�������ǲ���2����������  
	int power_of_two(int n)
	{
		if (n <= 0)
			return 0;
		return (n & (n - 1)) == 0;
	}

	/* ����load_texture
	* ��ȡһ��BMP�ļ���Ϊ����
	* ���ʧ�ܣ�����0������ɹ�������������
	*/
	GLuint load_texture(const char* file_name)
	{
		GLint width, height, total_bytes;
		GLubyte* pixels = 0;
		GLuint last_texture_ID = 0, texture_ID = 0;

		// ���ļ������ʧ�ܣ�����  
		FILE* pFile = fopen(file_name, "rb");
		if (pFile == 0)
			return 0;

		// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�  
		fseek(pFile, 0x0012, SEEK_SET);
		fread(&width, 4, 1, pFile);
		fread(&height, 4, 1, pFile);
		fseek(pFile, BMP_Header_Length, SEEK_SET);

		// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���  
		{
			GLint line_bytes = width * 3;
			while (line_bytes % 4 != 0)
				++line_bytes;
			total_bytes = line_bytes * height;
		}

		// �����������ֽ��������ڴ�  
		pixels = (GLubyte*)malloc(total_bytes);
		if (pixels == 0)
		{
			fclose(pFile);
			return 0;
		}

		// ��ȡ��������  
		if (fread(pixels, total_bytes, 1, pFile) <= 0)
		{
			free(pixels);
			fclose(pFile);
			return 0;
		}

		// �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������  
		// ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����  
		{
			GLint max;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
			if (!power_of_two(width)
				|| !power_of_two(height)
				|| width > max
				|| height > max)
			{
				const GLint new_width = 1024;
				const GLint new_height = 1024; // �涨���ź��µĴ�СΪ�߳���������  
				GLint new_line_bytes, new_total_bytes;
				GLubyte* new_pixels = 0;

				// ����ÿ����Ҫ���ֽ��������ֽ���  
				new_line_bytes = new_width * 3;
				while (new_line_bytes % 4 != 0)
					++new_line_bytes;
				new_total_bytes = new_line_bytes * new_height;

				// �����ڴ�  
				new_pixels = (GLubyte*)malloc(new_total_bytes);
				if (new_pixels == 0)
				{
					free(pixels);
					fclose(pFile);
					return 0;
				}

				// ������������  
				gluScaleImage(GL_RGB,
					width, height, GL_UNSIGNED_BYTE, pixels,
					new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

				// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height  
				free(pixels);
				pixels = new_pixels;
				width = new_width;
				height = new_height;
			}
		}

		// ����һ���µ�������  
		glGenTextures(1, &texture_ID);
		if (texture_ID == 0)
		{
			free(pixels);
			fclose(pFile);
			return 0;
		}

		// ���µ������������������������  
		// �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�  
		GLint lastTextureID = last_texture_ID;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
		glBindTexture(GL_TEXTURE_2D, texture_ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
			GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, lastTextureID);  //�ָ�֮ǰ�������  
		free(pixels);
		return texture_ID;
	}


	void display(void)
	{
		// �����Ļ  
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// �����ӽ�  
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(70, 1, 1, 21);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0, 7, -1.5, 0, 0, 0, 0, 0, -1);

		// ���Ƶ�Ӱ
		glBindTexture(GL_TEXTURE_2D, water);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -3.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.0f, -3.0f, 5.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(6.0f, -3.0f, 5.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -3.0f, 0.0f);
		glEnd();

		//������ʵ����
		glBindTexture(GL_TEXTURE_2D, shanghai);
		glTranslatef(0, -6, 0);
		glRotatef(180, 1, 0, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -3.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.0f, -3.0f, 5.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(6.0f, -3.0f, 5.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -3.0f, 0.0f);
		glEnd();

		glutSwapBuffers();
	}



	void LoadJson()
	{

		Json::CharReaderBuilder rbuilder;
		rbuilder["collectComments"] = false;
		std::string errs;
		Json::Value root;
		std::ifstream ifs;
		//ifs.open("data/womenhead.json");
		ifs.open("data/fbxjson.json");
		bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
		ifs.close();


		jsonModel.Load(root);

	}

	void CreateVertexBuffer()
	{



		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(*(jsonModel.pvertices)) * jsonModel.verticesCount / 3, jsonModel.pvertices, GL_STATIC_DRAW);


		/*
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);*/
	}

	static void CreateIndexBuffer()
	{
		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*(jsonModel.pindices)) * jsonModel.indicesVec.size() * 3, jsonModel.pindices, GL_STATIC_DRAW);

		/*unsigned int Indices[] = { 0, 3, 1,
			1, 3, 2,
			2, 3, 0,
			0, 1, 2 };

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);*/
	}


	static void RenderSceneCB()
	{
		// Clear Color and Depth Buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static float Scale = 0.0f;

		Scale += 0.5f;

		Pipeline p;
		p.Scale(1.0f);
		p.Rotate(0.0, 0.0, 0.0f);
		p.WorldPos(0.0f, 10000.0f, -10000.0f);
		Vector3f CameraPos(0.0f, 0.0f, -1000.0f);
		Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
		Vector3f CameraUp(0.0f, 1.0f, 0.0f);
		p.SetCamera(*pGameCamera);
		//p.SetCamera(CameraPos, CameraTarget, CameraUp);
		p.SetPerspectiveProj(gPersProjInfo);

		
		glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetWVPTrans());


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);


		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		pTexture->Bind(GL_TEXTURE0);
		//glDrawArrays(GL_POINTS, 0, 1);
		//glDrawElements(GL_TRIANGLES, jsonModel.indicesVec.size() * 3, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		//gSceneContext->Draw();

		glutSwapBuffers();
	}


	static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
	{
		GLuint ShaderObj = glCreateShader(ShaderType);

		if (ShaderObj == 0) {
			fprintf(stderr, "Error creating shader type %d\n", ShaderType);
			exit(1);
		}

		const GLchar* p[1];
		p[0] = pShaderText;
		GLint Lengths[1];
		Lengths[0] = strlen(pShaderText);
		glShaderSource(ShaderObj, 1, p, Lengths);
		glCompileShader(ShaderObj);
		GLint success;
		glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
			fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
			exit(1);
		}

		glAttachShader(ShaderProgram, ShaderObj);
	}

	static void CompileShaders()
	{
		GLuint ShaderProgram = glCreateProgram();

		if (ShaderProgram == 0) {
			fprintf(stderr, "Error creating shader program\n");
			exit(1);
		}

		string vs, fs;

		if (!ReadFile(pVSFileName, vs)) {
			exit(1);
		};

		if (!ReadFile(pFSFileName, fs)) {
			exit(1);
		};

		AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
		AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

		GLint Success = 0;
		GLchar ErrorLog[1024] = { 0 };

		glLinkProgram(ShaderProgram);
		glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
		if (Success == 0) {
			glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
			exit(1);
		}

		glValidateProgram(ShaderProgram);
		glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
		if (!Success) {
			glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
			exit(1);
		}

		glUseProgram(ShaderProgram);

		gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
		assert(gWVPLocation != 0xFFFFFFFF);

		gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
		assert(gSampler != 0xFFFFFFFF);
	}

	static void SpecialKeyboardCB(int Key, int x, int y)
	{
		OGLDEV_KEY OgldevKey = GLUTKeyToOGLDEVKey(Key);
		pGameCamera->OnKeyboard(OgldevKey);
	}
	static void PassiveMouseCB(int x, int y)
	{
		pGameCamera->OnMouse(x, y);
	}
	static void KeyboardCB(unsigned char Key, int x, int y)
	{
		switch (Key) {
		case 'q':
			glutLeaveMainLoop();
		}
	}
	// Trigger the display of the current frame.
	void TimerCallback(int)
	{
		// Ask to display the current frame only if necessary.
		//if (gSceneContext->GetStatus() == SceneContext::MUST_BE_REFRESHED)
		//{
		//	glutPostRedisplay();
		//}

		//gSceneContext->OnTimerClick();

		// Call the timer to display the next frame.
		//glutTimerFunc((unsigned int)gSceneContext->GetFrameTime().GetMilliSeconds(), TimerCallback, 0);
	}

	void init()
	{

		GLfloat light_ambient[] =
		{ 0.2, 0.2, 0.2, 1.0 };
		GLfloat light_diffuse[] =
		{ 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_specular[] =
		{ 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_position[] =
		{ 1.0, 1.0, 1.0, 0.0 };

		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);

		glEnable(GL_LIGHT0);
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);



		//to do to load model..
		//gSceneContext->LoadFile();

		//choose spicefic animation
		//animations in the fbx file array from 0 to --> no of animations..
		//gSceneContext->SetCurrentAnimStack(0); //Animation Array Selection..

											   //we need timer to calculate frame time..
											   // Call the timer to display the first frame.
		//glutTimerFunc((unsigned int)gSceneContext->GetFrameTime().GetMilliSeconds(), TimerCallback, 0);


	}
	int main__(int argc, char* argv[])
	{
		//Do();
		//Parse();
		// Use a custom memory allocator
	/*	FbxSetMallocHandler(FBXMemoryAllocator::FBXMalloc);
		FbxSetReallocHandler(FBXMemoryAllocator::FBXRealloc);
		FbxSetFreeHandler(FBXMemoryAllocator::FBXFree);
		FbxSetCallocHandler(FBXMemoryAllocator::FBXCalloc);*/




		// GLUT��ʼ��  
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		glutCreateWindow(WindowTitle);


		//glEnable(GL_DEPTH_TEST);
		//glEnable(GL_TEXTURE_2D);    // ��������  
		//shanghai = load_texture("data/building.bmp");  //��������  
		//											   //water = load_texture("data/water.bmp");
		//water = load_texture("data/waterblend.bmp");
		////glutDisplayFunc(&display);   //ע�ắ��    

		

		glutDisplayFunc(RenderSceneCB);
		glutIdleFunc(RenderSceneCB);
		glutSpecialFunc(SpecialKeyboardCB);
		glutPassiveMotionFunc(PassiveMouseCB);
		glutKeyboardFunc(KeyboardCB);

		pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);
		GLenum res = glewInit();
		if (res != GLEW_OK) {
			fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
			return 1;
		}

		/*
		FbxString lFilePath("data/women_head.FBX");
		gSceneContext = new FBXLoader(lFilePath, 800, 600);*/
		init();
		LoadJson();



		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// OpenGL init
		glEnable(GL_DEPTH_TEST);

		CreateVertexBuffer();
		CreateIndexBuffer();


		CompileShaders();



		glUniform1i(gSampler, 0);

		pTexture = new Texture(GL_TEXTURE_2D, "data/test.png");
		//pTexture = new Texture(GL_TEXTURE_2D, "data/head_women.jpg");
		if (!pTexture->Load()) {
			return 1;
		}

		gPersProjInfo.FOV = 60.0f;
		gPersProjInfo.Height = WINDOW_HEIGHT;
		gPersProjInfo.Width = WINDOW_WIDTH;
		gPersProjInfo.zNear = 0.1f;
		gPersProjInfo.zFar = 10000.0f;

		glutMainLoop(); //ѭ������  
		return 0;
	}