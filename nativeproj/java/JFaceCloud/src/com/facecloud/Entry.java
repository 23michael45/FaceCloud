package com.facecloud;
public class Entry {
	
	
	 public static void main(String[] args) {
		 
	        //System.loadLibrary("WindowsLib");
	        System.loadLibrary("LinuxLib");
	        
	        
	        FaceCloudLib fclib = new FaceCloudLib();
	        
	        fclib.Init();
	        

	        String currentModelID = "10002";
	        String outJsonModelOut = "";
	        String jsonfacepath = "/root/jni/data/face/photojson_raw.json";
	        String photopath = "/root/jni/data/face/photoface_raw.jpg";

	        String jsonfacestring = "";

	        String outPhotoPath = "/root/jni/data/export/outphoto.jpg";
	        String outjsonoffsetpath = "/root/jni/data/export/outjson.json";
	        
	        jsonfacestring = fclib.LoadJsonStringFromFile(jsonfacepath);
			
	        String basepath = "/tmp/outPhotoPath";
			String baseoffsetpath = "/tmp/outJsonPath";


			int i = 0;
			boolean b = true;
			while (b)
			{
				outPhotoPath = basepath + Integer.toString(i) + ".jpg";
				outJsonModelOut = baseoffsetpath + Integer.toString(i) + ".json";

				outJsonModelOut = fclib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath);
				fclib.SaveFile(outJsonModelOut, outjsonoffsetpath);
				i++;
			}


	        //System.out.println("jsonfacestring     :      " + jsonfacestring);
	        //outJsonModelOut = fclib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath);
	        //fclib.SaveFile(outJsonModelOut, outjsonoffsetpath);

	        //System.out.println("outJsonModelOut: " + outJsonModelOut);
			fclib.Finalize();
	        System.out.println("Finish");
	 }
	 
	 
}
