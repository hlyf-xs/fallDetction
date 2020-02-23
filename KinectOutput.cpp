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
//---- �����ʶ
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


//---- ����ڲ�ɫ������Լ���ʼ��
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

//---- �洢��ɫͼƬ
void KinectOutput::saveKinectColor(byte * data, int width, int height, string path, bool isLog, double timestamp)
{
	IOFileManager::saveImageBMP(data, width, height, path, IOFileManager::METHOD_WINDOWS);

	string fileName = path.substr(path.find_last_of('/') + 1) + ".bmp";
	if (isLog) { loggerFileColor << logFileColorIndex << "," << timestamp << "," << fileName << endl; }

	loggerFileColor.clear();
	//loggerFileColor.flush();
	logFileColorIndex++;
}

//---- �洢���ͼƬ
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




//---- �����������
void KinectOutput::saveKinectSkeleton(SkeletonBody skeleton, int index)
{
	static float usingSkeleton[11] = {0};
	// ��kienctd����������У�ÿһ�������ؼ��㶼��һ����ά����ռ䣬��������Ӧ��x, y , z��������
	// ����ϵ�Ľ�ϵ���򣬲��������ַ���z������Ľ�ϵ����
	// ���е�
	const Vector4 &spineBasePos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER];
	// ͷ��
	const Vector4 &headPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HEAD];
	// ����
	const Vector4 &hipLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT];
	// ����
	const Vector4 &hipRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT];
	// ��ϥ
	const Vector4 &kneeLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT];
	// ��ϥ
	const Vector4 &kneeRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT];
	// �����
	const Vector4 &ankleLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_ANKLE_LEFT];
	// �ҽ���
	const Vector4 &ankleRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_ANKLE_RIGHT];
	// ���
	const Vector4 &footLeftPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT];
	// �ҽ�
	const Vector4 &footRightPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT];
	// ���е�
	const Vector4 &spineShoulderPos = skeleton.skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	// ��������֮��ľ��� ����22������
	
	// ���β�����ϥ��֮��ľ���
	float a = sqrt(pow(hipLeftPos.x - kneeLeftPos.x, 2) + pow(hipLeftPos.y - kneeLeftPos.y, 2) + pow(hipLeftPos.z - kneeLeftPos.z, 2));
	// ���β�����е�֮��ľ���
	float b = sqrt(pow(spineBasePos.x - hipLeftPos.x, 2) + pow(spineBasePos.y - hipLeftPos.y, 2) + pow(spineBasePos.z - hipLeftPos.z, 2));
	// ���е�����ϥ��֮��ľ���
	float c = sqrt(pow(spineBasePos.x - kneeLeftPos.x, 2) + pow(spineBasePos.y - kneeLeftPos.y, 2) + pow(spineBasePos.z - kneeLeftPos.z, 2));
	// ���β�����ϥ��֮��ľ���
	float d = sqrt(pow(hipRightPos.x - kneeRightPos.x, 2) + pow(hipRightPos.y - kneeRightPos.y, 2) + pow(hipRightPos.z - kneeRightPos.z, 2));
	// ���е������β�֮��ľ���
	float e = sqrt(pow(spineBasePos.x - hipRightPos.x, 2) + pow(spineBasePos.y - hipRightPos.y, 2) + pow(spineBasePos.z - hipRightPos.z, 2));
	// ���е�����ϥ��֮��ľ���
	float f = sqrt(pow(spineBasePos.x - kneeRightPos.x, 2) + pow(spineBasePos.y - kneeRightPos.y, 2) + pow(spineBasePos.z - kneeRightPos.z, 2));
	// ���β��������֮��ľ���
	float g = sqrt(pow(hipLeftPos.x - ankleLeftPos.x, 2) + pow(hipLeftPos.y - ankleLeftPos.y, 2) + pow(hipLeftPos.z - ankleLeftPos.z, 2));
	// ��ϥ���������֮��ľ���
	float h = sqrt(pow(kneeLeftPos.x - ankleLeftPos.x, 2) + pow(kneeLeftPos.y - ankleLeftPos.y, 2) + pow(kneeLeftPos.z - ankleLeftPos.z, 2));
	// ���β����ҽ���֮�����
	float i = sqrt(pow(hipRightPos.x - ankleRightPos.x, 2) + pow(hipRightPos.y - ankleRightPos.y, 2) + pow(hipRightPos.z - ankleRightPos.z, 2));
	// ��ϥ�����ҽ���֮��ľ���
	float j = sqrt(pow(kneeRightPos.x - ankleRightPos.x, 2) + pow(kneeRightPos.y - ankleRightPos.y, 2) + pow(kneeRightPos.z - ankleRightPos.z, 2));
	// ��ϥ�������֮��ľ���
	float k = sqrt(pow(kneeLeftPos.x - footLeftPos.x, 2) + pow(kneeLeftPos.y - footLeftPos.y, 2) + pow(kneeLeftPos.z - footLeftPos.z, 2));
	// ����������֮��ľ���
	float l = sqrt(pow(ankleLeftPos.x - footLeftPos.x, 2) + pow(ankleLeftPos.y - footLeftPos.y, 2) + pow(ankleLeftPos.z - footLeftPos.z, 2));
	// ��ϥ�����ҽ�֮��ľ���
	float m = sqrt(pow(kneeRightPos.x - footRightPos.x, 2) + pow(kneeRightPos.y - footRightPos.y, 2) + pow(kneeRightPos.z - footRightPos.z, 2));
	// �ҽ������ҽ�֮��ľ���
	float n = sqrt(pow(ankleRightPos.x - footRightPos.x, 2) + pow(ankleRightPos.y - footRightPos.y, 2) + pow(ankleRightPos.z - footRightPos.z, 2));
	// 
	float o = sqrt(pow((0.5*(ankleLeftPos.x + ankleRightPos.x)) - footLeftPos.x, 2) + pow(ankleLeftPos.z - footLeftPos.z, 2));
	// 
	float p = sqrt(pow((0.5*(ankleLeftPos.x + ankleRightPos.x)) - footRightPos.x, 2) + pow(ankleRightPos.z - footRightPos.z, 2));
	// ������ҽ�֮��ľ���
	float q = sqrt(pow(footLeftPos.x - footRightPos.x, 2) + pow(footLeftPos.y - footRightPos.y, 2) + pow(footLeftPos.z - footRightPos.z, 2));
	// 
	float r = sqrt(pow(spineBasePos.x - spineShoulderPos.x, 2) + pow(spineBasePos.z - spineShoulderPos.z, 2));
	// 
	float s = sqrt(r + pow(spineShoulderPos.y - spineBasePos.y, 2));
	// ���е������е�֮��ľ���
	float t = sqrt(pow(spineShoulderPos.x - spineBasePos.x, 2) + pow(spineShoulderPos.y - spineBasePos.y, 2) + pow(spineShoulderPos.z - spineBasePos.z, 2));
	//
	float u = sqrt(pow(spineShoulderPos.x - ((kneeLeftPos.x + kneeRightPos.x) / 2), 2) + pow(spineShoulderPos.y - ((kneeLeftPos.y + kneeRightPos.y) / 2), 2) + pow(spineShoulderPos.z - ((kneeLeftPos.z + kneeRightPos.z) / 2), 2));
	// 
	float v = sqrt(pow(spineBasePos.x - ((kneeLeftPos.x + kneeRightPos.x) / 2), 2) + pow(spineBasePos.y - ((kneeLeftPos.y + kneeRightPos.y) / 2), 2) + pow(spineBasePos.z - ((kneeLeftPos.z + kneeRightPos.z) / 2), 2));

	//---- 7�������Ƕ�
	// �߶�
	float height = headPos.y - std::fmin(footLeftPos.y, footRightPos.y); // ͷ�������� - �ŵĵ���ʹ�
	// ���ιǽǶ�
	float leftHipAngle = acos((pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a*b)) * 180 / PI;  //180 - (acos(a/c) *180.0 / PI) - (acos(b/c) *180.0 / PI);
	// ���ι̽Ƕ�
	float rightHipAngle = acos((pow(e, 2) + pow(d, 2) - pow(f, 2)) / (2 * e*d)) * 180 / PI; // 180 - (acos(d / f) *180.0 / PI) - (acos(e / f) *180.0 / PI);
	// ��ϥ�ؽڽǶ�
	float leftKneeAngle = acos((pow(a, 2) + pow(h, 2) - pow(g, 2)) / (2 * a*h)) * 180 / PI; // 180 - (acos(a / g) *180.0 / PI) - (acos(h / g) *180.0 / PI);
	// ��ϥ�ؽڽǶ�
	float rightKneeAngle = acos((pow(d, 2) + pow(j, 2) - pow(i, 2)) / (2 * d*j)) * 180 / PI; // 180 - (acos(d / i) *180.0 / PI) - (acos(j / i) *180.0 / PI);
																							 //float leftAnkleAngle = acos((pow(h, 2) + pow(l, 2) - pow(k, 2)) / (2 * h*l)) * 180 / PI; // 180 - (acos(h / k) *180.0 / PI) - (acos(l / k) *180.0 / PI);
																							 //float rightAnkleAngle = acos((pow(j, 2) + pow(n, 2) - pow(m, 2)) / (2 * j*n)) * 180 / PI;  // 180 - (acos(j / m) *180.0 / PI) - (acos(n / m) *180.0 / PI);
																							 //float twoFeetAngle = acos((pow(o, 2) + pow(p, 2) - pow(q, 2)) / (2 * o*p)) * 180 / PI; // 180 - (acos(o / q) *180.0 / PI) - (acos(p / q) *180.0 / PI);
	// ��׵���봹ֱ�߽Ƕ�
	float chestAngle = 180 - (acos((pow(t, 2) + pow(s, 2) - pow(r, 2)) / (2 * t*s)) * 180 / PI); //acos((pow(r, 2) + pow(s, 2) - pow(t, 2)) / (2 * r*s)) * 180 / PI;
	// ��׵������ϥ�ǽǶ�
	float chestKneeAngle = acos((pow(t, 2) + pow(v, 2) - pow(u, 2)) / (2 * t*v)) * 180 / PI;
	// ������ȵ����ݲŽ���д���������������ʱ�������⼸�����ݲ�����д���ĵ�����
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
		
		// ������д�����ݿ�
		MysqlUtil.skeletonInsert(usingSkeleton);
	}

	logFIleSkeletonIndex++;

}

//---- ����������ݺ�ʱ���
void KinectOutput::saveKinectSkeleton(SkeletonBody  * skeletonPool, int timestamp)
{
	if (!loggerSkeletonJoint.is_open()) { return; }

	// ���㲶׽�Ķ������
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

			// �����������
			saveKinectSkeleton(skeletonPool[i], i);
		}
	}
}

//---- �����������
void KinectOutput::saveKinectSkeletonImage(SkeletonBody * skeletonPool, int width, int height, string path)
{
	// Ϊ���ں��汣�����ݣ���X,Y��˫��
	const int acsize = SkeletonBody::MAX_SKELETON_JOINT * 2;
	const int adsize = SkeletonBody::SKELETON_ATLAS_SIZE;

	for (int i = 0; i < KinectDevice::SKELETON_MAX_COUNT; i++)
	{
		if (skeletonPool[i].isTracked)
		{
			SkeletonBody skeleton = KinectDevice::convertSkeletonCoordinates(skeletonPool[i], width, height);

			// �������Ŀռ�ȥת����X,Y����
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