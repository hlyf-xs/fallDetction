import pymysql
from pymysql.cursors import DictCursor
from DBUtils.PooledDB import PooledDB
import datetime


# 父类连接池，用于初始化数据库连接
class BasePymysqlPool(object):
    def __init__(self):
        self.db_host = "localhost"
        self.db_port = 3306
        self.user = "root"
        self.password = "123456"  # 注意，此处是str类型
        self.db = "fallDetect"
        self.conn = None
        self.cursor = None

class mysqlHelper(BasePymysqlPool):
    """
    MYSQL数据库对象，负责产生数据库连接 , 此类中的连接采用连接池实现获取连接对象：conn = Mysql.getConn()
            释放连接对象;conn.close()或del conn
    """
    # 连接池对象
    __pool = None

    def __init__(self):
        super(mysqlHelper, self).__init__()
        # 数据库构造函数，从连接池中取出连接，并生成操作游标
        self._conn = self.__getConn()
        self._cursor = self._conn.cursor()

    def con(self):
        self._conn = self.__getConn()
        self._cursor = self._conn.cursor()

    def __getConn(self):
        """
        @summary: 静态方法，从连接池中取出连接
        @return MySQLdb.connection
        """
        if mysqlHelper.__pool is None:
            __pool = PooledDB(creator=pymysql,
                              mincached=1,
                              maxcached=10,
                              host=self.db_host,
                              port=self.db_port,
                              user=self.user,
                              passwd=self.password,
                              db=self.db,
                              use_unicode=True,  # 此处应设置为True，否则查询出来的数据会变成bytes类型
                              charset="utf8",
                              # cursorclass=DictCursor, # 这里是标记每列的标栏
                              )
        return __pool.connection()


    """
        所有操作函数
        @summary: 执行查询，并取出num条结果
        @param sql:查询ＳＱＬ，如果有查询条件，请只指定条件列表，并将条件值使用参数[param]传递进来
        @param num:取得的结果条数
        @param param: 可选参数，条件列表值（元组/列表）
        @return: result list/boolean 查询到的结果集
    """

    # 查询所有数据
    def queryAll(self, param=None):
        self.con()
        sql = "select * from skeleton"
        if param is None:
            count = self._cursor.execute(sql)
        else:
            count = self._cursor.execute(sql, param)
        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False
        self._conn.close()
        return result

    # 查询下一行数据
    def queryOne(self, timeIndex):
        self.con()
        sql = "select * from skeleton WHERE timeIndex > %d limit 1"

        count = self._cursor.execute(sql % timeIndex)

        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False

        self._conn.close()
        return result

        # 查询下一行数据

    def queryLastOne(self, timeIndex1):
        self.con()
        sql = "select * from skeleton WHERE timeIndex in (select max(timeIndex) from skeleton where timeIndex < %d)"
        count = self._cursor.execute(sql % timeIndex1)

        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False

        self._conn.close()
        return result

    # 查询上一次跌倒的所有信息
    def queryLastFall(self):
        self.con()
        sql = "select * from falldetect WHERE fallTime in (select max(fallTime) from falldetect)"

        count = self._cursor.execute(sql)

        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False

        self._conn.close()
        return result

    # 查询上一次跌倒的时间
    def queryLastFallTime(self, timeIndex):
        self.con()
        sql = "select max(fallTime) from falldetect WHERE objectIndex = %d"

        count = self._cursor.execute(sql % timeIndex)

        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False

        self._conn.close()
        return result

    # 查询所有行号
    def queryColumn(self, param=None):
        self.con()
        sql = "select count(*) from skeleton"
        if param is None:
            count = self._cursor.execute(sql)
        else:
            count = self._cursor.execute(sql, param)
        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False

        # self._cursor.close()
        self._conn.close()
        # print("result:", result)
        return result[0][0]

    # 查询平均值
    def queryAvgColumn(self, param=None):
        self.con()
        sql = "select AVG (%s) from skeleton"
        if param is None:
            count = self._cursor.execute(sql)
        else:
            count = self._cursor.execute(sql % param)
        if count > 0:
            result = self._cursor.fetchall()
        else:
            result = False

        self._conn.close()
        return result

    def __query(self, sql, param=None):
        self.con()
        try:
            if param is None:
                count = self._cursor.execute(sql)
                self._conn.commit()
            else:
                count = self._cursor.execute(sql, param)
                self._conn.commit()
            self._conn.close()
            return count
        except:
            self._conn.rollback()

    # 插入跌倒信息
    def insertResult(self, objectIndex, fallTime):
        sql = "insert into fallDetect(fallTime, objectIndex) values('%s', '%d')"
        return self.__query(sql % (fallTime, objectIndex))

    # 删除所有数据
    def deleteAll(self):
        sql = "delete from skeleton"
        return self.__query(sql)

    # 删除标签前的数据
    def deleteFromIndex(self, timeIndex, indexList):
        sql = "delete from skeleton WHERE timeIndex <= %d and indexList = %d"
        return self.__query(sql % (timeIndex, indexList))


    def begin(self):
        """
        @summary: 开启事务
        """
        self._conn.autocommit(0)

    def end(self, option='commit'):
        """
        @summary: 结束事务
        """
        if option == 'commit':
            self._conn.commit()
        else:
            self._conn.rollback()

    def dispose(self, isEnd=1):
        """
        @summary: 释放连接池资源
        """
        if isEnd == 1:
            self.end('commit')
        else:
            self.end('rollback')
        self._cursor.close()
        self._conn.close()


if __name__ == '__main__':
    # 查询一条数据的示例代码
    mysql = mysqlHelper()

    result = mysql.queryLastOne(52484)
    print(result)

    # 释放资源
    # mysql.dispose()