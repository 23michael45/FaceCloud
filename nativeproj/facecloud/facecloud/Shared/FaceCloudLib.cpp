#include "FaceCloudLib.h"
#include <iostream>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include "Predefined.h"
#include "OSMesaContext.h"
#include <mutex>          // std::mutex

#include "ImageOptimizedUtility.h"
#include <opencv2/photo.hpp>

bool WriteTGA(char *file, short int width, short int height, unsigned char *outImage)
{
	// To save a screen shot is just like reading in a image. All you do
	// is the opposite. Istead of calling fread to read in data you call
	// fwrite to save it.

	FILE *pFile; // The file pointer.
	unsigned char uselessChar; // used for useless char.
	short int uselessInt; // used for useless int.
	unsigned char imageType; // Type of image we are saving.
	int index; // used with the for loop.
	unsigned char bits; // Bit depth.
	long Size; // Size of the picture.
	int colorMode;
	unsigned char tempColors;

	// Open file for output.
	pFile = fopen(file, "wb");

	// Check if the file opened or not.
	if (!pFile) { fclose(pFile); return false; }

	// Set the image type, the color mode, and the bit depth.
	imageType = 2; colorMode = 3; bits = 24;

	// Set these two to 0.
	uselessChar = 0; uselessInt = 0;

	// Write useless data.
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

	// Now image type.
	fwrite(&imageType, sizeof(unsigned char), 1, pFile);

	// Write useless data.
	fwrite(&uselessInt, sizeof(short int), 1, pFile);
	fwrite(&uselessInt, sizeof(short int), 1, pFile);
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
	fwrite(&uselessInt, sizeof(short int), 1, pFile);
	fwrite(&uselessInt, sizeof(short int), 1, pFile);

	// Write the size that you want.
	fwrite(&width, sizeof(short int), 1, pFile);
	fwrite(&height, sizeof(short int), 1, pFile);
	fwrite(&bits, sizeof(unsigned char), 1, pFile);

	// Write useless data.
	fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

	// Get image size.
	Size = width * height * colorMode;

	// Now switch image from RGB to BGR.
	for (index = 0; index < Size; index += colorMode)
	{
		tempColors = outImage[index];
		outImage[index] = outImage[index + 2];
		outImage[index + 2] = tempColors;
	}

	// Finally write the image.
	fwrite(outImage, sizeof(unsigned char), Size, pFile);

	// close the file.
	fclose(pFile);

	return true;
}


std::mutex mtx;           // locks access to counter
bool hasInitSuccess = false;
bool hasInitDone = false;
bool OpenGLThread(FaceCloudLib *psender)
{
	bool boffscreen = psender->m_bOffscreen;
	hasInitSuccess = psender->InitReal(boffscreen);
	hasInitDone = true;

	while (psender->m_Running == true)
	{
		if (mtx.try_lock())
		{
			if (psender->m_RunningQueue.size() > 0)
			{
				CalculateData* pdata = psender->m_RunningQueue.front();
				psender->m_RunningQueue.pop();
				pdata->success = psender->CalculateReal(pdata->modelID, pdata->photoPath, pdata->jsonFace, pdata->photoPathOut, pdata->jsonModelOut);
				pdata->finished = true;
			}
			else
			{
	
			}


			mtx.unlock();
		}

		this_thread::sleep_for(chrono::microseconds(0));
	}
	return true;
}
FaceCloudLib::FaceCloudLib()
{
	m_pGameCamera = NULL;
	m_pSkinningRenderer = NULL;
	m_pSkinningMaskRenderer = NULL;
	m_FramebufferName = 0;

	m_Width = 2048;
	m_Height = 2048;
}
FaceCloudLib::~FaceCloudLib()
{

	SAFE_DELETE(m_pGameCamera);
	SAFE_DELETE(m_pSkinningRenderer);
	SAFE_DELETE(m_pSkinningMaskRenderer);
	SAFE_DELETE(m_pCommonRenderer);

	for (map<string,SkinnedMesh*>::iterator iter = m_MeshMap.begin();iter != m_MeshMap.end();iter++)
	{
		SAFE_DELETE(iter->second);
	}
	m_MeshMap.clear();

	for (map<string, Texture*>::iterator iter = m_ColorTextureMap.begin(); iter != m_ColorTextureMap.end(); iter++)
	{
		SAFE_DELETE(iter->second);
	}
	m_ColorTextureMap.clear();
}
bool FaceCloudLib::Init(bool offscreen)
{
	hasInitDone = false;
	hasInitSuccess = false;
	m_Running = true;
	m_bOffscreen = offscreen;
	m_OpenGLThread = thread(OpenGLThread,this);
	m_OpenGLThread.detach();

	while (hasInitDone == false)
	{
		this_thread::sleep_for(chrono::microseconds(1));
	}

	OSMesa::Log("\nInit Has Done!");
	return hasInitSuccess;
}
bool FaceCloudLib::Finalize()
{
	m_Running = false;
	return true;
}

