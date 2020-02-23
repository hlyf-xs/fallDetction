# -*- coding: utf-8 -*-
import tensorflow
import numpy as np
import os
from time import sleep
from common import common as cm
import datetime
import sys
from tkinter import Tk, StringVar, Label
from keras.models import load_model
from mysqlHelper import mysqlHelper
import time
import requests
import re

UBUNTU = True  # False

# 姿态分类 站姿 坐姿 躺姿 睡觉中 -- 睡觉中是躺在床上呢
CLASSIFICATION_OUTPUT_TO_STR = {0: "STANDING", 1: "SITTING", 2: "LYING DOWN", 3: "BENDING"}

# 初始化y的最低值
lowest_y_point = 1000

# 房间的最低点到躺在地上的人可以接受的距离的阈值 ？？？
M_FROM_FLOOR = 0.35

objects_per_room = {}
comm = cm()

# 导入数据
def importFloorData(roomNumber):
    filepath = "data/floorplans/" + str(roomNumber) + ".txt"
    print(filepath)

    if os.path.isfile(filepath):
        # 打开文件，读取文件
        file = open(filepath, 'r')
        objects_per_room[str(roomNumber)] = []  # This room has a list of objects
        objects = file.read().splitlines()
        num_objects = int(len(objects) / 4)  # Each file has 4 coords
        for i in range(num_objects):
            objects_per_room[str(roomNumber)].append(
                objects[(i * 4):(i * 4) + 4])  # Append the object to the list of objects for that particular room
    print("FLOOR OBJECT DATA IMPORTED FOR ROOM #" + str(roomNumber) + "... !")
    return

# 判断是否躺在地板上的另一种方法 -- 现在不用
def isWithinGroundRange(x, z, roomNumber):
    # 从房间中导入数据
    objects = objects_per_room[str(roomNumber)]  # Impoted floor data for that room
    for object in objects:
        if (x > float(object[0]) and x < float(object[1]) and z > float(object[2]) and z < float(object[3])):  # If person is on that object
            return False
    return True

# 进行分类
def getClassification(inputVals):

    # 如果高度小于30厘米就判断是在地上躺着 -- 这个高度还是比较准的
    if inputVals[0][0] < 0.3:
        return "LYING DOWN"
    classification_output = model.predict(np.array([tuple(inputVals)]).reshape(1, 7, 1))

    return CLASSIFICATION_OUTPUT_TO_STR[np.argmax(classification_output, 1)[0]]

# 躺在地板上
def isLayingOnTheFloor(footRightPosY, footLeftPosY):
    # 最低点加上床的高度 --左右脚加上床的高度
    # 我设想的是人躺在地板上,但是
    # 但是这个lowest_y_point 会出现负数
    if (footRightPosY < (lowest_y_point + M_FROM_FLOOR)) and (footLeftPosY < (lowest_y_point + M_FROM_FLOOR)):
        return True
    return False

# 加载模型
def loadModel():

    model = load_model('postureDetection_LSTM.h5')
    return model


def fallDetection():

    # 实例化数据库对象
    global skeletonData
    global lowest_y_point
    skeletonData = mysqlHelper()

    # 获取表中的行数
    # 主要是解决连接冲突问题
    if skeletonData.queryColumn() != False:
        colum = skeletonData.queryColumn()
        print("colum: ", colum)

        # 房间编号
        roomNumber = 0
        importFloorData(roomNumber)

        # 查询前30中最小的footRightPos, footleftPos
        lowest_y_point = -0.92

        # # 就取前 30 吧
        # if colum > 30:
        #     minFootRightPos = skeletonData.queryAvgColumn("footRightPos")[0][0]
        #     minFootLeftPos = skeletonData.queryAvgColumn("footLeftPos")[0][0]
        #
        #     lowest_y_point = min(minFootRightPos, minFootLeftPos)
        #
        #     print("lowest_y_point: ", lowest_y_point)
        #
        # print("LOWEST_Y_POINT === " + str(lowest_y_point))

        isLyingDown()


