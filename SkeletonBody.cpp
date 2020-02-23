#include <windows.h>


#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

#include "SkeletonBody.h"
#include "leakCheck.h"

SkeletonBody::SkeletonBody()
{
	isTracked = false;
}


SkeletonBody::SkeletonBody(const SkeletonBody &skeleton)
{
	this->isTracked = skeleton.isTracked;

	for (int i = 0; i < MAX_SKELETON_JOINT; i++)
	{
		this->skeletonPosition[i] = skeleton.skeletonPosition[i];
	}
}

void SkeletonBody::getSkeletonAtlasCoordinates(float * coordinates)
{
	for (int i = 0; i < MAX_SKELETON_JOINT; i++)
	{
		*(coordinates + 2 * i) = skeletonPosition[i].x;
		*(coordinates + 2 * i + 1) = skeletonPosition[i].y;
	}
}

// 所有骨骼数据
void SkeletonBody::getSkeletonAtlasDescription(int * indexList)
{
	//---- SKELETON [TORSO]
	indexList[0] = NUI_SKELETON_POSITION_HEAD;
	indexList[1] = NUI_SKELETON_POSITION_SHOULDER_CENTER;
	
	indexList[2] = NUI_SKELETON_POSITION_SHOULDER_CENTER;
	indexList[3] = NUI_SKELETON_POSITION_SHOULDER_LEFT;

	indexList[4] = NUI_SKELETON_POSITION_SHOULDER_CENTER; 
	indexList[5] = NUI_SKELETON_POSITION_SHOULDER_RIGHT;

	indexList[6] = NUI_SKELETON_POSITION_SHOULDER_CENTER; 
	indexList[7] = NUI_SKELETON_POSITION_SPINE;

	indexList[8] = NUI_SKELETON_POSITION_SPINE;
	indexList[9] = NUI_SKELETON_POSITION_HIP_CENTER;

	indexList[10] = NUI_SKELETON_POSITION_HIP_CENTER; 
	indexList[11] = NUI_SKELETON_POSITION_HIP_LEFT;

	indexList[12] = NUI_SKELETON_POSITION_HIP_CENTER; 
	indexList[13] = NUI_SKELETON_POSITION_HIP_RIGHT;

	//---- SKELETON [LEFT ARM]
	indexList[14] = NUI_SKELETON_POSITION_SHOULDER_LEFT; 
	indexList[15] = NUI_SKELETON_POSITION_ELBOW_LEFT;

	indexList[16] = NUI_SKELETON_POSITION_ELBOW_LEFT; 
	indexList[17] = NUI_SKELETON_POSITION_WRIST_LEFT;

	indexList[18] = NUI_SKELETON_POSITION_WRIST_LEFT; 
	indexList[19] = NUI_SKELETON_POSITION_HAND_LEFT;

	//---- SKELETON [RIGHT ARM]
	indexList[20] = NUI_SKELETON_POSITION_SHOULDER_RIGHT; 
	indexList[21] = NUI_SKELETON_POSITION_ELBOW_RIGHT;

	indexList[22] = NUI_SKELETON_POSITION_ELBOW_RIGHT; 
	indexList[23] = NUI_SKELETON_POSITION_WRIST_RIGHT;

	indexList[24] = NUI_SKELETON_POSITION_WRIST_RIGHT;
	indexList[25] = NUI_SKELETON_POSITION_HAND_RIGHT;

	//---- SKELETON [LEFT LEG]
	indexList[26] = NUI_SKELETON_POSITION_HIP_LEFT; 
	indexList[27] = NUI_SKELETON_POSITION_KNEE_LEFT;

	indexList[28] = NUI_SKELETON_POSITION_KNEE_LEFT; 
	indexList[29] = NUI_SKELETON_POSITION_ANKLE_LEFT;

	indexList[30] = NUI_SKELETON_POSITION_ANKLE_LEFT; 
	indexList[31] = NUI_SKELETON_POSITION_FOOT_LEFT;

	//---- SKELETON [RIGHT LEG]
	indexList[32] = NUI_SKELETON_POSITION_HIP_RIGHT; 
	indexList[33] = NUI_SKELETON_POSITION_KNEE_RIGHT;

	indexList[34] = NUI_SKELETON_POSITION_KNEE_RIGHT; 
	indexList[35] = NUI_SKELETON_POSITION_ANKLE_RIGHT;

	indexList[36] = NUI_SKELETON_POSITION_ANKLE_RIGHT; 
	indexList[37] = NUI_SKELETON_POSITION_FOOT_RIGHT;
}