bool FaceCloudLib::InitReal (bool offscreen)
{
	OSMesa::Log("\nStart Face Cloud Lib Init\n");
	int argc = 0;
	char** argv = 0;
	GLUTBackendInit(argc, argv, true, false);

	if (offscreen)
	{
		if (!GLUTBackendCreateContext(m_Width, m_Height)) {

			glutHideWindow();
			OSMesa::Log("\nGLUTBackendCreateContext Failed");
			return false;
		}
	}
	else
	{
		if (!GLUTBackendCreateWindow(m_Width, m_Height, false, "FaceCloudLib")) {

			OSMesa::Log("\nGLUTBackendCreateWindow Failed");
			return false;
		}
	
	}	
	bool rt = InitCamera();
	if (rt == false)
	{
		OSMesa::Log("\nInitCamera Failed");
		return false;
	}
	rt = InitMesh();
	if (rt == false)
	{
		OSMesa::Log("\nInitMesh Failed");
		return false;
	}
	rt = InitMat();
	if (rt == false)
	{
		OSMesa::Log("\nInitMat Failed");
		return false;
	}
	if (!InitJson())
	{
		OSMesa::Log("\nInitJson Failed");
		return false;
	}
	
	CreateRenderTarget();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	return true;
}
string FaceCloudLib::Calculate(string modelID, string photoPath, string jsonFace, string& photoPathOut, string& jsonModelOut)
{

	OSMesa::Log("\nCalculate Start!");
	string calculateSuccess = "";
	while (!mtx.try_lock())
	{
		this_thread::sleep_for(chrono::microseconds(1));
	}

	CalculateData* pdata = new CalculateData;
	pdata->modelID = modelID;
	pdata->photoPath = photoPath;
	pdata->jsonFace = jsonFace;
	pdata->photoPathOut = photoPathOut;
	pdata->jsonModelOut = jsonModelOut;
	pdata->finished = false;
	pdata->success = "";
	m_RunningQueue.push(pdata);

	mtx.unlock();


	OSMesa::Log("\nPushed Queue Has Done!");

	while (pdata->finished == false)
	{
		this_thread::sleep_for(chrono::microseconds(1));
	}

	while (!mtx.try_lock())
	{
		this_thread::sleep_for(chrono::microseconds(1));
	}
	photoPathOut = pdata->photoPathOut;
	jsonModelOut = pdata->jsonModelOut;
	calculateSuccess = pdata->success;
	SAFE_DELETE(pdata);

	mtx.unlock();

	if (calculateSuccess == "success")
	{
		return jsonModelOut;
	}
	else
	{
		return calculateSuccess;

	}


}
string FaceCloudLib::CalculateReal(string modelID, string photoPath, string jsonFace, string& photoPathOut, string& jsonModelOut)
{
	try
	{
		bool isFrontColorTrans = true;
		OSMesa::Log("\n\nStarting timer...");
		int start = OSMesa::getMilliCount();

		/*string s = format("\nmodelID:%s \nphotoPath:%s \njsonFace:%s \nphotoPathOut:%s \njsonModelOut:%s \n \n",
			modelID.c_str(),
			photoPath.c_str(),
			jsonFace.c_str(),
			photoPathOut.c_str(),
			"");*/
		OSMesa::Log(OSMesa::format("\nmodelID:%s \nphotoPath:%s \njsonFace:%s \nphotoPathOut:%s \njsonModelOut:%s \n \n",
			modelID.c_str(),
			photoPath.c_str(),
			jsonFace.c_str(),
			photoPathOut.c_str(),
			""));

		Texture* ptexture = new Texture(GL_TEXTURE_2D);

		//File Path
		if (!ptexture->LoadFile(photoPath)) {
			SAFE_DELETE(ptexture);
			OSMesa::Log("\nLoad Src Texture Error");
			return "error";
		}


		
		//Base 64 Code	
		/*Magick::Image img;
		Magick::Blob blob;
		img.read(photoPath);
		img.write(&blob, "RGBA");
		string base64string = blob.base64();
		if (!ptexture->LoadBase64(base64string)) {
		return;
		}*/

		ptexture->Bind(GL_TEXTURE1);

		bool isman = true;
		if (modelID == "10001")
		{
			isman = true;
		}
		else if (modelID == "10002")
		{
			isman = false;
		}

		if (isman)
		{
			m_targetcolor = cv::Vec3s(210, 165, 120);
			m_basecolor = cv::Vec3s(210, 165, 120);
		}
		else
		{
			m_targetcolor = cv::Vec3s(200, 169, 140);
			m_basecolor = cv::Vec3s(200, 169, 140);
		}

		JsonFaceInfo jsonfaceinfo;
		if (jsonfaceinfo.LoadFromString(jsonFace, true))
		{

			/*unsigned char* ptex;
			cv::Mat premat = GLTextureToMat(ptexture->GetTextureObj(), ptex);
			for (map<string,Vector2f>::iterator iter = jsonfaceinfo.landmarkdata.begin();iter != jsonfaceinfo.landmarkdata.end();iter++)
			{
				Vector2f pos = iter->second;
				cv::circle(premat, cv::Point(pos.x,pos.y), 10, cv::Scalar(255, 0, 0, 255));
			}

			SaveTextureToFile(premat, GL_RGBA, "data/export/precircle.jpg");
			SAFE_DELETE(ptex);*/

			unsigned char* refptr = nullptr;
			cv::Mat refmat;
			if (m_ColorTextureMap.find(modelID) != m_ColorTextureMap.end())
			{
				refmat = GLTextureToMat(m_ColorTextureMap[modelID]->GetTextureObj(), refptr);
			}




			OSMesa::Log("\nStart CalculateSkin");
			Texture* paftertex = m_BoneUtility.CalculateSkin(ptexture->GetTextureObj(),refmat, isman, m_JsonRoles.roles[modelID], jsonfaceinfo, isFrontColorTrans);
			m_pCurrentSkinTexture = paftertex;


			//////////////////////////////print after texture for testing
			//unsigned char* ptr;
			//cv::Mat mat = GLTextureToMat(m_pCurrentSkinTexture->GetTextureObj(), ptr);

			//for (map<string, Vector2f>::iterator iter = jsonfaceinfo.landmarkdata.begin(); iter != jsonfaceinfo.landmarkdata.end(); iter++)
			//{
			//	string name = iter->first;
			//	Vector2f pos = iter->second;
			//	cv::circle(mat, cv::Point(pos.x, 1024 - pos.y), 15, cv::Scalar(255, 0, 0, 255));

			//	/*if (name.find("contour_left") == std::string::npos && name.find("contour_right") == std::string::npos)
			//	{
			//		cv::putText(mat, iter->first, cv::Point(pos.x, 1024 - pos.y), CV_FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0, 0, 255, 255));

			//	}*/
			//	/*if (name.find("eye") != std::string::npos)
			//	{
			//		cv::putText(mat, iter->first, cv::Point(pos.x, 1024 - pos.y), CV_FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0, 0, 255, 255));

			//	}*/
			//	
			//	/*if (name.find("contour") != std::string::npos)
			//	{
			//		contour.push_back(cv::Point(pos.x, 1024 - pos.y));

			//	}*/
			//}
		

			//SaveTextureToFile(mat, GL_RGBA, "data/export/afterskin.jpg");
			//SAFE_DELETE(ptr);
			//////////////////////////////print after texture for testing



			Vector3f center;
			Vector2f uvsize;
			float yoffset = 0;

			OSMesa::Log("\nStart CalculateBone");
			CalculateBone(modelID, jsonfaceinfo, photoPathOut, jsonModelOut, center, uvsize, yoffset);

			if (m_pSkinningRenderer)
			{
				m_pSkinningRenderer->Enable();
				m_pSkinningRenderer->SetUVSize(uvsize);
				m_pSkinningRenderer->SetYOffset(yoffset);

			}
			if (m_pSkinningMaskRenderer)
			{
				m_pSkinningMaskRenderer->Enable();
				m_pSkinningMaskRenderer->SetUVSize(uvsize);
				m_pSkinningMaskRenderer->SetYOffset(yoffset);

			}


			Texture automasktex;
			Texture rendertex;
			OSMesa::Log("\nStart DrawOnce");
			if (m_bRenderToTexture)
			{

				m_pGameCamera->SetPos(Vector3f(0, 0, 0));
				if (BeginRenterTexture())
				{
					DrawMaskOnce(modelID, center, uvsize);
					automasktex.CloneFromTexture(m_RenderTexture);

					DrawOnce(modelID, center, uvsize);
					rendertex.CloneFromTexture(m_RenderTexture);

					EndRenderTexture();
				}
			}
			else
			{
				EndRenderTexture();
				DrawOnce(modelID, center, uvsize);
			}
			//////////////////////////////print render texture for testing
			//unsigned char* rtptr;
			//cv::Mat autortmat = GLTextureToMat(rendertex.GetTextureObj(), rtptr);
			//SaveTextureToFile(autortmat, GL_RGBA, "data/export/autorttest.jpg");
			//SAFE_DELETE(rtptr);
			////////////////////////////////print render texture for testing
		
			////////////////////////////////print mask texture for testing
			//unsigned char* maskptr;
			//cv::Mat automaskmat = GLTextureToMat(automasktex.GetTextureObj(), maskptr);
			//SaveTextureToFile(automaskmat, GL_RGBA, "data/export/automasktest.jpg");
			//SAFE_DELETE(maskptr);
			//////////////////////////////print mask texture for testing



			OSMesa::Log("\nStart CombineTexture");
			//CombineTexture(m_RenderTexture, refmat, &automasktex, photoPathOut, isFrontColorTrans);
			CombineTextureMaskBlend( refmat, &automasktex, photoPathOut);
			//CombineTexture(m_RenderTexture, m_ColorTextureMap[modelID], m_pMaskTexture, photoPathOut);

			SAFE_DELETE(refptr);


			if (m_bRenderToTexture)
			{
				SAFE_DELETE(ptexture);
				SAFE_DELETE(m_pCurrentSkinTexture);

			}


			int milliSecondsElapsed = OSMesa::getMilliSpan(start);
			OSMesa::Log(OSMesa::format("\njsonModelOut:%s", jsonModelOut.c_str()));
			OSMesa::Log(OSMesa::format("\n\nElapsed time = %u milliseconds", milliSecondsElapsed));
			printf("\n\nElapsed time = %u milliseconds", milliSecondsElapsed);
			return "success";
		}
		else
		{

			SAFE_DELETE(ptexture);
		}
	}
	catch (...)
	{
	}
	SAFE_DELETE(m_pCurrentSkinTexture);

	OSMesa::Log("\ncalculate error");
	return "error";


}

