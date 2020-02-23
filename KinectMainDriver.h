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
	// ���ڲ���֡״̬
	HANDLE hevents[4];

	// ��������
	SkeletonBody skeletonPool[KinectDevice::SKELETON_MAX_COUNT];

	// ��׽������ݱ���ΪͼƬ
	byte dataDepth[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT * 4];

	// Color data captured as image
	// ��׽��ɫ���ݱ���ΪͼƬ
	byte dataColor[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT * 4];

	byte dataBodyMask[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT];

	// ���ݷ�Χ
	unsigned short dataRange[KinectDevice::DEFAULT_WIDTH*KinectDevice::DEFAULT_HEIGHT];

	string PATH_DIRECTORY_RGB = "";
	string PATH_DIRECTORY_DPT = "";
	string PATH_DIRECTORY_DPTI = "";
	string PATH_DIRECTORY_SKLI = "";
	string PATH_DIRECTORY_SKL = "";

	// �ж��Ƿ��ʼ��
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
	//---- �˾����п�ͼ
	byte * getDataChromakey(bool isColor = true)
	{
		static byte chromaKey[KinectDevice::DEFAULT_WIDTH * KinectDevice::DEFAULT_HEIGHT * 4];
		// ʹ����ɫ����chromakey(���isColor = true)�����߽�����Ϊbody mask 
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

					// ����豸�ܹ���׽��ȷ�����ݣ�
					// �ҵõ���������������Χ�ڣ�
					// �ҵõ���������������ؼ������㣬
					// ��������Щ����ֵ������Ϊ��ɫ
					if (colorX >= 0 && colorY >= 0 && colorX < KinectDevice::DEFAULT_WIDTH && colorY < KinectDevice::DEFAULT_HEIGHT && dataRange[cindex] != 0 && dataBodyMask[cindex] != 0)
					{
						if (dataBodyMask[cindex] != 0)
						{
							long colorIndex = (colorX)+(colorY)* KinectDevice::DEFAULT_WIDTH;

							// ��ȡRGBA����
							chromaKey[4 * cindex] = dataColor[4 * colorIndex];
							chromaKey[4 * cindex + 1] = dataColor[4 * colorIndex + 1];
							chromaKey[4 * cindex + 2] = dataColor[4 * colorIndex + 2];
							chromaKey[4 * cindex + 3] = dataColor[4 * colorIndex + 3];
						}
					}
					else
					{
						// ���û�У���ȫ��ת����(0, 0, 0 ,0) -- ��ɫ
						chromaKey[4 * cindex] = 0;
						chromaKey[4 * cindex + 1] = 0;
						chromaKey[4 * cindex + 2] = 0;
						chromaKey[4 * cindex + 3] = 0;
					}
				}
			}
		}

		// �ڶ�ֵͼ���ϻ���������
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
					// ���û�У���ȫ��ת����(0, 0, 0 ,0) -- ��ɫ
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

	// ��ȡ�������ݣ������6
	SkeletonBody * getDataSkeletonPool();

};


#endif 

