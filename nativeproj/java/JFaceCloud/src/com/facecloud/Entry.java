package com.facecloud;
public class Entry {
	
	
	 public static void main(String[] args) {
		 
	        //System.loadLibrary("WindowsLib");
	        System.loadLibrary("LinuxLib");
	        
	        
	        FaceCloudLib fclib = new FaceCloudLib();
	        
	        fclib.Init();
	        

	        String currentModelID = "10002";
	        String outJsonModelOut = "";
	        String outPhotoPath = "data/export/outphoto.jpg";
	        String jsonfacepath = "data/face/photojson.json";
	        String jsonfacestring = "";
	        String outjsonoffsetpath = "data/export/outjson.json";
	        

	        jsonfacestring = fclib.LoadJsonStringFromFile(jsonfacepath);
	        
	        outJsonModelOut = fclib.Calculate(currentModelID, "data/face/photoface.jpg", jsonfacestring, outPhotoPath);
	    
	        fclib.SaveFile(outJsonModelOut, outjsonoffsetpath);


	        System.out.println("outJsonModelOut: " + outJsonModelOut);
	 }
	 
	 
}
