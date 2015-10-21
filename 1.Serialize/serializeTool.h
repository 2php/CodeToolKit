/*!
 @name     serializeTool
 @discription make float[] to char[], you can change it to used on other type
 @author   houwenbin
 @contact  houwenbin1986@gmail.com
 @copyright  Copyright 2015 -  houwenbin All rights reserved.
 */

#ifndef SERIALIZETOOL
#define SERIALIZETOOL


//序列化数据类
class SerializeData
{
public:
    SerializeData();
    virtual ~SerializeData();

public:
    char* encode(float* data, int num);
    int decode(float** data, int& num);
    //解码时配对
    void freeSpace(float* buff, int num);

public:
    const char* getData(void);
    int getSize(void);

private:
    char* m_data;//编码缓冲
    int m_size;//编码数据
    char* intelptr;//解码缓冲指针
};

#endif