string FaceCloudLib::DetectSkinStatus(string photoPath, string jsonFace, string &jsonModelOut)
{
	try
	{
		Mat img = imread(photoPath);
		JsonFaceInfo jsonfaceinfo;
		if (jsonfaceinfo.LoadFromString(jsonFace, true))
		{
			JsonSkinStatus jsonSkin;

			ImageOptimizedUtility iou;
			iou.DetectSkinStatus(img, jsonfaceinfo, jsonSkin);

			jsonSkin.gender = jsonfaceinfo.gender;
			jsonSkin.beauty_female = jsonfaceinfo.beauty_female;
			jsonSkin.beauty_male = jsonfaceinfo.beauty_male;
			jsonSkin.age = jsonfaceinfo.age;
			jsonSkin.dark_circle = jsonfaceinfo.dark_circle;
			jsonSkin.stain = jsonfaceinfo.stain;
			jsonSkin.acne = jsonfaceinfo.acne;
			jsonSkin.health = jsonfaceinfo.health;

			jsonModelOut = jsonSkin.ToString();
		}
		return jsonModelOut;
	}
	catch (...)
	{
		return "error";
	}
}
cv::Mat FaceCloudLib::AutoMask(cv::Mat srcMask,cv::Point& center)
{

	//make alpha mask
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;


	cv::Mat graymask;

	cvtColor(srcMask, graymask, CV_RGB2GRAY);
	graymask.convertTo(graymask, CV_8UC1);

	int largestindex = 0;
	int t = graymask.type();
	try
	{
		//findContours(graymask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point());
		findContours(graymask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		float maxArea = 0;
		for (int i = 0; i < contours.size(); i++)
		{
			float area = contourArea(contours[i]);

			if (maxArea < area)
			{
				maxArea = area;
				largestindex = i;
			}
		}



	}
	catch (...)
	{
	}
	cv::Rect br = boundingRect(contours[largestindex]);
	center.x = br.x + br.width / 2;
	center.y = br.y + br.height / 2;

	/*cv::Moments  m = moments(contours[largestindex]);
	center.x = m.m10 / m.m00;
	center.y = m.m01 / m.m00;*/
	

	cv::Mat outputContourMat(srcMask.size(),CV_16UC3,cv::Scalar(0,0,0));
	drawContours(outputContourMat, contours, largestindex, cv::Scalar(255, 255, 255), 5, 8);

	//cv::imwrite("data/export/contours.jpg", outputContourMat);
	return outputContourMat;
	//cv::Rect box = cv::boundingRect(contours[largestindex]);
	//int boardwidth = 20;

	////计算顶点到轮廓的距离
	//cv::Mat raw_dist(srcMask.size(), CV_32FC1, cv::Scalar(9999, 9999, 9999));

	//for (int j = box.tl().y - boardwidth / 2; j < box.br().y + boardwidth / 2; j++)
	//{
	//	for (int i = box.tl().x - boardwidth / 2; i < box.br().x + boardwidth / 2; i++)
	//	{
	//		raw_dist.at<float>(j, i) = pointPolygonTest(contours[largestindex], cv::Point2f(i, j), true);
	//	}
	//}

	//double minVal; double maxVal;
	//minMaxLoc(raw_dist, &minVal, &maxVal, 0, 0, cv::Mat());
	//minVal = abs(minVal); maxVal = abs(maxVal);

	////用户型化的方式显示距离
	//cv::Mat drawing = cv::Mat::zeros(srcMask.size(), CV_8UC4);

	//for (int j = 0; j < srcMask.rows; j++)
	//{
	//	for (int i = 0; i < srcMask.cols; i++)
	//	{
	//		float dist = raw_dist.at<float>(j, i);
	//		//在外部
	//		if (dist < 0)
	//		{
	//			drawing.at<cv::Vec3b>(j, i)[0] = 255 / 2 - (int)pow(abs(dist), 4);
	//			drawing.at<cv::Vec3b>(j, i)[1] = 255 / 2 - (int)pow(abs(dist), 4);
	//			drawing.at<cv::Vec3b>(j, i)[2] = 255 / 2 - (int)pow(abs(dist), 4);
	//			drawing.at<cv::Vec3b>(j, i)[3] = 255 / 2 - (int)pow(abs(dist), 4);
	//		}
	//		//在内部
	//		else if (dist > 0)
	//		{
	//			drawing.at<cv::Vec3b>(j, i)[0] = 255 / 2 + (int)pow(abs(dist), 4);
	//			drawing.at<cv::Vec3b>(j, i)[1] = 255 / 2 + (int)pow(abs(dist), 4);
	//			drawing.at<cv::Vec3b>(j, i)[2] = 255 / 2 + (int)pow(abs(dist), 4);
	//			drawing.at<cv::Vec3b>(j, i)[3] = 255 / 2 + (int)pow(abs(dist), 4);
	//		}
	//		else
	//			// 在边上
	//		{
	//			drawing.at<cv::Vec3b>(j, i)[0] = 255 / 2;
	//			drawing.at<cv::Vec3b>(j, i)[1] = 255 / 2;
	//			drawing.at<cv::Vec3b>(j, i)[2] = 255 / 2;
	//			drawing.at<cv::Vec3b>(j, i)[3] = 255 / 2;
	//		}
	//	}
	//}
	//return drawing;

}


void SeamlessT()
{
	// Read images : src image will be cloned into dst
	Mat src = imread("data/export/outphoto_predefmask.jpg");
	Mat dst = imread("data/export/color.jpg");
	Mat src_mask = imread("data/export/m.jpg");
	// Create a rough mask around the airplane.
	//Mat src_mask = Mat::zeros(src.rows, src.cols, src.depth());

	//// Define the mask as a closed polygon
	//Point poly[1][7];
	//poly[0][0] = Point(4, 80);
	//poly[0][1] = Point(30, 54);
	//poly[0][2] = Point(151, 63);
	//poly[0][3] = Point(254, 37);
	//poly[0][4] = Point(298, 90);
	//poly[0][5] = Point(272, 134);
	//poly[0][6] = Point(43, 122);

	//const Point* polygons[1] = { poly[0] };
	//int num_points[] = { 7 };

	//// Create mask by filling the polygon

	//fillPoly(src_mask, polygons, num_points, 1, Scalar(255, 255, 255));

	// The location of the center of the src in the dst
	Point center(400, 400);


	//imwrite("data/export/opencv-seamless-cloning-maskexample.jpg", src_mask);




	// Seamlessly clone src into dst and put the results in output
	Mat output;
	seamlessClone(src, dst, src_mask, center, output, NORMAL_CLONE);


	// Save result
	//imwrite("data/export/opencv-seamless-cloning-example.jpg", output);


}

/*
FaceTexure 脸部展平渲染图
pWhole 美术做好的颜色贴图
pMask  自动生成的MASK图(脸部轮廓)
photoPathOut 输出文件路径
*/
void FaceCloudLib::CombineTexture(GLuint FaceTexure, Mat bgColor, Texture* pMask,string& photoPathOut,bool frontTrans)
{
	unsigned char* faceptr;
	unsigned char* maskptr;
	//unsigned char* colorptr;


	//分别取MAT
	cv::Mat facemat = GLTextureToMat(FaceTexure, faceptr);
	cv::Mat maskmat = GLTextureToMat(pMask->GetTextureObj(), maskptr);
	cv::Mat colormat = bgColor;// GLTextureToMat(pWhole->GetTextureObj(), colorptr);


	//统一尺寸
	cv::Mat maskmat2;
	cv::resize(maskmat, maskmat2, cv::Size(m_Width, m_Height));
	
	cv::Mat colormat2;
	if (frontTrans)
	{
		colormat.copyTo(colormat2);
	}
	else
	{
		cv::resize(colormat, colormat2, cv::Size(800, 800));

	}


	//统一格式类型
	cv::Mat facemat2;

	facemat.convertTo(facemat2, CV_16UC3);
	maskmat2.convertTo(maskmat2, CV_16UC3);
	colormat2.convertTo(colormat2, CV_16UC3);


	//MASK与FACE 渲染时是头向下的，所以FLIP一次
	cv::flip(maskmat2, maskmat2, 0);
	cv::flip(facemat2, facemat2, 0);
	//cv::flip(colormat2, colormat2, 0);



	//取小块脸部区域
	cv::Range startRg(270, 1070);
	cv::Range endRg(624, 1424);


	facemat2 = facemat2(startRg, endRg);
	maskmat2 = maskmat2(startRg, endRg);

	if (frontTrans)
	{
		colormat2 = colormat2(startRg, endRg);

	}

	//自动生成一个线条轮廓MASK
	cv::Point maskcenter;
	cv::Mat contourEdge = AutoMask(maskmat2, maskcenter);

	
	//美白一次
	/*ImageOptimizedUtility iou;
	facemat2 = iou.UpdateDermabrasion(facemat2, 4);
	facemat2.convertTo(facemat2, CV_16UC3);*/


	//形态学，把MASK 先膨胀(去掉眼鼻的漏洞） 再腐蚀(变小一些防止融合边缘有黑缝） 再Blur一下生成边缘渐变
	cv::Mat graymask;
	cvtColor(maskmat2, graymask, CV_RGB2GRAY);
	graymask.convertTo(graymask, CV_8UC1);

	cv::Mat dilateStruct = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30));
	cv::dilate(graymask, graymask, dilateStruct);
	cv::Mat erodeStruct = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(45, 45));
	cv::erode(graymask, graymask, erodeStruct);	

	cv::Mat erodeStruct2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(40, 40));
	cv::erode(graymask, graymask, erodeStruct2);
	/*cv::erode(graymask, graymask, erodeStruct2);
	cv::erode(graymask, graymask, erodeStruct2);
	cv::erode(graymask, graymask, erodeStruct2);*/

	//cvSmooth(graymask, graymask, CV_BLUR, 11, 11);
	cv::blur(graymask, graymask, cv::Size(40, 40));
	//cv::GaussianBlur(graymask, graymask, cv::Size(100, 100),0,0);
	

	//blur后统一类型
	cv::Mat maskblur;
	cvtColor(graymask, maskblur, CV_GRAY2RGB);
	maskblur.convertTo(maskblur, CV_16UC3);


	//变成RGB统一通道数
	cv::cvtColor(maskmat2, maskmat2, CV_RGBA2RGB);
	cv::cvtColor(colormat2, colormat2, CV_RGBA2RGB);



	////////////////////////////////////////////////////////用之前预定义的MASK，已经弃用
	//face area mask is black
	//facemat2 = 1.0f / 255 * ((facemat2.mul(cv::Scalar(255, 255, 255) - maskmat2)) +colormat2.mul( maskmat2));




	//测试用，生成中间图片
	/*imwrite("data/export/f.jpg", facemat2);
	imwrite("data/export/m.jpg", maskmat2);
	imwrite("data/export/c.jpg", colormat2);
	imwrite("data/export/ce.jpg", contourEdge);
	imwrite("data/export/mb.jpg", maskblur);*/


	//计算每一下像素值
	
	//for (int j = 0; j < facemat2.rows; j++)
	//{
	//	for (int i = 0; i < facemat2.cols; i++)
	//	{
	//		cv::Vec3s f = facemat2.at<cv::Vec3s>(j, i);
	//		cv::Vec3s m = maskmat2.at<cv::Vec3s>(j, i);
	//		cv::Vec3s c = colormat2.at<cv::Vec3s>(j, i);
	//		cv::Vec3s e = contourEdge.at<cv::Vec3s>(j, i);
	//		cv::Vec3s mb = maskblur.at<cv::Vec3s>(j, i);
	//		
	//		//原始MASK(没有形态学和BLUR处理前的) 黑色像素的位置填美术颜色贴图的值
	//		if (m[0] + m[1] + m[2] == 0)
	//		{

	//			/*facemat2.at<cv::Vec3s>(j, i)[0] = c[0];
	//			facemat2.at<cv::Vec3s>(j, i)[1] = c[1];
	//			facemat2.at<cv::Vec3s>(j, i)[2] = c[2];*/

	//			facemat2.at<cv::Vec3s>(j, i)[0] = 0;
	//			facemat2.at<cv::Vec3s>(j, i)[1] = 0;
	//			facemat2.at<cv::Vec3s>(j, i)[2] = 0;
	//		}
	//		//融合
	//		else
	//		{
	//			//facemat2.at<cv::Vec3s>(j, i)[0] = 1.0f / 255 * (f[0] * mb[0] + c[0] * (255 - mb[0]));// +e[0];
	//			//facemat2.at<cv::Vec3s>(j, i)[1] = 1.0f / 255 * (f[1] * mb[1] + c[1] * (255 - mb[1]));// +e[1];
	//			//facemat2.at<cv::Vec3s>(j, i)[2] = 1.0f / 255 * (f[2] * mb[2] + c[2] * (255 - mb[2]));// +e[2];

	//			facemat2.at<cv::Vec3s>(j, i)[0] = 1.0f / 255 * f[0] * m[0];
	//			facemat2.at<cv::Vec3s>(j, i)[1] = 1.0f / 255 * f[1] * m[1];
	//			facemat2.at<cv::Vec3s>(j, i)[2] = 1.0f / 255 * f[2] * m[2];
	//		}

	//	}
	//}
	



	//Seamless Cloning 
	facemat2.convertTo(facemat2, CV_8UC3);
	maskmat2.convertTo(maskmat2, CV_8UC3);
	colormat2.convertTo(colormat2, CV_8UC3);


	Point center(facemat2.cols/2, facemat2.rows /2);
	center = maskcenter;
	Mat dst;


	//Mat testsrc= imread("data/export/airplane.jpg");
	//Mat testm = Mat::zeros(testsrc.rows, testsrc.cols, testsrc.depth());

	//// Define the mask as a closed polygon
	//Point poly[1][7];
	//poly[0][0] = Point(4, 80);
	//poly[0][1] = Point(30, 54);
	//poly[0][2] = Point(151, 63);
	//poly[0][3] = Point(254, 37);
	//poly[0][4] = Point(298, 90);
	//poly[0][5] = Point(272, 134);
	//poly[0][6] = Point(43, 122);

	//const Point* polygons[1] = { poly[0] };
	//int num_points[] = { 7 };

	//// Create mask by filling the polygon

	//fillPoly(testm, polygons, num_points, 1, Scalar(255, 255, 255));
	//imwrite("data/export/testmask.jpg", testm);

	/*Mat test, testm;
	test = imread("data/export/apple.jpg");
	testm = imread("data/export/applemask.jpg");
	Mat whitemask(colormat2.cols, colormat2.rows,CV_8UC3,cv::Scalar(255,255,255));*/
	cv::seamlessClone(facemat2, colormat2, maskmat2, center, dst, cv::NORMAL_CLONE);
	
	facemat2.convertTo(facemat2, CV_16UC3);
	maskmat2.convertTo(maskmat2, CV_16UC3);
	colormat2.convertTo(colormat2, CV_16UC3);


	dst.convertTo(dst, CV_16UC3);



	//计算每一下像素值,变暗融合
	for (int j = 0; j < facemat2.rows; j++)
	{
		for (int i = 0; i < facemat2.cols; i++)
		{
			cv::Vec3s f = facemat2.at<cv::Vec3s>(j, i);
			cv::Vec3s m = maskmat2.at<cv::Vec3s>(j, i);
			cv::Vec3s c = colormat2.at<cv::Vec3s>(j, i);
			cv::Vec3s e = contourEdge.at<cv::Vec3s>(j, i);
			cv::Vec3s mb = maskblur.at<cv::Vec3s>(j, i);
			cv::Vec3s clone = dst.at<cv::Vec3s>(j, i);
			
			//原始MASK(没有形态学和BLUR处理前的) 黑色像素的位置填美术颜色贴图的值
			if (m[0] + m[1] + m[2] == 0)
			{

				facemat2.at<cv::Vec3s>(j, i)[0] = c[0];
				facemat2.at<cv::Vec3s>(j, i)[1] = c[1];
				facemat2.at<cv::Vec3s>(j, i)[2] = c[2];

				/*facemat2.at<cv::Vec3s>(j, i)[0] = 0;
				facemat2.at<cv::Vec3s>(j, i)[1] = 0;
				facemat2.at<cv::Vec3s>(j, i)[2] = 0;*/
			}
			//融合
			else
			{
				float darkerR = 0.75f;
				float darkerG = 0.7f;
				float darkerB = 0.75f;

				facemat2.at<cv::Vec3s>(j, i)[0] = 1.0f / 255 * (clone[0] * mb[0] * darkerR + c[0] * (255 - mb[0]));// +e[0];
				facemat2.at<cv::Vec3s>(j, i)[1] = 1.0f / 255 * (clone[1] * mb[1] * darkerG + c[1] * (255 - mb[1]));// +e[1];
				facemat2.at<cv::Vec3s>(j, i)[2] = 1.0f / 255 * (clone[2] * mb[2] * darkerB + c[2] * (255 - mb[2]));// +e[2];

				/*facemat2.at<cv::Vec3s>(j, i)[0] = 1.0f / 255 * f[0] * m[0];
				facemat2.at<cv::Vec3s>(j, i)[1] = 1.0f / 255 * f[1] * m[1];
				facemat2.at<cv::Vec3s>(j, i)[2] = 1.0f / 255 * f[2] * m[2];*/
			}

		}
	}
	//计算每一下像素值,变暗融合


	//cv::blur(facemat2, facemat2, cv::Size(10, 10));
	//cv::flip(facemat2, facemat2, 0);

	//保存成文件
	SaveTextureToFile(facemat2,GL_RGBA, photoPathOut, false);

	//清理内存
	SAFE_DELETE(faceptr);
	SAFE_DELETE(maskptr);
	//SAFE_DELETE(colorptr);
}



