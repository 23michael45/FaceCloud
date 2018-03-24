#include "BoneUtility.h"
#include <fstream>
#include <json/json.h>

#include <json/json.h>
#include <fstream>
#include <sstream>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/core/core.hpp> 
#include "ImageOptimizedUtility.h"

#include <GL/glew.h>
#include "Predefined.h"
#include "OSMesaContext.h"

void JsonModelFormat::LoadFromFile(string filename)
{

	Json::CharReaderBuilder rbuilder;
	rbuilder["collectComments"] = false;
	std::string errs;
	std::ifstream ifs;
	ifs.open(filename);
	bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
	ifs.close();

	Load(root);
}
void JsonModelFormat::Load(Json::Value root)
{
	Json::Value mesh = root["model"]["meshes"][0];

	Json::Value verts = mesh["verts"];

	Json::Value vertElement = mesh["vertElement"];
	Json::Value vertIndices = vertElement["vertIndices"];
	Json::Value normals = vertElement["normals"];
	Json::Value uvs = vertElement["uvs"];
	Json::Value uv1 = uvs[0];
	Json::Value uv2 = uvs[1];

	Json::Value face = mesh["face"];
	Json::Value vertElementIndices = face["vertElementIndices"];

	Json::Value skin = mesh["skin"];
	Json::Value skinBones = skin["skinBones"];
	Json::Value skinWeights = skin["skinWeights"];



	Json::Value jnodes = root["hierarchy"]["nodes"];
	int nodescount = jnodes.size();

	for (int i = 0 ; i< nodescount;i++)
	{
		Json::Value jnode = jnodes[i];
		JsonModelFormat::node node;
		node.name = jnode["name"].asString();
		node.parent = jnode["parent"].asInt();
		/*node.pos = Vector3f(jnode["pos"][0].asFloat(), jnode["pos"][1].asFloat(), jnode["pos"][2].asFloat());
		node.scl = Vector3f(jnode["scl"][0].asFloat(), jnode["scl"][1].asFloat(), jnode["scl"][2].asFloat());
		node.rot = Vector4f(jnode["rot"][0].asFloat(), jnode["rot"][1].asFloat(), jnode["rot"][2].asFloat(), jnode["rot"][3].asFloat());*/

		node.pos = Vector3f(0, 0, 0);
		node.scl = Vector3f(1, 1, 1);
		node.rot = Vector4f(0, 0 ,0, 1);
		nodemap[node.name] = node;
	}

}
string JsonModelFormat::ToString()
{
	//All Info
	/*Json::Value jnodes = root["hierarchy"]["nodes"];
	for (int i = 0; i < jnodes.size(); i++)
	{
		jnodes[i]["pos"][0] = Json::Value(nodemap[jnodes[i]["name"].asString()].pos.x);
		jnodes[i]["pos"][1] = Json::Value(nodemap[jnodes[i]["name"].asString()].pos.y);
		jnodes[i]["pos"][2] = Json::Value(nodemap[jnodes[i]["name"].asString()].pos.z);
	}*/

	//Tiny Info
	Json::Value jnodes = root["hierarchy"]["nodes"];
	Json::Value oroot;
	Json::Value hierarchy;
	Json::Value nodes;
	for (int i = 0; i < jnodes.size(); i++)
	{
		jnodes[i]["pos"][0] = Json::Value(nodemap[jnodes[i]["name"].asString()].pos.x);
		jnodes[i]["pos"][1] = Json::Value(nodemap[jnodes[i]["name"].asString()].pos.y);
		jnodes[i]["pos"][2] = Json::Value(nodemap[jnodes[i]["name"].asString()].pos.z);

		jnodes[i]["localpos"][0] = Json::Value(nodemap[jnodes[i]["name"].asString()].localpos.x);
		jnodes[i]["localpos"][1] = Json::Value(nodemap[jnodes[i]["name"].asString()].localpos.y);
		jnodes[i]["localpos"][2] = Json::Value(nodemap[jnodes[i]["name"].asString()].localpos.z);

		Json::Value jnewnode;
		jnewnode["name"] = jnodes[i]["name"];
		//jnewnode["pos"][0] = jnodes[i]["pos"][0];Vector3f
		//jnewnode["pos"][1] = jnodes[i]["pos"][1];
		//jnewnode["pos"][2] = jnodes[i]["pos"][2];

		jnewnode["localpos"][0] = jnodes[i]["localpos"][0];
		jnewnode["localpos"][1] = jnodes[i]["localpos"][1];
		jnewnode["localpos"][2] = jnodes[i]["localpos"][2];


		Matrix4f worldmat = nodemap[jnodes[i]["name"].asString()].worldmatrix;
		Matrix4f localmat = nodemap[jnodes[i]["name"].asString()].localmatrix;
		for (int i = 0 ; i < 4 ;i++)
		{
			for (int j = 0; j < 4; j++)
			{
				char key[32];
				sprintf(key , "m%d%d", i, j);
				//jnewnode["worldmatrix"][key] = worldmat.m[i][j];
				//jnewnode["localmatrix"][key] = localmat.m[i][j];
			}
		}


		nodes.append(jnewnode);
	}
	hierarchy["nodes"] = nodes;
	oroot["hierarchy"] = hierarchy;



	//Json Value to String
	Json::StreamWriterBuilder  builder;
	builder.settings_["commentStyle"] = "None";
	//builder.settings_["indentation"] = "All";
	std::string s = Json::writeString(builder, nodes);

	return s;
}
void JsonModelFormat::Save(string filename)
{
	string s = ToString();

	ofstream write;
	write.open(filename.c_str(), ios::out | ios::binary);
	write.write(s.c_str(), s.length());
	write.close();
}
JsonModelFormat::~JsonModelFormat()
{
}




void BoneUtility::Init()
{
	hasMoveBones = false;
	pairs.LoadFromFile(RES_PATH + string("face/kp.json"));
	jsonModelFormat.LoadFromFile(RES_PATH + string("face/women_head_fix.JD"));
}

