#include <string>
#include <iomanip> 
#include <mysql.h>
#include "KinectMainDriver.h"
#include "KinectDevice.h"
#include "KinectOutput.h"
#include "leakCheck.h"
#include "mysqlUtil.h"

//static byte *chromaKey = new byte[KinectDevice::DEFAULT_WIDTH * KinectDevice::DEFAULT_HEIGHT * 4];

KinectMainDriver * KinectMainDriver::instance = nullptr;
mysqlUtil mysqlUtil1;
// ������ͷų���
template<class Interface>
static inline void safeRelease(Interface *&interfaceToRelease)
{
	if (interfaceToRelease != nullptr) {
		interfaceToRelease->Release();
		interfaceToRelease = nullptr;
	}
}

//---- ʵ��������
KinectMainDriver::KinectMainDriver()
{
	kinectDevice = KinectDevice::getInstance();

	cout << "size of kinectDevice: " << sizeof(kinectDevice) << endl;

	kinectOutput = KinectOutput::getInstance();
	cout << "size of kinectOutput: " << sizeof(kinectOutput) << endl; 

	isInit = false;
}

//---- ��ȡInstance ����ʵ���� instance����
KinectMainDriver * KinectMainDriver::getInstance()
{
	if (instance == 0)
	{
		instance = new KinectMainDriver();
	}

	return instance;
}


//---- kinect ���ݽ��г�ʼ������
// kinect �豸��ʼ��
// kinect �����ʼ��
// kinect �������ݳ�ʼ��
bool KinectMainDriver::init(string pathDirRGB, string pathDirDPT, string pathDirDPTI, string pathDirSKLI, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isDPTI, bool isSKLI, bool isSKL)
{
	PATH_DIRECTORY_RGB = pathDirRGB;
	PATH_DIRECTORY_DPT = pathDirDPT;
	PATH_DIRECTORY_DPTI = pathDirDPTI;
	PATH_DIRECTORY_SKLI = pathDirSKLI;

	isCaptureRGB = isRGB;
	isCaptureDPT = isDPT;
	isCaptureDPTI = isDPTI;
	isCaptureSKLI = isSKLI;
	isCaptureSKL = isSKL;

	// ��ʼ��kienct
	bool isKinectOK = kinectDevice->init();

	if (!isKinectOK) { return false; }

	// ��ʼ��kienct���¼����
	// RGB��DEPTH��SKELETON
	hevents[0] = kinectDevice->getEventStreamRGB();
	hevents[1] = kinectDevice->getEventStreamDEPTH();
	hevents[2] = kinectDevice->getEventStreamSKELETON();

	//cout << "size of havents:" << sizeof(hevents) << endl;

	// ��ʼ����־�ļ�
	kinectOutput->init(pathLoggerRGB, pathLoggerDPT, pathLoggerDPT);

	// ���ļ�д��
	kinectOutput->loggerSkeletonJointOPEN(pathFileSKL);

	isInit = true;

	return true;
}


//---- ��̬����
bool KinectMainDriver::init(string pathDirRGB, string pathDirDPT, string pathDirDPTI, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isDPTI, bool isSKL)
{
	return init(pathDirRGB, pathDirDPT, pathDirDPTI, "", pathFileSKL, pathLoggerRGB, pathLoggerDPT, isRGB, isDPT, isDPTI, false, isSKL);
}

bool KinectMainDriver::init(string pathDirRGB, string pathDirDPT, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isSKL)
{
	return init(pathDirRGB, pathDirDPT, "", pathFileSKL, pathLoggerRGB, pathLoggerDPT, isRGB, isDPT, false, isSKL);
}

//---- ���ݸ���
// ��ôԴԴ���ϵ����ݻ᲻�ᱻ�����أ�
// ���е�timestamp��ϵͳ���¼��Զ���
void KinectMainDriver::update(double timestamp)
{
	if (!isInit) { return; }

	// �ȴ�������
	// INFINTE ���޵ȴ���ֱ�����ŵ��������
	MsgWaitForMultipleObjects(3, hevents, FALSE, INFINITE, QS_ALLINPUT);

	// ��׽֡����
	// �ж��ͷŵ��Ƿ��� DPTI ���� DPT
	
	// ��ɫ����
	if (isCaptureRGB) { captureFrameRGB(timestamp);  }

	// �������
	if (isCaptureDPTI & !isCaptureDPT) { captureFrameDPTI(timestamp); }

	// ��������
	if (isCaptureDPT) 
	{ 
		captureFrameDPT(timestamp);

		// ֻ������ʾ���ͼ��ʱ�Ž��д�ת��
		if (isCaptureDPTI)
		{
			for (int i = 0, index = 0; i < KinectDevice::DEFAULT_WIDTH * KinectDevice::DEFAULT_HEIGHT; i++, index += 4)
			{
				dataDepth[index] = (int)((double)dataRange[i] * 255 / 8000);
				dataDepth[index + 1] = (int)((double)dataRange[i] * 255 / 8000);
				dataDepth[index + 2] = (int)((double)dataRange[i] * 255 / 8000);
				dataDepth[index + 3] = 255;

				// ��� 1M �������
				//cout << "sizeOf dataDepth:" << sizeof(dataDepth)/1024/1024 << endl;
			}
		}
	}
	// �й������ݵĻ����ͻ�ȡ��������
	if (isCaptureSKL) { captureFrameSKL(timestamp);  }
}