float getvalue(cv::Vec3s rgb) {

	float value;

	float r = rgb[0];
	float g = rgb[1];
	float b = rgb[2];

	if (r + g + b <= 1.5) {

		value = (r + g + b) / 1.5;

	}
	else {
		value = (3 - r - g - b) / 1.5;
	}

	return value;
}

void FaceCloudLib::getMean(Mat img, cv::Vec3s& left, cv::Vec3s& right)
{



	int n = 0;
	int m = 0;
	float mix_r_left = 0;
	float mix_g_left = 0;
	float mix_b_left = 0;

	float mix_r_right = 0;
	float mix_g_right = 0;
	float mix_b_right = 0;
	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			cv::Vec3s f = img.at<cv::Vec3s>(j, i);


			if (i < img.cols / 2)
			{
				if (f[0] + f[1] + f[2] >= 0.3f)
				{
					mix_r_left += f[0];
					mix_g_left += f[1];
					mix_b_left += f[2];
					n += 1;

				}
			}
			else
			{
				if (f[0] + f[1] + f[2] >= 0.3f)
				{
					mix_r_right += f[0];
					mix_g_right += f[1];
					mix_b_right += f[2];
					m += 1;

				}
			}
		}
	}


	//左右脸色彩平均值计算
	cv::Vec3s meancolorvalue_left(mix_r_left / n, mix_g_left / n, mix_b_left / n);
	cv::Vec3s meancolorvalue_right(mix_r_right / m, mix_g_right / m, mix_b_right / m);




	left = meancolorvalue_left;
	right = meancolorvalue_right;
}