int BoneUtility::ReadJsonFromFile(const char* filename)
{

	Json::Value root; // Json::Value��һ�ֺ���Ҫ�����ͣ����Դ����������͡���int, string, object, array         


					  // Here, using a specialized Builder, we discard comments and
					  // record errors as we parse.
	Json::CharReaderBuilder rbuilder;
	rbuilder["collectComments"] = false;
	std::string errs;



	std::ifstream ifs;
	ifs.open(filename);
	bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
	ifs.close();


	Json::StreamWriterBuilder wbuilder;
	wbuilder["indentation"] = "\t";
	std::string document = Json::writeString(wbuilder, root);



	float face_rectangle = root["face_rectangle"]["top"].asFloat();





	return 0;
}
Vector2f Bezier2(Vector2f p0, Vector2f p1, Vector2f p2 ,float t)
{
	Vector2f p;
	p.x = (1 - t) * (1 - t) * p0.x + 2 * t * (1 - t) * p1.x + t * t * p2.x;
	p.y = (1 - t) * (1 - t) * p0.y + 2 * t * (1 - t) * p1.y + t * t * p2.y;
	return p;
}


Texture* BoneUtility::CalculateSkin(GLuint texture, cv::Mat& refmat, bool isman, JsonRole bonedef, JsonFaceInfo& faceinfo)
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


	long size = wtex * htex * 4;
	unsigned char* output_image = new unsigned char[size];


	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, output_image);

	GLenum error = glGetError();
	const GLubyte * eb = gluErrorString(error);
	string errorstring((char*)eb);
	cv::Mat srcimg = cv::Mat(htex, wtex, CV_8UC4, (unsigned*)output_image);

	cv::Mat srcimg32;
	srcimg.convertTo(srcimg32, CV_32FC4);



	ImageOptimizedUtility iou;

	OSMesa::Log("\nStart FacePhotoProcess");
	//do photo pre process
	cv::Mat img = iou.FacePhotoProcess(faceinfo, bonedef, srcimg32);


	Mat rgbimg;
	cv::cvtColor(img, rgbimg, CV_RGBA2RGB);


	Vector3f ref_color;

	if (isman)
	{
		ref_color = Vector3f(201, 159, 117);
	}
	else {
		ref_color = Vector3f(205, 175, 141);
	}


	//org left bottom (unity)
	/*Vector2f leftpoint(350, 340);
	Vector2f rightpoint(475, 420);*/


	//org left top (opencv)
	/*Vector2f leftpoint(350, 684);
	Vector2f rightpoint(475, 604);*/

	/*Vector2f leftpoint = (faceinfo.landmarkdata["contour_left9"] + faceinfo.landmarkdata["nose_left_contour2"]) * 0.5;
	Vector2f rightpoint = (faceinfo.landmarkdata["contour_right3"] + faceinfo.landmarkdata["nose_right_contour2"])* 0.5;*/

	Mat rtrefmat;


	/////////////////////////////////////////////////////////肤色处理
	OSMesa::Log("\nStart ColorTransfer");
	
	//使用肤色处理
	iou.ColorTransfer(rgbimg, refmat, rtrefmat, faceinfo);
	rtrefmat.copyTo(refmat);

	//不用肤色处理
	//rgbimg.copyTo(rtmat);

	//iou.UpdateRef_RGB(faceinfo,rgbimg,ref_color, 1.0f, rtmat, leftpoint, rightpoint);
	/////////////////////////////////////////////////////////肤色处理




	/////////////////////////////////////////////////////////按特征点画轮廓
	vector<vector<cv::Point> > contours;
	vector<cv::Point> contour;

	Vector2f leftpos1 = faceinfo.landmarkdata["contour_left1"];
	Vector2f rightpos1 = faceinfo.landmarkdata["contour_right1"];
	Vector2f chinpos = faceinfo.landmarkdata["contour_chin"];

	Vector2f centerpos = (leftpos1 + rightpos1) * 0.5;
	centerpos.y = centerpos.y + (centerpos.y - chinpos.y)*0.5;
	contour.push_back(cv::Point(centerpos.x, 1024 - centerpos.y));


	Vector2f pleftcontrol(leftpos1.x, centerpos.y);
	
	int interLen = 20;
	for (int i = 1 ; i<interLen ;i++)
	{
		Vector2f p = Bezier2(centerpos, pleftcontrol, leftpos1, (float)i / interLen);
		contour.push_back(cv::Point(p.x, 1024 - p.y));

	}


	Vector2f pos;
	pos = faceinfo.landmarkdata["contour_left1"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left2"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left3"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left4"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left5"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left6"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left7"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left8"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left9"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left10"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left11"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left12"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left13"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left14"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left15"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left16"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_chin"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right16"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right15"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right14"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right13"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right12"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right11"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right10"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right9"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right8"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right7"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right6"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right5"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right4"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right3"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right2"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right1"]; contour.push_back(cv::Point(pos.x, 1024 - pos.y));



	Vector2f prightcontrol(rightpos1.x, centerpos.y);
	for (int i = 1; i < interLen; i++)
	{
		Vector2f p = Bezier2(rightpos1, prightcontrol, centerpos, (float)i / interLen);
		contour.push_back(cv::Point(p.x, 1024 - p.y));

	}

	contours.push_back(contour);



	cv::Mat contourmask(1024, 1024, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Scalar colorwhite = cv::Scalar(1, 1, 1);
	drawContours(contourmask, contours, -1, colorwhite, CV_FILLED);




	cv::multiply(rgbimg, contourmask, rgbimg);

	/////////////////////////////////////////////////////////按特征点画轮廓



	//convert to opengl texture
	Texture *ptexture = new Texture();
	ptexture->FromCVMat(GL_TEXTURE_2D, rgbimg);

	SAFE_DELETE(output_image);
	return ptexture;
}

