#pragma once
#include <stdio.h>

#include "ogldev_util.h"
#include "ogldev_glut_backend.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"

#include <json/json.h>
#include <fstream>
#include "ogldev_skinned_mesh.h"

using namespace std;

class JsonModelFormat
{
	class meta
	{
	public: 
		string version;
		string generator;
	};
	class materials
	{

	};
	class node
	{
	public:
		node()
		{
			localpos = Vector3f(0, 0, 0);
			pos = Vector3f(0, 0, 0);
			scl = Vector3f(1, 1, 1);
			rot = Vector4f(0,0,0, 1);

		}

		string name;
		int parent;
		Vector3f pos;
		Vector3f localpos;
		Vector3f scl;
		Vector4f rot;
	};

public: 
	JsonModelFormat() {};
	~JsonModelFormat();

	void LoadFromFile(string filename);
	void Load(Json::Value root);
	void Save(string filename);
	string ToString();
public:
	Json::Value root;
	vector<Vector3f> verts;
	map<string,node> nodemap;

};

class JsonRole
{
public:
	void Load(Json::Value jsonvalue)
	{

		Json::Value entity = jsonvalue;

		id = entity["id"].asString();
		assetbundle = entity["assetbundle"].asString();

		face_zero_pointy = entity["face_zero_pointy"].asFloat();
		uvsize = entity["uvsize"].asFloat();
		offset_y = entity["offset_y"].asFloat();





		vector<string> namesvec = entity.getMemberNames();

		vector<string> fixvec;
		vector<string> bonesvec;
		vector<string> offsetvec;

		for (vector<string>::iterator it = namesvec.begin(); it != namesvec.end(); it++)
		{
			string key = *it;
			if (startsWith(key, string("fix_")))
			{
				fixvec.push_back(key);
			}
			else if (startsWith(key, string("_")))
			{
				bonesvec.push_back(key);
			}
			else if (startsWith(key, string("offset")))
			{
				offsetvec.push_back(key);
			}
		}
		for (vector<string>::iterator it = fixvec.begin(); it != fixvec.end(); it++)
		{
			float offset = entity[*it].asFloat();
			fix_map[*it] = offset;
		}


		for (vector<string>::iterator it = bonesvec.begin(); it != bonesvec.end(); it++)
		{
			float x = entity[*it]["x"].asFloat();
			float y = entity[*it]["y"].asFloat();
			float z = entity[*it]["z"].asFloat();
			bonespos_map[*it] = Vector3f(x, y, z);
		}
		for (vector<string>::iterator it = offsetvec.begin(); it != offsetvec.end(); it++)
		{
			if (startsWith(*it, "offset_y"))
			{

			}
			else
			{

				float x = entity[*it]["x"].asFloat();
				float y = entity[*it]["y"].asFloat();
				float z = entity[*it]["z"].asFloat();
				offsets_map[*it] = Vector3f(x, y, z);

			}
		}

	}
	bool startsWith(std::string mainStr, std::string toMatch)
	{
		// std::string::find returns 0 if toMatch is found at starting
		if (mainStr.find(toMatch) == 0)
			return true;
		else
			return false;
	}


	string id;
	string assetbundle;
	float face_zero_pointy;
	float uvsize;
	float offset_y;

	map<string, float> fix_map;
	map<string, Vector3f> bonespos_map;
	map<string, Vector3f> offsets_map;
};
class jsonRoles
{
public:
	void LoadFromFile(string filename)
	{

		Json::CharReaderBuilder rbuilder;
		rbuilder["collectComments"] = false;
		std::string errs;
		Json::Value root;
		std::ifstream ifs;
		ifs.open(filename);
		bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
		ifs.close();

		for (int i = 0; i < root.size(); i++)
		{
			Json::Value entity = root[i];
			JsonRole r;
			r.Load(entity);

			roles[r.id] = r;
		}

	}

	map<string,JsonRole> roles;
};

