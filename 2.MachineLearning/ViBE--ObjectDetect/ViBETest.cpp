// ViBETest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ViBE.h"
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{ 
	CvCapture *capture=NULL;
	IplImage* frame=NULL;
	IplImage* pForeImg=NULL;
	IplImage* segImg=NULL; 

	char *file_path="video.avi";  //�����ļ�

	capture=cvCreateFileCapture(file_path);
	if (!capture)
	{
		cout<<"Read Video File Error!"<<endl;
		return -1;
	}
	frame=cvQueryFrame(capture);

	cvNamedWindow("img",1);
	cvNamedWindow("foreN",1);
	//cvNamedWindow("seg",1);

	Vibe* pV=new Vibe(frame);

	while( frame=cvQueryFrame(capture) )
	{
		//pV->SetMinMatch(80);
		//pV->SetRadius(5);
		//pV->SetSampleNum(100);
		//pV->SetThreshold(1);

		pV->Detect(frame);//���
		pForeImg=pV->GetForeground();//ǰ��
		//segImg=pV->GetSegMask();//����
		frame->origin=1;
		pForeImg->origin=1;
		cvShowImage("img",frame);
		cvShowImage("foreN",pForeImg);
		//cvShowImage("seg",segImg);
		cvWaitKey(33);
	}

	cvReleaseImage(&frame);
	cvReleaseImage(&pForeImg);
	cvReleaseCapture(&capture);
	return 0; 
}

