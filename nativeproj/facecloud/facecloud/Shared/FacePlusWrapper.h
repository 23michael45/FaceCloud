#pragma once

#include <stdio.h>
#include "opencv2/highgui.hpp"
#include <json/json.h>
using namespace std;

using namespace cv;


class FP_Rect
{
public:
	int top;
	int width;
	int left;
	int height;
};

class FP_headpose   //人脸属性
{
public:
	 float pitch_angle;  //抬头
	 float roll_angle;   //平面旋转
	 float yaw_angle;    //摇头

public:
	static FP_headpose FromJson(std::string jsonStr)
	{

		Json::Value root;
		Json::CharReaderBuilder rbuilder;
		Json::CharReader * reader = rbuilder.newCharReader();	
		string errors;
		bool ok = reader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root,&errors);
		delete reader;


		FP_headpose ret;
		ret.pitch_angle = root["pitch_angle"].asFloat();
		ret.roll_angle = root["roll_angle"].asFloat();
		ret.yaw_angle = root["yaw_angle"].asFloat();
		return ret;
	};

};

class FP_gender
{
public:
	string gender_data;

public:
	FP_gender(string data)
	{
		gender_data = data;
	}
};






class FP_Landmark
{
public:
	map<string, Vec2f> data;


public:
	static FP_Landmark FromJson(std::string jsonStr)
	{

		Json::Value root;	
		Json::CharReaderBuilder rbuilder;
		Json::CharReader * reader = rbuilder.newCharReader();
		string errors;
		bool ok = reader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root, &errors);
		delete reader;


		FP_Landmark ret;

		Json::Value::Members members = root.getMemberNames();
		
		for (int i = 0; i < members.size(); i++)
		{
			ret.data[members[i]] = Vec2f(root[members[i]]["x"].asFloat(), root[members[i]]["y"].asFloat());
		}

		return ret;
	}

	Vec2f Get(string name)
	{
		return data[name];
	}

	void Set(string name, Vec2f pos)
	{
		data[name] = pos;
	}
};

class FP_Attr
{
public:
	string value;
};

class FP_FaceAttributes
{


};

class FP_Face
{
public:
	string face_token;
	FP_Rect face_rectangle;
	FP_Landmark landmark;
	// public FP_FaceAttributes attributes;
	string face_gender;
	FP_headpose face_pose;

};

class FP_DetectRet
{
public:
	int time_used;
	string request_id;
	string error_message;
	vector<FP_Face> faces;
};


