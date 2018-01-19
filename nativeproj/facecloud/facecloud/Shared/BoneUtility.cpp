#include "BoneUtility.h"
#include <fstream>
#include <json/json.h>

#include <json/json.h>
#include <fstream>

void JsonModelFormat::LoadFromFile(string filename)
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



	Json::Value nodes = root["hierarchy"]["nodes"];
	int nodescount = nodes.size();

}
JsonModelFormat::~JsonModelFormat()
{
}




void BoneUtility::Init()
{


}

int BoneUtility::ReadJsonFromFile(const char* filename)
{

	Json::Value root; // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array         


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


void BoneUtility::CalculateFaceBone(SkinnedMesh* pmesh, JsonRoleBone bonedef, JsonFaceInfo faceinfo)
{
	//for (uint i = 0; i < pMesh->mNumBones; i++) {
	//	uint BoneIndex = 0;
	//	string BoneName(pMesh->mBones[i]->mName.data);

	//	if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
	//		// Allocate an index for a new bone
	//		BoneIndex = m_NumBones;
	//		m_NumBones++;
	//		BoneInfo bi;
	//		m_BoneInfo.push_back(bi);
	//		m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;
	//		m_BoneMapping[BoneName] = BoneIndex;
	//	}
	//	else {
	//		BoneIndex = m_BoneMapping[BoneName];
	//	}

	//	for (uint j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
	//		uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
	//		float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
	//		Bones[VertexID].AddBoneData(BoneIndex, Weight);
	//	}
	//}

	Matrix4f tooth_MID = pmesh->GetBoneOffsetMatrix("mouthLip_up_joint2");     
	Matrix4f toothup_Lf = pmesh->GetBoneOffsetMatrix("mouthLip_Lf_joint1");
	Matrix4f toothdown_Rt = pmesh->GetBoneOffsetMatrix("mouthLip_Rt_joint1");

	Vector3f tooth_MID_pos1 = tooth_MID.ExtractTranslation();
	Vector3f toothup_Lf_pos1 = toothup_Lf.ExtractTranslation();
	Vector3f toothdown_Rt_pos1 = toothdown_Rt.ExtractTranslation();

	//额头偏移

	Matrix4f forehead_RT = pmesh->GetBoneOffsetMatrix("chin_Rt_joint6");
	Matrix4f forehead_LF = pmesh->GetBoneOffsetMatrix("chin_Rt_joint6");


	Vector3f forhead_RT_pos1 = forehead_RT.ExtractTranslation();
	Vector3f forhead_LF_pos1 = forehead_LF.ExtractTranslation();

	//眉毛修正

	Matrix4f brow_LF02 = pmesh->GetBoneOffsetMatrix("brow_Lf_joint2");
	Matrix4f brow_LF03 = pmesh->GetBoneOffsetMatrix("brow_Lf_joint3");

	Vector3f brow_LF02_pos1 = brow_LF02.ExtractTranslation();
	Vector3f brow_LF03_pos1 = brow_LF03.ExtractTranslation();

	//鼻子修正
	Matrix4f nose_tr = pmesh->GetBoneOffsetMatrix("nose_joint2");
	Vector3f nose_tr_pos1 = nose_tr.ExtractTranslation();


	//嘴巴修正

	Matrix4f mouth_tr = pmesh->GetBoneOffsetMatrix("mouthLip_up_joint0");
	Vector3f mouth_tr_pos1 = mouth_tr.ExtractTranslation();


	//眼睛修正
	float righteye_conner_pos1 = pmesh->GetBoneOffsetMatrix("eyeLids_Rt_joint1").ExtractTranslation().x;
	float righteye_conner_pos2 = pmesh->GetBoneOffsetMatrix("eyeLids_Rt_joint2").ExtractTranslation().x;
	float Lefteye_conner_pos1 = pmesh->GetBoneOffsetMatrix("eyeLids_Lf_joint1").ExtractTranslation().x;
	float Lefteye_conner_pos2 = pmesh->GetBoneOffsetMatrix("eyeLids_Lf_joint2").ExtractTranslation().x;

	float Eye_Rt_dis = righteye_conner_pos1 - righteye_conner_pos2;
	float Eye_LF_dis = Lefteye_conner_pos1 - Lefteye_conner_pos2;


	Vector3f fixmouthlipdistance01 = pmesh->GetBoneOffsetMatrix("mouthLip_Lf_joint5").ExtractTranslation() - pmesh->GetBoneOffsetMatrix("mouthLip_Lf_joint7").ExtractTranslation();
	Vector3f fixmouthlipdistance02 = pmesh->GetBoneOffsetMatrix("mouthLip_Rt_joint5").ExtractTranslation() - pmesh->GetBoneOffsetMatrix("mouthLip_Rt_joint7").ExtractTranslation();
	Vector3f fixmouthlipdistance03 = pmesh->GetBoneOffsetMatrix("mouthLip_dn_joint2").ExtractTranslation() - pmesh->GetBoneOffsetMatrix("mouthLip_up_joint2").ExtractTranslation();


	//  Debug.Log("Eye_Rt_dis" + Eye_Rt_dis+ "Eye_LF_dis"+ Eye_LF_dis);

	Vector3f fixnose_lf1 = pmesh->GetBoneOffsetMatrix("nosewing_Lf_joint1").ExtractTranslation();
	Vector3f fixnose_RT1 = pmesh->GetBoneOffsetMatrix("nosewing_Rt_joint1").ExtractTranslation();




	Vector3f leftpos = pmesh->GetBoneOffsetMatrix("eyeLids_Lf_joint1").ExtractTranslation();
	Vector3f rightpos = pmesh->GetBoneOffsetMatrix("eyeLids_Rt_joint1").ExtractTranslation();
	Vector3f headCenter = (leftpos + rightpos) * 0.5f;


	// move bones // 
	MoveBone(pmesh, "chin_joint1", faceinfo, "contour_chin", bonedef, "offset_chin_joint1", headCenter);
	MoveBone(pmesh, "chin_joint2", faceinfo, "contour_left9", bonedef, "offset_chin_joint2", headCenter);
	MoveBone(pmesh, "chin_Lf_joint4", faceinfo, "contour_left7", bonedef, "offset_chin_Lf_joint4", headCenter);
	MoveBone(pmesh, "chin_Lf_joint3", faceinfo, "contour_left6", bonedef, "offset_chin_Lf_joint3", headCenter);
	MoveBone(pmesh, "chin_Lf_joint1", faceinfo, "contour_left5", bonedef, "offset_chin_Lf_joint1", headCenter);
	MoveBone(pmesh, "chin_Lf_joint2", faceinfo, "contour_left4", bonedef, "offset_chin_Lf_joint2", headCenter);
	MoveBone(pmesh, "chin_Lf_joint5", faceinfo, "contour_left2", bonedef, "offset_chin_Lf_joint5", headCenter);
	MoveBone(pmesh, "chin_Lf_joint6", faceinfo, "contour_left1", bonedef, "offset_chin_Lf_joint6", headCenter);
	MoveBone(pmesh, "chin_joint3", faceinfo, "contour_right9", bonedef, "offset_chin_joint3", headCenter);
	MoveBone(pmesh, "chin_Rt_joint4", faceinfo, "contour_right7", bonedef, "offset_chin_Rt_joint4", headCenter);

	MoveBone(pmesh, "chin_Rt_joint3", faceinfo, "contour_right6", bonedef, "offset_chin_Rt_joint3", headCenter);
	MoveBone(pmesh, "chin_Rt_joint1", faceinfo, "contour_right5", bonedef, "offset_chin_Rt_joint1", headCenter);
	MoveBone(pmesh, "chin_Rt_joint2", faceinfo, "contour_right4", bonedef, "offset_chin_Rt_joint2", headCenter);
	MoveBone(pmesh, "chin_Rt_joint5", faceinfo, "contour_right2", bonedef, "offset_chin_Rt_joint5", headCenter);
	MoveBone(pmesh, "chin_Rt_joint6", faceinfo, "contour_right1", bonedef, "offset_chin_Rt_joint6", headCenter);
	MoveBone(pmesh, "eyeBall_Lf_joint1", faceinfo, "left_eye_center", bonedef, "offset_eyeBall_Lf_joint1", headCenter);
	MoveBone(pmesh, "eyeBall_Rt_joint1", faceinfo, "right_eye_center", bonedef, "offset_eyeBall_Rt_joint1", headCenter);
	MoveBone(pmesh, "eyeLidsdown_Lf_joint2", faceinfo, "left_eye_bottom", bonedef, "offset_eyeLidsdown_Lf_joint2", headCenter);
	MoveBone(pmesh, "eyeLids_Lf_joint2", faceinfo, "left_eye_left_corner", bonedef, "offset_eyeLids_Lf_joint2", headCenter);
	MoveBone(pmesh, "eyeLidsdown_Lf_joint3", faceinfo, "left_eye_lower_left_quarter", bonedef, "offset_eyeLidsdown_Lf_joint3", headCenter);

	MoveBone(pmesh, "eyeLidsdown_Lf_joint1", faceinfo, "left_eye_lower_right_quarter", bonedef, "offset_eyeLidsdown_Lf_joint1", headCenter);
	MoveBone(pmesh, "eyeLids_Lf_joint1", faceinfo, "left_eye_right_corner", bonedef, "offset_eyeLids_Lf_joint1", headCenter);
	MoveBone(pmesh, "eyeLidsUp_Lf_joint2", faceinfo, "left_eye_top", bonedef, "offset_eyeLidsUp_Lf_joint2", headCenter);
	MoveBone(pmesh, "eyeLidsUp_Lf_joint3", faceinfo, "left_eye_upper_left_quarter", bonedef, "offset_eyeLidsUp_Lf_joint3", headCenter);
	MoveBone(pmesh, "eyeLidsUp_Lf_joint1", faceinfo, "left_eye_upper_right_quarter", bonedef, "offset_eyeLidsUp_Lf_joint1", headCenter);
	MoveBone(pmesh, "eyeLidsdown_Rt_joint2", faceinfo, "right_eye_bottom", bonedef, "offset_eyeLidsdown_Rt_joint2", headCenter);
	MoveBone(pmesh, "eyeLidsdown_Rt_joint1", faceinfo, "right_eye_left_corner", bonedef, "offset_eyeLids_Rt_joint1", headCenter);
	MoveBone(pmesh, "eyeLidsdown_Rt_joint1", faceinfo, "right_eye_lower_left_quarter", bonedef, "offset_eyeLidsdown_Rt_joint1", headCenter);
	MoveBone(pmesh, "eyeLidsdown_Rt_joint3", faceinfo, "right_eye_lower_right_quarter", bonedef, "offset_eyeLidsdown_Rt_joint3", headCenter);
	MoveBone(pmesh, "eyeLids_Rt_joint2", faceinfo, "right_eye_right_corner", bonedef, "offset_eyeLids_Rt_joint2", headCenter);

	MoveBone(pmesh, "eyeLidsUp_Rt_joint2", faceinfo, "right_eye_top", bonedef, "offset_eyeLidsUp_Rt_joint2", headCenter);
	MoveBone(pmesh, "eyeLidsUp_Rt_joint1", faceinfo, "right_eye_upper_left_quarter", bonedef, "offset_eyeLidsUp_Rt_joint1", headCenter);
	MoveBone(pmesh, "eyeLidsUp_Rt_joint3", faceinfo, "right_eye_upper_right_quarter", bonedef, "offset_eyeLidsUp_Rt_joint3", headCenter);
	MoveBone(pmesh, "brow_Lf_joint3", faceinfo, "left_eyebrow_left_corner", bonedef, "offset_brow_Lf_joint3", headCenter);
	MoveBone(pmesh, "brow_Lf_joint2", faceinfo, "left_eyebrow_lower_middle", bonedef, "offset_brow_Lf_joint2", headCenter);
	MoveBone(pmesh, "brow_Lf_joint1", faceinfo, "left_eyebrow_right_corner", bonedef, "offset_brow_Lf_joint1", headCenter);
	MoveBone(pmesh, "brow_Rt_joint1", faceinfo, "right_eyebrow_left_corner", bonedef, "offset_brow_Rt_joint1", headCenter);
	MoveBone(pmesh, "brow_Rt_joint2", faceinfo, "right_eyebrow_lower_middle", bonedef, "offset_brow_Rt_joint2", headCenter);
	MoveBone(pmesh, "brow_Rt_joint3", faceinfo, "right_eyebrow_right_corner", bonedef, "offset_brow_Rt_joint3", headCenter);
	MoveBone(pmesh, "mouthLip_up_joint0", faceinfo, "mouth_upper_lip_top", bonedef, "offset_mouthLip_up_joint0", headCenter);

	MoveBone(pmesh, "mouthLip_dn_joint0", faceinfo, "mouth_left_corner", bonedef, "offset_mouthLip_dn_joint0", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint1", faceinfo, "mouth_left_corner", bonedef, "offset_mouthLip_Lf_joint1", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint2", faceinfo, "mouth_upper_lip_left_contour2", bonedef, "offset_mouthLip_Lf_joint2", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint3", faceinfo, "contour_left8", bonedef, "offset_mouthLip_Lf_joint3", headCenter);
	MoveBone(pmesh, "mouthLip_Rt_joint5", faceinfo, "mouth_upper_lip_right_contour3", bonedef, "offset_mouthLip_Rt_joint5", headCenter);
	MoveBone(pmesh, "mouthLip_Rt_joint4", faceinfo, "mouth_upper_lip_right_contour1", bonedef, "offset_mouthLip_Rt_joint4", headCenter);
	MoveBone(pmesh, "mouthLip_Rt_joint7", faceinfo, "mouth_lower_lip_right_contour1", bonedef, "offset_mouthLip_Rt_joint7", headCenter);
	MoveBone(pmesh, "mouthLip_Rt_joint6", faceinfo, "mouth_lower_lip_right_contour2", bonedef, "offset_mouthLip_Rt_joint6", headCenter);
	MoveBone(pmesh, "mouthLip_dn_joint2", faceinfo, "mouth_lower_lip_top", bonedef, "offset_mouthLip_dn_joint2", headCenter);
	MoveBone(pmesh, "mouthLip_dn_joint1", faceinfo, "mouth_lower_lip_bottom", bonedef, "offset_mouthLip_dn_joint1", headCenter);

	MoveBone(pmesh, "mouthLip_Rt_joint1", faceinfo, "mouth_right_corner", bonedef, "offset_mouthLip_Rt_joint1", headCenter);
	MoveBone(pmesh, "mouthLip_Rt_joint2", faceinfo, "mouth_upper_lip_right_contour2", bonedef, "offset_mouthLip_Rt_joint2", headCenter);
	MoveBone(pmesh, "mouthLip_Rt_joint3", faceinfo, "contour_right8", bonedef, "offset_mouthLip_Rt_joint3", headCenter);
	MoveBone(pmesh, "mouthLip_up_joint2", faceinfo, "mouth_upper_lip_bottom", bonedef, "offset_mouthLip_up_joint2", headCenter);
	MoveBone(pmesh, "mouthLip_up_joint1", faceinfo, "mouth_upper_lip_top", bonedef, "offset_mouthLip_up_joint1", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint6", faceinfo, "mouth_lower_lip_left_contour2", bonedef, "offset_mouthLip_Lf_joint6", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint4", faceinfo, "mouth_upper_lip_left_contour1", bonedef, "offset_mouthLip_Lf_joint4", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint5", faceinfo, "mouth_upper_lip_left_contour3", bonedef, "offset_mouthLip_Lf_joint5", headCenter);
	MoveBone(pmesh, "mouthLip_Lf_joint7", faceinfo, "mouth_lower_lip_left_contour1", bonedef, "offset_mouthLip_Lf_joint7", headCenter);
	MoveBone(pmesh, "nose_joint2", faceinfo, "nose_contour_lower_middle", bonedef, "offset_nose_joint2", headCenter);

	MoveBone(pmesh, "nosewing_Lf_joint1", faceinfo, "nose_left", bonedef, "offset_nosewing_Lf_joint1", headCenter);
	MoveBone(pmesh, "nosewing_Rt_joint1", faceinfo, "nose_right", bonedef, "offset_nosewing_Rt_joint1", headCenter);
	MoveBone(pmesh, "nose_joint1", faceinfo, "nose_tip", bonedef, "offset_nose_joint1", headCenter);
	MoveBone(pmesh, "nosewing_Rt_joint2", faceinfo, "nose_contour_right3", bonedef, "offset_nosewing_Rt_joint2", headCenter);
	MoveBone(pmesh, "nosewing_Lf_joint2", faceinfo, "nose_contour_left3", bonedef, "offset_nosewing_Lf_joint2", headCenter);
	MoveBone(pmesh, "bridge_Lf_joint1", faceinfo, "nose_contour_left1", bonedef, "offset_bridge_Lf_joint1", headCenter);
	MoveBone(pmesh, "bridge_Rt_joint1", faceinfo, "nose_contour_right1", bonedef, "offset_bridge_Rt_joint1", headCenter);
	
















}


void BoneUtility::ResetBone()
{

}
//面部骨骼变形
void BoneUtility::MoveBone(SkinnedMesh* pmesh,string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRoleBone bonedef, string boneoffsetname, Vector3f headCenter,float offsetrate)
{
	Matrix4f parentbonetrs = pmesh->GetParentBoneOffsetMatrix(bonename);
	Matrix4f trs = pmesh->GetBoneOffsetMatrix(bonename);


	float zepos = bonedef.face_zero_pointy;

	// Vector3f  planpos = _faceplan.Matrix4f.position; 
	//重定义零点

	Vector3f planpos = headCenter;


	float sizeuv = bonedef.uvsize;// 30.0f;
	float scale_1024_to_model = sizeuv / 1024;
	float zero_px = planpos.x - sizeuv / 2;
	float zero_py = planpos.y - sizeuv / 2;

	Vector3f trspos = trs.ExtractTranslation();
	Vector3f trsrot = trs.ExtractRotation();
	Vector3f trsscale = trs.ExtractScale();

	Vector3f zero(zero_px, zero_py, trspos.z);
	//Debug.Log(pmesh->GetBoneOffsetMatrix("chin_joint1"].position);

	Vector3f OriPosition = trspos;

	float yaw_angle = (float)(faceinfo.yaw_angle);          //摇头
	float roll_angle = (float)(faceinfo.roll_angle);        //平面旋转
	float pitch_angle = (float)(faceinfo.pitch_angle);   //抬头


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

	//  Debug.LogError(faceOffset);


	//  tr.position  += faceOffset;  
	Vector3f parentpos = parentbonetrs.ExtractTranslation();
	
	Vector3f localposition = zero + Vector3f(face2dpos.x * scale_1024_to_model, face2dpos.y * scale_1024_to_model,0) - parentpos;

	localposition += bonedef.offsets_map[boneoffsetname] * offsetrate;


	Pipeline p;
	p.Scale(trsscale);
	p.Rotate(trsrot);
	p.WorldPos(localposition);

	Matrix4f finalTran = p.GetWorldTrans();

	//trs.InitTranslationTransform(localposition.x, localposition.y, localposition.z);
	pmesh->SetBoneOffsetMatrix(bonename, finalTran);
	//string name = tr.name;
	//Addlandmak(name);

}