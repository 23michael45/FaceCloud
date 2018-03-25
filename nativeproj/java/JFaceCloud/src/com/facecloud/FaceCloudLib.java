package com.facecloud;
public class FaceCloudLib {
	
	
	//初始化人脸计算库
	public native boolean Init();
	
	//销毁人脸计算库
	public native boolean Finalize();
	
	//计算人脸数据
	//modelID  角色模型ID   男10001 女10002
	//photoPath 输入图像路径    玩家上传的照片文件存放的路径
	//jsonFace  face++返回的人脸JSON特征点信息
	//photoPath 计算后生成的人脸贴图路径
	//return jsonModelOut  计算后生成的偏移JSON文件，返回给客户端
	public native String Calculate(String modelID,String photoPath, String jsonFace, String photoPathOut);
	
	
	//计算人脸皮肤状态
	//photoPath 输入图像路径    玩家上传的照片文件存放的路径
	//jsonFace  face++返回的人脸JSON特征点信息
	//return jsonModelOut  计算后生成的偏移JSON文件，返回给客户端
	public native String DetectSkinStatus(String photoPath, String jsonFace);
	
	//从文件读JSON字串（测试用）
	public native String LoadJsonStringFromFile(String filepath);
	
	
	//保存字符串到文件（测试用）
	public native void SaveFile(String s,String path);
}
