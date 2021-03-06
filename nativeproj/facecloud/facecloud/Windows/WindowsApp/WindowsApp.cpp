#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
#include "ImageOptimizedUtility.h"

#include <opencv2/core/core.hpp>
#include <json/json.h>
#include <fstream>

using namespace cv;

/** Global Variables */
const int alpha_slider_max = 500;
const int alpha_slider_divide = 100;
int alpha_slider = 500;
double alpha;
double beta;

/** Matrices to store images */
Mat src1;
Mat src2;
Mat dst;


ImageOptimizedUtility gUtility;

static void on_trackbar(int, void*)
{
	alpha = (double)alpha_slider / alpha_slider_divide;

	beta = (1.0 - alpha);

	dst = gUtility.UpdateDermabrasion(src1, alpha);

	imshow("Linear Blend", dst);
}
int main_()
{

	Json::CharReaderBuilder rbuilder;
	rbuilder["collectComments"] = false;
	std::string errs;
	Json::Value root;
	std::ifstream ifs;
	ifs.open("data/facejson.txt");
	bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
	ifs.close();


	Json::Value landmark = root["landmark"]["data"];
	Json::Value::Members members = landmark.getMemberNames();

	std::vector<std::string> memberNames;

	for (int i = 0 ; i < members.size() ; i++)
	{
		std::string membername = members[i];
		memberNames.push_back(membername);
	}

	//![load]
	/// Read images ( both have to be of the same size and type )
	src1 = imread("data/faceimg.jpg");
	src2 = imread("data/faceimg.jpg");
	//![load]

	if (src1.empty()) { printf("Error loading src1 \n"); return -1; }
	if (src2.empty()) { printf("Error loading src2 \n"); return -1; }

	/// Initialize values
	alpha_slider = 500;

	//![window]
	namedWindow("Linear Blend", WINDOW_AUTOSIZE); // Create Window
												  //![window]

												  //![create_trackbar]
	char TrackbarName[50];
	sprintf(TrackbarName, "input value1 %d", alpha_slider_max);
	createTrackbar(TrackbarName, "Linear Blend", &alpha_slider, alpha_slider_max, on_trackbar);
	//![create_trackbar]

	/// Show some stuff
	on_trackbar(alpha_slider, 0);

	/// Wait until user press some key
	waitKey(0);
	return 0;
}