# 是不是真的持续20帧呢？
def isRealFall(inp, currentIndex):

    #  -------- 这里是进行跌倒检测的核心
    while True:

        # 如果不为空
        if skeletonData.queryOne(currentIndex) != False:

            # 获取下一帧
            inpOne = skeletonData.queryOne(currentIndex)

            print("inpOne: ", inpOne)

            # inpOne也是一个二维数组
            areadyIndex = inpOne[0][10]
            print("areadyIndex1: ", areadyIndex)

            inputVals = np.random.rand(1, 7)

            currentIndex = inpOne[0][10]

            inputVals[0] = inpOne[0][:7]

            posture = getClassification(inputVals)
            print('posture1: ', posture)

            labelText.set(posture)
            root.update()

            print("indexList: ", inpOne[0][9])
            print("有效跌倒检测数: ", memberList[inpOne[0][9]][1])
            print("memberAllowedNotOnFloor: ", memberAllowedNotOnFloor[inpOne[0][9]][1])
            print("memberAllowed: ", memberAllowed[inpOne[0][9]][1])

            # 如果同一个对象检测到了lyingDown的可疑情况，但是还没有出现跌倒
            if memberList[inpOne[0][9]][1] >= 1 and memberList[inpOne[0][9]][1] < 15:

                # 如果还是躺姿 -- 那咱们再严格检测呗
                if posture == "LYING DOWN":
                    print('posture2: LYING DOWN')

                    lastObjectIndex = skeletonData.queryLastOne(inpOne[0][10])[0][9]
                    fallObjectIndexDel(inpOne[0][9], lastObjectIndex)

                    # 如果是躺在床上了
                    if isLayingOnTheFloor(float(inpOne[0][7]), float(inpOne[0][8])) == False:

                        if memberAllowedNotOnFloor[inpOne[0][9]][1] == 0:
                            print("检测目标: ", inpOne[0][9], " IS NOT LAYING ON THE FLOOR! No fall..!")
                            areadyIndex = inpOne[0][10]

                            # 初始化
                            memberAllowedNotOnFloor[inpOne[0][9]][1] = 5
                            memberList[inpOne[0][9]][1] = 0

                            # 删除已检测数据
                            print("NOT LAYING ON THE FLOOR areadyIndex: ", areadyIndex)
                            skeletonData.deleteFromIndex(areadyIndex, inpOne[0][9])

                            # 重新检测 -- 这样的话还是在这个循环里面
                            return isLyingDown()

                        else:
                            # 如果连续5次是这样的话，那么就-1
                            memberAllowedNotOnFloor[inpOne[0][9]][1] -= 1

                            # 这种情况也是有效的
                            memberList[inpOne[0][9]][1] += 1

                    # 这才是正常的啊
                    else:
                        # 这种情况也是有效的
                        memberList[inpOne[0][9]][1] += 1

                # 10% allowed to not be LYING DOWN (2/20)
                # 如果不是躺着了，那么在这2秒里面没有摔倒呢
                # 如果接下来的2帧不是躺着的，那么就判定为不躺了
                else:
                    if memberAllowed[inpOne[0][9]][1] == 0:
                        print("检测目标: ", inpOne[0][9],
                              " HAS NOT BEEN LAYING ON THE FLOOR FOR MORE THAN 2 SECONDS! No fall..!")
                        areadyIndex = inpOne[0][10]

                        # 重新初始化
                        memberAllowed[inpOne[0][9]][1] = 5
                        memberList[inpOne[0][9]][1] = 0

                        skeletonData.deleteFromIndex(areadyIndex, inpOne[0][9])

                        # 重新检测
                        return isLyingDown()

                    else:

                        # 这种情况也是有效的
                        memberList[inpOne[0][9]][1] += 1
                        # 同时是要减少的呦
                        memberAllowed[inpOne[0][9]][1] -= 1

            # 这是对于不同对象第一次检测到LyingDown
            # 这里是个大块头
            elif memberList[inpOne[0][9]][1] == 0 and posture == "LYING DOWN":

                # 这里需要激活它
                memberList[inpOne[0][9]][1] += 1
                # 这个返回不需要再次递归，激活它即可，但是逻辑上是有些漏洞的
                # 如果返回，那么就会导致前面的数据再次计算，费时费力
                # 因为这里是一帧一帧读取的
                # return isRealFall(inpOne, currentIndex)

            # 如果其他的对象出现非跌倒的情况
            elif memberList[inpOne[0][9]][1] == 0:
                memberList[inpOne[0][9]][3] = memberList[inpOne[0][9]][3] + 1
                if memberList[inpOne[0][9]][3] == 50:
                    skeletonData.deleteFromIndex(currentIndex, inpOne[0][9])

            # 这里已经是检测出摔倒了
            elif memberList[inpOne[0][9]][1] == 15:


                # 将检测信息插入数据库表中
                fallTime1 = datetime.datetime.now()
                fallTime = fallTime1.strftime("%Y-%m-%d %H:%M:%S")

                # 查询上次跌倒对象
                fallLastObject = skeletonData.queryLastFall()[0][2]
                # 上次对象跌倒时间
                fallLastTime = skeletonData.queryLastFall()[0][0]

                print("---------------")
                print("----FALLEN!----")
                print("---------------")


                # 该对象在上次跌倒的时间
                fallTimeBefore = skeletonData.queryLastFallTime(inpOne[0][9])
                print("fallTimeBefore", fallTimeBefore[0][0])

                # 同一对象，如果在30秒之外再次摔倒，那么就再次报警
                # 这个时间为处理理论值，程序为单线程，处理速度不行
                if (fallTime1 - fallTimeBefore[0][0]).seconds > 30:

                    labelText.set("FALLEN!")
                    labelTimeForFall.set(fallTime)

                    # 此处的逻辑时有点问题的
                    fallNum = 1
                    setFallTotalNum(1)
                    labelObjectNumber.set(str(fallNum))
                    # 要有刷新
                    root.update()

                    # 如果出现多人情况 必须是在2小时之内
                    print("fallLastTime:", fallLastTime)

                    # 这里强调的是同时性，注意情况
                    # 不同对象，如果在2个小时内出现跌倒那么就算是两个人跌倒，这种逻辑很有问题
                    # (fallTime1 - fallLastTime).seconds < 7200 这是上次对象在两小时内出现这种情况所发现的，逻辑有问题
                    # 我们想要的情况是：多人检测，如果一个出现跌倒能检测，另一个人出现跌倒也能检测
                    # 只有两个人同时出现跌倒，那才能算是出现两人，一个出现跌倒，就是一人
                    # 这个逻辑的理论上是没有问题的，但是出现的情况却不行。。。
                    # 这样效果会好一点儿
                    # 总共有不同的编号，这个就是比较大的痛点问题
                    fallObject = getFallObjectIndex()
                    print("fallObject:", fallObject)

                    if fallLastObject != inpOne[0][9] and fallObject == 2:
                        fallTotalNum = 2
                        setFallTotalNum(fallTotalNum)
                        labelObjectNumber.set(str(fallTotalNum))
                        root.update()

                    print("fallTime1.second - fallTimeBefore: ", (fallTime1 - fallTimeBefore[0][0]).seconds)
                    skeletonData.insertResult(inpOne[0][9], fallTime)

                    fallNum = getFallTotalNum()
                    r = requests.get("http://www.muoo.xyz:8080/ddjc/fall?type=video&device=001&fallNum=" + str(fallNum))
                    key = r.text
                    filePath = "C:\\Users\\23975\Desktop\KinectToolV1\KinectToolsV1\\test\RGB"

                    # 确保数据能够被C++访问到
                    time.sleep(0.5)
                    fileName = fallTime1.strftime("%Y%m%d%H%M%S.bmp")
                    files = {'file': open(filePath + '\\' + fileName, 'rb')}

                    r = requests.post('http://www.muoo.xyz:8080/ddjc/upload?id='+key, files=files)

                # 将其进行初始化
                memberList[inpOne[0][9]][1] = 0
                memberAllowed[inpOne[0][9]][1] = 0
                memberAllowedNotOnFloor[inpOne[0][9]][1] = 0

                while True:
                    print("areadyIndex1: ", areadyIndex)
                    if skeletonData.queryOne(areadyIndex) != False:
                        inp = skeletonData.queryOne(areadyIndex)

                        # 获取下一帧数据，这个数据可能不是先前对象的
                        print("inp: ", inp)

                        # 要是同一个对象啊
                        if inp[0][9] == inpOne[0][9]:
                            inputVals = np.random.rand(1, 7)
                            inputVals[0] = inp[0][:7]

                            print("inp最后:", inp)
                            areadyIndex = inp[0][10]
                            posture = getClassification(inputVals)
                            print('posture4: ', posture)

                            # 一直检测到不是躺姿和最大允许出错帧
                            if posture != "LYING DOWN" and memberList[inp[0][9]][2] != 0:

                                memberList[inp[0][9]][2] -= 1

                                labelText.set(posture)
                                root.update()

                                # 检测过一轮了，那么就重新采集数据呗, 跳出循环

                                # 允许有5帧的误差
                                if memberList[inp[0][9]][2] == 0:
                                    print("最后删除：")
                                    print("areadyIndex: ", areadyIndex)
                                    skeletonData.deleteFromIndex(areadyIndex, inp[0][9])

                                    # 初始化
                                    memberList[inp[0][9]][2] = 4

                                    fallTotalNum = getFallTotalNum()
                                    print("fallTotalNum: ", fallTotalNum)

                                    if fallTotalNum > 0:
                                        fallTotalNum = fallTotalNum - 1
                                        labelObjectNumber.set(str(fallTotalNum))
                                        root.update()

                                    # 重新开始
                                    return isLyingDown()

                        # 不是同一个对象
                        else:
                            # 至少是没有删除，所以会不会造成影响还是要进一步观察
                            # 还是有一定的漏洞，这期间忽略几帧数据也是正常的，按照逻辑，一旦检测到跌倒，那么就会有监护人员出现

                            areadyIndex = inp[0][10]
                            # 那么这不属于不同对象的东西，会不会出现就会忽略了呢？？？？，确实出现了忽略，但是没啥影响

                    else:
                        # 如果没有之后的数据了，那么就删除该对象以前的数据
                        skeletonData.deleteFromIndex(areadyIndex, inpOne[0][9])
                        return isLyingDown()


