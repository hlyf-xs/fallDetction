#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <Shlobj.h>
#include <fstream>
#include <math.h>
#include <string.h>
#include <string>
#include <chrono>
#include <NuiApi.h>
#include <NuiSkeleton.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>
#include <strsafe.h>
#include <stdlib.h>

#include "mysqlUtil.h"
#include "leakCheck.h"
#include "IOFileManager.h"
#include "KinectOutput.h"
#include "KinectDevice.h"
#include "leakCheck.h"
using namespace std;

#define PI 3.14159265

//-----------------------------------------------------------------
//---- 输出标识
KinectOutput * KinectOutput::instance = 0;
mysqlUtil MysqlUtil;

KinectOutput::KinectOutput()
{
	logFileColorIndex = 0;
	logFileDepthIndex = 0;
	logFileRangeIndex = 0;
	logFIleSkeletonIndex = 0;
}

KinectOutput * KinectOutput::getInstance()
{
	if (instance == 0)
	{
		instance = new KinectOutput();
	}

	return instance;
}


//---- 针对于彩色、深度以及初始化
void KinectOutput::init(string pathFileColor, string pathFileDepth, string pathFileRange)
{
	loggerFileColor = ofstream(pathFileColor);
	loggerFileDepth = ofstream(pathFileDepth);
	loggerFileRange = ofstream(pathFileRange);

	cout << "pathFileRange:" << pathFileRange << endl;

	logFileColorIndex = 0;
	logFileDepthIndex = 0;
	logFileRangeIndex = 0;
	logFIleSkeletonIndex = 0;
}

//---- 存储彩色图片
void KinectOutput::saveKinectColor(byte * data, int width, int height, string path, bool isLog, double timestamp)
{
	IOFileManager::saveImageBMP(data, width, height, path, IOFileManager::METHOD_WINDOWS);

	string fileName = path.substr(path.find_last_of('/') + 1) + ".bmp";
	if (isLog) { loggerFileColor << logFileColorIndex << "," << timestamp << "," << fileName << endl; }

	loggerFileColor.clear();
	//loggerFileColor.flush();
	logFileColorIndex++;
}

//---- 存储深度图片
void KinectOutput::saveKinectDepth(byte * data, int width, int height, string path, bool isLog, double timestamp)
{
	IOFileManager::saveImageBMP(data, width, height, path, IOFileManager::METHOD_WINDOWS);

	string fileName = path.substr(path.find_last_of('/') + 1) + ".bmp";
	if (isLog) { loggerFileDepth << logFileDepthIndex << "," << timestamp << "," << fileName << endl; }
	
	//loggerFileDepth.clear();
	cout << "loggerFileDepth: " << sizeof(loggerFileDepth)<<endl;
	//loggerFileDepth.flush();
	logFileDepthIndex++;
	
}

void KinectOutput::saveKinectRange(unsigned short * data, int width, int height, string path, bool isLog, double timestamp)
{
	IOFileManager::saveImageBIN(data, width, height, path, IOFileManager::METHOD_LZ4);

	string fileName = path.substr(path.find_last_of('/') + 1) + ".lz4"; 
	if (isLog) { loggerFileRange << logFileRangeIndex << "," << timestamp << "," << fileName << endl; }
	
	//loggerFileRange.clear();
	//loggerFileRange.flush();
	logFileRangeIndex++;
}




