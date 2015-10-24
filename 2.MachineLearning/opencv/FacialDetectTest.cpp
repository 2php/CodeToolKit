// FacialDetectTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>


static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;

void detect_and_draw( IplImage* image );

const char* cascade_name = "module/haarcascade_frontalface_alt.xml";

int main(int argc, char* argv[])
{
	CvCapture* capture = 0;

	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	if( !cascade )
	{
		fprintf( stderr, "ERROR: Could not load classifier cascade/n" );
		return -1;
	}

	storage = cvCreateMemStorage(0);

	cvNamedWindow("result", 1);
	cvNamedWindow("gray", 1);
	cvNamedWindow("hist", 1);

	const char* filename = "../Probe/test.jpg";
	IplImage* image = cvLoadImage(filename );
	if( image )
	{
		//Haar级联器检测与显示人脸
		detect_and_draw( image );
		cvWaitKey(0);
		cvReleaseImage( &image );
	}

	cvDestroyWindow("result");
	cvDestroyWindow("gray");
	cvDestroyWindow("hist");
	cvWaitKey(0);
	return 0;
}

void detect_and_draw( IplImage* img )
{
	static CvScalar colors[] = 
	{
		{{0,0,255}},//blue
		{{0,128,255}},//
		{{0,255,255}},//
		{{0,255,0}},//green
		{{255,128,0}},//
		{{255,255,0}},//
		{{255,0,0}},//red
		{{255,0,255}}//
	};

	double scale = 1.3;
	IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
	IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
		cvRound (img->height/scale)),
		8, 1 );
	int i;

	cvCvtColor(img, gray, CV_BGR2GRAY);//原图转成灰度图	
	cvResize(gray, small_img, CV_INTER_LINEAR);//灰度图缩放scale倍
	cvShowImage("gray", small_img);
	cvEqualizeHist(small_img, small_img);//均衡化直方图(去噪)
	cvShowImage("hist", small_img);
	
	cvClearMemStorage( storage );

	if( cascade )
	{
		double t = (double)cvGetTickCount();
		CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
			1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
			cvSize(30, 30) );
		t = (double)cvGetTickCount() - t;
		printf( "detection time = %gms/n", t/((double)cvGetTickFrequency()*1000.) );
		for( i = 0; i < (faces ? faces->total : 0); i++ )
		{
			CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
			CvPoint center, topLeft, bottomRight;
			
			//绘制圆
			int radius;
			center.x = cvRound((r->x + r->width*0.5)*scale);
			center.y = cvRound((r->y + r->height*0.5)*scale);
			radius = cvRound((r->width + r->height)*0.25*scale);
			//cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
			
			//绘制正方形
			topLeft.x = r->x * scale;
			topLeft.y = r->y * scale;
			bottomRight.x = topLeft.x + r->width * scale;
			bottomRight.y = topLeft.y + r->height * scale;
			cvRectangle( img, topLeft, bottomRight, colors[i%8], 2);
		}
	}

	cvShowImage( "result", img );
	cvReleaseImage( &gray );
	cvReleaseImage( &small_img );
} 

