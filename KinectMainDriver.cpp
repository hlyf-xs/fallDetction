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
// 这个是释放程序
template<class Interface>
static inline void safeRelease(Interface *&interfaceToRelease)
{
	if (interfaceToRelease != nullptr) {
		interfaceToRelease->Release();
		interfaceToRelease = nullptr;
	}
}

//---- 实例化对象
KinectMainDriver::KinectMainDriver()
{
	kinectDevice = KinectDevice::getInstance();

	cout << "size of kinectDevice: " << sizeof(kinectDevice) << endl;

	kinectOutput = KinectOutput::getInstance();
	cout << "size of kinectOutput: " << sizeof(kinectOutput) << endl; 

	isInit = false;
}

//---- 获取Instance 对象，实例化 instance对象
KinectMainDriver * KinectMainDriver::getInstance()
{
	if (instance == 0)
	{
		instance = new KinectMainDriver();
	}

	return instance;
}


//---- kinect 数据进行初始化操作
// kinect 设备初始化
// kinect 输出初始化
// kinect 骨骼数据初始化
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

	// 初始化kienct
	bool isKinectOK = kinectDevice->init();

	if (!isKinectOK) { return false; }

	// 初始化kienct的事件句柄
	// RGB，DEPTH，SKELETON
	hevents[0] = kinectDevice->getEventStreamRGB();
	hevents[1] = kinectDevice->getEventStreamDEPTH();
	hevents[2] = kinectDevice->getEventStreamSKELETON();

	//cout << "size of havents:" << sizeof(hevents) << endl;

	// 初始化日志文件
	kinectOutput->init(pathLoggerRGB, pathLoggerDPT, pathLoggerDPT);

	// 打开文件写入
	kinectOutput->loggerSkeletonJointOPEN(pathFileSKL);

	isInit = true;

	return true;
}


//---- 多态函数
bool KinectMainDriver::init(string pathDirRGB, string pathDirDPT, string pathDirDPTI, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isDPTI, bool isSKL)
{
	return init(pathDirRGB, pathDirDPT, pathDirDPTI, "", pathFileSKL, pathLoggerRGB, pathLoggerDPT, isRGB, isDPT, isDPTI, false, isSKL);
}

bool KinectMainDriver::init(string pathDirRGB, string pathDirDPT, string pathFileSKL, string pathLoggerRGB, string pathLoggerDPT, bool isRGB, bool isDPT, bool isSKL)
{
	return init(pathDirRGB, pathDirDPT, "", pathFileSKL, pathLoggerRGB, pathLoggerDPT, isRGB, isDPT, false, isSKL);
}

//---- 数据更新
// 那么源源不断的数据会不会被更新呢？
// 此中的timestamp由系统中事件自定的
void KinectMainDriver::update(double timestamp)
{
	if (!isInit) { return; }

	// 等待对象传输
	// INFINTE 无限等待，直到有信道传输过来
	MsgWaitForMultipleObjects(3, hevents, FALSE, INFINITE, QS_ALLINPUT);

	// 捕捉帧数据
	// 判断释放的是否是 DPTI 或者 DPT
	
	// 彩色数据
	if (isCaptureRGB) { captureFrameRGB(timestamp);  }

	// 深度数据
	if (isCaptureDPTI & !isCaptureDPT) { captureFrameDPTI(timestamp); }

	// 骨骼数据
	if (isCaptureDPT) 
	{ 
		captureFrameDPT(timestamp);

		// 只有在显示深度图像时才进行此转换
		if (isCaptureDPTI)
		{
			for (int i = 0, index = 0; i < KinectDevice::DEFAULT_WIDTH * KinectDevice::DEFAULT_HEIGHT; i++, index += 4)
			{
				dataDepth[index] = (int)((double)dataRange[i] * 255 / 8000);
				dataDepth[index + 1] = (int)((double)dataRange[i] * 255 / 8000);
				dataDepth[index + 2] = (int)((double)dataRange[i] * 255 / 8000);
				dataDepth[index + 3] = 255;

				// 大概 1M 大的数据
				//cout << "sizeOf dataDepth:" << sizeof(dataDepth)/1024/1024 << endl;
			}
		}
	}
	// 有骨骼数据的话，就获取骨骼数据
	if (isCaptureSKL) { captureFrameSKL(timestamp);  }
}


//---- 获取彩色数据图像
static int savedFileCount = 0;
static string sql_row_befor;
static MYSQL_RES *result;
void KinectMainDriver::captureFrameRGB(double timestamp)
{
	bool isCapturedFrame = kinectDevice->getKinectFrameColor(dataColor);
	// 被捕捉了
	if (isCapturedFrame)
	{
		int savedFileCount = kinectOutput->getCountSavedFileColor();

		// !DEBUG! 只存一张图片

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
					// 基于当前系统的当前日期/时间
					time_t now = time(0);

					// 把 now 转换为字符串形式
					char* dt = ctime(&now);

					cout << "本地日期和时间：" << dt << endl;

					kinectOutput->saveKinectColor(dataColor, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute, true, timestamp);
				}
				sql_row_befor = sql_row[0];
			}
		}

	}
}

//---- 捕捉Frame
void KinectMainDriver::captureFrameDPT(double timestamp)
{
	// 如果无法获取body mask，就发送NULL
	bool isCapturedRange = kinectDevice->getKinectFrameRange(dataRange, dataBodyMask);

	if (isCapturedRange)
	{
	
		int savedFileCount = kinectOutput->getCountSavedFileRange();

		// !DEBUG! 只存一张图片
		savedFileCount = 0;

		// 产生文件名
		stringstream fileName;
		fileName << setw(8) << setfill('0') << savedFileCount;

		string pathFileAbsolute = PATH_DIRECTORY_DPT + fileName.str();

		//kinectOutput->saveKinectRange(dataRange, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute, true, timestamp);
	}
}

//---- 获取深度数据
void KinectMainDriver::captureFrameDPTI(double timestamp)
{
	bool isCapturedDepth = kinectDevice->getKinectFrameDepth(dataDepth);

	if (isCapturedDepth)
	{
		// 保存文件数量
		int savedFileCount = kinectOutput->getCountSavedFileDepth();

		savedFileCount = 0;

		// 产生文件名
		stringstream fileName;
		fileName << setw(8) << setfill('0') << savedFileCount;

		string pathFileAbsolute = PATH_DIRECTORY_DPTI + fileName.str();

		kinectOutput->saveKinectDepth(dataDepth, KinectDevice::DEFAULT_WIDTH, KinectDevice::DEFAULT_HEIGHT, pathFileAbsolute, false);
	}
}

//---- 获得骨骼数据
void KinectMainDriver::captureFrameSKL(double timestamp)
{
	bool isCapturedSkeleton = kinectDevice->getKinectSkeleton(skeletonPool);

	kinectOutput->saveKinectSkeleton(skeletonPool, timestamp);

	// 保存骨骼数据
	if (isCaptureSKLI)
	{
		int savedFileCount = kinectOutput->getCountSavedFileDepth();

		savedFileCount = 0;

		// 产生文件名
		stringstream fileName;
		fileName << setw(8) << setfill('0') << savedFileCount;

		string pathFileAbsolute = PATH_DIRECTORY_SKLI + fileName.str();

		// 保存骨骼图片
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