cv::Vec3s lerp(cv::Vec3f start, cv::Vec3f end, cv::Vec3f rate)
{
	cv::Vec3f rt;
	rt = start + (end - start).mul(rate);
	return rt;
}


void FaceCloudLib::CombineTextureMaskBlend(Mat bgColor, Texture* pforeColor, string& photoPathOut)
{
	Mat bgr;
	unsigned char* forecolorptr;

	cv::Mat foreColorMat = GLTextureToMat(pforeColor->GetTextureObj(), forecolorptr);
	cv::Mat bgColormat = bgColor;
	cv::Mat bgfull;
	bgColor.copyTo(bgfull);

	bgfull.convertTo(bgfull, CV_16UC3);
	foreColorMat.convertTo(foreColorMat, CV_16UC3);
	bgColormat.convertTo(bgColormat, CV_16UC3);


	//MASK与FACE 渲染时是头向下的，所以FLIP一次
	cv::flip(foreColorMat, foreColorMat, 0);




	//取小块脸部区域
	cv::Range rowRg(270, 1070);
	cv::Range colRg(624, 1424);


	foreColorMat = foreColorMat(rowRg, colRg);
	bgColormat = bgColormat(rowRg, colRg);
	
	_Facemap_left_Mask = _Facemap_left_Mask(rowRg, colRg);
	_Facemap_right_Mask = _Facemap_right_Mask(rowRg, colRg);
	_BG_left_Mask = _BG_left_Mask(rowRg, colRg); 
	_BG_right_Mask = _BG_right_Mask(rowRg, colRg);
	_sampling_LF_mask = _sampling_LF_mask(rowRg, colRg); 
	_sampling_RT_mask = _sampling_RT_mask(rowRg, colRg); 
	_SampleButtom_LF_mask = _SampleButtom_LF_mask(rowRg, colRg); 
	_SampleButtom_RT_mask = _SampleButtom_RT_mask(rowRg, colRg);

	_Facemap_left_Mask.convertTo(_Facemap_left_Mask, CV_16UC3);
	_Facemap_right_Mask.convertTo(_Facemap_right_Mask, CV_16UC3);
	_BG_left_Mask.convertTo(_BG_left_Mask, CV_16UC3);
	_BG_right_Mask.convertTo(_BG_right_Mask, CV_16UC3);
	_sampling_LF_mask.convertTo(_sampling_LF_mask, CV_16UC3);
	_sampling_RT_mask.convertTo(_sampling_RT_mask, CV_16UC3);
	_SampleButtom_LF_mask.convertTo(_SampleButtom_LF_mask, CV_16UC3);
	_SampleButtom_RT_mask.convertTo(_SampleButtom_RT_mask, CV_16UC3);


	/*cv::cvtColor(foreColorMat, bgr, CV_RGBA2BGR);
	cv::imwrite("data/export/stepn1Mat.jpg", bgr);*/
	//Step 0
	Mat step0Mat;
	foreColorMat.copyTo(step0Mat);

	for (int j = 0; j < foreColorMat.rows; j++)
	{
		for (int i = 0; i < foreColorMat.cols; i++)
		{

			cv::Vec3s f = foreColorMat.at<cv::Vec3s>(j, i);
			cv::Vec3s ml = _sampling_LF_mask.at<cv::Vec3s>(j, i) / 255;
			cv::Vec3s mr = _sampling_RT_mask.at<cv::Vec3s>(j, i) / 255;

		

			Vec3s left = lerp(f, cv::Vec3s(0, 0, 0), ml);
			Vec3s right = lerp(f, cv::Vec3s(0, 0, 0), mr);


			cv::Vec3s c = left + right;

			step0Mat.at<cv::Vec3s>(j, i) = c;
		}
	}
	//cv::cvtColor(step0Mat, bgr, CV_RGBA2BGR);
	//cv::imwrite("data/export/step0Mat.jpg", bgr); 



	cv::Vec3s outcolor_left, outcolor_right;
	getMean(step0Mat, outcolor_left, outcolor_right);

	cv::Vec3s offset_left = m_targetcolor - outcolor_left;
	outcolor_left = offset_left + cv::Vec3s(255, 255, 255) / 2;


	cv::Vec3s offset_right = m_targetcolor - outcolor_right;
	outcolor_right = offset_right + cv::Vec3s(255, 255, 255) / 2;

	//step1
	Mat step1Mat;
	foreColorMat.copyTo(step1Mat);
	for (int j = 0; j < foreColorMat.rows; j++)
	{
		for (int i = 0; i < foreColorMat.cols; i++)
		{

			cv::Vec3s f = foreColorMat.at<cv::Vec3s>(j, i);
			cv::Vec3s ml = _SampleButtom_LF_mask.at<cv::Vec3s>(j, i) / 255;
			cv::Vec3s mr = _SampleButtom_RT_mask.at<cv::Vec3s>(j, i) / 255;


			Vec3s left = f + (outcolor_left - cv::Vec3s(255, 255, 255) / 2);
			left = lerp(left, cv::Vec3s(0, 0, 0), ml);


			Vec3s right = f + (outcolor_right - cv::Vec3s(255, 255, 255) / 2);
			right = lerp(right, cv::Vec3s(0, 0, 0), mr);

			cv::Vec3s c = left + right;

			step1Mat.at<cv::Vec3s>(j, i) = c;
		}
	}
	//cv::cvtColor(step1Mat, bgr, CV_RGBA2BGR);
	//cv::imwrite("data/export/step1Mat.jpg", bgr);

	/*cv::cvtColor(_Facemap_left_Mask, bgr, CV_RGBA2BGR);
	cv::imwrite("data/export/_Facemap_left_Mask.jpg", bgr);
	cv::cvtColor(_Facemap_right_Mask, bgr, CV_RGBA2BGR);
	cv::imwrite("data/export/_Facemap_right_Mask.jpg", bgr);
	cv::cvtColor(_BG_left_Mask, bgr, CV_RGBA2BGR);
	cv::imwrite("data/export/_BG_left_Mask.jpg", bgr);
	cv::cvtColor(_BG_right_Mask, bgr, CV_RGBA2BGR);
	cv::imwrite("data/export/_BG_right_Mask.jpg", bgr);*/

	cv::Vec3s outcolorbg_left, outcolorbg_right;
	getMean(step1Mat, outcolorbg_left, outcolorbg_right);


	cv::Vec3s offsetbg_left = outcolorbg_left - m_basecolor;
	outcolorbg_left = offsetbg_left + cv::Vec3s(255, 255, 255) / 2;


	cv::Vec3s offsetbg_right = outcolorbg_right - m_basecolor;
	outcolorbg_right = offsetbg_right + cv::Vec3s(255, 255, 255) / 2;

	//step2
	Mat step2Mat;
	foreColorMat.copyTo(step2Mat);
	for (int j = 0; j < foreColorMat.rows; j++)
	{
		for (int i = 0; i < foreColorMat.cols; i++)
		{

			cv::Vec4s bg4 = bgColormat.at<cv::Vec4s>(j, i);
			cv::Vec3s bg(bg4[0], bg4[1], bg4[2]);
			cv::Vec3s f = foreColorMat.at<cv::Vec3s>(j, i);
			cv::Vec3f _BG_left_Mask_var = cv::Vec3f(_BG_left_Mask.at<cv::Vec3s>(j, i)) / 255;
			cv::Vec3f _BG_right_Mask_var = cv::Vec3f(_BG_right_Mask.at<cv::Vec3s>(j, i)) / 255;

			cv::Vec3f _Facemap_left_Mask_var = cv::Vec3f(_Facemap_left_Mask.at<cv::Vec3s>(j, i)) / 255;
			cv::Vec3f _Facemap_right_Mask_var = cv::Vec3f(_Facemap_right_Mask.at<cv::Vec3s>(j, i)) / 255;


			Vec3s _Facemap_left_col = f + (outcolor_left - cv::Vec3s(255, 255, 255) / 2);
			Vec3s _Facemap_right_col = f + (outcolor_left - cv::Vec3s(255, 255, 255) / 2);

			Vec3s _BG_left_Mrg = bg + (outcolorbg_left - cv::Vec3s(255, 255, 255) / 2);
			Vec3s _BG_right_Mrg = bg + (outcolorbg_right - cv::Vec3s(255, 255, 255) / 2);

			Vec3s _BG_Mrg_left = lerp(_BG_left_Mrg, bg, _BG_left_Mask_var);
			Vec3s _BG_Mrgf = lerp(_BG_right_Mrg, _BG_Mrg_left, _BG_right_Mask_var);
			Vec3s _FACE_Mrg_left = lerp(_Facemap_left_col, _BG_Mrgf, _Facemap_left_Mask_var);
			Vec3s _FACE_Mrg = lerp(_Facemap_right_col, _FACE_Mrg_left, _Facemap_right_Mask_var);

			if (_FACE_Mrg[0] == 0 && _FACE_Mrg[1] == 0 && _FACE_Mrg[2] == 0)
			{
				int xxx = 0;
			}

			step2Mat.at<cv::Vec3s>(j, i) = _FACE_Mrg;
		}
	}
	//cv::cvtColor(step2Mat, bgr, CV_RGBA2BGR);
	//cv::imwrite("data/export/step2Mat.jpg", bgr);



	//测试全图
	for (int j = 0; j < bgfull.rows; j++)
	{
		for (int i = 0; i < bgfull.cols; i++)
		{

			if (i >= colRg.start && i < colRg.end && j >= rowRg.start &&j < rowRg.end)
			{
				cv::Vec3s f = step2Mat.at<cv::Vec3s>(j - rowRg.start, i - colRg.start);
				bgfull.at<cv::Vec4s>(j, i) = cv::Vec4s(f[0],f[1],f[2],255);

			}

		}
	}
	cv::cvtColor(bgfull, bgr, CV_RGBA2BGR);
	cv::imwrite("data/export/bgfull.jpg", bgr);

	//保存成文件
	SaveTextureToFile(step2Mat, GL_RGBA, photoPathOut, false);

	//清理内存
	SAFE_DELETE(forecolorptr);
	//SAFE_DELETE(colorptr);
}

