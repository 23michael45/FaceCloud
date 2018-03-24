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
#include "ImageOptimizedUtility.h"

FaceCloudLib lib;
string currentModelID = "10002";
string outJsonModelOut = "";
//string jsonfacepath = "data/face/photojson_raw_server.json";
//string photopath = "data/face/photoface_raw_server.jpg";
string jsonfacepath = "data/skindetect/color/color_white_org.json";
string photopath = "data/skindetect/color/color_white_org.jpg";
//string jsonfacepath = "data/skindetect/color/color_nature _org.json";
//string photopath = "data/skindetect/color/color_nature _org.jpg";
//string jsonfacepath = "data/skindetect/color/color_dark_org.json";
//string photopath = "data/skindetect/color/color_dark_org.jpg";

string jsonfacestring;

string outPhotoPath = "data/export/outphoto.jpg";
string outjsonoffsetpath = "data/export/outjsonoffset.json";
string outjsonskinpath = "data/export/outjsonskin.json";



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

		//只计算一次
		lib.m_bRenderToTexture = false;
		lib.CalculateReal(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);

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

		lib.CalculateReal(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);
		SaveFile(outJsonModelOut, outjsonoffsetpath);



		milliSecondsElapsed = OSMesa::getMilliSpan(start);
		printf("\n\nElapsed time = %u milliseconds", milliSecondsElapsed);

		break;
	default:
		lib.GetCamera()->OnKeyboard(OgldevKey);
		break;
	}
}

void LoadFaceImgAndJson(string tname, string lname, string& photopath, string& jsonstr)
{
	string path = "data/skindetect/";
	string imgpath = path + tname + "/" + lname + "_org.jpg";
	string jsonpath = path + tname + "/" + lname + "_org.json";

	jsonstr = LoadJsonStringFromFile(jsonpath);
	photopath = imgpath;
}

void FaceSkinStatusDetect(string tname, string lname)
{
	string photopath;
	string jsonFace;
	LoadFaceImgAndJson(tname, lname, photopath, jsonFace);

	string jsonModelOut;
	string s = lib.DetectSkinStatus(photopath, jsonFace, jsonModelOut);	
	SaveFile(s, outjsonskinpath);

}
int main(int argc, char** argv)
{
	FaceSkinStatusDetect("color", "color_white");
	return 0;
}


int main_(int argc, char** argv)
{
	bool bRenderToTarget = true;


	jsonfacestring = LoadJsonStringFromFile(jsonfacepath);

	if (bRenderToTarget)
	{
		if (!lib.Init(false))
		{
			printf("Face Cloud Lib Init Failed");
			return -1;
		}
		//glutSpecialFunc(SpecialKeyboardCB);
	}
	else
	{
		if (!lib.InitReal(false))
		{
			printf("Face Cloud Lib Init Failed");
			return -1;
		}
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
		lib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);
		SaveFile(outJsonModelOut, outjsonoffsetpath);

		int i = 0;

		string basepath = "/tmp/outPhotoPath";
		string baseoffsetpath = "/tmp/outJsonPath";




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

		//实际服务器不生成文件，只返回STRING就可以
		//jsonfacestring = LoadJsonStringFromFile("data/face/photojson_raw.json");
	}
	else
	{
		glutPassiveMotionFunc(PassiveMouseCB);
		glutMainLoop();
	}

	while (true)
	{
		getchar();
		lib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath, outJsonModelOut);

	}
	return 0;
}