//---- ��ȡ��ɫ����ͼ��
static int savedFileCount = 0;
static string sql_row_befor;
static MYSQL_RES *result;
void KinectMainDriver::captureFrameRGB(double timestamp)
{
	bool isCapturedFrame = kinectDevice->getKinectFrameColor(dataColor);
	// ����׽��
	if (isCapturedFrame)
	{
		int savedFileCount = kinectOutput->getCountSavedFileColor();

		// !DEBUG! ֻ��һ��ͼƬ

		result = mysqlUtil1.QueryFallTime();

		if (result)
		{
			MYSQL_ROW sql_row;

			int count = 0;
			while (sql_row = mysql_fetch_row(result))
			{
				if (sql_row_befor != sql_row[0])
				{
					string lastFallTime = sql_row[0];

					char buffer[20];

					for (int i = 0; i < lastFallTime.length(); i++)
					{
						if (lastFallTime[i] != ' ' && lastFallTime[i] != '-' && lastFallTime[i] != ':')
						{
							buffer[count] = lastFallTime[i];
							count++;
						}
					}

					//fileName << savePicture << savedFileCount;
					string pathFileAbsolute = PATH_DIRECTORY_RGB + buffer;

					cout << "pathFileAbsolute: " << pathFileAbsolute << endl;
					// ���ڵ�ǰϵͳ�ĵ�ǰ����/ʱ��
					time_t now = time(0);

					// �� now ת��Ϊ�ַ�����ʽ
					char* dt = ctime(&now);

					cout << "�������ں�ʱ�䣺" << dt << endl;

					kinectOutput->saveKinectColor(dataColor, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute, true, timestamp);
				}
				sql_row_befor = sql_row[0];
			}
		}

	}
}

//---- ��׽Frame
void KinectMainDriver::captureFrameDPT(double timestamp)
{
	// ����޷���ȡbody mask���ͷ���NULL
	bool isCapturedRange = kinectDevice->getKinectFrameRange(dataRange, dataBodyMask);

	if (isCapturedRange)
	{
	
		int savedFileCount = kinectOutput->getCountSavedFileRange();

		// !DEBUG! ֻ��һ��ͼƬ
		savedFileCount = 0;

		// �����ļ���
		stringstream fileName;
		fileName << setw(8) << setfill('0') << savedFileCount;

		string pathFileAbsolute = PATH_DIRECTORY_DPT + fileName.str();

		//kinectOutput->saveKinectRange(dataRange, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute, true, timestamp);
	}
}

//---- ��ȡ�������
void KinectMainDriver::captureFrameDPTI(double timestamp)
{
	bool isCapturedDepth = kinectDevice->getKinectFrameDepth(dataDepth);

	if (isCapturedDepth)
	{
		// �����ļ�����
		int savedFileCount = kinectOutput->getCountSavedFileDepth();

		savedFileCount = 0;

		// �����ļ���
		stringstream fileName;
		fileName << setw(8) << setfill('0') << savedFileCount;

		string pathFileAbsolute = PATH_DIRECTORY_DPTI + fileName.str();

		kinectOutput->saveKinectDepth(dataDepth, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute, false);
	}
}

//---- ��ù�������
void KinectMainDriver::captureFrameSKL(double timestamp)
{
	bool isCapturedSkeleton = kinectDevice->getKinectSkeleton(skeletonPool);

	kinectOutput->saveKinectSkeleton(skeletonPool, timestamp);

	// �����������
	if (isCaptureSKLI)
	{
		int savedFileCount = kinectOutput->getCountSavedFileDepth();

		savedFileCount = 0;

		// �����ļ���
		stringstream fileName;
		fileName << setw(8) << setfill('0') << savedFileCount;

		string pathFileAbsolute = PATH_DIRECTORY_SKLI + fileName.str();

		// �������ͼƬ
		//kinectOutput->saveKinectSkeletonImage(skeletonPool, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute);
	}
}


byte * KinectMainDriver::getDataColor()
{
	return dataColor;
}

byte * KinectMainDriver::getDataDepth()
{
	return dataDepth;
}

SkeletonBody * KinectMainDriver::getDataSkeletonPool()
{
	return skeletonPool;
}

unsigned short * KinectMainDriver::getDataRange()
{
	return dataRange;
}