# isLyingDown中数据的初始化，放在这里虽说有点脏了数据，但是。。。先这样

# 成员属性的定义
# 0 -- 对象 (0-5) 共6位
# 1 -- 检测已跌倒的帧数 (0-20) 共20帧
# 2 -- allowed 躺是躺着的，但是不是在地板上 (0-5) 共5次
# 3 -- allowedNotOnFloor 不是躺姿的 (0-5) 共5次


memberList = [[0, 0, 4, 0], [1, 0, 4, 0], [2, 0, 4, 0], [3, 0, 4, 0], [4, 0, 4, 0], [5, 0, 4, 0]]

# 可能会有大多数情况是躺着的
memberAllowed = [[0, 5], [1, 5], [2, 5], [3, 5], [4, 5], [5, 5]]
# 不在上面
memberAllowedNotOnFloor = [[0, 5], [1, 5], [2, 5], [3, 5], [4, 5], [5, 5]]



fallTotalNum = 0

#######-------------   用于区分有几个人
fallObjectIndex = 0
def setFallObjectIndex(fallObjectIndex1):
    global fallObjectIndex
    fallObjectIndex = fallObjectIndex1

def getFallObjectIndex():
    return fallObjectIndex


differTotal = 0
def setDifferTotal(differTotalNum):
    global differTotal
    differTotal = differTotalNum

