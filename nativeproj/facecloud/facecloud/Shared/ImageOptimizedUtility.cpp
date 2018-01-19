#include "ImageOptimizedUtility.h"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/core/core.hpp> 
#include <stdio.h>
#include <iostream>

void ImageOptimizedUtility::Init()
{
	string cascadeName = "data/haarcascades/haarcascade_frontalface_alt.xml";
	if (!_cascade.load(cascadeName))
	{
		cerr << "ERROR: Could not load classifier cascade" << endl;
		return;
	}

}

cv::Mat ImageOptimizedUtility::UpdateDermabrasion(cv::Mat img, float v1, float v2)
{
	try
	{

		cv::Mat srcImg;
		img.convertTo(srcImg,CV_32FC3);

		cv::Mat highpassImg;
		cv::Mat frontpassImg;

		srcImg.copyTo(highpassImg);
		srcImg.copyTo(frontpassImg);



		float dx = v1 * 5;    //双边滤波参数之一  
		double fc = v1 * 12.5f; //双边滤波参数之一  
		float alpha = 50; //透明度  

						  //双边滤波  
		int i = 409;
		int j = 409;

		cv::Mat bilateralImg;

		cv::bilateralFilter(highpassImg, bilateralImg, (int)dx, fc, fc);
		bilateralImg.copyTo(highpassImg);


		highpassImg -= srcImg;
		highpassImg += cv::Scalar(128, 128, 128);



		cv::GaussianBlur(highpassImg, highpassImg, cv::Size(2 * v2 + 1, 2 * v2 + 1), 0, 0);


		highpassImg = highpassImg * 2;

		highpassImg = srcImg + highpassImg;
		highpassImg = highpassImg - cv::Scalar(255, 255, 255);
		highpassImg = highpassImg * alpha;

		frontpassImg = frontpassImg * (100 - alpha);




		srcImg = frontpassImg + highpassImg;
		srcImg /= 100;

		//cv::Vec3f color = srcImg.at<cv::Vec3f>(srcImg.cols / 2, srcImg.rows / 2);


		cv::Mat finalmat;

		srcImg.convertTo(finalmat, CV_8UC3);
		return finalmat;
	}
	catch (const std::exception &e)
	{
		char TrackbarName[50];
		sprintf(TrackbarName, "%s", e.what());
	}


	return img;
}

vector<Rect> ImageOptimizedUtility::detectFace(Mat src)
{

	Mat gray, smallImg;
	cvtColor(src, gray, COLOR_BGR2GRAY);;//因为用的是类haar特征，所以都是基于灰度图像的，这里要转换成灰度图像

															//Imgproc.resize(temp8uc1, temp8uc1, temp8uc1.size(), 0, 0, Imgproc.INTER_LINEAR);//将尺寸缩小到1/scale,用线性插值
															//Imgproc.equalizeHist(temp8uc1, temp8uc1);//直方图均衡
	float scale = 1;
	double fx = 1 / scale;
	resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
	equalizeHist(smallImg, smallImg);


	vector<Rect> faces;
	_cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CASCADE_FIND_BIGGEST_OBJECT
		//|CASCADE_DO_ROUGH_SEARCH
		| CASCADE_SCALE_IMAGE,
		Size(30, 30));
	return faces;
}