void BoneUtility::CalculateFaceBone(SkinnedMesh* pmesh, JsonRole bonedef, JsonFaceInfo faceinfo,string& outOffsetJson,Vector3f& centerpos,Vector2f& uvsize,float& yoffset)
{

	Matrix4f tooth_MID = pmesh->GetBoneNode("face_mouthLip_up_joint2");
	Matrix4f toothup_Lf = pmesh->GetBoneNode("face_mouthLip_Lf_joint1");
	Matrix4f toothdown_Rt = pmesh->GetBoneNode("face_mouthLip_Rt_joint1");

	Vector3f tooth_MID_pos1 = tooth_MID.ExtractTranslation();
	Vector3f toothup_Lf_pos1 = toothup_Lf.ExtractTranslation();
	Vector3f toothdown_Rt_pos1 = toothdown_Rt.ExtractTranslation();

	//��ͷƫ��

	Matrix4f forehead_RT = pmesh->GetBoneNode("face_temple_Rt_joint1");
	Matrix4f forehead_LF = pmesh->GetBoneNode("face_temple_Lf_joint1");


	Vector3f forhead_RT_pos1 = forehead_RT.ExtractTranslation();
	Vector3f forhead_LF_pos1 = forehead_LF.ExtractTranslation();

	//üë����

	Matrix4f brow_LF02 = pmesh->GetBoneNode("face_brow_Lf_joint2");
	Matrix4f brow_LF03 = pmesh->GetBoneNode("face_brow_Lf_joint3");

	Vector3f brow_LF02_pos1 = brow_LF02.ExtractTranslation();
	Vector3f brow_LF03_pos1 = brow_LF03.ExtractTranslation();

	//��������
	Matrix4f nose_tr = pmesh->GetBoneNode("face_nose_joint2");
	Vector3f nose_tr_pos1 = nose_tr.ExtractTranslation();


	//�������

	//嘴巴修正

	Matrix4f mouth_tr = pmesh->GetBoneNode("face_mouthLip_up_joint0");
	Vector3f mouth_tr_pos1 = mouth_tr.ExtractTranslation();

	//�۾�����
	float righteye_conner_pos1 = pmesh->GetBoneNode("face_eyeLids_Rt_joint1").ExtractTranslation().x;
	float righteye_conner_pos2 = pmesh->GetBoneNode("face_eyeLids_Rt_joint2").ExtractTranslation().x;
	float Lefteye_conner_pos1 = pmesh->GetBoneNode("face_eyeLids_Lf_joint1").ExtractTranslation().x;
	float Lefteye_conner_pos2 = pmesh->GetBoneNode("face_eyeLids_Lf_joint2").ExtractTranslation().x;

	float Eye_Rt_dis = righteye_conner_pos1 - righteye_conner_pos2;
	float Eye_LF_dis = Lefteye_conner_pos1 - Lefteye_conner_pos2;


	Vector3f fixmouthlipdistance01 = pmesh->GetBoneNode("face_mouthLip_Lf_joint5").ExtractTranslation() - pmesh->GetBoneNode("face_mouthLip_Lf_joint7").ExtractTranslation();
	Vector3f fixmouthlipdistance02 = pmesh->GetBoneNode("face_mouthLip_Rt_joint5").ExtractTranslation() - pmesh->GetBoneNode("face_mouthLip_Rt_joint7").ExtractTranslation();
	Vector3f fixmouthlipdistance03 = pmesh->GetBoneNode("face_mouthLip_dn_joint2").ExtractTranslation() - pmesh->GetBoneNode("face_mouthLip_up_joint2").ExtractTranslation();


	//  Debug.OSMesa::Log("Eye_Rt_dis" + Eye_Rt_dis+ "Eye_LF_dis"+ Eye_LF_dis);

	Vector3f fixnose_lf1 = pmesh->GetBoneNode("face_nosewing_Lf_joint1").ExtractTranslation();
	Vector3f fixnose_RT1 = pmesh->GetBoneNode("face_nosewing_Rt_joint1").ExtractTranslation();


	Matrix4f  face_eyeLids_Lf_joint1  = pmesh->GetBoneNode("face_eyeLids_Lf_joint1");
	Matrix4f face_eyeBall_Lf_joint1 = pmesh->GetBoneNode("face_eyeBall_Lf_joint1");

	Vector3f leftpos = pmesh->GetBoneNode("face_eyeLids_Lf_joint1").ExtractTranslation();
	Vector3f rightpos = pmesh->GetBoneNode("face_eyeLids_Rt_joint1").ExtractTranslation();
	Vector3f headCenter = (leftpos + rightpos) * 0.5f;


	rtjson.clear();
	for (int i = 0; i < pairs.pairs.size(); i++)
	{
		KP kp = pairs.pairs[i];
		MoveBone(pmesh, kp.bonename, faceinfo, kp.facekeypointname, bonedef, kp.offsetname, headCenter);
	}
	
	//MoveUV(pmesh, bonedef);

	outOffsetJson = jsonModelFormat.ToString();
	yoffset = bonedef.offset_y / 100;
	centerpos = headCenter;
	uvsize = Vector2f(bonedef.uvsize, bonedef.uvsize);

	/*string s = "data/export/bonertdb.json";
	m_JsonDB.Save(s);*/








	/////////////////////////////////
	float righteye_conner_posf1 = GetLocalPosition(pmesh ,"face_eyeLids_Rt_joint1").x;
	float righteye_conner_posf2 = GetLocalPosition(pmesh, "face_eyeLids_Rt_joint2").x;
	float Lefteye_conner_posf1 = GetLocalPosition(pmesh, "face_eyeLids_Lf_joint1").x;
	float Lefteye_conner_posf2 = GetLocalPosition(pmesh, "face_eyeLids_Lf_joint2").x;

	float Eye_Rt_dis2 = righteye_conner_posf1 - righteye_conner_posf2;
	float Eye_LF_dis2 = Lefteye_conner_posf1 - Lefteye_conner_posf2;


	float eyescalmap = (Eye_Rt_dis2 / Eye_Rt_dis + Eye_LF_dis2 / Eye_LF_dis) / 2;
	eyemapscale(eyescalmap);

	Vector3f brow_LF02_pos2 = brow_LF02.ExtractTranslation();
	Vector3f brow_LF03_pos2 = brow_LF03.ExtractTranslation();

	float brow_LF02_offset = brow_LF02_pos2.x - brow_LF02_pos1.x;
	float brow_LF03_offset = brow_LF03_pos2.x - brow_LF03_pos1.x;



	SetLocalPositionOffset(pmesh, "face_brow_Lf_joint2",  Vector3f(0, 0, brow_LF02_offset / 2));
	SetLocalPositionOffset(pmesh, "face_brow_Lf_joint3",Vector3f(0, 0, brow_LF03_offset / 2));
	SetLocalPositionOffset(pmesh, "face_brow_Rt_joint2", Vector3f(0, 0, brow_LF02_offset / 2));
	SetLocalPositionOffset(pmesh, "face_brow_Rt_joint3", Vector3f(0, 0, brow_LF03_offset / 2));

	/////////////////////////////////////        


	SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint4",Vector3f(0, brow_LF03_offset / 2, brow_LF03_offset / 2));
	SetLocalPositionOffset(pmesh, "face_forehead_Rt_joint2", Vector3f(0, brow_LF03_offset / 2, -brow_LF03_offset / 2));
	SetLocalPositionOffset(pmesh, "face_chin_Lf_joint7", Vector3f(0, brow_LF03_offset / 2, brow_LF03_offset / 2));
	SetLocalPositionOffset(pmesh, "face_chin_Rt_joint7", Vector3f(0, brow_LF03_offset / 2, -brow_LF03_offset / 2));









	Vector3f nose_tr_pos2 = nose_tr.ExtractTranslation();
	float nose_offset = nose_tr_pos2.y - nose_tr_pos1.y;
	//  _bones["face_nose_joint0"].localPosition += Vector3f(0, nose_offset/3f, 0);

	Vector3f mouth_tr_pos2 = mouth_tr.ExtractTranslation();
	float mouth_offset = mouth_tr_pos2.y - mouth_tr_pos1.y;
	//  _bones["face_mouthLip_joint0"].localPosition -= Vector3f(0, mouth_offset/3f, 0);



	// Debug.OSMesa::Log("Eye_Rt_dis2" + Eye_Rt_dis2 + "Eye_LF_dis2" + Eye_LF_dis2);
	// float eyescal = ((Eye_Rt_dis2 / Eye_Rt_dis) + (Eye_LF_dis2 / Eye_LF_dis)) / 2 ;
	// Debug.OSMesa::Log("eyescal" + eyescal);

	///////////////////////////////////////////////       

	Vector3f tooth_MID_pos2 = tooth_MID.ExtractTranslation();
	Vector3f toothup_Lf_pos2 = toothup_Lf.ExtractTranslation();
	Vector3f toothdown_Rt_pos2 = toothdown_Rt.ExtractTranslation();

	float toothpos_y = tooth_MID_pos2.y - tooth_MID_pos1.y;
	float toothLf_x = toothup_Lf_pos2.x - toothup_Lf_pos1.x;
	float toothRT_z = toothdown_Rt_pos2.x - toothdown_Rt_pos2.x;

	SetLocalPositionOffset(pmesh,"face_tooth_down_joint1",Vector3f(0, toothpos_y, 0));
	SetLocalPositionOffset(pmesh,"face_tooth_up_joint3",Vector3f(-toothpos_y, 0, 0));
	SetLocalPositionOffset(pmesh,"face_tooth_down_joint3",Vector3f(toothLf_x, toothpos_y, 0));
	SetLocalPositionOffset(pmesh,"face_tooth_up_joint1",Vector3f(-toothpos_y, 0, toothLf_x));
	SetLocalPositionOffset(pmesh,"face_tooth_down_joint2",Vector3f(toothLf_x, toothpos_y, 0));
	SetLocalPositionOffset(pmesh,"face_tooth_up_joint2",Vector3f(-toothpos_y, 0, toothLf_x));






	////////////////////////////////////////








	//修正鼻子

	//Vector3 fixnose_lf2 = _bones["nosewing_Lf_joint1"].localPosition;
	//Vector3 fixnose_RT2 = _bones["nosewing_Rt_joint1"].localPosition;

	//float fixnosedis_LF = (fixnose_lf2.x - fixnose_lf1.x) / 4;
	//float fixnosedis_RT = (fixnose_RT2.x - fixnose_RT1.x) / 4;

	//_bones["nosewing_Lf_joint1"].localPosition += Vector3f(-fixnosedis_LF, 0, 0);
	//_bones["nosewing_Rt_joint1"].localPosition += Vector3f(-fixnosedis_RT, 0, 0);

	Vector3f forhead_RT_pos2 = forehead_RT.ExtractTranslation();
	Vector3f forhead_LF_pos2 = forehead_LF.ExtractTranslation();

	Vector3f forhead_RT_offset = forhead_RT_pos2 - forhead_RT_pos1;
	Vector3f forhead_LF_offset = forhead_LF_pos2 - forhead_LF_pos1;
	///////////////////////////////////////////





	SetLocalPositionOffset(pmesh, "face_temple_Lf_joint2", forhead_LF_offset * 0.5);
	SetLocalPositionOffset(pmesh, "face_temple_Lf_joint3", forhead_LF_offset * 0.5);
	SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint1", forhead_LF_offset * 0.5);
	SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint4", forhead_LF_offset * 0.5);
	SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint2", forhead_LF_offset * 0.5);


	SetLocalPositionOffset(pmesh, "face_temple_Rt_joint2", forhead_RT_offset * -0.5);
	SetLocalPositionOffset(pmesh, "face_temple_Rt_joint3", forhead_RT_offset * -0.5);
	SetLocalPositionOffset(pmesh, "face_forehead_Rt_joint1", forhead_RT_offset * -0.5);
	SetLocalPositionOffset(pmesh, "face_forehead_Rt_joint2", forhead_RT_offset * -0.5);
	SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint3", forhead_RT_offset * -0.5);












	float face_calvaria_joint1updown = forhead_RT_pos2.z - forhead_RT_pos1.z;


	if (face_calvaria_joint1updown < 0)
	{

		SetLocalPositionOffset(pmesh, "face_calvaria_joint1",Vector3f(face_calvaria_joint1updown, 0, 0));
		SetLocalPositionOffset(pmesh, "face_temple_Lf_joint3",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_temple_Rt_joint3",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_forehead_joint1",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint3",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint2",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint6",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_forehead_joint2",Vector3f(0, face_calvaria_joint1updown, 0));
		SetLocalPositionOffset(pmesh, "face_forehead_Lf_joint5",Vector3f(0, face_calvaria_joint1updown, 0));
	}




	fixfacedistans_x(pmesh,"face_temple_Lf_joint1" , "face_temple_Rt_joint1", 0);
	fixfacedistans_x(pmesh, "face_temple_Lf_joint2" , "face_temple_Rt_joint2", 0);
	fixfacedistans_y(pmesh, "face_temple_Lf_joint1" , "face_temple_Lf_joint1");
	fixfacedistans_y(pmesh, "face_temple_Lf_joint2" , "face_temple_Rt_joint2");
	fixfacedistans_z(pmesh, "face_temple_Lf_joint1" , "face_temple_Lf_joint1", 0);
	fixfacedistans_z(pmesh, "face_temple_Lf_joint2" , "face_temple_Rt_joint2", 0);

	fixfacedistans_x(pmesh, "face_forehead_Lf_joint1" , "face_forehead_Rt_joint1", 0);
	fixfacedistans_x(pmesh, "face_forehead_Lf_joint2" , "face_forehead_Lf_joint3", 0);
	fixfacedistans_x(pmesh, "face_forehead_Lf_joint4" , "face_forehead_Rt_joint2", 0);
	fixfacedistans_x(pmesh, "face_forehead_Lf_joint5" , "face_forehead_Lf_joint6", 0);

	fixfacedistans_y(pmesh, "face_forehead_Lf_joint1" , "face_forehead_Rt_joint1");
	fixfacedistans_y(pmesh, "face_forehead_Lf_joint2" , "face_forehead_Lf_joint3");
	fixfacedistans_y(pmesh, "face_forehead_Lf_joint4" , "face_forehead_Rt_joint2");
	fixfacedistans_y(pmesh, "face_forehead_Lf_joint5" , "face_forehead_Lf_joint6");

	fixfacedistans_z(pmesh, "face_forehead_Lf_joint1" , "face_forehead_Rt_joint1", 0);
	fixfacedistans_z(pmesh, "face_forehead_Lf_joint2" , "face_forehead_Lf_joint3", 0);
	fixfacedistans_z(pmesh, "face_forehead_Lf_joint4" , "face_forehead_Rt_joint2", 0);
	fixfacedistans_z(pmesh, "face_forehead_Lf_joint5" , "face_forehead_Lf_joint6", 0);



	////面部修正


	fixfacedistans_half_x(pmesh, "face_chin_joint2" , "face_chin_joint3");
	fixfacedistans_half_x(pmesh, "face_chin_Lf_joint02" , "face_chin_Rt_joint02");
	fixfacedistans_half_x(pmesh, "face_chin_Lf_joint03" , "face_chin_Rt_joint03");
	fixfacedistans_half_x(pmesh, "face_chin_Lf_joint04" , "face_chin_Rt_joint04");



	fixfacedistans_half_z(pmesh, "face_chin_Lf_joint05" , "face_chin_Rt_joint05", 3);
	fixfacedistans_half_z(pmesh, "face_chin_Lf_joint06" , "face_chin_Rt_joint06", 2);
	fixfacedistans_half_z(pmesh, "face_chin_Lf_joint07" , "face_chin_Rt_joint07", 1);

	fixfacedistans_z(pmesh, "face_chin_Lf_joint08" , "face_chin_Rt_joint08", 0);
	fixfacedistans_z(pmesh, "face_chin_Lf_joint09" , "face_chin_Rt_joint09", 0);

	fixfacedistans_z(pmesh, "face_temple_Lf_joint1" , "face_temple_Rt_joint1", 0);
	fixfacedistans_z(pmesh, "face_temple_Lf_joint3" , "face_temple_Rt_joint3", 0);






	return;


}
void BoneUtility::MoveUV(SkinnedMesh* pmesh, JsonRole bonedef)
{

	uint NumVertices = 0;
	int len = 0;
	int startpos;
	int meshid = 0;
	for (uint i = 0; i < pmesh->m_Entries.size(); i++) {

		if (pmesh->m_Entries[i].NumBones > 80)
		{
			meshid = i;
			len = pmesh->m_Entries[i].BaseVertex;
			startpos = NumVertices;
		}


		NumVertices += pmesh->m_pScene->mMeshes[i]->mNumVertices;
	}

	vector<Vector2f> uvs2;
	uvs2.reserve(len);
	for (int i = 0 ; i< len;i++)
	{
		float uvsize = bonedef.uvsize;
		float offset_y = bonedef.offset_y / 100;

		Vector3f vertex = pmesh->TotalPositions[startpos + i];

		uint VertexID = startpos + i;
		Matrix4f BoneTransform;
		BoneTransform.SetZero();
		SkinnedMesh::VertexBoneData vertexboneinfo = pmesh->TotalBones[VertexID];
		for (int i = 0; i < sizeof(*(vertexboneinfo.IDs)); i++ )
		{
			int boneid = vertexboneinfo.IDs[i];
			string bonename = pmesh->m_BoneInfo[boneid].Name;

			if (pmesh->m_BoneGlobalTrasMap.find(bonename) != pmesh->m_BoneGlobalTrasMap.end())
			{
				Matrix4f globaltran = pmesh->m_BoneGlobalTrasMap[bonename];
				Matrix4f offset = pmesh->m_BoneInfo[boneid].BoneOffset;
				//BoneTransform = BoneTransform + globaltran * offset * vertexboneinfo.Weights[i];

			}
		}

		Vector4f vertexafter = BoneTransform * Vector4f( vertex.x, vertex.y, vertex.z,1.0);

		Vector2f uv(vertex.x / uvsize + 0.5f, -vertex.y / uvsize - offset_y);
		uvs2.push_back(uv);
	}

	vector<Vector2f> newuv2 = pmesh->TotalTexCoords2;

	for (int i = 0; i < len; i++)
	{
		newuv2[startpos + i] = uvs2[i];
	}

	//pmesh->RefreshUV2(newuv2);
}