//--------------------------------
/*
class FacePlusWrapper
{
	enum EState
	{
		Idle,
		Requesting,
		Done
	}


	public delegate void ResultCallBack(bool success, FP_Face ret);
	ResultCallBack resultCallBack = null;

	EState mCurState = EState.Idle;
	Thread mThread = null;
	HttpWebRequest mWebRequest;
	HttpWebResponse mResponse;
	int mResponseCode;
	string mResponseContent;

	string uploadPhotoPath;
	int rawWidth, rawHeight;
	int uploadMaxWidth = 500;
	float uploadScale = 1.0f;

	string faceplusUrl = "https://api-cn.faceplusplus.com/facepp/v3";
	string faceplusAppID = "Nw5aC8b7t0GpAam-rJTxEarzvr9TwWCL";
	string faceplusAppSecret = "kRTNVOC6MUKS1k87gYlPiU-bolSovp46";

	string LFAPI_info = "/info/api";
	string LFAPI_face_detect = "/detect"; // 上传图片并获取面部信息 //


	void Update()
	{
		if (mThread != null)
		{
			if (mCurState == EState.Done)
			{
				mCurState = EState.Idle;
				mThread.Interrupt();
				mThread = null;
				OnResponse();
			}
		}
	}

	string thread_api;
	bool thread_useQueryString;
	NameValueCollection thread_key_values = null;
	NameValueCollection thread_key_files = null;
	void CallHttpApi(string api, NameValueCollection key_values = null, NameValueCollection key_files = null)
	{
		if (mCurState == EState.Requesting)
		{
			Debug.LogError("CallHttpApi Requesting");
			return;
		}

		thread_api = api;
		thread_key_values = key_values;
		thread_key_files = key_files;

		mThread = new Thread(StartRequest);
		mThread.Start();
	}

	void StartRequest()
	{
		mResponse = null;
		mResponseCode = 0;
		mResponseContent = null;

		string boundary = "-------" + DateTime.Now.Ticks.ToString("x");
		byte[] boundarybytes = Encoding.ASCII.GetBytes("\r\n--" + boundary + "\r\n");

		ServicePointManager.ServerCertificateValidationCallback = MyRemoteCertificateValidationCallback;

		mWebRequest = HttpWebRequest.Create(faceplusUrl + thread_api) as HttpWebRequest;

		// form request props //
		mWebRequest.ContentType = "multipart/form-data; boundary=" + boundary;
		mWebRequest.Method = "POST";
		//mWebRequest.KeepAlive = true;
		mWebRequest.Credentials = CredentialCache.DefaultCredentials;
		mWebRequest.Timeout = 10000;
		// form request context //
		using (Stream stream = mWebRequest.GetRequestStream())
		{
			//key/value
			string formdataTemplate = "Content-Disposition: form-data; name=\"{0}\"\r\n\r\n{1}";
			if (thread_key_values == null)
			{
				thread_key_values = new NameValueCollection();
			}
			thread_key_values.Add("api_key", faceplusAppID);
			thread_key_values.Add("api_secret", faceplusAppSecret);

			foreach(string key in thread_key_values.Keys)
			{
				stream.Write(boundarybytes, 0, boundarybytes.Length);
				string formitem = string.Format(formdataTemplate, key, thread_key_values[key]);
				byte[] formitembytes = Encoding.UTF8.GetBytes(formitem);
				stream.Write(formitembytes, 0, formitembytes.Length);
			}

			//files
			string headerTemplate = "Content-Disposition: form-data; name=\"{0}\"; filename=\"{1}\"\r\nContent-Type: application/octet-stream\r\n\r\n";
			byte[] buffer = new byte[4096];
			int bytesRead = 0;

			if (thread_key_files != null)
			{
				foreach(string key in thread_key_files.Keys)
				{
					stream.Write(boundarybytes, 0, boundarybytes.Length);
					string header = string.Format(headerTemplate, key, Path.GetFileName(thread_key_files[key]));
					byte[] headerbytes = Encoding.UTF8.GetBytes(header);
					stream.Write(headerbytes, 0, headerbytes.Length);
					using (FileStream fileStream = new FileStream(thread_key_files[key], FileMode.Open, FileAccess.Read))
					{
						while ((bytesRead = fileStream.Read(buffer, 0, buffer.Length)) != 0)
						{
							stream.Write(buffer, 0, bytesRead);
						}
					}
				}
			}
			//form end
			byte[] endbytes = Encoding.UTF8.GetBytes("\r\n--" + boundary + "--\r\n");
			stream.Write(endbytes, 0, endbytes.Length);
		}

		// get response //
		mResponse = mWebRequest.GetResponse() as HttpWebResponse;
		mResponseCode = (int)mResponse.StatusCode;

		using (var httpStreamReader = new StreamReader(mResponse.GetResponseStream(), Encoding.GetEncoding("utf-8")))
		{
			mResponseContent = httpStreamReader.ReadToEnd();
		}

		mResponse.Close();
		mWebRequest.Abort();
		mResponse = null;
		mWebRequest = null;

		mCurState = EState.Done;
	}


	public bool MyRemoteCertificateValidationCallback(System.Object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
	{
		bool isOk = true;
		if (sslPolicyErrors != SslPolicyErrors.None)
		{
			for (int i = 0; i < chain.ChainStatus.Length; i++)
			{
				if (chain.ChainStatus[i].Status != X509ChainStatusFlags.RevocationStatusUnknown)
				{
					chain.ChainPolicy.RevocationFlag = X509RevocationFlag.EntireChain;
					chain.ChainPolicy.RevocationMode = X509RevocationMode.Online;
					chain.ChainPolicy.UrlRetrievalTimeout = new TimeSpan(0, 1, 0);
					chain.ChainPolicy.VerificationFlags = X509VerificationFlags.AllFlags;
					bool chainIsValid = chain.Build((X509Certificate2)certificate);
					if (!chainIsValid)
					{
						isOk = false;
					}
				}
			}
		}
		return isOk;
	}


	public void UploadImage(string imagePath, ResultCallBack callback)
	{
		this.resultCallBack = callback;

		NameValueCollection key_values = new NameValueCollection();
		key_values.Add("return_attributes", "gender");
		key_values.Add("return_attributes", "headpose");
		key_values.Add("return_landmark", "1");

		// compress //
		uploadPhotoPath = Application.persistentDataPath + "/tmp/lf_upload.jpg";
		if (!Directory.Exists(Path.GetDirectoryName(uploadPhotoPath)))
		{
			Directory.CreateDirectory(Path.GetDirectoryName(uploadPhotoPath));
		}
		if (File.Exists(uploadPhotoPath))
		{
			File.Delete(uploadPhotoPath);
		}
		PhotoProcessor.CompressToLimit(imagePath, uploadPhotoPath, 500, out this.uploadScale, out this.rawWidth, out this.rawHeight, OnFinishCompress, uploadPhotoPath, LFAPI_face_detect, key_values);


	}
	public void OnFinishCompress(Matrix mat, params object[] objs)
	{
		string uploadPhotoPath = (string)objs[0];
		string LFAPI_face_detect = (string)objs[1];
		NameValueCollection key_values = (NameValueCollection)objs[2];
		// upload //
		NameValueCollection key_files = new NameValueCollection();
		key_files.Add("image_file", uploadPhotoPath);
		CallHttpApi(LFAPI_face_detect, key_values, key_files);
	}

	void OnResponse()
	{
		if (mResponseCode == 200)
			Debug.OSMesa::Log("UploadImage succeed!");


		if (this.resultCallBack != null)
		{
			Debug.LogError(mResponseContent);

			// try
			{
				FP_DetectRet ret = LitJson.JsonMapper.ToObject<FP_DetectRet>(mResponseContent);
				if (ret.faces != null && ret.faces.Length > 0)
				{
					// for landmark //
					LitJson.JsonData data = LitJson.JsonMapper.ToObject(mResponseContent);
					LitJson.JsonData landMark = data["faces"][0]["landmark"];
					//LitJson.JsonData attributes = data["faces"][0]["attributes"]; 


					FP_Face face = ret.faces[0];
					face.landmark = FP_Landmark.FromJson(landMark);

					string gender = data["faces"][0]["attributes"]["gender"]["value"].ToString();
					face.face_gender = gender;
					LitJson.JsonData headposeData = data["faces"][0]["attributes"]["headpose"];
					face.face_pose = FP_headpose.FromJson(headposeData);

					// scale //
					List<string> list = new List<string>();
					list.AddRange(face.landmark.data.Keys);
					foreach(var key in list)
					{
						face.landmark.data[key] /= uploadScale;
						face.landmark.data[key].y = 1024 - face.landmark.data[key].y;
						face.landmark.data[key].y = Mathf.Max(0, face.landmark.data[key].y);
						face.landmark.data[key].y = Mathf.Min(face.landmark.data[key].y, 1023);
					}

					this.resultCallBack(true, face);
				}
				else
				{
					this.resultCallBack(false, null);
				}
			}
			//             catch (System.Exception ex)
			//             {
			//                 Debug.LogError("OnResponse: exp: " + ex.ToString());
			//                 this.resultCallBack(false, null);
			//             }
		}
	}
}
*/
