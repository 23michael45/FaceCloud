#pragma once
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include <stdio.h>

using namespace std;
using namespace cv;

class ImageOptimizedUtility
{

	CascadeClassifier _cascade;

public:
	void Init();
	cv::Mat UpdateDermabrasion(cv::Mat img, float v1, float v2 = 0);

	vector<Rect> detectFace(Mat src);


	void UpdateRef_RGB(Mat img, cv::Vec3f refcolor, float value, cv::Vec2f leftpoint, cv::Vec2f rightpoint);
};