void BoneUtility::ResetBone()
{

}
//�沿��������
void BoneUtility::MoveBonePYR(SkinnedMesh* pmesh,string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRole bonedef, string boneoffsetname, Vector3f headCenter,float offsetrate)
{
	SkinnedMesh::BoneInfo boneinfo = pmesh->GetBoneInfo(bonename);



	Matrix4f finaltransold = boneinfo.BoneOffset;
	Vector3f trspos, trsscl;
	Matrix4f trsrot;
	finaltransold.MatrixDecompose(trspos, trsscl, trsrot);

	float zepos = bonedef.face_zero_pointy;

	// Vector3f  planpos = _faceplan.Matrix4f.position; 
	//�ض������

	Vector3f planpos = headCenter;


	float sizeuv = bonedef.uvsize;// 30.0f;
	float scale_1024_to_model = sizeuv / 1024;
	float zero_px = planpos.x - sizeuv / 2;
	float zero_py = planpos.y - sizeuv / 2;


	Vector3f zero(zero_px, zero_py, trspos.z);
	//Debug.OSMesa::Log(pmesh->GetBoneInfo("chin_joint1"].position);

	Vector3f OriPosition = trspos;

	float yaw_angle = (float)(faceinfo.yaw_angle);          //ҡͷ
	float roll_angle = (float)(faceinfo.roll_angle);        //ƽ����ת
	float pitch_angle = (float)(faceinfo.pitch_angle);   //̧ͷ


	float x0 = OriPosition.x;
	float y0 = OriPosition.y;
	float z0 = OriPosition.z;

	float sinx = sin(yaw_angle); float cosx = cos(yaw_angle);
	float siny = sin(pitch_angle); float cosy = cos(pitch_angle);
	float sinz = sin(roll_angle); float cosz = cos(roll_angle);


	float x1 = x0 * (cosy * cosz - sinx * siny * sinz) - y0 * cosx * sinz + z0 * (siny * cosz + sinx * cosy * sinz);
	float y1 = x0 * (cosy * sinz + sinx * siny * cosz) + y0 * cosx * cosz + z0 * (siny * sinz - sinx * cosy * cosz);
	float z1 = x0 * (-cosx * siny) + y0 * sinx + z0 * cosx * cosy;

	Vector3f RotatOriglepostion(x1, y1, z1);


	//  Debug.LogError(OriPosition +"  "+ RotatOriglepostion);


	Vector2f face2dpos = faceinfo.landmarkdata[facekeypoint];
	Vector3f Newpositon = zero + Vector3f(face2dpos.x * scale_1024_to_model, face2dpos.y * scale_1024_to_model, 0);

	//   Debug.LogError(Newpositon);

	//float xdis = Newpositon.x - RotatOriglepostion.x;
	//float ydis = Newpositon.y - RotatOriglepostion.y;
	//float zdis = Newpositon.z - RotatOriglepostion.z;


	float xdis = Newpositon.x - OriPosition.x;
	float ydis = Newpositon.y - OriPosition.y;
	float zdis = Newpositon.z - OriPosition.z;



	float sinx2 = sin(-yaw_angle); float cosx2 = cos(-yaw_angle);
	float siny2 = sin(-pitch_angle); float cosy2 = cos(-pitch_angle);
	float sinz2 = sin(-roll_angle); float cosz2 =cos(-roll_angle);


	float xf = xdis * (cosy2 * cosz2 - sinx2 * siny2 * sinz2) - ydis * cosx2 * sinz2 + zdis * (siny2 * cosz2 + sinx2 * cosy2 * sinz2);
	float yf = xdis * (cosy2 * sinz2 + sinx2 * siny2 * cosz2) + ydis * cosx2 * cosz2 + zdis * (siny2 * sinz2 - sinx2 * cosy2 * cosz2);
	float zf = xdis * (-cosx2 * siny2) + ydis * sinx2 + zdis * cosx2 * cosy2;



	Vector3f faceOffset(xf, yf, zf);

	
	Vector3f finalpos = zero + Vector3f(face2dpos.x * scale_1024_to_model, face2dpos.y * scale_1024_to_model,0);


	
}