def getDifferTotal():
    return differTotal

def addDifferTotal(differTotalNum1):
    fallTotalNum1 = differTotalNum1 + 1
    return fallTotalNum1

def setFallTotalNum(fallNum):
    global fallTotalNum
    fallTotalNum = fallNum

def getFallTotalNum():
    return fallTotalNum

def addFallTotalNum(fallTotalNum1):
    fallTotalNum1 = fallTotalNum1 + 1
    return fallTotalNum1

def deleteFallTotalNum(fallTotalNum1):
    fallTotalNum1 = fallTotalNum1 - 1
    return fallTotalNum1

def fallObjectIndexDel(object, lastObject):

    # 此帧数据对象与上一帧数据对象不同
    if object != labelObject:
        global fallObjectIndex
        fallObjectIndex = 1
        print("object:", object)
        print("lastObject:", lastObject)
        print("fallObjectIndexDel:", fallObjectIndex)

    else:
        global differTotal
        differTotal = differTotal + 1
        # 如果连续出现50帧数据，那就是一个人
        if getDifferTotal() == 50:
            fallObjectIndex = 1
            print("fallObjectIndexDel", fallObjectIndex)


#  初始化步骤结束
def isLyingDown():

    while True:
        # 总姿势
        global posture
        lenOfAllSkeleton = 0

        # 从数据库中获取 ---所有数据
        # 是一个[n, 11] list
        if skeletonData.queryColumn() != False:
            lenOfAllSkeleton = skeletonData.queryColumn()

        print("lenOfAllSkeleton: ", lenOfAllSkeleton)

        # 非跌倒数全局初始化
        for i in range(6):
           memberList[i][3] = 0

        # 这里会进行初始化呢
        # 这个其实不太好
        for index in range(lenOfAllSkeleton):

            # 就相当于初始化一个列表
            inputVals = np.random.rand(1, 7)

            print("index: ", index)

            # 开始检测
            if skeletonData.queryAll()[index] != False:
                inp = skeletonData.queryAll()[index]

                # 7个属性，其他两个用于检查平面
                inputVals[0] = inp[:7]

                # 从模型中提取姿势库
                posture = getClassification(inputVals)
                print("posture:", posture, "检测对象：", inp[9])

                labelText.set(posture)
                root.update()

                # 如果是躺姿 -- 就有可能出现跌倒的趋势，需要一步判断
                # 所以下面一帧一帧的读取是必须的 -- 注意看啊，宝宝

                if posture == "LYING DOWN":
                    # 确定是躺在地上了
                    if isLayingOnTheFloor(float(inp[7]), float(inp[8])):
                        currentIndex = inp[10]
                        memberList[inp[9]][1] += 1

                        print("currentIndex:", currentIndex)

                        # 再跌倒检测两秒 20帧 10帧/秒
                        # 那么这种检测方式可能是无效的
                        # 这是针对一个对象进行20帧的检测

                        isRealFall(inp, currentIndex)

                # 非LyingDown
                else:
                    memberList[inp[9]][3] += 1

                    print("非跌倒数：", memberList[inp[9]][3])

                    if memberList[inp[9]][3] == 70:
                        print("inp[10]: ", inp[10], " inp[9]:", inp[9])
                        skeletonData.deleteFromIndex(inp[10], inp[9])
                        # 初始化数据
                        memberList[inp[9]][3] = 0

                        # 这个地方没有必要再次返回，那么就会出现没有那么多的数据了
                        # 如果就是一帧一帧的读呢？？？
                        # 可能还是这样比较好
                        isLyingDown()




