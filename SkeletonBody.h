#ifndef SKELETONB_H
#define SKELETONB_H

#include <windows.h>


#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

typedef struct BONE { int j1; int j2; };

namespace JOINT_ID_KINECTV1
{
	static const int JOINT_HIP_CENTER = 0;
	static const int JOINT_SPINE = 1;
	static const int JOINT_SHOULDER_CENTER = 2;
	static const int JOINT_HEAD = 3;

	static const int JOINT_LEFT_SHOULDER = 4;
	static const int JOINT_LEFT_ELBOW = 5;
	static const int JOINT_LEFT_WRIST = 6;
	static const int JOINT_LEFT_HAND = 7;

	static const int JOINT_RIGHT_SHOULDER = 8;
	static const int JOINT_RIGHT_ELBOW = 9;
	static const int JOINT_RIGHT_WRIST = 10;
	static const int JOINT_RIGHT_HAND = 11;

	static const int JOINT_LEFT_HIP = 12;
	static const int JOINT_LEFT_KNEE = 13;
	static const int JOINT_LEFT_ANKLE = 14;
	static const int JOINT_LEFT_FOOT = 15;

	static const int JOINT_RIGHT_HIP = 16;
	static const int JOINT_RIGHT_KNEE = 17;
	static const int JOINT_RIGHT_ANKLE = 18;
	static const int JOINT_RIGHT_FOOT = 19;

	static const int ATLAS_SIZE = 20;

	static const BONE ATLAS[]
	{
		//---- Head & torso
		{JOINT_HEAD, JOINT_SHOULDER_CENTER},
		{JOINT_SHOULDER_CENTER, JOINT_LEFT_SHOULDER},
		{JOINT_SHOULDER_CENTER, JOINT_RIGHT_SHOULDER},
		{JOINT_SHOULDER_CENTER, JOINT_SPINE},
		{JOINT_SPINE, JOINT_HIP_CENTER},
		{JOINT_HIP_CENTER, JOINT_LEFT_HIP},
		{JOINT_HIP_CENTER, JOINT_RIGHT_HIP},

		//----- Left arm
		{JOINT_LEFT_SHOULDER, JOINT_LEFT_ELBOW},
		{JOINT_LEFT_ELBOW, JOINT_LEFT_WRIST},
		{JOINT_LEFT_WRIST, JOINT_LEFT_HAND},

		//----- Right arm
		{JOINT_RIGHT_SHOULDER, JOINT_RIGHT_ELBOW },
		{JOINT_RIGHT_ELBOW, JOINT_RIGHT_WRIST },
		{JOINT_RIGHT_WRIST, JOINT_RIGHT_HAND },

		//----- Left leg
		{JOINT_LEFT_HIP, JOINT_LEFT_KNEE},
		{JOINT_LEFT_KNEE, JOINT_LEFT_ANKLE},
		{JOINT_LEFT_ANKLE, JOINT_LEFT_FOOT},

		//----- Right leg
		{ JOINT_RIGHT_HIP, JOINT_RIGHT_KNEE},
		{ JOINT_RIGHT_KNEE, JOINT_RIGHT_ANKLE},
		{ JOINT_RIGHT_ANKLE, JOINT_RIGHT_FOOT}
	};

	static const int JOINT_COUNT = 20;

	static const int JOINT_COLLECTION[]
	{
		JOINT_HIP_CENTER, 
		JOINT_SPINE,
		JOINT_SHOULDER_CENTER,
		JOINT_HEAD,

		JOINT_LEFT_SHOULDER,
		JOINT_LEFT_ELBOW,
		JOINT_LEFT_WRIST,
		JOINT_LEFT_HAND,

		JOINT_RIGHT_SHOULDER,
		JOINT_RIGHT_ELBOW,
		JOINT_RIGHT_WRIST,
		JOINT_RIGHT_HAND,

		JOINT_LEFT_HIP,
		JOINT_LEFT_KNEE,
		JOINT_LEFT_ANKLE,
		JOINT_LEFT_FOOT,

		JOINT_RIGHT_HIP,
		JOINT_RIGHT_KNEE,
		JOINT_RIGHT_ANKLE,
		JOINT_RIGHT_FOOT
	};
}

class SkeletonBody
{
public:
	//----------------------------------------

	//---- 计算人体骨骼所有的关键点
	static const int MAX_SKELETON_JOINT = NUI_SKELETON_POSITION_COUNT;

	//---- 表示骨骼图中元素数量
	static const int SKELETON_ATLAS_SIZE = 38;

	//---- 这些骨骼关键点的位置
	Vector4 skeletonPosition[NUI_SKELETON_POSITION_COUNT];

	//---- 如果被追踪
	bool isTracked;

	//----------------------------------------

	SkeletonBody();
	SkeletonBody(const SkeletonBody &skeleton);

	void getSkeletonAtlasCoordinates(float * coordinates);

	void getSkeletonAtlasDescription(int * indexList);

	
};

#endif 