bool FaceCloudLib::InitCamera()
{
	Vector3f Pos(0.0f, 0, 0);
	Vector3f Target(0.0f,0.0f, 1.0f);
	Vector3f Up(0.0, 1.0f, 0.0f);



	m_persProjInfo.FOV = 60.0f;
	m_persProjInfo.Height = m_Width;
	m_persProjInfo.Width = m_Height;
	m_persProjInfo.zNear = 0.1f;
	m_persProjInfo.zFar = 10000.0f;

	m_orthoProjInfo.b = -20;
	m_orthoProjInfo.t = 20;
	m_orthoProjInfo.l = -20;
	m_orthoProjInfo.r = 20;
	m_orthoProjInfo.n = -99999;
	m_orthoProjInfo.f = 99999;


	m_pGameCamera = new Camera(m_Width, m_Height, Pos, Target, Up);

	printf("\nStart UnlitSkinningTechnique init\n");


	m_pSkinningRenderer = new UnlitSkinningTechnique(); 
	if (!m_pSkinningRenderer->Init()) {
		printf("Error initializing the UnlitSkinningTechnique\n");
		return false;
	}
	m_pSkinningRenderer->Enable();
	m_pSkinningRenderer->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
	m_pSkinningRenderer->SetDetailTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 1);
	m_pSkinningRenderer->SetMaskTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 2);


	m_pSkinningMaskRenderer = new MaskSkinningTechnique();
	if (!m_pSkinningMaskRenderer->Init()) {
		printf("Error initializing the UnlitSkinningTechnique\n");
		return false;
	}
	m_pSkinningMaskRenderer->Enable();
	m_pSkinningMaskRenderer->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
	m_pSkinningMaskRenderer->SetDetailTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 1);
	m_pSkinningMaskRenderer->SetMaskTextureUnit(COLOR_TEXTURE_UNIT_INDEX + 2);




	printf("\nStart CommonTechnique init\n");
	m_pCommonRenderer = new CommonTechnique();
	if (!m_pCommonRenderer->Init()) {
		printf("Error initializing the CommonTechnique\n");
		return false;
	}

	return true;
}
bool FaceCloudLib::InitMesh()
{
	vector<string> modelIDs;
	modelIDs.push_back("10001");
	modelIDs.push_back("10002");

	for (vector<string>::iterator iter = modelIDs.begin();iter != modelIDs.end();iter++)
	{
		SkinnedMesh* pmesh = new SkinnedMesh;
		if (!pmesh->LoadMesh(RES_PATH + "facecloud/" + *iter + ".fbx")) {
			printf("Mesh load failed\n");
			return false;
		}

		m_MeshMap[*iter] = pmesh;

		Texture* ptexture = new Texture(GL_TEXTURE_2D);
		if (!ptexture->LoadFile(RES_PATH + "facecloud/" + *iter + ".jpg")) {
			return false;
		}
		m_ColorTextureMap[*iter] = ptexture;
	}
	m_pMaskTexture = new Texture(GL_TEXTURE_2D);
	if (!m_pMaskTexture->LoadFile(RES_PATH + string("facecloud/mask.jpg"))) {
		return false;
	}
	

	return true;
}
bool FaceCloudLib::InitJson()
{
	m_BoneUtility.Init();
	m_JsonRoles.LoadFromFile(RES_PATH + string("facecloud/model_offset.json"));

	return true;
}
bool FaceCloudLib::InitMat()
{
	_Facemap_left_Mask = cv::imread(RES_PATH + "facecloud/face_mask_left.jpg");
	_Facemap_right_Mask = cv::imread(RES_PATH + "facecloud/face_mask_right.jpg");
	_BG_left_Mask = cv::imread(RES_PATH + "facecloud/mask_left.jpg");
	_BG_right_Mask = cv::imread(RES_PATH + "facecloud/mask_right.jpg");
	_sampling_LF_mask = cv::imread(RES_PATH + "facecloud/samplface_LF.jpg");
	_sampling_RT_mask = cv::imread(RES_PATH + "facecloud/samplface_RT.jpg");
	_SampleButtom_LF_mask = cv::imread(RES_PATH + "facecloud/samplbuttom_LF.jpg");
	_SampleButtom_RT_mask = cv::imread(RES_PATH + "facecloud/samplbuttom_RT.jpg");




	return true;
}



