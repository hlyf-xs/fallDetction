#ifndef KINECTGAMEDRIVER_H
#define KINECTGAMEDRIVER_H

#include <string>
#include <Windows.h>

#include "KinectDevice.h"
#include "KinectOutput.h"


class KinectMainDriver
{
private:
	
	static KinectMainDriver * instance;

	KinectMainDriver();
	

	KinectDevice * kinectDevice;
	KinectOutput * kinectOutput;

	//---- These events are needed to check status of captured frames
	// 用于捕获帧状态
	HANDLE hevents[4];

	// 骨骼数据
	SkeletonBody skeletonPool[KinectDevice::SKELETON_MAX_COUNT];

	// 捕捉深度数据保存为图片
	byte dataDepth[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT * 4];

	// Color data captured as image
	// 捕捉颜色数据保存为图片
	byte dataColor[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT * 4];

	byte dataBodyMask[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT];

	// 数据范围
	unsigned short dataRange[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT];

	string PATH_DIRECTORY_RGB = "";
	string PATH_DIRECTORY_DPT = "";
	string PATH_DIRECTORY_DPTI = "";
	string PATH_DIRECTORY_SKLI = "";
	string PATH_DIRECTORY_SKL = "";

	// 判断是否初始化
	bool isInit=false;

	//---- Flags - true if capturing 
	//---- RGB - color images
	//---- DPT - depth binary files
	//---- SKL - skeleton data as text file
	//---- DPTI - depth as image
	//---- SKLI - skeleton as image
	bool isCaptureRGB;
	bool isCaptureDPT;
	bool isCaptureSKL;
	bool isCaptureDPTI;
	bool isCaptureSKLI;

	//----------------------------------------

	void captureFrameRGB(double timestamp = 0);
	void captureFrameDPT(double timestamp = 0);
	void captureFrameSKL(double timestamp = 0);
	void captureFrameDPTI(double timestamp = 0);

public:
	//----------------------------------------

	static KinectMainDriver * getInstance();
	
	//----------------------------------------
	//---- Initialize the main driver
	//---- In [pathDirRGB] ----- path to the DIRECTORY that will contain colored images
	//---- In [pathDirDPT] ----- path to the DIRECTORY that will conatin depth binary files
	//---- <In (optional) [pathDirDPTI] -- DEBUG!!! path to the DIRECTORY that will contain depth image files
	//---- <In (optional) [pathDirSKLI] -- DEBUG!!! path to the DIRECTORY that will contain skeleton image files
	//---- In [pathFileSKL] ---- path to the FILE that will contain skeleton data
	//---- In [pathLoggerRGB] -- path to the FILE that will contain LOG info about saved color files
	//---- In [pathLoggerDPT] -- path to the FILE that will contain LOG info about saved depth (range) files
	//---- In [isRGB] - flag, set to true if want to capture color
	//---- In [isDPT] - flag, set to true if want to capture depth binary
	//---- <In (optional) [isDPTI] -- flag, set to true if want to capture depth image
	//---- <In (optional) [isSKLI] -- flag, set to true if want to capture skeleton as image (set SKL to true!)
	//---- In [isSKL] - flag, set to true if capture skeleton

	bool init(string pathDirRGB, string pathDirDPT, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isSKL);
	bool init(string pathDirRGB, string pathDirDPT, string pathDirDPTI, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isDPTI, bool isSKL);
	bool init(string pathDirRGB, string pathDirDPT, string pathDirDPTI, string pathDirSKLI, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isDPTI, bool isSKLI, bool isSKL);
	void update(double timestamp = 0);
	void finalize();

	//---- Get kinect RGB data, size (640*480*4)
	byte * getDataColor();

	//---- Get kinect DEPTH data as an RGB image, size (640*480*4)
	byte * getDataDepth();

	//---- Get a body silhouette (color), size (640*480*4)
	//---- 滤镜进行抠图
	byte * getDataChromakey(bool isColor = true)
	{
		static byte chromaKey[KinectDevice::DEFAULT_WIDTH * KinectDevice::DEFAULT_HEIGHT * 4];
		// 使用颜色绘制chromakey(如果isColor = true)，或者仅仅作为body mask 
		if (isColor)
		{
			for (int y = 0; y < KinectDevice::DEFAULT_HEIGHT; y++)
			{
				for (int x = 0; x < KinectDevice::DEFAULT_WIDTH; x++)
				{
					int cindex = x + y * KinectDevice::DEFAULT_WIDTH;

					long colorX = 0;
					long colorY = 0;

					KinectDevice::convertColorToDepthXY(x, y, dataRange[cindex], &colorX, &colorY);

					// 如果设备能够捕捉正确的数据，
					// 且得到的坐标在正常范围内，
					// 且得到的像素属于人体关键骨骼点，
					// 则设置这些像素值，否则都为黑色
					if (colorX >= 0 && colorY >= 0 && colorX < KinectDevice::DEFAULT_WIDTH && colorY < KinectDevice::DEFAULT_HEIGHT && dataRange[cindex] != 0 && dataBodyMask[cindex] != 0)
					{
						if (dataBodyMask[cindex] != 0)
						{
							long colorIndex = (colorX)+(colorY)* KinectDevice::DEFAULT_WIDTH;

							// 获取RGBA数据
							chromaKey[4 * cindex] = dataColor[4 * colorIndex];
							chromaKey[4 * cindex + 1] = dataColor[4 * colorIndex + 1];
							chromaKey[4 * cindex + 2] = dataColor[4 * colorIndex + 2];
							chromaKey[4 * cindex + 3] = dataColor[4 * colorIndex + 3];
						}
					}
					else
					{
						// 如果没有，就全部转换成(0, 0, 0 ,0) -- 黑色
						chromaKey[4 * cindex] = 0;
						chromaKey[4 * cindex + 1] = 0;
						chromaKey[4 * cindex + 2] = 0;
						chromaKey[4 * cindex + 3] = 0;
					}
				}
			}
		}

		// 在二值图像上画身体轮廓
		else
		{
			for (int i = 0; i < KinectDevice::DEFAULT_WIDTH * KinectDevice::DEFAULT_HEIGHT; i++)
			{
				if (dataBodyMask[i] != 0)
				{
					chromaKey[4 * i] = 255;
					chromaKey[4 * i + 1] = 255;
					chromaKey[4 * i + 2] = 255;
					chromaKey[4 * i + 3] = 255;
				}
				else
				{
					// 如果没有，就全部转换成(0, 0, 0 ,0) -- 黑色
					chromaKey[4 * i] = 0;
					chromaKey[4 * i + 1] = 0;
					chromaKey[4 * i + 2] = 0;
					chromaKey[4 * i + 3] = 0;
				}
			}
		}

		//cout << "size of chromaKey:" << sizeof(chromaKey) << endl;

		return chromaKey;
	}

	//---- Get kinect DEPTH data, (in millimeters), size (640*480)
	unsigned short * getDataRange();

	// 获取骨骼数据，最大是6
	SkeletonBody * getDataSkeletonPool();

};


#endif 

