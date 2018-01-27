package com.facecloud;
public class FaceCloudLib {
	
	
	//��ʼ�����������
	public native boolean Init();
	
	//������������
	//modelID  ��ɫģ��ID   ��10001 Ů10002
	//photoPath ����ͼ��·��    ����ϴ�����Ƭ�ļ���ŵ�·��
	//jsonFace  face++���ص�����JSON��������Ϣ
	//photoPath ��������ɵ�������ͼ·��
	//return jsonModelOut  ��������ɵ�ƫ��JSON�ļ������ظ��ͻ���
	public native String Calculate(String modelID,String photoPath, String jsonFace, String photoPathOut);
	
	
	//���ļ���JSON�ִ��������ã�
	public native String LoadJsonStringFromFile(String filepath);
	
	
	//�����ַ������ļ��������ã�
	public native void SaveFile(String s,String path);
}