bool FaceCloudLib::CreateRenderTarget()
{
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	
	glGenFramebuffers(1, &m_FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferName);


	// The texture we're going to render to
	glGenTextures(1, &m_RenderTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);


	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool FaceCloudLib::BeginRenterTexture()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferName);
		glViewport(0, 0, m_Width, m_Height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

											 // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
	return false;
}
void FaceCloudLib::EndRenderTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glutSwapBuffers();
}
void FaceCloudLib::CalculateBone(string modelID, JsonFaceInfo jsonfaceinfo, string& photoPathOut, string& jsonModelOut, Vector3f& centerpos, Vector2f& uvsize,float& yOffset)
{

	SkinnedMesh* pmesh = m_MeshMap[modelID];
	m_BoneUtility.CalculateFaceBone(pmesh, m_JsonRoles.roles[modelID], jsonfaceinfo, jsonModelOut, centerpos,uvsize,yOffset);
}
bool FaceCloudLib::DrawOnce(string modelID,Vector3f& center,Vector2f& uvsize)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Lastcenter = center;
	m_Lastuvsize = uvsize;

	m_pGameCamera->OnRender();
	// Always check that our framebuffer is ok

	Pipeline p;
	p.WorldPos(0.0f, 0, 0.0f);
	p.Rotate(0.0f, 180.0f, 0.0f);
	p.Scale(1, 1, 1);
	p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());

	m_orthoProjInfo.b = -uvsize.x/2;
	m_orthoProjInfo.t = uvsize.x / 2;
	m_orthoProjInfo.l = -uvsize.y / 2;
	m_orthoProjInfo.r = uvsize.y / 2;

	p.SetOrthographicProj(m_orthoProjInfo);
	p.SetPerspectiveProj(m_persProjInfo);


	if (m_pSkinningRenderer)
	{
		m_pSkinningRenderer->Enable();

		if (m_bRenderToTexture)
		{
			m_pSkinningRenderer->SetWVP(p.GetWVOrthoPTrans());
		}
		else
		{
			m_pSkinningRenderer->SetWVP(p.GetWVPTrans());

		}
	}


	/*m_pCommonRenderer->Enable();
	m_pCommonRenderer->SetWVP(p.GetWVPTrans());*/
	if (m_MeshMap.find(modelID) != m_MeshMap.end())
	{
		if (m_ColorTextureMap.find(modelID) != m_ColorTextureMap.end())
		{
			m_ColorTextureMap[modelID]->Bind(GL_TEXTURE0);
			//m_ColorTextureMap[modelID]->Bind(GL_TEXTURE1);
			if (m_pCurrentSkinTexture != NULL)
			{
				m_pCurrentSkinTexture->Bind(GL_TEXTURE1);
			}
			if (m_pMaskTexture != NULL)
			{
				m_pMaskTexture->Bind(GL_TEXTURE2);
			}

			SkinnedMesh* pmesh = m_MeshMap[modelID];
			vector<Matrix4f> Transforms;
			pmesh->BoneTransform(0, Transforms);
			for (uint i = 0; i < Transforms.size(); i++) {

				if (m_pSkinningRenderer)
				{
					m_pSkinningRenderer->SetBoneTransform(i, Transforms[i]);
				}
			}
			pmesh->Render();
		}
	}

	return true;

}



