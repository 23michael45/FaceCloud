//#include "FBXImportUtility.h"
//
//namespace  FBXImportPrint
//{
//	int numTabs = 0;
//
//	void PrintTabs()  //打印tabs，造出像xml那样的效果
//	{
//		for (int i = 0; i < numTabs; i++)
//		{
//			printf("\t");
//		}
//	}
//
//
//	/**
//	*根据节点属性的不同，返回字符串。就是返回节点属性的名字
//	**/
//	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type)
//	{
//		switch (type)
//		{
//		case FbxNodeAttribute::eUnknown: return "UnknownAttribute";
//		case FbxNodeAttribute::eNull: return "Null";
//		case FbxNodeAttribute::eMarker: return "marker";  //马克……
//		case FbxNodeAttribute::eSkeleton: return "Skeleton"; //骨骼
//		case FbxNodeAttribute::eMesh: return "Mesh"; //网格
//		case FbxNodeAttribute::eNurbs: return "Nurbs"; //曲线
//		case FbxNodeAttribute::ePatch: return "Patch"; //Patch面片
//		case FbxNodeAttribute::eCamera: return "Camera"; //摄像机
//		case FbxNodeAttribute::eCameraStereo: return "CameraStereo"; //立体
//		case FbxNodeAttribute::eCameraSwitcher: return "CameraSwitcher"; //切换器
//		case FbxNodeAttribute::eLight: return "Light"; //灯光
//		case FbxNodeAttribute::eOpticalReference: return "OpticalReference"; //光学基准
//		case FbxNodeAttribute::eOpticalMarker: return "OpticalMarker";
//		case FbxNodeAttribute::eNurbsCurve: return "Nurbs Curve";//NURBS曲线
//		case FbxNodeAttribute::eTrimNurbsSurface: return "Trim Nurbs Surface"; //曲面剪切？
//		case FbxNodeAttribute::eBoundary: return "Boundary"; //边界
//		case FbxNodeAttribute::eNurbsSurface: return "Nurbs Surface"; //Nurbs曲面
//		case FbxNodeAttribute::eShape: return "Shape"; //形状
//		case FbxNodeAttribute::eLODGroup: return "LODGroup"; //
//		case FbxNodeAttribute::eSubDiv: return "SubDiv";
//		default: return "UnknownAttribute";
//		}
//	}
//
//
//
//	/**
//	*打印一个属性
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
//		//FbxString.Buffer() 才是我们需要的字符数组
//		printf("<attribute type='%s' name='%s'/>\n ", typeName.Buffer(), attrName.Buffer());
//	}
//
//
//	/**
//	*打印出一个节点的属性，并且递归打印出所有子节点的属性;
//	**/
//	void PrintNode(FbxNode* pnode)
//	{
//		PrintTabs();
//
//		const char* nodeName = pnode->GetName(); //获取节点名字
//
//		FbxDouble3 translation = pnode->LclTranslation.Get();//获取这个node的位置、旋转、缩放
//		FbxDouble3 rotation = pnode->LclRotation.Get();
//		FbxDouble3 scaling = pnode->LclScaling.Get();
//
//		//打印出这个node的概览属性
//		printf("<node name='%s' translation='(%f,%f,%f)' rotation='(%f,%f,%f)' scaling='(%f,%f,%f)'>\n",
//			nodeName,
//			translation[0], translation[1], translation[2],
//			rotation[0], rotation[1], rotation[2],
//			scaling[0], scaling[1], scaling[2]);
//
//		numTabs++;
//
//		//打印这个node 的所有属性
//		for (int i = 0; i < pnode->GetNodeAttributeCount(); i++)
//		{
//			PrintAttribute(pnode->GetNodeAttributeByIndex(i));
//		}
//
//		//递归打印所有子node的属性
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
//	/*** 初始化FbxSDK ***/
//	bool InitializeFbxSDK()
//	{
//		bool bRet = false;
//
//		do
//		{
//			//创建FbxManager
//			g_pFbxManager = FbxManager::Create();
//
//			//创建FbxIOSetting
//			FbxIOSettings *pFbxIOSettings = FbxIOSettings::Create(g_pFbxManager, IOSROOT);
//
//			//绑定关系
//			g_pFbxManager->SetIOSettings(pFbxIOSettings);
//
//			bRet = true;
//
//		} while (0);
//		return bRet;
//	}
//
//	/***  获取Fbx SDK 支持读入的格式  ***/
//	void GetFileCanImport()
//	{
//		int count = g_pFbxManager->GetIOPluginRegistry()->GetReaderFormatCount();
//
//		printf("支持导入以下 %d 种文件格式：\n", count);
//
//		FbxString s;
//		int i = 0;
//		for (int i = 0; i < count; i++)
//		{
//			s += g_pFbxManager->GetIOPluginRegistry()->GetReaderFormatDescription(i); //获取描述
//																					  //s+=g_pFbxManager->GetIOPluginRegistry()->GetReaderFormatExtension(i); //获取文件后缀
//			s = "%d : " + s + " \n";
//			printf(s.Buffer(), i);
//			s.Clear();
//		}
//	}
//
//	/***  获取Fbx SDK 可以导出的格式 ***/
//	void GetFileCanExport()
//	{
//		int count = g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount();
//		printf("支持导出以下 %d 种文件格式：\n", count);
//
//		FbxString s;
//		int i = 0;
//		for (int i = 0; i < count; i++)
//		{
//			s += g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatDescription(i); //获取描述
//																					  //s+=g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatExtension(i);//获取文件后缀
//			s = "%d : " + s + " \n";
//			printf(s.Buffer(), i);
//			s.Clear();
//		}
//	}
//
//	/***  读入一个Fbx文件到FbxScene  ***/
//	bool ImportFbxModel(FbxScene* scene, const char* importfilename)
//	{
//		int fileVerMajorNum, fileVerMinorNum, fileVerRevision; //文件大版本号、小版本号，修正版本号
//		int sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision; //FBX SDK版本号
//		int animStackCount; //动画的数量
//		bool bRet = false;
//		char password[1024]; //密码，文件可能加密了需要输入密码
//
//							 //获取FBX SDK的版本号
//		FbxManager::GetFileFormatVersion(sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision);
//
//		//创建FbxImporter
//		FbxImporter *pFbxImporter = FbxImporter::Create(g_pFbxManager, importfilename);
//
//		//初始化FbxImporter
//		const bool importret = pFbxImporter->Initialize(importfilename, -1, g_pFbxManager->GetIOSettings());
//
//		//获取Fbx文件的版本号
//		pFbxImporter->GetFileVersion(fileVerMajorNum, fileVerMinorNum, fileVerRevision);
//
//		if (!importret) //导入出错
//		{
//			FbxString errorStr = pFbxImporter->GetStatus().GetErrorString();
//			printf("\nFbxImporter初始化失败，错误原因：%s\n", errorStr.Buffer());
//
//			if (pFbxImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion) //如果是文件版本不正确
//			{
//				printf("\n FBX SDK 版本：%d.%d.%d\n", sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision);
//				printf("\nFBX 文件版本 ：%d.%d.%d\n", fileVerMajorNum, fileVerMinorNum, fileVerRevision);
//			}
//			return false;
//		}
//
//		printf("\n ***** 导入文件成功****** \n");
//
//		printf("\n FBX SDK 版本：%d.%d.%d \n", sdkVerMajorNum, sdkVerMinorNum, sdkVerRevision);
//
//		if (pFbxImporter->IsFBX()) //如果导入的文件是FBX格式
//		{
//			printf("\n FBX 文件版本 ：%d.%d.%d \n", fileVerMajorNum, fileVerMinorNum, fileVerRevision);
//
//			//在FBX文件中，一个Scene中可能有一个或多个 "animation stack"，一个"animation stack"里面存放一个动画数据，如果想获取"animation stack"的信息，不必要载入全部的Scene
//
//			printf("\n Animation stack 信息：\n");
//			animStackCount = pFbxImporter->GetAnimStackCount();
//
//			printf("数量：%d\n ", animStackCount);
//			printf("名称：%s\n ", pFbxImporter->GetActiveAnimStackName());
//
//			for (int i = 0; i < animStackCount; i++)
//			{
//				FbxTakeInfo *pFbxTakeInfo = pFbxImporter->GetTakeInfo(i);
//				printf("Animation Stack %d\n", i);
//				printf("		Name: %s\n", pFbxTakeInfo->mName.Buffer());
//				printf("		Description: %s\n", pFbxTakeInfo->mDescription.Buffer());
//
//				printf("		Import Name: %s\n", pFbxTakeInfo->mImportName.Buffer()); //导入进来的名字
//				printf("		Import State: %s\n", pFbxTakeInfo->mSelect ? "true" : "false");
//			}
//
//			//导入内容设置，默认导入所有内容
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
//		}
//
//		//如果导入的文件不是FBX格式，那就没有上面的逻辑
//
//		//导入Fbx到场景
//		bRet = pFbxImporter->Import(scene);
//
//		//如果文件导入出错，并且是返回错误Code是密码错误
//		if (bRet == false && pFbxImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
//		{
//			printf("请输入密码：\n");
//			password[0] = '\0';
//			FBXSDK_CRT_SECURE_NO_WARNING_BEGIN//这个宏是用来关闭4996警告。类似scanf strcpy strcat在vs下都会有警告
//				scanf("%s", password);
//			FBXSDK_CRT_SECURE_NO_WARNING_END
//				FbxString passwdStr(password);
//
//			g_pFbxManager->GetIOSettings()->SetStringProp(IMP_FBX_PASSWORD, passwdStr);//对FbxIOSetting设置StringProp，即字符串属性，Prop这里指property
//			g_pFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);//设置Bool属性，是否使用密码
//
//			bRet = pFbxImporter->Import(scene); //输入密码后重新Import
//			if (bRet == false && pFbxImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
//			{
//				printf("文件导入错误，密码错误！");
//			}
//
//		}
//		pFbxImporter->Destroy();
//		return bRet;
//	}
//
//	/***  导出FbxScene到模型文件  ***/
//	bool ExportFbxSceneToModel(FbxScene* scene, const char* exportfilename, int exportformat, bool pexportmedia)
//	{
//		bool bRet = false;
//
//		//创建FbxExporter
//		FbxExporter *pFbxExport = FbxExporter::Create(g_pFbxManager, "");
//
//		if (exportformat < 0 || exportformat >= g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount())
//		{
//			//如果选择导出的文件格式不支持
//			printf(" 不支持导出该种格式!!  \n");
//			return false;
//
//			exportformat = g_pFbxManager->GetIOPluginRegistry()->GetNativeWriterFormat();
//			printf("  尝试默认的格式(FBX)导出：%d  ", exportformat);
//
//			if (!pexportmedia) //如果不导出多媒体
//			{
//				int formatcount = g_pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount();
//
//				//尝试导出FBX的 ascii文件，即能看到内容能够的
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
//		//选择导出格式正确的话就没有上面的逻辑
//		if (!pFbxExport->Initialize(exportfilename, -1, g_pFbxManager->GetIOSettings()))
//		{
//			printf("FbxExport->Initialize Faild \n");
//			printf("FbxExport 初始化失败原因：%s", pFbxExport->GetStatus().GetErrorString());
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
//	/***  转换一个模型文件  ***/
//	void ConvertModelFile(const char *importfilename, const char *exportfilename, int writefileformat)
//	{
//		printf("导入文件路径：%s\n 导出文件路径：%s \n 导出文件格式：%d\n", importfilename, exportfilename, writefileformat);
//
//		//创建FbxScene,名字就叫做宝箱吧 Baoxiang
//		FbxScene *pFbxScene = FbxScene::Create(g_pFbxManager, "Baoxiang");
//
//		printf("\n  ******   转换开始  ******  \n");
//
//		bool b = ImportFbxModel(pFbxScene, importfilename);
//
//		if (b)
//		{
//			printf("\n**  模型文件载入成功  ****\n");
//		}
//		else
//		{
//			printf("\n**  模型文件载入失败  ****\n");
//			pFbxScene->Destroy();
//			return;
//		}
//
//		printf("\n**  开始导出 ****\n");
//
//		b = ExportFbxSceneToModel(pFbxScene, exportfilename, writefileformat, false);
//
//		if (b)
//		{
//			printf("\n**  导出模型文件成功  ****\n");
//		}
//		else
//		{
//			printf("\n**  导出模型文件失败  ****\n");
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
//	//创建SDKManager
//	FbxManager* pSdkManager = FbxManager::Create();
//
//
//	FbxIOSettings *pFbxIOSettings = FbxIOSettings::Create(pSdkManager, filename);
//
//	//设置归属
//	pSdkManager->SetIOSettings(pFbxIOSettings);
//
//	//创建FbxImporter用来导入fbx文件
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
//	//递归打印出这个场景下面的节点的属性
//	//注意我们不会打印出Root节点因为root节点不包含任何属性
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
//	//printf("\n请输入导入文件路径：");
//	//scanf("%s", importfilename);
//
//	//printf("\n请输入导出格式：");
//	//scanf("%d", &exportformat);
//
//	//printf("\n请输入导出文件路径：");
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