Matrix4f BoneUtility::GetLocalMatrixFromGlobal(SkinnedMesh* pmesh,string bonename, Matrix4f globalmat)
{
	Matrix4f parentMat = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]->mParent);

	Matrix4f parentinv = parentMat;
	parentinv.Inverse();

	Matrix4f x = parentMat * globalmat;
	return x;

}





void BoneUtility::eyemapscale(float scale) {

	/*Vector2f oldoffest = _eyes.GetComponent<Renderer>().material.GetTextureOffset("_Diffusemap");
	Vector2f oldscale = _eyes.GetComponent<Renderer>().material.GetTextureScale("_Diffusemap");

	Vector2f newscale = oldscale / scale;
	Vector2f newoffset = oldoffest + Vector2f((1 - 1 / scale) / 2, (1 - 1 / scale) / 2);
	_eyes.GetComponent<Renderer>().material.SetTextureScale("_Diffusemap", newscale);
	_eyes.GetComponent<Renderer>().material.SetTextureOffset("_Diffusemap", newoffset);*/


}
void BoneUtility::fixfacedistans_half_x(SkinnedMesh* pmesh,string bons1, string bons2)
{
	Vector3f local1 = GetLocalPosition(pmesh, bons1);
	Vector3f local2 = GetLocalPosition(pmesh, bons2);


	float posx = (abs(local1.x) + abs(local2.x)) / 2;

	float posxfix = abs(abs(local1.x) - abs(local2.x)) / 2;




	if (abs(local1.x) > abs(local2.x))
	{

		if (local2.x < 0)
		{
			SetLocalPosition(pmesh,bons2, Vector3f(local2.x - posxfix, local2.y, local2.z));
		}
		else
		{
			SetLocalPosition(pmesh, bons2, Vector3f(local2.x + posxfix, local2.y, local2.z));
		}

	}
	else {


		if (local1.x < 0)
		{
			SetLocalPosition(pmesh, bons1 , Vector3f(local1.x - posxfix, local1.y, local1.z));
		}
		else
		{
			SetLocalPosition(pmesh, bons1, Vector3f(local1.x + posxfix, local1.y, local1.z));
		}


	}



}