bool FaceCloudLib::DrawMaskOnce(string modelID, Vector3f& center, Vector2f& uvsize)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Lastcenter = center;
	m_Lastuvsize = uvsize;

	m_pGameCamera->OnRender();
	// Always check that our framebuffer is ok

	Pipeline p;
	p.WorldPos(0.0f, 0, 0.0f);
	p.Rotate(0.0f, 180.0f, 0.0f);
	p.Scale(1, 1, 1);
	p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());

	m_orthoProjInfo.b = -uvsize.x / 2;
	m_orthoProjInfo.t = uvsize.x / 2;
	m_orthoProjInfo.l = -uvsize.y / 2;
	m_orthoProjInfo.r = uvsize.y / 2;

	p.SetOrthographicProj(m_orthoProjInfo);
	p.SetPerspectiveProj(m_persProjInfo);


	if (m_pSkinningMaskRenderer)
	{
		m_pSkinningMaskRenderer->Enable();

		if (m_pSkinningMaskRenderer)
		{
			m_pSkinningMaskRenderer->SetWVP(p.GetWVOrthoPTrans());
		}
		else
		{
			m_pSkinningMaskRenderer->SetWVP(p.GetWVPTrans());

		}
	}


	/*m_pCommonRenderer->Enable();
	m_pCommonRenderer->SetWVP(p.GetWVPTrans());*/
	if (m_MeshMap.find(modelID) != m_MeshMap.end())
	{
		if (m_ColorTextureMap.find(modelID) != m_ColorTextureMap.end())
		{
			m_ColorTextureMap[modelID]->Bind(GL_TEXTURE0);
			//m_ColorTextureMap[modelID]->Bind(GL_TEXTURE1);
			if (m_pCurrentSkinTexture != NULL)
			{
				m_pCurrentSkinTexture->Bind(GL_TEXTURE1);
			}
			if (m_pMaskTexture != NULL)
			{
				m_pMaskTexture->Bind(GL_TEXTURE2);
			}

			SkinnedMesh* pmesh = m_MeshMap[modelID];
			vector<Matrix4f> Transforms;
			pmesh->BoneTransform(0, Transforms);
			for (uint i = 0; i < Transforms.size(); i++) {

				if (m_pSkinningRenderer)
				{
					m_pSkinningRenderer->SetBoneTransform(i, Transforms[i]);
				}
			}
			pmesh->Render();
		}
	}

	return true;

}

cv::Mat FaceCloudLib::GLTextureToMat(GLuint texture, unsigned char*& outimagptr)
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

	long size = 0;

	if (comp == GL_RGB)
	{

		size = wtex * htex * 3;
	}
	else if (comp == GL_RGBA)
	{
		size = wtex * htex * 4;
	}

	outimagptr = new unsigned char[size];

	glGetTexImage(GL_TEXTURE_2D, 0, comp, GL_UNSIGNED_BYTE, outimagptr);


	GLenum error = glGetError();
	const GLubyte * eb = gluErrorString(error);
	string errorstring((char*)eb);

	cv::Mat  img;

	if (comp == GL_RGB)
	{
		img = cv::Mat(htex, wtex, CV_8UC3, (unsigned*)outimagptr);
	}
	else if (comp == GL_RGBA)
	{
		img = cv::Mat(htex, wtex, CV_8UC4, (unsigned*)outimagptr);
	}


	return img;
}
void FaceCloudLib::SaveTextureToFile(cv::Mat imag, int format,string path,bool flip)
{
	//WriteTGA((char*)"data/export/rendertexture.tga", wtex, htex, output_image);

	cv::Mat  bgra;

	if (format == GL_RGB)
	{
		cv::cvtColor(imag, bgra, cv::COLOR_RGB2BGRA);
	}
	else if (format == GL_RGBA)
	{
		cv::cvtColor(imag, bgra, cv::COLOR_RGBA2BGRA);
	}

	if (flip)
	{
		cv::Mat flipimg;
		/*	flipCode	Anno
		1	ˮƽ��ת
		0	��ֱ��ת
		- 1	ˮƽ��ֱ��ת*/
		cv::flip(bgra, flipimg, 0);
		cv::imwrite(path, flipimg);
	}
	else
	{

		cv::imwrite(path, bgra);
	}

}

void FaceCloudLib::DisplayGrid()
{
	Pipeline p;
	p.WorldPos(0.0f, 0, 0.0f);
	p.Rotate(0.0f, 0.0f, 0.0f);
	p.Scale(1, 1, 1);
	p.SetOrthographicProj(m_orthoProjInfo);
	p.SetPerspectiveProj(m_persProjInfo);
	p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
	m_pCommonRenderer->Enable();
	m_pCommonRenderer->SetWVP(p.GetWVPTrans());


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

}
void FaceCloudLib::SaveFile(string& s, string& path)
{
	ofstream write;

	write.open(path.c_str(), ios::out | ios::binary);
	write.write(s.c_str(), s.length());
	write.close();
}
void FaceCloudLib::SaveJsonFile(Json::Value jvalue, string& path)
{
	Json::StreamWriterBuilder  builder;
	builder.settings_["commentStyle"] = "All";
	std::string s = Json::writeString(builder, jvalue);

	SaveFile(s, path);
}
Json::Value FaceCloudLib::LoadJsonValueFromFile(string filepath)
{
	Json::CharReaderBuilder rbuilder;
	rbuilder["collectComments"] = false;
	std::string errs;
	Json::Value root;
	std::ifstream ifs;
	ifs.open(filepath);
	bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
	if (ok)
	{
		printf("\nLoadJsonValueFromFile Ok :  %s", filepath.c_str());
	}
	else
	{

		printf("\nLoadJsonValueFromFile Failed  : %s", filepath.c_str());
	}
	ifs.close();
	return root;
}
string FaceCloudLib::LoadJsonStringFromFile(string filepath)
{
	//printf("\nLoadJSF Path:%s",filepath.c_str());
	string path = filepath;
	Json::Value root = LoadJsonValueFromFile(path);

	Json::StreamWriterBuilder  builder;
	builder.settings_["commentStyle"] = "All";
	std::string s = Json::writeString(builder, root);


	//printf("\nLoadJSF:%s",s.c_str());
	return s;
}