package com.facecloud;
public class Entry {
	
	
	 public static void main(String[] args) {
		 
	        //System.loadLibrary("WindowsLib");
	        System.loadLibrary("LinuxLib");
	        
	        
	        FaceCloudLib fclib = new FaceCloudLib();
	        
	        fclib.Init();
	        

	        String currentModelID = "10002";
	        String outJsonModelOut = "";
	        String jsonfacepath = "data/face/photojson_raw.json";
	        String photopath = "data/face/photoface_raw.jpg";

	        String jsonfacestring = "";

	        String outPhotoPath = "data/export/outphoto.jpg";
	        String outjsonoffsetpath = "data/export/outjson.json";
	        
	        

	        jsonfacestring = fclib.LoadJsonStringFromFile(jsonfacepath);

	        //System.out.println("jsonfacestring     :      " + jsonfacestring);
	        outJsonModelOut = fclib.Calculate(currentModelID, photopath, jsonfacestring, outPhotoPath);
	    
	        fclib.SaveFile(outJsonModelOut, outjsonoffsetpath);

	        //System.out.println("outJsonModelOut: " + outJsonModelOut);

	        System.out.println("Finish");
	 }
	 
	 
}
