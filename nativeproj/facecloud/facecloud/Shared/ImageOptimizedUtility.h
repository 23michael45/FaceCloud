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
using namespace std;
using namespace cv;

class ImageOptimizedUtility
{

	CascadeClassifier _cascade;

public:
	void Init();
	cv::Mat UpdateDermabrasion(cv::Mat img, float v1, float v2 = 0);

	vector<Rect> detectFace(Mat src);


	void UpdateRef_RGB(Mat img, Vector3f refcolor, float value, Mat& outputimg, Vector2f leftpoint, Vector2f rightpoint);

	Vector3f UpdateRefSkin(Mat inputTexture, Vector3f ref_RGB, float value, Mat& outputTexture, Vector2f leftpoint, Vector2f rightpoint);




};
