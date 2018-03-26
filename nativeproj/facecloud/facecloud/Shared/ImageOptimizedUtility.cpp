#include "ImageOptimizedUtility.h"

#include <stdio.h>
#include <iostream>
#include "OSMesaContext.h"

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



		float dx = v1 * 5;    //˫���˲�����֮һ  
		double fc = v1 * 12.5f; //˫���˲�����֮һ  
		float alpha = 50; //͸����  

						  //˫���˲�  
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
	cvtColor(src, gray, COLOR_BGR2GRAY);;//��Ϊ�õ�����haar���������Զ��ǻ��ڻҶ�ͼ��ģ�����Ҫת���ɻҶ�ͼ��

															//Imgproc.resize(temp8uc1, temp8uc1, temp8uc1.size(), 0, 0, Imgproc.INTER_LINEAR);//���ߴ���С��1/scale,�����Բ�ֵ
															//Imgproc.equalizeHist(temp8uc1, temp8uc1);//ֱ��ͼ����
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



//计算图片轮廓内平均色
Scalar GetMeanColorInContour(Mat src, vector<Point> contour)
{
	int count = 0;
	Scalar meanSrc(0, 0, 0);
	for (int j = 0; j < src.rows; j++)
	{
		for (int i = 0; i < src.cols; i++)
		{
			float dist = pointPolygonTest(contour, Point2f(i, j), true);
			if (dist > 0)
			{
				Vec3b color = src.at<Vec3b>(i, j);
				meanSrc[0] += color[0];
				meanSrc[1] += color[1];
				meanSrc[2] += color[2];
				count++;
			}
		}
	}

	meanSrc = meanSrc / count;
	return meanSrc;
}

//计算色调转换算法的中间SCALAR
Scalar GetColorTransScalar(Mat src, Scalar mean)
{
	Scalar rt(0,0,0);
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			Vec3f s;
			s = src.at<Vec3b>(i, j);
			rt[0] = rt[0] + (s.val[0] - mean[0])*(s.val[0] - mean[0]);
			rt[1] = rt[1] + (s.val[1] - mean[1])*(s.val[1] - mean[1]);
			rt[2] = rt[2] + (s.val[2] - mean[2])*(s.val[2] - mean[2]);
		}
	}

	for (int i = 0; i < 3; i++)
	{
		rt[i] = sqrt(rt[i] / ((src.cols)*(src.rows)));
	}
	return rt;
}

//两图按给定平均色转色调
Mat ColorTransferBlend(Mat src, Mat ref, Scalar srcmean, Scalar refmean)
{
	Mat rt; 
	src.copyTo(rt);
	//算法计算，两张图输出两个颜色值 var1 var2
	Scalar var1 = GetColorTransScalar(rt, srcmean);
	Scalar var2 = GetColorTransScalar(ref, refmean);

	//根据计算var1 var2融合
	float blend = 0.5;
	for (int i = 0; i < rt.rows; i++)
	{
		for (int j = 0; j < rt.cols; j++)
		{
			Vec3f s;
			s = rt.at<Vec3b>(i, j);
			s.val[0] = (s.val[0] - srcmean[0])*(var2[0] / var1[0]) * blend * 2 + refmean[0] * (1 - blend) * 2;
			s.val[1] = (s.val[1] - srcmean[1])*(var2[1] / var1[1]) * blend * 2 + refmean[1] * (1 - blend) * 2;
			s.val[2] = (s.val[2] - srcmean[2])*(var2[2] / var1[2]) * blend * 2 + refmean[2] * (1 - blend) * 2;
			rt.at<Vec3b>(i, j) = s;
		}
	}
	return rt;
}


