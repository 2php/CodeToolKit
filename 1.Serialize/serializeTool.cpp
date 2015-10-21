#include "serializeTool.h"
#include <iostream>

//序列化数据类
SerializeData::SerializeData()
    : m_data(NULL)
    , m_size(0)
    , intelptr(NULL)
{
}

SerializeData::~SerializeData()
{
    if( m_data )
    {
        delete []m_data;
        m_data = NULL;
    }
    if( intelptr )
    {
        std::cout<<"自动回收分配空间！";
        delete []intelptr;
        intelptr = NULL;
    }
}

char* SerializeData::encode(float* data, int num)
{
    //std::cout<<"序列化数据开始："<<data<<num;
    if( !data || num<=0 )
    {
        return NULL;
    }

    if( m_data )
    {
        delete[] m_data;
        m_data = NULL;
    }

    m_size = num * sizeof(float);
    m_data = new char[ m_size ];
    memcpy_s(m_data, m_size, data, m_size);

    return m_data;
}

int SerializeData::decode(float** data, int& num)
{
    //std::cout<<"反序列化数据开始："<<data<<num;
    if( !m_data || m_size<=0 )
    {
        return 0;
    }

    if( *data )//参数非法性判断
    {
        std::cout<<"指针已分配空间~~~";
        return 0;
    }

    if( intelptr )//防止泄露
    {
        delete []intelptr;
        intelptr = NULL;
    }

    num = m_size / sizeof(float);
    if( num>0 )
    {
        *data = new float[num];
        memcpy_s(*data, m_size, m_data, m_size);
        intelptr = m_data;
    }

    return num;
}

void SerializeData::freeSpace(float *buff, int num)
{
    if( !buff || num<=0 )
    {
        return ;
    }

    if( num>=1 )
    {
        delete[] buff;
        buff = NULL;
    }
    else
    {
        delete buff;
        buff = NULL;
    }
    intelptr = NULL;
    std::cout<<"手动回收空间";
}

const char *SerializeData::getData(void)
{
    return m_data;
}

int SerializeData::getSize(void)
{
    return m_size;
}


#ifdef MAINTEST

int main(int argc, char* []argv)
{
    float hh[5] = {0.1, 0.2, 0.3, 0.4, 0.5};

    SerializeData coder;
    coder.encode(hh, 5);

    float* decode = NULL;
    int count = 0;
    coder.decode(&decode, count);
    for(int i=0; decode&&i<count; i++)
    {
        std::cout<<"反序列后的数据："<<decode[i];
    }
    //手动回收
    coder.freeSpace(decode, count);
    //或者注释，让其自动回收
}

#endif