class JsonFaceInfo
{
public:
	void LoadFromString(string jsonstring,bool israw = true)
	{
		Json::Value root;
		Json::CharReaderBuilder rbuilder;
		Json::CharReader * reader = rbuilder.newCharReader();
		string errors;
		bool ok = reader->parse(jsonstring.c_str(), jsonstring.c_str() + jsonstring.size(), &root, &errors);
		delete reader;
		
		if (israw)
		{
			LoadRaw(root);

		}
		else {

			Load(root);
		}
	}
	void LoadFromFile(string filename)
	{

		Json::CharReaderBuilder rbuilder;
		rbuilder["collectComments"] = false;
		std::string errs;
		Json::Value root;
		std::ifstream ifs;
		ifs.open(filename);
		bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
		ifs.close();

		Load(root);
	}
	void LoadRaw(Json::Value& root)
	{
		Json::Value froot = root["faces"][0];
		face_token = froot["face_token"].asString();

		float top = froot["face_rectangle"]["top"].asFloat();
		float width = froot["face_rectangle"]["width"].asFloat();
		float left = froot["face_rectangle"]["left"].asFloat();
		float height = froot["face_rectangle"]["height"].asFloat();

		face_rectangle.x = top;
		face_rectangle.y = left;
		face_rectangle.z = width;
		face_rectangle.w = height;

		yaw_angle = froot["attributes"]["headpose"]["yaw_angle"].asDouble();
		roll_angle = froot["attributes"]["headpose"]["roll_angle"].asDouble();
		pitch_angle = froot["attributes"]["headpose"]["pitch_angle"].asDouble();

		gender = froot["attributes"]["gender"]["value"].asString();

		Json::Value landmarkdataValue = froot["landmark"];
		vector<string> namesvec = landmarkdataValue.getMemberNames();
		for (vector<string>::iterator it = namesvec.begin(); it != namesvec.end(); it++)
		{
			float x = landmarkdataValue[*it]["x"].asFloat();
			float y = landmarkdataValue[*it]["y"].asFloat();
			landmarkdata[*it] = Vector2f(x, y);
		}

	}
	void Load(Json::Value& root)
	{

		face_token = root["face_token"].asString();

		float top = root["face_rectangle"]["top"].asFloat();
		float width = root["face_rectangle"]["width"].asFloat();
		float left = root["face_rectangle"]["left"].asFloat();
		float height = root["face_rectangle"]["height"].asFloat();

		face_rectangle.x = top;
		face_rectangle.y = left;
		face_rectangle.z = width;
		face_rectangle.w = height;

		yaw_angle = root["face_pose"]["yaw_angle"].asFloat();
		roll_angle = root["face_pose"]["roll_angle"].asFloat();
		pitch_angle = root["face_pose"]["pitch_angle"].asFloat();

		Json::Value landmarkdataValue = root["landmark"]["data"];
		vector<string> namesvec = landmarkdataValue.getMemberNames();
		for (vector<string>::iterator it = namesvec.begin(); it != namesvec.end(); it++)
		{
			float x = landmarkdataValue[*it]["x"].asFloat();
			float y = 1024 - landmarkdataValue[*it]["y"].asFloat();
			landmarkdata[*it] = Vector2f(x, y);
		}

	}

	

public:
	string face_token;
	Vector4f face_rectangle;

	float yaw_angle;
	float roll_angle;
	float pitch_angle;

	string gender;

	map<string, Vector2f> landmarkdata;
};

class KP
{
public:
	KP() {};
	~KP() {};

	string bonename;
	string facekeypointname;
	string offsetname;

private:

};

class  JsonDebug
{
public:
	JsonDebug()
	{

	};
	~JsonDebug()
	{

	};
	void AddNode(vector<string>& keys, vector<string>& values)
	{
		Json::Value jnode;

		for (int i = 0; i < keys.size(); i++)
		{
			jnode[keys[i]] = values[i];
			root.append(jnode);
		}

	}
	void Save(string& path)
	{
		SaveJsonFile(root, path);
	}
	void Clear()
	{
		root.clear();
	}
private:
	void SaveFile(string& s, string& path)
	{
		ofstream write;

		write.open(path.c_str(), ios::out | ios::binary);
		write.write(s.c_str(), s.length());
		write.close();
	}
	void SaveJsonFile(Json::Value jvalue, string& path)
	{
		Json::StreamWriterBuilder  builder;
		builder.settings_["commentStyle"] = "All";
		std::string s = Json::writeString(builder, jvalue);

		SaveFile(s, path);
	}

	Json::Value root;
};
class JsonKeyPointBonePairs
{
	
public:
	void LoadFromFile(string filename)
	{

		Json::CharReaderBuilder rbuilder;
		rbuilder["collectComments"] = false;
		std::string errs;
		Json::Value root;
		std::ifstream ifs;
		ifs.open(filename);
		bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
		ifs.close();

		int count = root.size();

		vector<string> fixvec;
		vector<string> bonesvec;
		vector<string> offsetvec;

		for (int i = 0;i< count;i++)
		{
			KP kp;
			kp.bonename = root[i][0].asString();
			kp.facekeypointname = root[i][1].asString();
			kp.offsetname = root[i][2].asString();

			pairs.push_back(kp);
		}
		

	}
	
public:
	vector<KP> pairs;

};

class BoneUtility
{
public:
	JsonModelFormat jsonModelFormat;
	JsonKeyPointBonePairs pairs;
	Json::Value rtjson;

	bool hasMoveBones;
public:
	void Init();
	int ReadJsonFromFile(const char* filename);

	Texture* CalculateSkin(GLuint texture,bool isman, JsonRole bonedef, JsonFaceInfo& faceinfo);
	void CalculateFaceBone(SkinnedMesh* pmesh, JsonRole bonfdef, JsonFaceInfo faceinfo, string& outOffsetJson, Vector3f& centerpos, Vector2f& uvsize,float& yOffset);

	void ResetBone();
	void MoveBone(SkinnedMesh* pmesh, string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRole bonedef, string boneoffsetname, Vector3f headCenter, float offsetrate = 0.01f);
	void MoveBonePYR(SkinnedMesh* pmesh, string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRole bonedef, string boneoffsetname, Vector3f headCenter, float offsetrate = 0.01f);

	void MoveUV(SkinnedMesh* pmesh, JsonRole bonedef);

	Matrix4f GetLocalMatrixFromGlobal(SkinnedMesh* pmesh ,string bonename,Matrix4f globalmat);

	JsonDebug m_JsonDB;
};