void BoneUtility::fixfacedistans_half_z(SkinnedMesh* pmesh, string bons1, string bons2, float  div)
{

	Vector3f local1 = GetLocalPosition(pmesh, bons1);
	Vector3f local2 = GetLocalPosition(pmesh, bons2);

	float posx = (abs(local1.z) + abs(local2.z)) / 2;

	float posxfix = abs(abs(local1.z) - abs(local2.z)) / 2;

	posxfix = posxfix / div;


	if (abs(local1.z) > abs(local2.z))
	{

		if (local2.z < 0)
		{
			local2 = Vector3f(local2.x, local2.y, local2.z - posxfix);
		}
		else
		{
			local2 = Vector3f(local2.x, local2.y, local2.z + posxfix);
		}

	}
	else
	{


		if (local1.x < 0)
		{
			local1 = Vector3f(local1.x, local1.y, local1.z - posxfix);
		}
		else
		{
			local1 = Vector3f(local1.x, local1.y, local1.z + posxfix);
		}


	}



}









void BoneUtility::fixfacedistans_y(SkinnedMesh* pmesh, string bons1, string bons2)
{

	Vector3f local1 = GetLocalPosition(pmesh, bons1);
	Vector3f local2 = GetLocalPosition(pmesh, bons2);
	if (local1.y > local2.y)
	{
		local1 = Vector3f(local1.x, local2.y, local1.z);
	}

	else {
		local2 = Vector3f(local2.x, local1.y, local2.z);
	}
}