void ImageOptimizedUtility::ColorTransfer(Mat src, Mat ref, Mat& outputimg, JsonFaceInfo &faceinfo)
{

	//原始图变LAB空间
	Mat srcimg2;
	cv::cvtColor(src, srcimg2, CV_RGB2Lab);

	//参与图变LAB空间
	Mat refimg2;
	cv::resize(ref, refimg2, cv::Size(src.cols, src.rows));
	cv::cvtColor(refimg2, refimg2, CV_RGB2Lab);

	int i, j;

	//取照片中，脸部区域，计算平均色
	Scalar meanleft;
	Scalar meanright;
	vector<Point> contour;
	Vector2f pos;
	vector<Point> leftfaceContour;
	pos = faceinfo.landmarkdata["contour_left5"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_left10"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["mouth_left_corner"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["nose_left_contour3"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["nose_bridge2"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["left_eye_bottom"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	meanleft = GetMeanColorInContour(srcimg2, leftfaceContour);

	vector<Point> rightfaceContour;
	pos = faceinfo.landmarkdata["contour_right5"]; rightfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["contour_right10"]; rightfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["mouth_right_corner"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["nose_right_contour3"]; rightfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["nose_bridge2"]; leftfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	pos = faceinfo.landmarkdata["right_eye_bottom"]; rightfaceContour.push_back(cv::Point(pos.x, 1024 - pos.y));
	meanright = GetMeanColorInContour(srcimg2, rightfaceContour);

	int lx = 270 / 2;
	int rx = 1070 / 2;
	int ty = 624 / 2;
	int by = 1424 / 2;

	//取小块脸区域，再向内取Edge，取这一小块的平均色
	int edge = 60;
	Mat subrefimg = refimg2(cv::Range( lx + edge,rx- edge), cv::Range(ty + edge, by - edge));
	Scalar mean2 = cv::mean(subrefimg);


	//调前景色方案
	//Mat leftmat = ColorTransferBlend(srcimg2, refimg2, meanleft, mean2);
	//Mat rightmat = ColorTransferBlend(srcimg2, refimg2, meanright, mean2);
	Mat subrefimg2 = refimg2(cv::Range(lx , rx ), cv::Range(ty , by ));
	//调背景色方案
	Mat leftbgmat = ColorTransferBlend(subrefimg2, srcimg2, mean2, meanleft);
	Mat rightbgmat = ColorTransferBlend(subrefimg2, srcimg2, mean2, meanright);


	Mat leftbgtmp;
	cv::cvtColor(leftbgmat, leftbgtmp, CV_Lab2BGR);
	imwrite("data/export/leftbg.jpg", leftbgtmp);


	Mat rightbgtmp;
	cv::cvtColor(rightbgmat, rightbgtmp, CV_Lab2BGR);
	imwrite("data/export/rightbg.jpg", rightbgtmp);

	subrefimg2.copyTo(outputimg);
	for (int i = 0; i < outputimg.rows; i++)
	{
		for (int j = 0; j < outputimg.cols; j++)
		{
			;
			Vec3b l = leftbgmat.at<Vec3b>(i, j);
			Vec3b r = rightbgmat.at<Vec3b>(i, j);
			Vec3b s;

			float blend = (float)(j) / outputimg.cols;

			s.val[0] = l.val[0] * (1 - blend) + r.val[0] * blend;
			s.val[1] = l.val[1] * (1 - blend) + r.val[1] * blend;
			s.val[2] = l.val[2] * (1 - blend) + r.val[2] * blend;

			outputimg.at<Vec3b>(i, j) = s;
		}
	}

	cv::cvtColor(outputimg, outputimg, CV_Lab2RGB);

	Mat  outtmp;
	cv::cvtColor(outputimg, outtmp, CV_RGB2BGR);
	imwrite("data/export/blendbg.jpg", outtmp);
}

void ImageOptimizedUtility::UpdateRef_RGB(JsonFaceInfo& faceinfo,Mat img, Vector3f refcolor, float value, Mat& outputimg, Vector2f leftpoint, Vector2f rightpoint)
{



	//���������
	Mat main_channel[3];
	cv::split(img, main_channel);

	Mat mainRedChannel = main_channel[0];
	Mat mainGreenChannel = main_channel[1];
	Mat mainBlueChannel = main_channel[2];

	Mat image_LF;
	Mat image_RT;

	img.copyTo(image_LF);
	img.copyTo(image_RT);


	//���ȡ��

	Vector2f eyedown_pos1 = rightpoint;
	Vector2f face_pos1 = leftpoint;

	Point left_1(face_pos1.x, face_pos1.y);
	Point right_1(eyedown_pos1.x, eyedown_pos1.y);

	//lb
	//Rect roi1(left_1.x, left_1.y, right_1.x - left_1.x, right_1.y - left_1.y);
	//lt
	Rect roi1(min(left_1.x,right_1.x), min(left_1.y,right_1.y), abs(right_1.x - left_1.x), abs(right_1.y - left_1.y));

	Mat subimg_1(img,roi1);// = img.(new Range((int)left_1.y, (int)right_1.y), new Range((int)left_1.x, (int)right_1.x));
	Mat subimgcopy_1;
	subimg_1.copyTo(subimgcopy_1);

	//��������

	Mat LF_vChannels[3];
	cv::split(subimgcopy_1, LF_vChannels);

	Mat imageRedChannel = LF_vChannels[0];
	Mat imageGreenChannel = LF_vChannels[1];
	Mat imageBlueChannel = LF_vChannels[2];

	Scalar imageBlueChannelAvg_LF = cv::mean(imageBlueChannel);
	Scalar imageGreenChannelAvg_LF = cv::mean(imageGreenChannel);
	Scalar imageRedChannelAvg_LF = cv::mean(imageRedChannel);





	//�Ҳ�ȡ��

	double left2_x = img.rows - face_pos1.x;
	double right2_x = img.rows - eyedown_pos1.x;
	
	
	//lb
	//Point left_2(left2_x, left_1.y);
	//Point right_2(right2_x, right_1.y);

	//lt
	//Point left_2(left2_x, right_1.y);
	//Point right_2(right2_x, left_1.y);

	//both
	Point left_2(left2_x, min(left_1.y,right_1.y));
	Point right_2(right2_x, max(left_1.y, right_1.y));

	// Debug.OSMesa::Log(left_2 + "................" + right_2);

	//��1�ε�ɫ 


	//lb
	//Rect roi2(left_2.x, left_2.y, left_2.x - right_2.x, right_2.y - left_2.y);
	//lt
	Rect roi2(min(left_2.x,right_2.x), min(left_2.y,right_2.y), abs(left_2.x - right_2.x), abs(right_2.y - left_2.y));


	Mat subimg_2(img, roi2);// img.submat(new Range((int)left_2.y, (int)right_2.y), new Range((int)right_2.x, (int)left_2.x));
	Mat subimgcopy_2;
	subimg_2.copyTo(subimgcopy_2);


	Mat RT_vChannels[3];
	cv::split(subimgcopy_2, RT_vChannels);

	Mat imageRedChannel_RT = RT_vChannels[0];
	Mat imageGreenChannel_RT = RT_vChannels[1];
	Mat imageBlueChannel_RT = RT_vChannels[2];



	Scalar imageRedChannelAvg_RT = cv::mean(imageRedChannel_RT);
	Scalar imageGreenChannelAvg_RT = cv::mean(imageGreenChannel_RT);
	Scalar imageBlueChannelAvg_RT = cv::mean(imageBlueChannel_RT);


	double r_avg_LF = imageRedChannelAvg_LF.val[0];
	double g_avg_LF = imageGreenChannelAvg_LF.val[0];
	double b_avg_LF = imageBlueChannelAvg_LF.val[0];


	double r_avg_RT = imageRedChannelAvg_RT.val[0];
	double g_avg_RT = imageGreenChannelAvg_RT.val[0];
	double b_avg_RT = imageBlueChannelAvg_RT.val[0];



	//Debug.OSMesa::Log(r_avg_LF+"....."+ g_avg_LF+"....."+ b_avg_LF);
	// Debug.OSMesa::Log(r_avg_RT+"....."+ g_avg_RT+ "....." + b_avg_RT);



	//double Red_offest   ;
	//double Green_offest ;
	//double Blue_offest;

	double Red_offest_LF = refcolor.x - r_avg_LF;
	double Green_offest_LF = refcolor.y - g_avg_LF;
	double Blue_offest_LF = refcolor.z - b_avg_LF;
	double aphaf = 0;



	double Red_offest_RT = refcolor.x - r_avg_RT;
	double Green_offest_RT = refcolor.y - g_avg_RT;
	double Blue_offest_RT = refcolor.z - b_avg_RT;
	//����ɫ


	for (int col = 0; col < img.cols; col++)
	{


		for (int row = 0; row < img.rows; row++)
		{
			Vec3b rgb = img.at<Vec3b>(row, col);

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

			image_LF.at<Vec3b>(row, col) = rgb;

		}

	}


	//�Ҳ��ɫ

	for (int col = 0; col < img.cols; col++)
	{


		for (int row = 0; row < img.rows; row++)
		{
			Vec3b rgb = img.at<Vec3b>(row, col);

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

			image_RT.at<Vec3b>(row, col) = rgb;

		}

	}





	//���պϳ�
	for (int col = 0; col < img.cols; col++)
	{


		for (int row = 0; row < img.rows; row++)
		{
			Vec3b rgb = img.at<Vec3b>(row, col);

			Vec3b rgb_LF = image_LF.at<Vec3b>(row, col);
			Vec3b rgb_RT = image_RT.at<Vec3b>(row, col);


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
			rgb[1]= Green;
			rgb[2] = Blue;

			img.at<Vec3b>(row, col) = rgb;


		}
		
	}
	outputimg = img;

}


Mat ImageOptimizedUtility::FacePhotoProcess(JsonFaceInfo& faceinfo, JsonRole bonedef, Mat src32)
{
	int srcWidth = src32.cols;
	int srcHeight = src32.rows;

	int resultWidth = 1024;
	int resultHeight = 1024;

	
	Vector2f leftp = faceinfo.landmarkdata["left_eye_right_corner"];
	//leftp.y = srcHeight - leftp.y;

	Vector2f rightp = faceinfo.landmarkdata["right_eye_left_corner"];
	//rightp.y = srcHeight - rightp.y;

	Vector2f srcPos1 = (leftp + rightp) * 0.5f;

	Vector2f srcPos2 = faceinfo.landmarkdata["contour_chin"];
	//srcPos2.y = srcHeight - srcPos2.y;

	float targetpos2_y = bonedef.face_zero_pointy / 1024;

	Vector2f targetPos1(resultWidth * 0.5f, resultHeight * 0.5f); // 百分比 //
	Vector2f targetPos2(resultWidth * 0.5f, resultHeight * (1 - targetpos2_y));

	//cv::Mat mat;



	Vector2f srcVec = srcPos2 - srcPos1;
	Vector2f dstVec = targetPos2 - targetPos1;
	float srcLen = srcVec.dist();
	float dstLen = dstVec.dist();


	float yaw_angle = (float)(faceinfo.yaw_angle);          //摇头
	float roll_angle = (float)(faceinfo.roll_angle);        //平面旋转
	float pitch_angle = (float)(faceinfo.pitch_angle);      //抬头


	float yaw_dstLen = cos(yaw_angle) * dstLen;
	float pitch_dstlen = cos(pitch_angle) * yaw_dstLen;


	float scale = dstLen / srcLen;
	float rotrad = VectorAngle(srcVec, dstVec);


	Point2f srcCenterPoint(srcPos1.x, srcPos1.y);

	//cv::Mat affineMat = cv::Mat::eye(3, 2, CV_64FC1);
	cv::Mat rotateMat = cv::getRotationMatrix2D(srcCenterPoint, rotrad, scale);

	Point2f dstPt = ApplyAffineMat(rotateMat,srcCenterPoint);
	
	cv::Point2f delta = Point2f(targetPos1.x,targetPos1.y)- dstPt;

	Mat t = (Mat_<double>(2, 3) << 0, 0, delta.x, 0, 0, delta.y);
	
	Mat rt32;
	Mat affine = t + rotateMat;
	cv::warpAffine(src32, rt32, affine,Size(resultWidth,resultHeight));

	Mat rt8;
	rt32.convertTo(rt8, CV_8UC3);

	for (map<string, Vector2f>::iterator iter = faceinfo.landmarkdata.begin(); iter != faceinfo.landmarkdata.end(); iter++)
	{
		Vector2f v = iter->second;
		Point2f pt(v.x, v.y);
		pt = ApplyAffineMat(affine, pt);

		iter->second = Vector2f(pt.x, 1024 - pt.y);
	}
	Point2f rectlt(faceinfo.face_rectangle.x, faceinfo.face_rectangle.y);
	Point2f rectrb(faceinfo.face_rectangle.x + faceinfo.face_rectangle.z, faceinfo.face_rectangle.y + faceinfo.face_rectangle.w);
	rectlt = ApplyAffineMat(affine, rectlt);
	rectrb = ApplyAffineMat(affine, rectrb);

	faceinfo.face_rectangle.x = rectlt.x;
	faceinfo.face_rectangle.y = rectlt.y;
	faceinfo.face_rectangle.z = rectrb.x - rectlt.x;
	faceinfo.face_rectangle.w = rectrb.y - rectlt.y;

	return rt8;
}




void ImageOptimizedUtility::DetectSkinStatus(Mat src, JsonFaceInfo faceinfo,JsonSkinStatus& skinjson)
{
	Mat src16;
	src.convertTo(src16, CV_16UC3);

	Vector2f pos;
	vector<Point> noseContour;
	pos = faceinfo.landmarkdata["nose_bridge1"]; noseContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["nose_left_contour3"]; noseContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["nose_right_contour3"]; noseContour.push_back(cv::Point(pos.x, pos.y));

	vector<Point> leftfaceContour;
	pos = faceinfo.landmarkdata["contour_left5"]; leftfaceContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left10"]; leftfaceContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["nose_left_contour3"]; leftfaceContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["left_eye_bottom"]; leftfaceContour.push_back(cv::Point(pos.x, pos.y));

	vector<Point> rightfaceContour;
	pos = faceinfo.landmarkdata["contour_right5"]; rightfaceContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right10"]; rightfaceContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["nose_right_contour3"]; rightfaceContour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["right_eye_bottom"]; rightfaceContour.push_back(cv::Point(pos.x, pos.y));


	vector<Point> lefteyeUnder;
	pos = faceinfo.landmarkdata["left_eye_bottom"]; lefteyeUnder.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["left_eye_right_corner"]; lefteyeUnder.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["left_eye_top"]; lefteyeUnder.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["left_eye_left_corner"]; lefteyeUnder.push_back(cv::Point(pos.x, pos.y));

	lefteyeUnder[1] = lefteyeUnder[1] * 2 - lefteyeUnder[0];
	lefteyeUnder[2] = lefteyeUnder[2] * 2 - lefteyeUnder[0];
	lefteyeUnder[3] = lefteyeUnder[3] * 2 - lefteyeUnder[0];


	vector<Point> righteyeUnder;
	pos = faceinfo.landmarkdata["right_eye_bottom"]; righteyeUnder.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["right_eye_left_corner"]; righteyeUnder.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["right_eye_top"]; righteyeUnder.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["right_eye_right_corner"]; righteyeUnder.push_back(cv::Point(pos.x, pos.y));

	righteyeUnder[1] = righteyeUnder[1] * 2 - righteyeUnder[0];
	righteyeUnder[2] = righteyeUnder[2] * 2 - righteyeUnder[0];
	righteyeUnder[3] = righteyeUnder[3] * 2 - righteyeUnder[0];






	/////////////////////////////////////////////////////////按特征点画轮廓 无额头和头发	
	vector<vector<cv::Point> > contours;
	vector<cv::Point> contour;
	
	pos;
	pos = faceinfo.landmarkdata["contour_left1"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left2"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left3"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left4"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left5"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left6"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left7"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left8"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left9"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left10"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left11"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left12"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left13"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left14"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left15"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_left16"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_chin"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right16"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right15"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right14"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right13"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right12"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right11"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right10"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right9"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right8"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right7"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right6"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right5"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right4"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right3"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right2"]; contour.push_back(cv::Point(pos.x, pos.y));
	pos = faceinfo.landmarkdata["contour_right1"]; contour.push_back(cv::Point(pos.x, pos.y));

	contours.push_back(contour);
	cv::Mat contourmask(src.rows, src.cols, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Scalar colorwhite = cv::Scalar(1, 1, 1);
	drawContours(contourmask, contours, -1, colorwhite, CV_FILLED);
	cv::Mat faceimg;
	cv::multiply(src, contourmask, faceimg);

	cv::Mat tmp;
	faceimg.convertTo(tmp, CV_RGB2BGR);
	imwrite("data/export/skindetecttmp.jpg",tmp);



	Rect box = boundingRect(contour);
	int count = 0;
	Vec3f meanColor;

	int eyeunderCount = 0;
	Vec3f meanEyeUnderColor;
	for (int j = box.tl().y; j < +box.br().y; j++)
	{
		for (int i = box.tl().x; i < +box.br().x; i++)
		{

			cv::Vec3s c = src16.at<cv::Vec3s>(j, i);


			//鼻子有高光不计算
			float dist1 = pointPolygonTest(noseContour, cv::Point2f(i, j), true);

			float dist2 = pointPolygonTest(leftfaceContour, cv::Point2f(i, j), true);
			float dist3 = pointPolygonTest(rightfaceContour, cv::Point2f(i, j), true);

			if (dist1 > 0 ||  dist2 > 0 || dist3 > 0)
			{
				count++;

				meanColor[0] += c[0];
				meanColor[1] += c[1];
				meanColor[2] += c[2];
			}



			float eyeldist = pointPolygonTest(lefteyeUnder, cv::Point2f(i, j), true);
			float eyerdist = pointPolygonTest(righteyeUnder, cv::Point2f(i, j), true);

			if (eyeldist > 0 || eyerdist > 0)
			{
				eyeunderCount++;

				meanEyeUnderColor[0] += c[0];
				meanEyeUnderColor[1] += c[1];
				meanEyeUnderColor[2] += c[2];
			}

		}
	}
	meanColor = meanColor / count;
	meanEyeUnderColor = meanEyeUnderColor / eyeunderCount;

	Mat rgbMeanMat(1, 1, CV_32FC3,meanColor);
	Mat hsvMeanMat;
	cvtColor(rgbMeanMat, hsvMeanMat, CV_RGB2HSV);

	Vec3f HSVMean = hsvMeanMat.at<Vec3f>(0, 0);

	map<SKINCOLORTYPE, Vec3f> RGBMap;
	RGBMap[SCT_WHITE] = Vec3f(231, 222, 225);
	RGBMap[SCT_BLOND] = Vec3f(215, 180, 190);
	RGBMap[SCT_NATURE] = Vec3f(205, 168, 150);
	RGBMap[SCT_WHEAT] = Vec3f(176, 139, 121);
	RGBMap[SCT_DARK] = Vec3f(117, 88, 78);
	RGBMap[SCT_BLACK] = Vec3f(96, 73, 65);

	map<SKINCOLORTYPE, Vec3f> HSVMap;
	for (int i = 0; i < SCT_MAX; i++)
	{

		Mat rgbM(1, 1, CV_32FC3, RGBMap[(SKINCOLORTYPE)i]);
		Mat hsvM;
		cvtColor(rgbM, hsvM, CV_RGB2HSV);

		HSVMap[(SKINCOLORTYPE)i] = hsvM.at<Vec3f>(0, 0);
	}

	int minIndex = 0;//皮肤最接近的颜色
	double minDist = 9999;
	for (int i = 0; i < SCT_MAX; i++)
	{
		//double hsvDist = norm(HSVMean, HSVMap[(SKINCOLORTYPE)i]);
		//double hsvDist = norm(meanColor, RGBMap[(SKINCOLORTYPE)i]);

		double hsvDist = abs(HSVMean[2] - HSVMap[(SKINCOLORTYPE)i][2]);

		if (hsvDist < minDist)
		{
			minDist = hsvDist;
			minIndex = i;
		}
	}

	//设置输出肤色索引
	skinjson.skincolor = (SKINCOLORTYPE)minIndex;


	Vec3f HSVEyeUnderMean;
	Mat rgbM(1, 1, CV_32FC3, meanEyeUnderColor);
	Mat hsvM;
	cvtColor(rgbM, hsvM, CV_RGB2HSV);
	HSVEyeUnderMean = hsvM.at<Vec3f>(0, 0);

	double hsvEyeDist = 0;//眼袋部分颜色与平均肤色的距离值
	hsvEyeDist = norm(HSVMean, HSVEyeUnderMean);

	//设置输出黑头程度值 【0，100】
	float blackhead = findBlackHead(faceimg);


	/*Mat gray;
	faceimg.convertTo(gray, CV_RGB2GRAY);

	cv::threshold(gray, gray, 30, 255, THRESH_BINARY);


	gray.convertTo(tmp, CV_GRAY2BGR);
	imwrite("data/export/skindetectgray.jpg", tmp);*/
	
	skinjson.blackhead = blackhead;
}
float ImageOptimizedUtility::findBlackHead(Mat img)
{
	//img = imread("G:/DevelopProj/FaceCloud/nativeproj/facecloud/facecloud/data/skindetect/blackhead/blackhead1_severe_org.jpg");

	//Mat gray,grayscaled;
	////img.convertTo(gray, CV_RGB2GRAY);
	//cv::cvtColor(img, gray, CV_RGB2GRAY);

	//threshold(gray, grayscaled, 150,255, CV_THRESH_BINARY);


	//cv::Mat tmp;
	//grayscaled.convertTo(tmp, CV_GRAY2BGR);
	//imwrite("data/export/grayscaled.jpg", grayscaled);
	return 0;
}



bool ImageOptimizedUtility::findPimples(Mat img)
{
	vector<vector<Point> > contours;
	vector<Point> pts;
	Mat bw, bgr[3];
	split(img, bgr);
	bw = bgr[0];
	int pimplescount = 0;

	adaptiveThreshold(bw, bw, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, 5);
	dilate(bw, bw, Mat(), Point(-1, -1), 1);

	contours.clear();
	findContours(bw, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	for (size_t i = 0; i < contours.size(); i++)
	{

		if (contourArea(contours[i]) > 20 & contourArea(contours[i]) < 150)
		{
			Rect minRect = boundingRect(Mat(contours[i]));
			Mat imgroi(img, minRect);

			cvtColor(imgroi, imgroi, COLOR_BGR2HSV);
			Scalar color = mean(imgroi);
			cvtColor(imgroi, imgroi, COLOR_HSV2BGR);


			rectangle(img, minRect, Scalar(0, 255, 0));

			if (color[0] < 10 && color[1] > 70 && color[2] > 50)
			{
				Point2f center, vtx[4];
				float radius = 0;
				minEnclosingCircle(Mat(contours[i]), center, radius);

				if (radius < 20)
				{
					rectangle(img, minRect, Scalar(0, 255, 0));
					pimplescount++;
				}
			}
		}
	}
	putText(img, format("%d", pimplescount), Point(50, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 0), 2);
	imwrite("data/export/pimples.jpg",img);
	return true;
}