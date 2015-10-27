#pragma once
#include "stdafx.h"
#include <opencv2/opencv.hpp>

#define  WINSIZE 3

class Vibe
{
public:
	Vibe(void);
	Vibe(IplImage *img);
	
public:
	void SetMinMatch(int nthreshold){g_MinMatch=nthreshold;}
	void SetRadius(int radius){g_Radius=radius;}
	void SetSampleNum(int num){g_SampleNum=num;}
	void SetThreshold(double t){g_threshold=t;}
	IplImage* GetForeground(){return g_ForeImg;}
	IplImage* GetSegMask(){return g_SegementMask;}
	
	void Detect(IplImage *img);
	void ForegroundCombineEdge(); // ��ϱ�Ե��Ϣ
	void DeleteSmallAreaInForeground(double minArea=20);//ɾ��С�������
	
	// ʵ�ֱ������»���
	void Update();
	// ʵ�ֺ�����Ҫ����̬ѧ����
	void PostProcess();

public:
 ~Vibe(void);

private:
	void ClearLongLifeForeground(int i_lifeLength=200); // ��������д���ʱ��ϳ������أ�i_lifeLength���ڿ���������ڵ��ʱ��
	double AreaDense(IplImage *pFr,int AI,int AJ,int W,int H); //����(i,j)�������СΪW��H���ܶ�
	int GetRandom(int istart,int iend); // Ĭ��istart=0,iend=15
	int GetRandom(int random);
	int GetRandom();// ����һ�������
	// ������������֮���ŷʽ����
	double CalcPixelDist(CvScalar bkCs,CvScalar curCs);
	// ����Kim�ķ�����������ɫ����
	double CalcuColorDist(CvScalar bkCs,CvScalar curCs);
	int g_SampleNum;// Sample number for the models��Ĭ��Ϊ20
	int g_MinMatch; // ��ǰ�����뱳��ģ��ƥ������ٸ�����Ĭ��Ϊ2
	int g_Height;
	int g_Width;
	int g_Radius;// ����İ뾶��Ĭ��Ϊ20
	int g_offset; //�߽�Ŀ�͸�
	double g_threshold; // �����������ֵ
	unsigned char ***g_Model;// ���汳��ģ��
	IplImage *g_ForeImg;// ����ǰ��ͼ
	IplImage *g_Edge;
	
	IplConvKernel* element;
	
	IplImage *g_SegementMask; //�ָ���Ĥ
	IplImage *g_UpdateMask; // ������Ĥ
	IplImage *g_Gray;
	int ** LifeLength; // ��¼ǰ�������������,���ǰ������������ȵ���һ������ֵ���������뱳����ȥ,��Ҫ������Ρ� 
};