//---- 保存骨骼数据
void KinectOutput::saveKinectSkeleton(SkeletonBody skeleton, int index)
{
	static float usingSkeleton[11] = {0};
	// 在kienctd的深度数据中，每一个骨骼关键点都是一个三维立体空间，因而都会对应有x, y , z三个坐标
	// 坐标系的建系规则，采用了右手法则，z轴向外的建系规则
	// 肩中点
	const Vector4 &spineBasePos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER];
	// 头部
	const Vector4 &headPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HEAD];
	// 左臀
	const Vector4 &hipLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT];
	// 右臀
	const Vector4 &hipRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT];
	// 左膝
	const Vector4 &kneeLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT];
	// 右膝
	const Vector4 &kneeRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT];
	// 左脚踝
	const Vector4 &ankleLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_ANKLE_LEFT];
	// 右脚踝
	const Vector4 &ankleRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_ANKLE_RIGHT];
	// 左脚
	const Vector4 &footLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT];
	// 右脚
	const Vector4 &footRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT];
	// 腰中点
	const Vector4 &spineShoulderPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	// 计算点与点之间的距离 共计22个距离
	
	// 左臀部与左膝盖之间的距离
	float a = sqrt(pow(hipLeftPos.x - kneeLeftPos.x, 2) + pow(hipLeftPos.y - kneeLeftPos.y, 2) + pow(hipLeftPos.z - kneeLeftPos.z, 2));
	// 左臀部与肩中点之间的距离
	float b = sqrt(pow(spineBasePos.x - hipLeftPos.x, 2) + pow(spineBasePos.y - hipLeftPos.y, 2) + pow(spineBasePos.z - hipLeftPos.z, 2));
	// 肩中点与左膝盖之间的距离
	float c = sqrt(pow(spineBasePos.x - kneeLeftPos.x, 2) + pow(spineBasePos.y - kneeLeftPos.y, 2) + pow(spineBasePos.z - kneeLeftPos.z, 2));
	// 右臀部与右膝盖之间的距离
	float d = sqrt(pow(hipRightPos.x - kneeRightPos.x, 2) + pow(hipRightPos.y - kneeRightPos.y, 2) + pow(hipRightPos.z - kneeRightPos.z, 2));
	// 肩中点与右臀部之间的距离
	float e = sqrt(pow(spineBasePos.x - hipRightPos.x, 2) + pow(spineBasePos.y - hipRightPos.y, 2) + pow(spineBasePos.z - hipRightPos.z, 2));
	// 肩中点与右膝盖之间的距离
	float f = sqrt(pow(spineBasePos.x - kneeRightPos.x, 2) + pow(spineBasePos.y - kneeRightPos.y, 2) + pow(spineBasePos.z - kneeRightPos.z, 2));
	// 左臀部与左脚踝之间的距离
	float g = sqrt(pow(hipLeftPos.x - ankleLeftPos.x, 2) + pow(hipLeftPos.y - ankleLeftPos.y, 2) + pow(hipLeftPos.z - ankleLeftPos.z, 2));
	// 左膝盖与左脚踝之间的距离
	float h = sqrt(pow(kneeLeftPos.x - ankleLeftPos.x, 2) + pow(kneeLeftPos.y - ankleLeftPos.y, 2) + pow(kneeLeftPos.z - ankleLeftPos.z, 2));
	// 右臀部与右脚踝之间距离
	float i = sqrt(pow(hipRightPos.x - ankleRightPos.x, 2) + pow(hipRightPos.y - ankleRightPos.y, 2) + pow(hipRightPos.z - ankleRightPos.z, 2));
	// 右膝盖与右脚踝之间的距离
	float j = sqrt(pow(kneeRightPos.x - ankleRightPos.x, 2) + pow(kneeRightPos.y - ankleRightPos.y, 2) + pow(kneeRightPos.z - ankleRightPos.z, 2));
	// 左膝盖与左脚之间的距离
	float k = sqrt(pow(kneeLeftPos.x - footLeftPos.x, 2) + pow(kneeLeftPos.y - footLeftPos.y, 2) + pow(kneeLeftPos.z - footLeftPos.z, 2));
	// 左脚踝与左脚之间的距离
	float l = sqrt(pow(ankleLeftPos.x - footLeftPos.x, 2) + pow(ankleLeftPos.y - footLeftPos.y, 2) + pow(ankleLeftPos.z - footLeftPos.z, 2));
	// 右膝盖与右脚之间的距离
	float m = sqrt(pow(kneeRightPos.x - footRightPos.x, 2) + pow(kneeRightPos.y - footRightPos.y, 2) + pow(kneeRightPos.z - footRightPos.z, 2));
	// 右脚踝与右脚之间的距离
	float n = sqrt(pow(ankleRightPos.x - footRightPos.x, 2) + pow(ankleRightPos.y - footRightPos.y, 2) + pow(ankleRightPos.z - footRightPos.z, 2));
	// 
	float o = sqrt(pow((0.5*(ankleLeftPos.x + ankleRightPos.x)) - footLeftPos.x, 2) + pow(ankleLeftPos.z - footLeftPos.z, 2));
	// 
	float p = sqrt(pow((0.5*(ankleLeftPos.x + ankleRightPos.x)) - footRightPos.x, 2) + pow(ankleRightPos.z - footRightPos.z, 2));
	// 左脚与右脚之间的距离
	float q = sqrt(pow(footLeftPos.x - footRightPos.x, 2) + pow(footLeftPos.y - footRightPos.y, 2) + pow(footLeftPos.z - footRightPos.z, 2));
	// 
	float r = sqrt(pow(spineBasePos.x - spineShoulderPos.x, 2) + pow(spineBasePos.z - spineShoulderPos.z, 2));
	// 
	float s = sqrt(r + pow(spineShoulderPos.y - spineBasePos.y, 2));
	// 肩中点与腰中点之间的距离
	float t = sqrt(pow(spineShoulderPos.x - spineBasePos.x, 2) + pow(spineShoulderPos.y - spineBasePos.y, 2) + pow(spineShoulderPos.z - spineBasePos.z, 2));
	//
	float u = sqrt(pow(spineShoulderPos.x - ((kneeLeftPos.x + kneeRightPos.x) / 2), 2) + pow(spineShoulderPos.y - ((kneeLeftPos.y + kneeRightPos.y) / 2), 2) + pow(spineShoulderPos.z - ((kneeLeftPos.z + kneeRightPos.z) / 2), 2));
	// 
	float v = sqrt(pow(spineBasePos.x - ((kneeLeftPos.x + kneeRightPos.x) / 2), 2) + pow(spineBasePos.y - ((kneeLeftPos.y + kneeRightPos.y) / 2), 2) + pow(spineBasePos.z - ((kneeLeftPos.z + kneeRightPos.z) / 2), 2));

	//---- 7个特征角度
	// 高度
	float height = headPos.y - std::fmin(footLeftPos.y, footRightPos.y); // 头的纵坐标 - 脚的的最低处
	// 左臀骨角度
	float leftHipAngle = acos((pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a*b)) * 180 / PI;  //180 - (acos(a/c) *180.0 / PI) - (acos(b/c) *180.0 / PI);
	// 右臀固角度
	float rightHipAngle = acos((pow(e, 2) + pow(d, 2) - pow(f, 2)) / (2 * e*d)) * 180 / PI; // 180 - (acos(d / f) *180.0 / PI) - (acos(e / f) *180.0 / PI);
	// 左膝关节角度
	float leftKneeAngle = acos((pow(a, 2) + pow(h, 2) - pow(g, 2)) / (2 * a*h)) * 180 / PI; // 180 - (acos(a / g) *180.0 / PI) - (acos(h / g) *180.0 / PI);
	// 右膝关节角度
	float rightKneeAngle = acos((pow(d, 2) + pow(j, 2) - pow(i, 2)) / (2 * d*j)) * 180 / PI; // 180 - (acos(d / i) *180.0 / PI) - (acos(j / i) *180.0 / PI);
																							 //float leftAnkleAngle = acos((pow(h, 2) + pow(l, 2) - pow(k, 2)) / (2 * h*l)) * 180 / PI; // 180 - (acos(h / k) *180.0 / PI) - (acos(l / k) *180.0 / PI);
																							 //float rightAnkleAngle = acos((pow(j, 2) + pow(n, 2) - pow(m, 2)) / (2 * j*n)) * 180 / PI;  // 180 - (acos(j / m) *180.0 / PI) - (acos(n / m) *180.0 / PI);
																							 //float twoFeetAngle = acos((pow(o, 2) + pow(p, 2) - pow(q, 2)) / (2 * o*p)) * 180 / PI; // 180 - (acos(o / q) *180.0 / PI) - (acos(p / q) *180.0 / PI);
	// 脊椎骨与垂直线角度
	float chestAngle = 180 - (acos((pow(t, 2) + pow(s, 2) - pow(r, 2)) / (2 * t*s)) * 180 / PI); //acos((pow(r, 2) + pow(s, 2) - pow(t, 2)) / (2 * r*s)) * 180 / PI;
	// 脊椎骨与左膝盖角度
	float chestKneeAngle = acos((pow(t, 2) + pow(v, 2) - pow(u, 2)) / (2 * t*v)) * 180 / PI;
	// 对于相等的数据才进行写入操作，但是联调时出现了这几个数据不完整写入文档问题
	/*
	if ((height == height && leftHipAngle == leftHipAngle && rightHipAngle == rightHipAngle && leftKneeAngle == leftKneeAngle
		&& rightKneeAngle == rightKneeAngle && chestAngle == chestAngle && chestKneeAngle == chestKneeAngle && footRightPos.y == footRightPos.y
		&& footLeftPos.y == footLeftPos.y) && (height != NULL || leftHipAngle != NULL || rightHipAngle != NULL || leftKneeAngle != NULL || rightKneeAngle != NULL
			|| chestAngle != NULL || chestKneeAngle != NULL || footRightPos.y != 0 || footLeftPos.y != 0)){
		loggerSkeletonJoint << height<< "\n" << leftHipAngle << "\n" << rightHipAngle << "\n" << leftKneeAngle << "\n" << rightKneeAngle << "\n" << chestAngle <<"\n" << chestKneeAngle << "\n" << footRightPos.y<<"\n" << footLeftPos.y <<"\n"<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "\n";
	}
	*/
	usingSkeleton[0] = height;
	usingSkeleton[1] = leftHipAngle;
	usingSkeleton[2] = rightHipAngle;
	usingSkeleton[3] = leftKneeAngle;
	usingSkeleton[4] = rightKneeAngle;
	usingSkeleton[5] = chestAngle;
	usingSkeleton[6] = chestKneeAngle;
	usingSkeleton[7] = footRightPos.y;
	usingSkeleton[8] = footLeftPos.y;
	usingSkeleton[9] = index;

	if (height == height && leftHipAngle == leftHipAngle && rightHipAngle == rightHipAngle && leftKneeAngle == leftKneeAngle
		&& rightKneeAngle == rightKneeAngle && chestAngle == chestAngle && chestKneeAngle == chestKneeAngle && footRightPos.y == footRightPos.y
		&& footLeftPos.y == footLeftPos.y) {
		
		// 将数据写入数据库
		MysqlUtil.skeletonInsert(usingSkeleton);
	}

	logFIleSkeletonIndex++;

}