void BoneUtility::fixfacedistans_x(SkinnedMesh* pmesh, string bons1, string bons2, float add) {


	Vector3f local1 = GetLocalPosition(pmesh, bons1);
	Vector3f local2 = GetLocalPosition(pmesh, bons2);
	float posx = (abs(local1.x) + abs(local2.x)) / 2 + add;

	if (local1.x < 0)
	{
		local1 = Vector3f(-posx, local1.y, local1.z);
	}
	else {
		local1 = Vector3f(posx, local1.y, local1.z);
	}

	if (local2.x < 0)
	{
		local2 = Vector3f(-posx, local2.y, local2.z);
	}
	else {
		local2 = Vector3f(posx, local2.y, local2.z);
	}


	/*
	if (abs(local1.x) > abs(local2.x))
	{
	local2 = Vector3f(-local1.x, local2.y, local2.z); }

	else {
	local1 = Vector3f(-local2.x, local1.y, local1.z);
	}
	*/

}
void BoneUtility::fixfacedistans_z(SkinnedMesh* pmesh, string bons1, string bons2, float add)
{

	Vector3f local1 = GetLocalPosition(pmesh, bons1);
	Vector3f local2 = GetLocalPosition(pmesh, bons2);

	float posz = (abs(local1.z) + abs(local2.z)) / 2 + add;

	if (local2.z < 0)
	{
		local2 = Vector3f(local2.x, local2.y, -posz);

	}
	else
	{
		local2 = Vector3f(local2.x, local2.y, posz);
	}



	if (local1.z < 0)
	{
		local1 = Vector3f(local1.x, local1.y, -posz);

	}
	else
	{
		local1 = Vector3f(local1.x, local1.y, posz);
	}


}
Matrix4f BoneUtility::GetLocalMatrix(Matrix4f totalTrs, Matrix4f parentTrs)
{

	Matrix4f parentinv = parentTrs;
	parentinv.Inverse();
	Matrix4f curlocal = parentinv * totalTrs;
	return curlocal;
}
Vector3f BoneUtility::GetLocalPosition(Matrix4f totalTrs, Matrix4f parentTrs)
{
	Matrix4f localmat = GetLocalMatrix(totalTrs, parentTrs);
	Vector3f localpos = localmat.ExtractTranslation();
	return localpos;
}
Vector3f BoneUtility::GetLocalPosition(SkinnedMesh* pmesh, string boneName)
{
	if (pmesh->m_BoneNodeMap.find(boneName) != pmesh->m_BoneNodeMap.end())
	{
		Matrix4f curparent = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[boneName]->mParent);

		Matrix4f total;
		if (pmesh->m_BoneGlobalTrasMap.find(boneName) != pmesh->m_BoneGlobalTrasMap.end())
		{
			total = pmesh->m_BoneGlobalTrasMap[boneName];
		}
		else
		{
			total = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[boneName]);
		}
		Vector3f localpos = GetLocalPosition(total, curparent);

		return localpos;
	}
	return Vector3f(0, 0, 0);
}
void BoneUtility::SetLocalPositionOffset(SkinnedMesh* pmesh, string boneName, Vector3f offset,float rate)
{
	Vector3f localpos = GetLocalPosition(pmesh, boneName);
	Vector3f rtoffset(-offset.x, offset.y, offset.z);
	SetLocalPosition(pmesh, boneName, localpos + rtoffset * rate);
}
void BoneUtility::SetLocalPosition(SkinnedMesh* pmesh, string boneName,Vector3f localpos)
{

	Matrix4f curparent = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[boneName]->mParent);

	Matrix4f total;
	if (pmesh->m_BoneGlobalTrasMap.find(boneName) != pmesh->m_BoneGlobalTrasMap.end())
	{
		total = pmesh->m_BoneGlobalTrasMap[boneName];
	}
	else
	{
		total = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[boneName]);
	}


	Matrix4f curlocalMat = GetLocalMatrix(total, curparent);

	Vector3f cpos, cscale;
	Matrix4f crot;
	curlocalMat.MatrixDecompose(cpos, cscale, crot);

	Pipeline cp;
	cp.Scale(cscale);
	Matrix4f crs = crot * cp.GetWorldTrans();
	Matrix4f crsinv = crs;
	crsinv.Inverse();


	Matrix4f tx = curlocalMat * crsinv;
	tx.m[0][3] = localpos.x;
	tx.m[1][3] = localpos.y;
	tx.m[2][3] = localpos.z;

	Matrix4f rt = curparent * tx *crs;
	pmesh->m_BoneGlobalTrasMap[boneName] = rt;

}
//�沿��������
void BoneUtility::MoveBone(SkinnedMesh* pmesh, string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRole bonedef, string boneoffsetname, Vector3f headCenter, float offsetrate)
{
	Matrix4f transformtochange;
	Matrix4f currentlocalMat;

	transformtochange = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]);

	aiMatrix4x4 aicurrentMat = pmesh->m_BoneNodeMap[bonename]->mTransformation;
	currentlocalMat = Matrix4f(aicurrentMat);

	Vector3f trspos, trsscl;
	Matrix4f trsrot;
	transformtochange.MatrixDecompose(trspos, trsscl, trsrot);





	float zepos = bonedef.face_zero_pointy;

	float sizeuv = bonedef.uvsize;
	float scale_1024_to_model = sizeuv / 1024;

	float zero_px = headCenter.x + sizeuv / 2;//open �������� unity �������� 
	float zero_py = headCenter.y - sizeuv / 2;

	Vector3f zero(zero_px, zero_py, trspos.z);
	trspos = zero + Vector3f(-faceinfo.landmarkdata[facekeypoint].x, faceinfo.landmarkdata[facekeypoint].y, 0) *scale_1024_to_model;


	//Vector3f offset = bonedef.offsets_map[boneoffsetname] * offsetrate;
	//trspos += offset;

	Matrix4f parentMat = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]->mParent);


	Matrix4f parentinv = parentMat;
	parentinv.Inverse();

	Matrix4f prelocal = parentinv * transformtochange;

	Pipeline ps;
	ps.Scale(trsscl);


	Pipeline pt;
	pt.WorldPos(trspos);
	Matrix4f mt = pt.GetWorldTrans();


	Matrix4f mat = mt  * trsrot * ps.GetWorldTrans();


	/*uint index = pmesh->m_BoneMapping[bonename];
	pmesh->m_BoneInfo[index].BoneOffset = mat;*/


	Matrix4f identity;
	identity.InitIdentity();

	/*uint index = pmesh->m_BoneMapping[bonename];
	pmesh->m_BoneInfo[index].BoneOffset = identity;*/


	Vector3f cpos, cscale;
	Matrix4f crot;
	currentlocalMat.MatrixDecompose(cpos, cscale, crot);

	Pipeline cp;
	cp.Scale(cscale);
	Matrix4f crs = crot * cp.GetWorldTrans();
	Matrix4f crsinv = crs;
	crsinv.Inverse();



	Matrix4f x = parentinv * mat;
	Matrix4f tx = x * crsinv;



	Vector3f offset = bonedef.offsets_map[boneoffsetname];
	tx.m[0][3] += -offset.x;
	tx.m[1][3] += offset.y;
	tx.m[2][3] += offset.z;

	/*Matrix4f txoffset;
	txoffset.InitTranslationTransform(offset.x,-offset.y,-offset.z);*/
	Vector3f localoffset = tx.ExtractTranslation();
	Matrix4f final = tx * crs;



	/*char str[255];
	vector<string> keys;
	vector<string> values;
	keys.push_back("name");
	values.push_back(bonename);
	keys.push_back("afteroffsetlocal");
	sprintf(str, "afteroffsetlocal: x : %f y: %f z: %f", localoffset.x, localoffset.y, localoffset.z);
	values.push_back(string(str));
	m_JsonDB.AddNode(keys, values);*/



	Vector3f aftloct = final.ExtractTranslation();
	if (pmesh->m_BoneNodeMap.find(bonename) != pmesh->m_BoneNodeMap.end())
	{
		//pmesh->m_BoneNodeMap[bonename]->mTransformation = final.GetaiMatrix4x4();
		
		Matrix4f totalfinal = parentMat * final;
		pmesh->m_BoneGlobalTrasMap[bonename] = totalfinal;
		Vector3f diff = totalfinal.ExtractTranslation() - transformtochange.ExtractTranslation();

		Matrix4f curparent = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]->mParent);
		Vector3f localdiff = GetLocalPosition(totalfinal, curparent) - GetLocalPosition(transformtochange, curparent);

		jsonModelFormat.nodemap[bonename].localpos = localdiff;
		jsonModelFormat.nodemap[bonename].pos = diff;
		jsonModelFormat.nodemap[bonename].worldmatrix =totalfinal;
		jsonModelFormat.nodemap[bonename].localmatrix = curparent.Inverse() * totalfinal;
		
	}
	else
	{
		printf("");
	}

	//if (pmesh->m_BoneMapping.find(bonename) != pmesh->m_BoneMapping.end())
	//{
	//	uint index = pmesh->m_BoneMapping[bonename];
	//	Matrix4f matinv = mat;
	//	matinv.Inverse();
	//	//pmesh->m_BoneInfo[index].BoneOffset = matinv;


	//	Matrix4f globalm = Matrix4f(pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]));

	//	Matrix4f test = globalm * pmesh->m_BoneInfo[index].BoneOffset;
	//	printf("");
	//}
	//else
	//{
	//	printf("");
	//}

	return;

	//int i = 0;

	//if (boneinfo.pMeshVec.size() > 0)
	//{
	//	for (vector<aiMesh*>::iterator it = boneinfo.pMeshVec.begin(); it < boneinfo.pMeshVec.end(); it++)
	//	{
	//		if (*it != NULL)
	//		{
	//			//(*it)->mBones[boneinfo.BoneIndexVec[i]]->mOffsetMatrix = boneinfo.BoneOffset.GetaiMatrix4x4();

	//			/*Matrix4f identity;
	//			identity.InitIdentity();
	//			(*it)->mBones[boneinfo.BoneIndexVec[i]]->mOffsetMatrix = identity.GetaiMatrix4x4();
	//			boneinfo.BoneOffset = identity;*/


	//			//pmesh->m_BoneInfo[pmesh->m_BoneMapping[bonename]].NodeTransformation = mat;
	//			//pmesh->m_NodeMap[bonename]->mTransformation = mat.GetaiMatrix4x4();



	//			//pmesh->m_BoneInfo[pmesh->m_BoneMapping[bonename]] = boneinfo;
	//		}
	//		i++;

	//	}

	//	boneinfo.BoneOffset = mat;// .Inverse();
	//	pmesh->m_BoneInfo[pmesh->m_BoneMapping[bonename]] = boneinfo;
	//}
	//else
	//{
	//	Matrix4f identity;
	//	identity.InitIdentity();
	//	boneinfo.BoneOffset = mat;


	//	//printf(bonename.c_str());
	//}




	//boneinfo.FinalTransformation = boneinfo.GlobalInverseTransform * boneinfo.Parentformation * boneinfo.NodeTransformation * boneinfo.BoneOffset;

}
