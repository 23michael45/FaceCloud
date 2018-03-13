/*

Copyright 2010 Etay Meiri

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

Tutorial 13 - Camera Space
*/

#include <stdio.h>
#include <string.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "FaceCloudLib.h"
#include "OSMesaContext.h"
#include "glContext.hpp"

FaceCloudLib lib;
string currentModelID = "10002";
string outJsonModelOut = "";
string jsonfacepath = "/root/jni/data/face/photojson_raw.json";
string photopath = "/root/jni/data/face/photoface_raw.jpg";
string jsonfacestring;

string outPhotoPath = "/root/jni/data/export/outphoto.jpg";
string outjsonoffsetpath = "/root/jni/data/export/outjson.json";



void SaveFile(string& s,string& path)
{
	ofstream write;

	write.open(path.c_str(), ios::out | ios::binary);
	write.write(s.c_str(), s.length());
	write.close();
}
void SaveJsonFile(Json::Value jvalue,string& path)
{
	Json::StreamWriterBuilder  builder;
	builder.settings_["commentStyle"] = "All";
	std::string s = Json::writeString(builder, jvalue);
	
	SaveFile(s, path);
}
Json::Value LoadJsonValueFromFile(string filepath)
{
	Json::CharReaderBuilder rbuilder;
	rbuilder["collectComments"] = false;
	std::string errs;
	Json::Value root;
	std::ifstream ifs;
	ifs.open(filepath);
	bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
	ifs.close();
	return root;
}
string LoadJsonStringFromFile(string filepath)
{
	Json::Value root = LoadJsonValueFromFile(filepath);

	Json::StreamWriterBuilder  builder;
	builder.settings_["commentStyle"] = "All";
	std::string s = Json::writeString(builder, root);
	return s;
}

bool hasdone = false;
void RenderSceneCB()
{
	
	if (!hasdone)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lib.GetCamera()->SetPos(Vector3f(0, 175, -50));

		//ֻ����һ��
		lib.m_bRenderToTexture = false;
		lib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);

		hasdone = true;
		glutSwapBuffers();
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lib.DrawOnce(currentModelID, lib.m_Lastcenter, lib.m_Lastuvsize);
		lib.DisplayGrid();

		glutSwapBuffers();
	}
}
static void PassiveMouseCB(int x, int y)
{
	lib.GetCamera()->OnMouse(x, y);
}

static void SpecialKeyboardCB(int Key, int x, int y)
{
	int start;
	int milliSecondsElapsed;
 	OGLDEV_KEY OgldevKey = GLUTKeyToOGLDEVKey(Key);

	switch (OgldevKey) {
	case OGLDEV_KEY_ESCAPE:
		GLUTBackendLeaveMainLoop();
		break;
	case OGLDEV_KEY_F5:
		

		printf("\n\nStarting timer...");
		start = OSMesa::getMilliCount();

		lib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);
		SaveFile(outJsonModelOut, outjsonoffsetpath);



		milliSecondsElapsed = OSMesa::getMilliSpan(start);
		printf("\n\nElapsed time = %u milliseconds", milliSecondsElapsed);

		break;
	default:
		lib.GetCamera()->OnKeyboard(OgldevKey);
		break;
	}
}
int main(int argc, char** argv)
{
	bool bRenderToTarget = true;

	if (!lib.Init(false))
	{
		printf("Face Cloud Lib Init Failed");
		return -1;
	}



	jsonfacestring = LoadJsonStringFromFile(jsonfacepath);

	if (bRenderToTarget)
	{
		//glutSpecialFunc(SpecialKeyboardCB);

	}
	else
	{
		glutDisplayFunc(RenderSceneCB);
		glutIdleFunc(RenderSceneCB);
		glutSpecialFunc(SpecialKeyboardCB);

	}

	//while (true)
	//{
	//	printf("loop");
	//	this_thread::sleep_for(chrono::microseconds(0));
	//}

	if (bRenderToTarget)
	{
		int i = 0;

		string basepath = "/tmp/outPhotoPath%d.jpg";
		string baseoffsetpath = "/tmp/outJsonPath%d.json";
		while (true)
		{
			stringstream s1;
			s1 << basepath << i << ".jpg";
			outPhotoPath = s1.str();

			stringstream s2;
			s2 << baseoffsetpath << i << ".json";
			baseoffsetpath = s2.str();
			outJsonModelOut = s2.str();

			lib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);
			SaveFile(outJsonModelOut, outjsonoffsetpath);
			i++;
		}
		
		//glutMainLoop();

		//ʵ�ʷ������������ļ���ֻ����STRING�Ϳ���
		//jsonfacestring = LoadJsonStringFromFile("data/face/photojson_raw.json");
	}
	else
	{
		glutPassiveMotionFunc(PassiveMouseCB);
		glutMainLoop();
	}

	getchar();
	return 0;
}
