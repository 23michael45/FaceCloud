//#include "FaceKeyPointParser.h"
//#include <dlib/image_processing/frontal_face_detector.h>
//#include <dlib/image_processing/render_face_detections.h>
//#include <dlib/image_processing.h>
//#include <dlib/gui_widgets.h>
//#include <dlib/image_io.h>
//#include "Predefined.h"
//#include "OSMesaContext.h"
//#include <iostream>
//#include <fstream>
//using namespace dlib;
//
//FaceKeyPointParser::FaceKeyPointParser()
//{
//}
//
//
//FaceKeyPointParser::~FaceKeyPointParser()
//{
//}
//
//void FaceKeyPointParser::Calculate()
//{
//	try
//	{
//		// We need a face detector.  We will use this to get bounding boxes for
//		// each face in an image.
//		frontal_face_detector detector = get_frontal_face_detector();
//		// And we also need a shape_predictor.  This is the tool that will predict face
//		// landmark positions given an image and face bounding box.  Here we are just
//		// loading the model from the shape_predictor_68_face_landmarks.dat file you gave
//		// as a command line argument.
//		shape_predictor sp;
//		deserialize(RES_PATH + std::string("facecloud/shape_predictor_68_face_landmarks.dat")) >> sp;
//
//		std::vector<std::string> list;
//		list.push_back(RES_PATH + "face/photo68-1.jpg");
//		list.push_back(RES_PATH + "face/photo68-2.jpg");
//		list.push_back(RES_PATH + "face/photo68-3.jpg");
//
//		// Loop over all the images provided on the command line.
//		for (int i = 0; i < list.size(); ++i)
//		{
//
//			OSMesa::Log("\n\nStarting timer...");
//			int start = OSMesa::getMilliCount();
//
//			std::string filepath = list[i];
//			array2d<rgb_pixel> img;
//			load_image(img, filepath);
//
//			long orgrow = img.nr();
//			long orgcol = img.nc();
//
//
//			int milliSecondsElapsed = OSMesa::getMilliSpan(start);
//			OSMesa::Log(OSMesa::format("\n\load_image : %u milliseconds", milliSecondsElapsed));
//
//
//			start = OSMesa::getMilliCount();
//			pyramid_up(img);
//			long prorow = img.nr();
//			long procol = img.nc();
//
//			milliSecondsElapsed = OSMesa::getMilliSpan(start);
//			OSMesa::Log(OSMesa::format("\n\pyramid_up : %u milliseconds", milliSecondsElapsed));
//
//			// Now tell the face detector to give us a list of bounding boxes
//			// around all the faces in the image.
//
//			start = OSMesa::getMilliCount();
//			std::vector<rectangle> dets = detector(img);
//			milliSecondsElapsed = OSMesa::getMilliSpan(start);
//			OSMesa::Log(OSMesa::format("\n\detector : %u milliseconds", milliSecondsElapsed));
//
//
//			// Now we will go ask the shape_predictor to tell us the pose of
//			// each face we detected.
//			std::vector<full_object_detection> shapes;
//			for (unsigned long j = 0; j < dets.size(); ++j)
//			{
//				full_object_detection shape = sp(img, dets[j]);
//				std::cout << "\nnumber of parts: " << shape.num_parts() << std::endl;
//
//				shapes.push_back(shape);
//			}
//
//			std::ofstream myfile;
//			myfile.open(filepath + ".pts");
//			myfile << "version: 1\n";
//			myfile << "n_points:  68\n";
//			myfile << "{\n";
//
//			full_object_detection kps = shapes[0];
//			for (int i = 0 ; i < kps.num_parts() ; i++)
//			{
//				dlib::point pt = kps.part(i);
//				myfile << pt.x() * orgcol / procol;
//				myfile << " ";
//				myfile << pt.y() * orgrow / prorow;
//				myfile << "\n";
//				
//			}
//
//
//			myfile << "}\n";
//			myfile.close();
//
//
//
//
//
//			// We can also extract copies of each face that are cropped, rotated upright,
//			// and scaled to a standard size as shown here:
//			dlib::array<array2d<rgb_pixel> > face_chips;
//			extract_image_chips(img, get_face_chip_details(shapes), face_chips);
//		
//			std::cout << "Hit enter to process the next image..." << std::endl;
//
//			getchar();
//		}
//	}
//	catch (...)
//	{
//		std::cout << "\nfacekey point parsser exception thrown!" << std::endl;
//	}
//}