void ImageOptimizedUtility::UpdateRef_RGB(Mat img, cv::Vec3f refcolor, float value, cv::Vec2f leftpoint, cv::Vec2f rightpoint)
{
	//主画面分离

	vector<Mat> main_channel;
	split(img, main_channel);

	Mat mainRedChannel = main_channel[0];
	Mat mainGreenChannel = main_channel[1];
	Mat mainBlueChannel = main_channel[2];

	Mat image_LF;
	Mat image_RT;

	img.copyTo(image_LF);
	img.copyTo(image_RT);


	//左侧取样

	Vec2f eyedown_pos1 = rightpoint;
	Vec2f face_pos1 = leftpoint;

	Point left_1(face_pos1[0], face_pos1[1]);
	Point right_1(eyedown_pos1[0], eyedown_pos1[1]);

	Mat subimg_1 = img(Range((int)left_1.y, (int)right_1.y), Range((int)left_1.x, (int)right_1.x));
	Mat subimgcopy_1;
	subimg_1.copyTo(subimgcopy_1);

	//采样分离
	vector<Mat> LF_vChannels;
	split(subimgcopy_1, LF_vChannels);

	Mat imageRedChannel = LF_vChannels[0];
	Mat imageGreenChannel = LF_vChannels[1];
	Mat imageBlueChannel = LF_vChannels[2];

	Scalar imageBlueChannelAvg_LF = mean(imageBlueChannel);
	Scalar imageGreenChannelAvg_LF = mean(imageGreenChannel);
	Scalar imageRedChannelAvg_LF = mean(imageRedChannel);





	//右侧取样
	
	double left2_x = img.rows - face_pos1[0];
	double right2_x = img.rows - eyedown_pos1[0];

	Point left_2(left2_x, left_1.y);
	Point right_2(right2_x, right_1.y);
	
	//第1次调色 

	Mat subimg_2 = img(Range((int)left_2.y, (int)right_2.y), Range((int)right_2.x, (int)left_2.x));
	Mat subimgcopy_2;
	subimg_2.copyTo(subimgcopy_2);


	vector<Mat> RT_vChannels;
	split(subimgcopy_2, RT_vChannels);

	Mat imageRedChannel_RT = RT_vChannels[0];
	Mat imageGreenChannel_RT = RT_vChannels[1];
	Mat imageBlueChannel_RT = RT_vChannels[2];



	Scalar imageRedChannelAvg_RT = mean(imageRedChannel_RT);
	Scalar imageGreenChannelAvg_RT = mean(imageGreenChannel_RT);
	Scalar imageBlueChannelAvg_RT = mean(imageBlueChannel_RT);


	double r_avg_LF = imageRedChannelAvg_LF.val[0];
	double g_avg_LF = imageGreenChannelAvg_LF.val[0];
	double b_avg_LF = imageBlueChannelAvg_LF.val[0];


	double r_avg_RT = imageRedChannelAvg_RT.val[0];
	double g_avg_RT = imageGreenChannelAvg_RT.val[0];
	double b_avg_RT = imageBlueChannelAvg_RT.val[0];
	


	//double Red_offest   ;
	//double Green_offest ;
	//double Blue_offest;

	double Red_offest_LF = refcolor[0] - r_avg_LF;
	double Green_offest_LF = refcolor[1] - g_avg_LF;
	double Blue_offest_LF = refcolor[2] - b_avg_LF;
	double aphaf = 0;



	double Red_offest_RT = refcolor[0] - r_avg_RT;
	double Green_offest_RT = refcolor[1] - g_avg_RT;
	double Blue_offest_RT = refcolor[2] - b_avg_RT;
	//左侧调色


	for (int col = 0; col < img.cols; col++)
	{


		for (int row = 0; row < img.rows; row++)
		{
			Vec3f rgb = img.at<Vec3f>(row, col);

			double Red = rgb[0];
			double Green = rgb[1];
			double Blue = rgb[2];


			double light_img = rgb[0] + rgb[1] + rgb[2];
			double light_avg = r_avg_LF + g_avg_LF + b_avg_LF;
			//double light_avg;
			int min_light = 100;


			//if (col < img.cols() / 3)
			//{

			//    Red_offest_LF = refcolor.x - imageRedChannelAvg_LF.val[0] ;
			//    Green_offest_LF = refcolor.y - imageGreenChannelAvg_LF.val[0];
			//    Blue_offest_LF = refcolor.z - imageBlueChannelAvg_LF.val[0] ;

			//    light_avg = imageRedChannelAvg_LF.val[0] + imageGreenChannelAvg_LF.val[0] + imageBlueChannelAvg_LF.val[0];

			//}

			//else if (col >= img.cols() / 3 && col < img.cols() * 2 / 3) {



			//    Red_offest_LF = refcolor.x - imageRedChannelAvg_LF.val[0] *(col*3 / img.cols());
			//    Green_offest_LF = refcolor.y - imageGreenChannelAvg_LF.val[0] * (col * 3 / img.cols());
			//    Blue_offest_LF = refcolor.z - imageBlueChannelAvg_LF.val[0] * (col * 3 / img.cols());

			//    light_avg = imageRedChannelAvg_LF.val[0] + imageGreenChannelAvg_LF.val[0] + imageBlueChannelAvg_LF.val[0];


			//}                

			//else
			//{

			//    Red_offest_LF = refcolor.x - imageRedChannelAvg_RT.val[0] / 2;
			//    Green_offest_LF = refcolor.y - imageGreenChannelAvg_RT.val[0] / 2;
			//    Blue_offest_LF = refcolor.z - imageBlueChannelAvg_RT.val[0] / 2;
			//    light_avg = imageRedChannelAvg_RT.val[0] + imageGreenChannelAvg_RT.val[0] + imageBlueChannelAvg_RT.val[0];

			//}



			if (col > 200 && col <= 569 && row <= 1024 / 255 * (col - 200) || col > 569 && col < 824 && row <= 1024 / 255 * (824 - col))
			{
				//Red = 0;
				//Green = 0;
				//Blue = 0;

				if (light_img < min_light)
				{
					Red = rgb[0];
					Green = rgb[1];
					Blue = rgb[2];

				}
				else if (light_img >= min_light && light_img < light_avg)
				{

					Red = rgb[0] + Red_offest_LF * ((light_img - min_light) / (light_avg - min_light));
					Green = rgb[1] + Green_offest_LF * ((light_img - min_light) / (light_avg - min_light));
					Blue = rgb[2] + Blue_offest_LF * ((light_img - min_light) / (light_avg - min_light));

				}
				else
				{
					Red = rgb[0] + Red_offest_LF;
					Green = rgb[1] + Green_offest_LF;
					Blue = rgb[2] + Blue_offest_LF;
				}
			}
			else if (col > 100 && col <= 569 && row > 1024 / 255 * (col - 200) && row < 1024 / 255 * (col - 100)
				|| col > 569 && col < 934 && row > 1024 / 255 * (824 - col) && row < 1024 / 255 * (934 - col))

			{


				float value_L;
				if (col < 512)
				{
					value_L = (col - (row * 255.0f / 1024.0f) - 100) / 100.0f;
				}
				else
				{
					value_L = (934 - (row * 255.0f / 1024.0f) - col) / 100.0f;
				}


				if (light_img < min_light)
				{
					Red = rgb[0] * value_L + (rgb[0] + Red_offest_LF) * (1 - value_L);
					Green = rgb[1] * value_L + (rgb[1] + Green_offest_LF) * (1 - value_L);
					Blue = rgb[2] * value_L + (rgb[2] + Blue_offest_LF) * (1 - value_L);

				}
				else if (light_img >= min_light && light_img < light_avg)
				{



					double Red_1 = rgb[0] + Red_offest_LF * ((light_img - min_light) / (light_avg - min_light));
					double Green_1 = rgb[1] + Green_offest_LF * ((light_img - min_light) / (light_avg - min_light));
					double Blue_1 = rgb[2] + Blue_offest_LF * ((light_img - min_light) / (light_avg - min_light));

					Red = Red_1 * value_L + (rgb[0] + Red_offest_LF) * (1 - value_L);
					Green = Green_1 * value_L + (rgb[1] + Green_offest_LF) * (1 - value_L);
					Blue = Blue_1 * value_L + (rgb[2] + Blue_offest_LF) * (1 - value_L);
				}
				else
				{
					Red = rgb[0] + Red_offest_LF;
					Green = rgb[1] + Green_offest_LF;
					Blue = rgb[2] + Blue_offest_LF;
				}
			}
			else
			{   //Red = 255;
				//Green = 255;
				//Blue = 255;

				Red = rgb[0] + Red_offest_LF;
				Green = rgb[1] + Green_offest_LF;
				Blue = rgb[2] + Blue_offest_LF;
			}

			rgb[0] = Red;
			rgb[1] = Green;
			rgb[2] = Blue;

			image_LF.at<Vec3f>(row, col) = rgb;

		}

	}


	//右侧调色

	for (int col = 0; col < img.cols; col++)
	{


		for (int row = 0; row < img.rows; row++)
		{
			Vec3f rgb = img.at<Vec3f>(row, col);

			double Red = rgb[0];
			double Green = rgb[1];
			double Blue = rgb[2];


			double light_img = rgb[0] + rgb[1] + rgb[2];
			double light_avg = r_avg_RT + g_avg_RT + b_avg_RT;

			int min_light = 100;


			if (col > 200 && col <= 569 && row <= 1024 / 255 * (col - 200) || col > 569 && col < 824 && row <= 1024 / 255 * (824 - col))
			{

				if (light_img < min_light)
				{
					Red = rgb[0];
					Green = rgb[1];
					Blue = rgb[2];

				}
				else if (light_img >= min_light && light_img < light_avg)
				{

					Red = rgb[0] + Red_offest_RT * ((light_img - min_light) / (light_avg - min_light));
					Green = rgb[1] + Green_offest_RT * ((light_img - min_light) / (light_avg - min_light));
					Blue = rgb[2] + Blue_offest_RT * ((light_img - min_light) / (light_avg - min_light));

				}
				else
				{
					Red = rgb[0] + Red_offest_RT;
					Green = rgb[1] + Green_offest_RT;
					Blue = rgb[2] + Blue_offest_RT;
				}
			}
			else if (col > 100 && col <= 569 && row > 1024 / 255 * (col - 200) && row < 1024 / 255 * (col - 100)
				|| col > 569 && col < 934 && row > 1024 / 255 * (824 - col) && row < 1024 / 255 * (934 - col))

			{


				float value_L;
				if (col < 512)
				{
					value_L = (col - (row * 255.0f / 1024.0f) - 100) / 100.0f;
				}
				else
				{
					value_L = (934 - (row * 255.0f / 1024.0f) - col) / 100.0f;
				}


				if (light_img < min_light)
				{
					Red = rgb[0] * value_L + (rgb[0] + Red_offest_RT) * (1 - value_L);
					Green = rgb[1] * value_L + (rgb[1] + Green_offest_RT) * (1 - value_L);
					Blue = rgb[2] * value_L + (rgb[2] + Blue_offest_RT) * (1 - value_L);

				}
				else if (light_img >= min_light && light_img < light_avg)
				{



					double Red_1 = rgb[0] + Red_offest_RT * ((light_img - min_light) / (light_avg - min_light));
					double Green_1 = rgb[1] + Green_offest_RT * ((light_img - min_light) / (light_avg - min_light));
					double Blue_1 = rgb[2] + Blue_offest_RT * ((light_img - min_light) / (light_avg - min_light));

					Red = Red_1 * value_L + (rgb[0] + Red_offest_RT) * (1 - value_L);
					Green = Green_1 * value_L + (rgb[1] + Green_offest_RT) * (1 - value_L);
					Blue = Blue_1 * value_L + (rgb[2] + Blue_offest_RT) * (1 - value_L);
				}
				else
				{
					Red = rgb[0] + Red_offest_RT;
					Green = rgb[1] + Green_offest_RT;
					Blue = rgb[2] + Blue_offest_RT;
				}
			}
			else
			{   //Red = 255;
				//Green = 255;
				//Blue = 255;

				Red = rgb[0] + Red_offest_RT;
				Green = rgb[1] + Green_offest_RT;
				Blue = rgb[2] + Blue_offest_RT;
			}

			rgb[0] = Red;
			rgb[1] = Green;
			rgb[2] = Blue;

			image_RT.at<Vec3f>(row, col) = rgb; 

		}

	}







	//最终合成
	for (int col = 0; col < img.cols; col++)
	{


		for (int row = 0; row < img.rows; row++)
		{
			Vec3f rgb = img.at<Vec3f>(row, col);

			Vec3f rgb_LF = image_LF.at<Vec3f>(row, col);
			Vec3f rgb_RT = image_RT.at<Vec3f>(row, col);


			double Red = rgb[0];
			double Green = rgb[1];
			double Blue = rgb[2];


			if (col < img.cols / 3)
			{


				Red = rgb_LF[0];
				Green = rgb_LF[1];
				Blue = rgb_LF[2];


			}
			else if (col >= img.cols / 3 && col <= img.cols * 2 / 3)
			{

				float mix_vlaue;

				mix_vlaue = (col - img.cols / 3.000f) / (img.cols / 3.000f);



				Red = rgb_LF[0] * (1 - mix_vlaue) + rgb_RT[0] * mix_vlaue;
				Green = rgb_LF[1] * (1 - mix_vlaue) + rgb_RT[1] * mix_vlaue;
				Blue = rgb_LF[2] * (1 - mix_vlaue) + rgb_RT[2] * mix_vlaue;

				//Red = rgb_LF[0]    * 0.5f + rgb_RT[0] * 0.5f;
				//Green = rgb_LF[1]  * 0.5f + rgb_RT[1] * 0.5f;
				//Blue = rgb_LF[2]   * 0.5f + rgb_RT[2] * 0.5f;







			}
			else {
				Red = rgb_RT[0];
				Green = rgb_RT[1];
				Blue = rgb_RT[2];
			}



			rgb[0] = Red;
			rgb[1] = Green;
			rgb[2] = Blue;

			img.at<Vec3f>(row, col) =  rgb;


		}
	}






}


