#pragma once

#include <stdio.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"

#include "ogldev_util.h"
#include "ogldev_glut_backend.h"
#include "ogldev_pipeline.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/core/core.hpp> 
#include "FacePlusWrapper.h"
#include "BoneUtility.h"
using namespace std;
using namespace cv;

#define MATH_PI  3.14159265358979323846264338327950288419716939937510
class Matrix2f;

enum SKINCOLORTYPE {
	SCT_WHITE = 0,
	SCT_BLOND,
	SCT_NATURE,
	SCT_WHEAT,
	SCT_DARK,
	SCT_BLACK,
	SCT_MAX,
};

class  JsonSkinStatus
{
public:
	JsonSkinStatus()
	{

	};
	~JsonSkinStatus()
	{

	};
	void SetValue(string key, float value)
	{
		root[key] = value;
	}
	void SetValue(string key, string value)
	{
		root[key] = value;
	}
	void Save(string& path)
	{
		SaveJsonFile(root, path);
	}
	string ToString()
	{
		Update();
		Json::StreamWriterBuilder  builder;
		builder.settings_["commentStyle"] = "All";
		std::string s = Json::writeString(builder, root);
		return s;
	}

	//变量值写入JSON
	void Update()
	{
		SetValue("gender", gender);
		SetValue("beauty_female", beauty_female);
		SetValue("beauty_male", beauty_male);
		SetValue("age", age);
		SetValue("dark_circle", dark_circle);
		SetValue("stain", stain);
		SetValue("acne", acne);
		SetValue("health", health);


		SetValue("blackhead", blackhead);
		SetValue("skincolor", skincolor);
	}

	//性别 Male Female
	string gender;

	//颜值识别结果 女 范围 [0,100]，小数点后 3 位有效数字
	float beauty_female;


	//颜值识别结果 男 范围 [0,100]，小数点后 3 位有效数字
	float beauty_male;

	//年龄 岁
	float age;

	//黑眼圈 范围 [0,100]，小数点后 3 位有效数字
	float dark_circle;

	//色斑 范围 [0,100]，小数点后 3 位有效数字
	float stain;

	//青春痘 范围 [0,100]，小数点后 3 位有效数字
	float acne;

	//健康 范围 [0,100]，小数点后 3 位有效数字
	float health;

	//黑头 范围 [0,100]，小数点后 3 位有效数字
	float blackhead;

	//肤色 6种颜色INDEX值 0 - 5
	SKINCOLORTYPE skincolor;
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
		string s = ToString();
		SaveFile(s, path);
	}

	Json::Value root;
	

};
class ImageOptimizedUtility
{

	CascadeClassifier _cascade;

public:
	void Init();
	cv::Mat UpdateDermabrasion(cv::Mat img, float v1, float v2 = 0);

	vector<Rect> detectFace(Mat src);


	void UpdateRef_RGB(JsonFaceInfo& faceinfo,Mat img, Vector3f refcolor, float value, Mat& outputimg, Vector2f leftpoint, Vector2f rightpoint);


	void ColorTransfer(Mat src, Mat ref, Mat& outputimg, JsonFaceInfo &faceinfo ,bool isFrontOrBg = true);
	Mat FacePhotoProcess_(JsonFaceInfo& faceinfo, JsonRole bonedef, Mat src32);
	Mat FacePhotoProcess(JsonFaceInfo& faceinfo, JsonRole bonedef, Mat src32);
	

	void DetectSkinStatus(Mat src, JsonFaceInfo faceinfo, JsonSkinStatus& skinjson);
	bool findPimples(Mat img);
	float findBlackHead(Mat img);

	void SaveTextureToFile(cv::Mat imag, int format, string path, bool flip = false)
	{
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
			cv::flip(bgra, flipimg, 0);
			cv::imwrite(path, flipimg);
		}
		else
		{

			cv::imwrite(path, bgra);
		}

	}

	Point ApplyAffineMat(Mat& affineMat, Point srcPt)
	{

		double m00 = affineMat.at<double>(0, 0);
		double m01 = affineMat.at<double>(0, 1);
		double m02 = affineMat.at<double>(0, 2);
		double m10 = affineMat.at<double>(1, 0);
		double m11 = affineMat.at<double>(1, 1);
		double m12 = affineMat.at<double>(1, 2);

		Mat_<double> M(3, 3);
		M << m00, m01, m02,
			m10, m11, m12,
			0, 0, 1;

		Mat_<double> pm(3, 1);
		pm << srcPt.x, srcPt.y, 1.0;

		Mat_<double> pr = M * pm;

		double rm00 = pr.at<double>(0, 0);
		double rm10 = pr.at<double>(1, 0);
		double rm20 = pr.at<double>(2, 0);

		return Point2f(rm00, rm10);
	}

private:
	float VectorAngle(Vector2f from, Vector2f to)
	{
		float angle;

		Vector3f fromv3 = Vector3f(from.x, from.y, 0);
		Vector3f tov3 = Vector3f(to.x, to.y, 0);

		Vector3f cross = fromv3.Cross(tov3);

		angle = from.angle_rad(to);

		angle = 180 / MATH_PI * angle;
		return cross.z > 0 ? angle : -angle;
	}
};