//---- 保存骨骼数据和时间戳
void KinectOutput::saveKinectSkeleton(SkeletonBody  * skeletonPool, int timestamp)
{
	if (!loggerSkeletonJoint.is_open()) { return; }

	// 计算捕捉的对象个数
	int countSkeletonDetected = 0;
	for (int i = 0; i < KinectDevice::SKELETON_MAX_COUNT; i++) 
	{ 
		if (skeletonPool[i].isTracked) 
		{ 
			countSkeletonDetected++; 
		} 
	}

	// loggerSkeletonJoint << timestamp << ", " << countSkeletonDetected << endl;

	for (int i = 0; i < KinectDevice::SKELETON_MAX_COUNT; i++)
	{
		if (skeletonPool[i].isTracked)
		{
			// loggerSkeletonJoint << i << endl;

			cout << "i: " << i << endl;

			// 保存骨骼数据
			saveKinectSkeleton(skeletonPool[i], i);
		}
	}
}

//---- 保存骨骼数据
void KinectOutput::saveKinectSkeletonImage(SkeletonBody * skeletonPool, int width, int height, string path)
{
	// 为了在后面保存数据，有X,Y的双倍
	const int acsize = SkeletonBody::MAX_SKELETON_JOINT * 2;
	const int adsize = SkeletonBody::SKELETON_ATLAS_SIZE;

	for (int i = 0; i < KinectDevice::SKELETON_MAX_COUNT; i++)
	{
		if (skeletonPool[i].isTracked)
		{
			SkeletonBody skeleton = KinectDevice::convertSkeletonCoordinates(skeletonPool[i], width, height);

			// 用两倍的空间去转换成X,Y坐标
			float jointAtlasCoordinates[acsize];
			int jointAtlasDescription[adsize];

			skeleton.getSkeletonAtlasCoordinates(jointAtlasCoordinates);
			skeleton.getSkeletonAtlasDescription(jointAtlasDescription);

			IOFileManager::saveSkeletonImage(jointAtlasCoordinates, acsize, jointAtlasDescription, adsize, width, height, path);
			
			return;
		}
	}
}


//-----------------------------------------------------------------
void KinectOutput::loggerSkeletonJointOPEN(string path)
{
	loggerSkeletonJoint.open(path);
}

void KinectOutput::loggerSkeletonJointCLOSE()
{
	loggerSkeletonJoint.close();
}

//-----------------------------------------------------------------
int KinectOutput::getCountSavedFileColor()
{
	return logFileColorIndex;
}

int KinectOutput::getCountSavedFileDepth()
{
	return logFileDepthIndex;
}

int KinectOutput::getCountSavedFileRange()
{
	return logFileRangeIndex;
}