if __name__ == "__main__":
    print("Loading model..")
    model = loadModel()

    # 加载 TKINTER UI 图形化界面
    global root
    global labelText
    global labelObjectNumber
    global labelTimeForFall

    root = Tk()
    root.title("POSTURE DETECTION")
    root.geometry("300x480")
    labelPostureStatus = StringVar()
    labelPostureStatus.set("姿态")

    labelText = StringVar()
    labelText.set('Starting...!')

    labelObject = StringVar()
    labelObject.set("跌倒人数")

    labelObjectNumber = StringVar()
    labelObjectNumber.set("0")

    labelTime = StringVar()
    labelTime.set("跌倒时间")

    labelTimeForFall = StringVar()
    labelTimeForFall.set("0")

    Label(root, textvariable=labelPostureStatus, font=("宋体", 30)).pack()
    Label(root, textvariable=labelText, font=("Helvetica", 40), fg="green").pack()

    Label(root, textvariable=labelObject, font=("宋体", 30)).pack()
    Label(root, textvariable=labelObjectNumber, font=("Helvetica", 40), fg="red").pack()

    Label(root, textvariable=labelTime, font=("宋体", 30)).pack()
    Label(root, textvariable=labelTimeForFall, font=("Helvetica", 20), fg="red").pack()

    fallDetection()