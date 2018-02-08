//#include "FBXImportUtility.h"
//
//namespace  FBXImportPrint
//{
//	int numTabs = 0;
//
//	void PrintTabs()  //��ӡtabs�������xml������Ч��
//	{
//		for (int i = 0; i < numTabs; i++)
//		{
//			printf("\t");
//		}
//	}
//
//
//	/**
//	*���ݽڵ����ԵĲ�ͬ�������ַ��������Ƿ��ؽڵ����Ե�����
//	**/
//	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type)
//	{
//		switch (type)
//		{
//		case FbxNodeAttribute::eUnknown: return "UnknownAttribute";
//		case FbxNodeAttribute::eNull: return "Null";
//		case FbxNodeAttribute::eMarker: return "marker";  //��ˡ���
//		case FbxNodeAttribute::eSkeleton: return "Skeleton"; //����
//		case FbxNodeAttribute::eMesh: return "Mesh"; //����
//		case FbxNodeAttribute::eNurbs: return "Nurbs"; //����
//		case FbxNodeAttribute::ePatch: return "Patch"; //Patch��Ƭ
//		case FbxNodeAttribute::eCamera: return "Camera"; //�����
//		case FbxNodeAttribute::eCameraStereo: return "CameraStereo"; //����
//		case FbxNodeAttribute::eCameraSwitcher: return "CameraSwitcher"; //�л���
//		case FbxNodeAttribute::eLight: return "Light"; //�ƹ�
//		case FbxNodeAttribute::eOpticalReference: return "OpticalReference"; //��ѧ��׼
//		case FbxNodeAttribute::eOpticalMarker: return "OpticalMarker";
//		case FbxNodeAttribute::eNurbsCurve: return "Nurbs Curve";//NURBS����
//		case FbxNodeAttribute::eTrimNurbsSurface: return "Trim Nurbs Surface"; //������У�
//		case FbxNodeAttribute::eBoundary: return "Boundary"; //�߽�
//		case FbxNodeAttribute::eNurbsSurface: return "Nurbs Surface"; //Nurbs����
//		case FbxNodeAttribute::eShape: return "Shape"; //��״
//		case FbxNodeAttribute::eLODGroup: return "LODGroup"; //
//		case FbxNodeAttribute::eSubDiv: return "SubDiv";
//		default: return "UnknownAttribute";
//		}
//	}
//
//
//
//	/**
//	*��ӡһ������
//	**/
//	void PrintAttribute(FbxNodeAttribute* pattribute)
//	{
//		if (!pattribute)
//		{
//			return;
//		}
//		FbxString typeName = GetAttributeTypeName(pattribute->GetAttributeType());
//		FbxString attrName = pattribute->GetName();
//		PrintTabs();
//
//		//FbxString.Buffer() ����������Ҫ���ַ�����
//		printf("<attribute type='%s' name='%s'/>\n ", typeName.Buffer(), attrName.Buffer());
//	}
//
//
//	/**
//	*��ӡ��һ���ڵ�����ԣ����ҵݹ��ӡ�������ӽڵ������;
//	**/
//	void PrintNode(FbxNode* pnode)
//	{
//		PrintTabs();
//
//		const char* nodeName = pnode->GetName(); //��ȡ�ڵ�����
//
//		FbxDouble3 translation = pnode->LclTranslation.Get();//��ȡ���node��λ�á���ת������
//		FbxDouble3 rotation = pnode->LclRotation.Get();
//		FbxDouble3 scaling = pnode->LclScaling.Get();
//
//		//��ӡ�����node�ĸ�������
//		printf("<node name='%s' translation='(%f,%f,%f)' rotation='(%f,%f,%f)' scaling='(%f,%f,%f)'>\n",
//			nodeName,
//			translation[0], translation[1], translation[2],
//			rotation[0], rotation[1], rotation[2],
//			scaling[0], scaling[1], scaling[2]);
//
//		numTabs++;
//
//		//��ӡ���node ����������
//		for (int i = 0; i < pnode->GetNodeAttributeCount(); i++)
//		{
//			PrintAttribute(pnode->GetNodeAttributeByIndex(i));
//		}
//
//		//�ݹ��ӡ������node������
//		for (int j = 0; j < pnode->GetChildCount(); j++)
//		{
//			PrintNode(pnode->GetChild(j));
//		}
//
//		numTabs--;
//		PrintTabs();
//		printf("</node>\n");
//	}
//
//
//
//}
//
//namespace FBXImportParse
//{
//
//	FbxManager *g_pFbxManager = NULL;
//
//	/*** ��ʼ��FbxSDK ***/
//	bool InitializeFbxSDK()
//	{
//		bool bRet = false;
//
//		do
//		{
//			//����FbxManager
//			g_pFbxManager = FbxManager::Create();
//
//			//����FbxIOSetting
//			FbxIOSettings *pFbxIOSettings = FbxIOSettings::Create(g_pFbxManager, IOSROOT);
//
//			//�󶨹�ϵ
//			g_pFbxManager->SetIOSettings(pFbxIOSettings);
//
//			bRet = true;
//
//		} while (0);
//		return bRet;
//	}
//
//	/***  ��ȡFbx SDK ֧�ֶ���ĸ�ʽ  ***/
//	void GetFileCanImport()
//	{
//		int count = g_pFbxManager->GetIOPluginRegistry()->GetReaderFormatCount();
//
//		printf("֧�ֵ������� %d ���ļ���ʽ��\n", count);
//
//		FbxString s;
//		int i = 0;
//		for (int i = 0; i < count; i++)
//		{
//			s += g_pFbxManager->GetIOPluginRegistry()->GetReaderFormatDescription(i); //��ȡ����
//																					  //s+=g_pFbxManager->GetIOPluginRegistry()->GetReaderFormatExtension(i); //��ȡ�ļ���׺
//			s = "%d : " + s + " \n";
//			printf(s.Buffer(), i);
//			s.Clear();
//		}
//	}
//
//	/***  ��ȡFbx SDK ���Ե����ĸ�ʽ ***/
//	void GetFileCanExport()
//	{
//		int count = g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount();
//		printf("֧�ֵ������� %d ���ļ���ʽ��\n", count);
//
//		FbxString s;
//		int i = 0;
//		for (int i = 0; i < count; i++)
//		{
//			s += g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatDescription(i); //��ȡ����
//																					  //s+=g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatExtension(i);//��ȡ�ļ���׺
//			s = "%d : " + s + " \n";
//			printf(s.Buffer(), i);
//			s.Clear();
//		}
//	}
//
//	/***  ����һ��Fbx�ļ���FbxScene  ***/
//	bool ImportFbxModel(FbxScene* scene, const char* importfilename)
//	{
//		int fileVerMajorNum, fileVerMinorNum, fileVerRevision; //�ļ���汾�š�С�汾�ţ������汾��
//		int sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision; //FBX SDK�汾��
//		int animStackCount; //����������
//		bool bRet = false;
//		char password[1024]; //���룬�ļ����ܼ�������Ҫ��������
//
//							 //��ȡFBX SDK�İ汾��
//		FbxManager::GetFileFormatVersion(sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision);
//
//		//����FbxImporter
//		FbxImporter *pFbxImporter = FbxImporter::Create(g_pFbxManager, importfilename);
//
//		//��ʼ��FbxImporter
//		const bool importret = pFbxImporter->Initialize(importfilename, -1, g_pFbxManager->GetIOSettings());
//
//		//��ȡFbx�ļ��İ汾��
//		pFbxImporter->GetFileVersion(fileVerMajorNum, fileVerMinorNum, fileVerRevision);
//
//		if (!importret) //�������
//		{
//			FbxString errorStr = pFbxImporter->GetStatus().GetErrorString();
//			printf("\nFbxImporter��ʼ��ʧ�ܣ�����ԭ��%s\n", errorStr.Buffer());
//
//			if (pFbxImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion) //������ļ��汾����ȷ
//			{
//				printf("\n FBX SDK �汾��%d.%d.%d\n", sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision);
//				printf("\nFBX �ļ��汾 ��%d.%d.%d\n", fileVerMajorNum, fileVerMinorNum, fileVerRevision);
//			}
//			return false;
//		}
//
//		printf("\n ***** �����ļ��ɹ�****** \n");
//
//		printf("\n FBX SDK �汾��%d.%d.%d \n", sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision);
//
//		if (pFbxImporter->IsFBX()) //���������ļ���FBX��ʽ
//		{
//			printf("\n FBX �ļ��汾 ��%d.%d.%d \n", fileVerMajorNum, fileVerMinorNum, fileVerRevision);
//
//			//��FBX�ļ��У�һ��Scene�п�����һ������ "animation stack"��һ��"animation stack"������һ���������ݣ�������ȡ"animation stack"����Ϣ������Ҫ����ȫ����Scene
//
//			printf("\n Animation stack ��Ϣ��\n");
//			animStackCount = pFbxImporter->GetAnimStackCount();
//
//			printf("������%d\n ", animStackCount);
//			printf("���ƣ�%s\n ", pFbxImporter->GetActiveAnimStackName());
//
//			for (int i = 0; i < animStackCount; i++)
//			{
//				FbxTakeInfo *pFbxTakeInfo = pFbxImporter->GetTakeInfo(i);
//				printf("Animation Stack %d\n", i);
//				printf("		Name: %s\n", pFbxTakeInfo->mName.Buffer());
//				printf("		Description: %s\n", pFbxTakeInfo->mDescription.Buffer());
//
//				printf("		Import Name: %s\n", pFbxTakeInfo->mImportName.Buffer()); //�������������
//				printf("		Import State: %s\n", pFbxTakeInfo->mSelect ? "true" : "false");
//			}
//
//			//�����������ã�Ĭ�ϵ�����������
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
//		}
//
//		//���������ļ�����FBX��ʽ���Ǿ�û��������߼�
//
//		//����Fbx������
//		bRet = pFbxImporter->Import(scene);
//
//		//����ļ�������������Ƿ��ش���Code���������
//		if (bRet == false && pFbxImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
//		{
//			printf("���������룺\n");
//			password[0] = '\0';
//			FBXSDK_CRT_SECURE_NO_WARNING_BEGIN//������������ر�4996���档����scanf strcpy strcat��vs�¶����о���
//				scanf("%s", password);
//			FBXSDK_CRT_SECURE_NO_WARNING_END
//				FbxString passwdStr(password);
//
//			g_pFbxManager->GetIOSettings()->SetStringProp(IMP_FBX_PASSWORD, passwdStr);//��FbxIOSetting����StringProp�����ַ������ԣ�Prop����ָproperty
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);//����Bool���ԣ��Ƿ�ʹ������
//
//			bRet = pFbxImporter->Import(scene); //�������������Import
//			if (bRet == false && pFbxImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
//			{
//				printf("�ļ���������������");
//			}
//
//		}
//		pFbxImporter->Destroy();
//		return bRet;
//	}
//
//	/***  ����FbxScene��ģ���ļ�  ***/
//	bool ExportFbxSceneToModel(FbxScene* scene, const char* exportfilename, int exportformat, bool pexportmedia)
//	{
//		bool bRet = false;
//
//		//����FbxExporter
//		FbxExporter *pFbxExport = FbxExporter::Create(g_pFbxManager, "");
//
//		if (exportformat < 0 || exportformat >= g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount())
//		{
//			//���ѡ�񵼳����ļ���ʽ��֧��
//			printf(" ��֧�ֵ������ָ�ʽ!!  \n");
//			return false;
//
//			exportformat = g_pFbxManager->GetIOPluginRegistry()->GetNativeWriterFormat();
//			printf("  ����Ĭ�ϵĸ�ʽ(FBX)������%d  ", exportformat);
//
//			if (!pexportmedia) //�����������ý��
//			{
//				int formatcount = g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount();
//
//				//���Ե���FBX�� ascii�ļ������ܿ��������ܹ���
//				for (int i = 0; i < formatcount; i++)
//				{
//					if (g_pFbxManager->GetIOPluginRegistry()->WriterIsFBX(i))
//					{
//						FbxString desStr = g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatDescription(i);
//						if (desStr.Find("ascii") >= 0)
//						{
//							exportformat = i;
//							break;
//						}
//					}
//				}
//
//			}
//
//		}
//
//		//ѡ�񵼳���ʽ��ȷ�Ļ���û��������߼�
//		if (!pFbxExport->Initialize(exportfilename, -1, g_pFbxManager->GetIOSettings()))
//		{
//			printf("FbxExport->Initialize Faild \n");
//			printf("FbxExport ��ʼ��ʧ��ԭ��%s", pFbxExport->GetStatus().GetErrorString());
//			return false;
//		}
//
//		if (g_pFbxManager->GetIOPluginRegistry()->WriterIsFBX(exportformat))
//		{
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_MATERIAL, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_TEXTURE, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_EMBEDDED, pexportmedia);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_SHAPE, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_GOBO, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_ANIMATION, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
//		}
//
//		bRet = pFbxExport->Export(scene);
//
//		pFbxExport->Destroy();
//
//
//		return bRet;
//	}
//
//	/***  ת��һ��ģ���ļ�  ***/
//	void ConvertModelFile(const char *importfilename, const char *exportfilename, int writefileformat)
//	{
//		printf("�����ļ�·����%s\n �����ļ�·����%s \n �����ļ���ʽ��%d\n", importfilename, exportfilename, writefileformat);
//
//		//����FbxScene,���־ͽ�������� Baoxiang
//		FbxScene *pFbxScene = FbxScene::Create(g_pFbxManager, "Baoxiang");
//
//		printf("\n  ******   ת����ʼ  ******  \n");
//
//		bool b = ImportFbxModel(pFbxScene, importfilename);
//
//		if (b)
//		{
//			printf("\n**  ģ���ļ�����ɹ�  ****\n");
//		}
//		else
//		{
//			printf("\n**  ģ���ļ�����ʧ��  ****\n");
//			pFbxScene->Destroy();
//			return;
//		}
//
//		printf("\n**  ��ʼ���� ****\n");
//
//		b = ExportFbxSceneToModel(pFbxScene, exportfilename, writefileformat, false);
//
//		if (b)
//		{
//			printf("\n**  ����ģ���ļ��ɹ�  ****\n");
//		}
//		else
//		{
//			printf("\n**  ����ģ���ļ�ʧ��  ****\n");
//			pFbxScene->Destroy();
//			return;
//		}
//
//	}
//
//
//}
//
//
//using namespace FBXImportPrint;
//int Do()
//{
//	const char* filename = "data/women_head.FBX";
//
//	//����SDKManager
//	FbxManager* pSdkManager = FbxManager::Create();
//
//
//	FbxIOSettings *pFbxIOSettings = FbxIOSettings::Create(pSdkManager, filename);
//
//	//���ù���
//	pSdkManager->SetIOSettings(pFbxIOSettings);
//
//	//����FbxImporter��������fbx�ļ�
//	FbxImporter* pImporter = FbxImporter::Create(pSdkManager, "");
//
//	if (!pImporter->Initialize(filename, -1, pSdkManager->GetIOSettings()))
//	{
//		printf("Call to FbxImporter::Initialize() failed");
//		printf("Error returned :%s\n\n", pImporter->GetStatus().GetErrorString());
//		return -1;
//	}
//
//	FbxScene* pScene = FbxScene::Create(pSdkManager, "woman");
//
//	pImporter->Import(pScene);
//
//	pImporter->Destroy();
//
//	//�ݹ��ӡ�������������Ľڵ������
//	//ע�����ǲ����ӡ��Root�ڵ���Ϊroot�ڵ㲻�����κ�����
//	FbxNode* pRootNode = pScene->GetRootNode();
//	if (pRootNode)
//	{
//		int nodeCount = pRootNode->GetChildCount();
//		for (int i = 0; i < nodeCount; i++)
//		{
//			PrintNode(pRootNode->GetChild(i));
//		}
//	}
//	pSdkManager->Destroy();
//
//	return 0;
//}
//
//using namespace FBXImportParse;
//int Parse()
//{
//
//	InitializeFbxSDK();
//
//	GetFileCanImport();
//
//	GetFileCanExport();
//
//	//char importfilename[1024];
//	//int exportformat = 0;
//	//char exportfilename[1024];
//
//	//printf("\n�����뵼���ļ�·����");
//	//scanf("%s", importfilename);
//
//	//printf("\n�����뵼����ʽ��");
//	//scanf("%d", &exportformat);
//
//	//printf("\n�����뵼���ļ�·����");
//	//scanf("%s", exportfilename);
//	const char* importfilename = "data/women_head.FBX";
//	const char* exportfilename = "data/women_head.obj";
//
//
//	ConvertModelFile(importfilename, exportfilename, 7);
//
//	system("pause");
//
//	return 0;
//
//}