/*
Scalar ImageOptimizedUtility::CalculateMeanRefColor_new(Mat tex, FP_Face face)   //新的计算原始图片
{
	if (tex != null)
	{
		Mat ReferenceMat = new Mat(tex.width, tex.height, CvType.CV_8UC3);
		Utils.texture2DToMat(tex, ReferenceMat);

		//  OpenCVForUnity.Rect[] facesarray = detectFace(ReferenceMat);



		Vector2 eyedown_pos1 = (Vector2)(face.landmark.Get("left_eye_lower_right_quarter"));
		Vector2 face_pos1 = (Vector2)(face.landmark.Get("contour_left5"));

		Vector2 eyedown_pos2 = (Vector2)(face.landmark.Get("right_eye_lower_left_quarter"));
		Vector2 face_pos2 = (Vector2)(face.landmark.Get("contour_right5"));

		Point left_pos1 = new Point(face_pos1.x, face_pos1.y);
		Point left_pos2 = new Point(eyedown_pos1.x, eyedown_pos1.y);

		Imgproc.rectangle(ReferenceMat, left_pos1, left_pos2, new Scalar(255, 0, 0));

		Mat subimg = ReferenceMat.submat(new Range((int)left_pos1.y, (int)left_pos2.y), new Range((int)left_pos1.x, (int)left_pos2.x));
		Mat subimgcopy = new Mat();

		subimg.copyTo(subimgcopy);


		//Imgproc.cvtColor(subimgcopy, subimgcopy, Imgproc.COLOR_RGB2HSV);
		for (int col = 0; col < subimgcopy.cols(); col++)
		{
			for (int row = 0; row < subimgcopy.rows(); row++)
			{
				double[] hsv = subimgcopy.get(row, col);

				Color rgbcol = new Color((float)hsv[0] / 255, (float)hsv[1] / 255, (float)hsv[2] / 255);

				float h;
				float s;
				float v;
				Color.RGBToHSV(rgbcol, out h, out s, out v);

				hsv[0] = h * 255;
				hsv[1] = s * 255;
				hsv[2] = v * 255;

				subimgcopy.put(row, col, hsv);
			}
		}


		Scalar meancolor = Core.mean(subimgcopy);

		return meancolor;
	}


	return new Scalar(0);


}
*/
