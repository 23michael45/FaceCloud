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
	hasMoveBones = false;
	pairs.LoadFromFile("data/face/kp.json");

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


void BoneUtility::CalculateFaceBone(SkinnedMesh* pmesh, JsonRole bonedef, JsonFaceInfo faceinfo)
{
	if (hasMoveBones)
	{
		return;
	}
	hasMoveBones = true;
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

	Matrix4f tooth_MID = pmesh->GetBoneInfo("face_mouthLip_up_joint2").BoneOffset;
	Matrix4f toothup_Lf = pmesh->GetBoneInfo("face_mouthLip_Lf_joint1").BoneOffset;
	Matrix4f toothdown_Rt = pmesh->GetBoneInfo("face_mouthLip_Rt_joint1").BoneOffset;

	Vector3f tooth_MID_pos1 = tooth_MID.ExtractTranslation();
	Vector3f toothup_Lf_pos1 = toothup_Lf.ExtractTranslation();
	Vector3f toothdown_Rt_pos1 = toothdown_Rt.ExtractTranslation();

	//额头偏移

	Matrix4f forehead_RT = pmesh->GetBoneInfo("face_temple_Rt_joint1").BoneOffset;
	Matrix4f forehead_LF = pmesh->GetBoneInfo("face_temple_Lf_joint1").BoneOffset;


	Vector3f forhead_RT_pos1 = forehead_RT.ExtractTranslation();
	Vector3f forhead_LF_pos1 = forehead_LF.ExtractTranslation();

	//眉毛修正

	Matrix4f brow_LF02 = pmesh->GetBoneInfo("face_brow_Lf_joint2").BoneOffset;
	Matrix4f brow_LF03 = pmesh->GetBoneInfo("face_brow_Lf_joint3").BoneOffset;

	Vector3f brow_LF02_pos1 = brow_LF02.ExtractTranslation();
	Vector3f brow_LF03_pos1 = brow_LF03.ExtractTranslation();

	//鼻子修正
	Matrix4f nose_tr = pmesh->GetBoneInfo("face_nose_joint2").BoneOffset;
	Vector3f nose_tr_pos1 = nose_tr.ExtractTranslation();


	//嘴巴修正

	SkinnedMesh::BoneInfo mouth_tr =  pmesh->GetBoneInfo("face_mouthLip_up_joint0");
	Vector3f mouth_tr_pos1 = mouth_tr.BoneOffset.ExtractTranslation();

	//眼睛修正
	float righteye_conner_pos1 = pmesh->GetBoneInfo("face_eyeLids_Rt_joint1").BoneOffset.ExtractTranslation().x;
	float righteye_conner_pos2 = pmesh->GetBoneInfo("face_eyeLids_Rt_joint2").BoneOffset.ExtractTranslation().x;
	float Lefteye_conner_pos1 = pmesh->GetBoneInfo("face_eyeLids_Lf_joint1").BoneOffset.ExtractTranslation().x;
	float Lefteye_conner_pos2 = pmesh->GetBoneInfo("face_eyeLids_Lf_joint2").BoneOffset.ExtractTranslation().x;

	float Eye_Rt_dis = righteye_conner_pos1 - righteye_conner_pos2;
	float Eye_LF_dis = Lefteye_conner_pos1 - Lefteye_conner_pos2;


	Vector3f fixmouthlipdistance01 = pmesh->GetBoneInfo("face_mouthLip_Lf_joint5").BoneOffset.ExtractTranslation() - pmesh->GetBoneInfo("face_mouthLip_Lf_joint7").BoneOffset.ExtractTranslation();
	Vector3f fixmouthlipdistance02 = pmesh->GetBoneInfo("face_mouthLip_Rt_joint5").BoneOffset.ExtractTranslation() - pmesh->GetBoneInfo("face_mouthLip_Rt_joint7").BoneOffset.ExtractTranslation();
	Vector3f fixmouthlipdistance03 = pmesh->GetBoneInfo("face_mouthLip_dn_joint2").BoneOffset.ExtractTranslation() - pmesh->GetBoneInfo("face_mouthLip_up_joint2").BoneOffset.ExtractTranslation();


	//  Debug.Log("Eye_Rt_dis" + Eye_Rt_dis+ "Eye_LF_dis"+ Eye_LF_dis);

	Vector3f fixnose_lf1 = pmesh->GetBoneInfo("face_nosewing_Lf_joint1").BoneOffset.ExtractTranslation();
	Vector3f fixnose_RT1 = pmesh->GetBoneInfo("face_nosewing_Rt_joint1").BoneOffset.ExtractTranslation();


	SkinnedMesh::BoneInfo boneinfo = pmesh->GetBoneInfo("face_eyeLids_Lf_joint1");
	SkinnedMesh::BoneInfo boneinfo1 = pmesh->GetBoneInfo("face_eyeBall_Lf_joint1");

	Vector3f leftpos = pmesh->GetBoneInfo("face_eyeLids_Lf_joint1").BoneOffset.ExtractTranslation();
	Vector3f rightpos = pmesh->GetBoneInfo("face_eyeLids_Rt_joint1").BoneOffset.ExtractTranslation();
	Vector3f headCenter = (leftpos + rightpos) * 0.5f;



	for (int i = 0; i < pairs.pairs.size(); i++)
	{
		KP kp = pairs.pairs[i];
		MoveBone(pmesh, kp.bonename, faceinfo, kp.facekeypointname, bonedef, kp.offsetname, headCenter);
	}


	MoveUV(pmesh,bonedef);

}
void BoneUtility::MoveUV(SkinnedMesh* pmesh, JsonRole bonedef)
{

	uint NumVertices = 0;
	int len = 0;
	int startpos;
	for (uint i = 0; i < pmesh->m_Entries.size(); i++) {

		if (pmesh->m_Entries[i].NumBones > 80)
		{
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
		Vector2f uv(pmesh->TotalPositions[startpos + i].x / uvsize + 0.5f, -pmesh->TotalPositions[startpos + i].y / uvsize - offset_y + 0.27);
		uvs2.push_back(uv);
	}

	vector<Vector2f> newuv2 = pmesh->TotalTexCoords2;

	for (int i = 0; i < len; i++)
	{
		newuv2[startpos + i] = uvs2[i];
	}

	pmesh->RefreshUV2(newuv2);
}


void BoneUtility::ResetBone()
{

}
//面部骨骼变形
void BoneUtility::MoveBonePYR(SkinnedMesh* pmesh,string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRole bonedef, string boneoffsetname, Vector3f headCenter,float offsetrate)
{
	SkinnedMesh::BoneInfo boneinfo = pmesh->GetBoneInfo(bonename);



	Matrix4f finaltransold = boneinfo.BoneOffset;
	Vector3f trspos = finaltransold.ExtractTranslation();
	Vector3f trsrot = finaltransold.ExtractRotation();
	Vector3f trsscale = finaltransold.ExtractScale();

	float zepos = bonedef.face_zero_pointy;

	// Vector3f  planpos = _faceplan.Matrix4f.position; 
	//重定义零点

	Vector3f planpos = headCenter;


	float sizeuv = bonedef.uvsize;// 30.0f;
	float scale_1024_to_model = sizeuv / 1024;
	float zero_px = planpos.x - sizeuv / 2;
	float zero_py = planpos.y - sizeuv / 2;


	Vector3f zero(zero_px, zero_py, trspos.z);
	//Debug.Log(pmesh->GetBoneInfo("chin_joint1"].position);

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




	
	
	Vector3f finalpos = zero + Vector3f(face2dpos.x * scale_1024_to_model, face2dpos.y * scale_1024_to_model,0);




	Pipeline p;
	p.Scale(trsscale);
	p.Rotate(trsrot);
	p.WorldPos(finalpos);


	//Matrix4f finaltransnew = p.GetWorldTrans();

	/*Matrix4f localtrans = boneinfo.Parentformation.Inverse() * finaltransnew;
	Vector3f lpos = localtrans.ExtractTranslation();
	Vector3f lrot = localtrans.ExtractRotation();
	Vector3f lscale = localtrans.ExtractScale();*/


	//lpos += bonedef.offsets_map[boneoffsetname] * offsetrate;

	boneinfo.BoneOffset = p.GetWorldTrans();

	boneinfo.FinalTransformation = boneinfo.GlobalInverseTransform * boneinfo.Parentformation * boneinfo.NodeTransformation * boneinfo.BoneOffset;
	
}



//面部骨骼变形
void BoneUtility::MoveBone(SkinnedMesh* pmesh, string bonename, JsonFaceInfo faceinfo, string facekeypoint, JsonRole bonedef, string boneoffsetname, Vector3f headCenter, float offsetrate)
{

	bool isbone = false;
	Matrix4f transformtochange;
	if (pmesh->m_BoneMapping.find(bonename) != pmesh->m_BoneMapping.end())
	{
		uint index = pmesh->m_BoneMapping[bonename];
		transformtochange = pmesh->m_BoneInfo[index].BoneOffset;

		//transformtochange.Inverse();
		isbone = true;
	}
	else
	{
		transformtochange = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]);
		isbone = false;

	}
	Vector3f trspos = transformtochange.ExtractTranslation();
	Vector3f trsrot = transformtochange.ExtractRotation();
	Vector3f trsscale = transformtochange.ExtractScale();

	float zepos = bonedef.face_zero_pointy;

	float sizeuv = bonedef.uvsize;
	float scale_1024_to_model = sizeuv / 1024;

	float zero_px = headCenter.x - sizeuv / 2;//open 右手坐标 unity 左手坐标 
	float zero_py = headCenter.y + sizeuv / 2;

	Vector3f zero(zero_px, zero_py, trspos.z);
	if (isbone)
	{

		Vector3f zero(zero_px, zero_py, trspos.z);
		trspos = zero + Vector3f(faceinfo.landmarkdata[facekeypoint].x, -faceinfo.landmarkdata[facekeypoint].y, 0) *scale_1024_to_model;


		/*Vector3f zero(zero_px, -zero_py, trspos.z);
		trspos = zero + Vector3f(faceinfo.landmarkdata[facekeypoint].x, faceinfo.landmarkdata[facekeypoint].y, 0) *scale_1024_to_model;*/
	}
	else
	{
		Vector3f zero(zero_px, -zero_py, trspos.z);
		trspos = zero + Vector3f(faceinfo.landmarkdata[facekeypoint].x, faceinfo.landmarkdata[facekeypoint].y, 0) *scale_1024_to_model;

	}

	//Vector3f offset = bonedef.offsets_map[boneoffsetname] * offsetrate;
	//trspos += offset;


	Pipeline p;
	p.Scale(trsscale);
	p.Rotate(trsrot);
	p.WorldPos(trspos);

	Matrix4f mat = p.GetWorldTrans();

	Matrix4f nodetranformation = pmesh->m_BoneNodeMap[bonename]->mTransformation;

	if (isbone)
	{

		uint index = pmesh->m_BoneMapping[bonename];
		pmesh->m_BoneInfo[index].BoneOffset = mat;


		/*Matrix4f identity;
		identity.InitIdentity();

		uint index = pmesh->m_BoneMapping[bonename];
		pmesh->m_BoneInfo[index].BoneOffset = identity;*/

		/*
		aiMatrix4x4 aicurrentMat = pmesh->m_BoneNodeMap[bonename]->mTransformation;
		Matrix4f currentMat = Matrix4f(aicurrentMat);
		Vector3f cpos = transformtochange.ExtractTranslation();
		Vector3f crot = transformtochange.ExtractRotation();
		Vector3f cscale = transformtochange.ExtractScale();
		Pipeline cp;
		cp.Scale(cscale);
		cp.Rotate(crot);
		Matrix4f crs = cp.GetWorldTrans();
		Matrix4f crsinv = crs;
		crsinv.Inverse();


		Matrix4f parentMat = pmesh->GetNodeGlobalTransformation(pmesh->m_BoneNodeMap[bonename]->mParent);


		Matrix4f parentinv = parentMat;
		parentinv.Inverse();
		Matrix4f x = parentinv * mat;*/

		//x = x * crs;


		//pmesh->m_BoneNodeMap[bonename]->mTransformation = x.GetaiMatrix4x4();
	}
	else
	{
		aiMatrix4x4 parentMat = pmesh->m_BoneNodeMap[bonename]->mParent->mTransformation;
		Matrix4f parentinv(parentMat.Inverse());
		Matrix4f x = parentinv * mat;
		pmesh->m_BoneNodeMap[bonename]->mTransformation = x.GetaiMatrix4x4();
	}
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
