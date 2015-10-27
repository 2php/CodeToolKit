#include "StdAfx.h"
#include "Vibe.h"
#include <iostream>
using namespace std;

Vibe::Vibe(void)
{
	g_Radius=20;
	g_MinMatch=2;
	g_SampleNum=20;
	g_offset=(WINSIZE-1) / 2;
}

Vibe::Vibe(IplImage *img)
{
	if (!img)
	{
		cout<<" The parameter referenced to NUll Pointer!"<<endl;
		return;
	}
	
	this->g_Height=img->height;
	this->g_Width=img->width;
	
	g_Radius=20;
	g_MinMatch=2;
	g_SampleNum=20;
	g_threshold=50;
	g_offset=(WINSIZE-1)/2;
	
	g_ForeImg=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
	g_Gray=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
	g_Edge=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
	g_SegementMask=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
	g_UpdateMask=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);	
	element=cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_CROSS,NULL);	
	cvCvtColor(img, g_Gray, CV_BGR2GRAY);
	// 以上完成相关的初始化操作
	
	/**********************  以下实现第一帧在每个像素的8邻域内的采样功能，建立对应的背景模型*****************************/
	int i=0,j=0,k=0;
	g_Model=new unsigned char**[g_SampleNum];
	for (k=0;k<g_SampleNum;k++)
	{
		g_Model[k]=new unsigned char *[g_Height];
		for(i=0;i<g_Height;i++)
		{
			g_Model[k][i]=new unsigned char [g_Width];
			for (j=0;j<g_Width;j++)
			{
				g_Model[k][i][j]=0;
			}
		}
	}
	// 采样进行背景建模
	double dVal;
	int ri=0,rj=0; //随机采样的值
	for (i=g_offset;i<g_Height-g_offset;i++)
	{
		for (j=g_offset;j<g_Width-g_offset;j++)
		{
			// 周围3*3的邻域内进行采样
			for(k=0;k<g_SampleNum;k++)
			{
				ri=GetRandom(i);
				rj=GetRandom(j);
				dVal=cvGetReal2D(g_Gray,ri,rj);
				g_Model[k][i][j]=dVal;
			}
		}
	}
	
	// 初始化前景点掩膜的生命长度
	LifeLength=new int *[g_Height];
	for (i=0;i<g_Height;i++)
	{
		LifeLength[i]=new int [g_Width];
		for(j=0;j<g_Width;j++)
		{
			LifeLength[i][j]=0;
		}
	}
}


void Vibe::Detect(IplImage *img)
{
	cvZero(g_ForeImg);
	cvCvtColor(img, g_Gray, CV_BGR2GRAY);
	int i=0,j=0,k=0;
	double dModVal,dCurrVal;
	int tmpCount=0;// 距离比较在阈值内的次数
	double tmpDist=0;
	int iR1,iR2;//产生随机数
	int Ri,Rj; // 产生邻域内X和Y的随机数
	
	for (i=0;i<g_Height;i++)
	{
		for (j=0;j<g_Width;j++)
		{
			if( i < g_offset || j < g_offset || i> g_Height - g_offset || j> g_Width - g_offset )
			{
				cvSetReal2D(g_ForeImg, i, j, 0);
				continue;
			}
			else
			{
				tmpCount=0;
				dCurrVal=cvGetReal2D(g_Gray, i, j);
				for (k=0; k<g_SampleNum && tmpCount<g_MinMatch; k++)
				{
					dModVal=g_Model[k][i][j];
					//tmpDist=CalcPixelDist(dCurrVal,dModVal);
					//tmpDist=CalcuColorDist(dCurrVal,dModVal);
					tmpDist=fabs(dModVal-dCurrVal);
					if (tmpDist<g_Radius)
					{
						tmpCount++;
					}
				}
				
				//判断是否匹配上
				if (tmpCount>=g_MinMatch)
				{
					cvSetReal2D(g_ForeImg,i,j,0);
					// 背景模型的更新
					iR1=GetRandom(0,15);
					if (iR1==0)
					{
						iR2=GetRandom();
						g_Model[iR2][i][j]=dCurrVal;
					}
					
					//进一步更新邻域模型
					iR1=GetRandom(0,15);
					if (iR1==0)
					{
						Ri=GetRandom(i);
						Rj=GetRandom(j);
						iR2=GetRandom();
						g_Model[iR2][Ri][Rj]=dCurrVal;
					}
				}
				else
				{
					cvSetReal2D(g_ForeImg,i,j,255);
				}
			}
		}
	}
	//ForegroundCombineEdge();
	DeleteSmallAreaInForeground(80);
	ClearLongLifeForeground();
	//PostProcess();
}

double Vibe::AreaDense(IplImage *pFr,int AI,int AJ,int W,int H)
{
	if (AI<=2 || AJ<=2 || AJ>=(g_Width-2) || AI>=(g_Height-2))
	{
		return 0;
	}
	int Num=0,i=0,j=0;
	double dVal=0,dense=0;
	int Total=(2*H+1)*(2*W+1);
	for (i=AI-H;i<=AI+H;i++)
	{
		for (j=AJ-W;j<=AJ+W;j++)
		{
			dVal=cvGetReal2D(pFr,i,j);
			if (dVal>200)
			{
				Num++;
			}
		}
	}
	dense=(double)Num/(double)Total;
	return dense;
}

void Vibe::ForegroundCombineEdge()
{
	cvZero(g_Edge);
	//cvZero(g_SegementMask);
	//cvCopy(g_ForeImg,g_SegementMask);
	cvCanny(g_Gray,g_Edge,30,200,3);
	int i=0,j=0;
	double dense;
	double dVal;
	for (i=g_offset;i<g_Height-g_offset;i++)
	{
		for (j=g_offset;j<g_Width-g_offset;j++)
		{
			dense=AreaDense(g_ForeImg,i,j,2,2);
			dVal=cvGetReal2D(g_Edge,i,j);
			if (dense>0.2 && dVal>200)
			{
				cvSetReal2D(g_ForeImg,i,j,255);
			}
		}
	}
}

//清理前景中小区域
void Vibe::DeleteSmallAreaInForeground(double minArea/* =20 */)
{
	//cvZero(g_SegementMask);
	//cvCopy(g_ForeImg,g_SegementMask);
	int region_count = 0;
	CvSeq *first_seq = NULL, *prev_seq = NULL, *seq = NULL;
	CvMemStorage*  storage = cvCreateMemStorage();
	cvClearMemStorage(storage);
	cvFindContours( g_ForeImg, storage, &first_seq, sizeof(CvContour), CV_RETR_LIST );
	for( seq = first_seq; seq; seq = seq->h_next )
	{
		CvContour* cnt = (CvContour*)seq;
		if( cnt->rect.width * cnt->rect.height < minArea )
		{
			prev_seq = seq->h_prev;
			if( prev_seq )
			{
				prev_seq->h_next = seq->h_next;
				if( seq->h_next ) seq->h_next->h_prev = prev_seq;
			}
			else
			{
				first_seq = seq->h_next;
				if( seq->h_next ) seq->h_next->h_prev = NULL;
			}
		}
		else
		{
			region_count++;
		}
	}	
	cvZero(g_ForeImg);
	cvDrawContours(g_ForeImg, first_seq, CV_RGB(0, 0, 255), CV_RGB(0, 0, 255), 10, -1);

	/*
	CvContourScanner scanner = cvStartFindContours( g_ForeImg, storage,sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	CvSeq *contours=NULL,*c=NULL;
	int poly1Hull0=0;
	int nContours=0;
	double perimScale=100;
	while( (c = cvFindNextContour( scanner )) != 0 )
	{
		double len = cvContourPerimeter( c );
		double q = (g_ForeImg->height + g_ForeImg->width)/perimScale; // calculate perimeter len threshold
		if( len < q ) //Get rid of blob if it's perimeter is too small
			cvSubstituteContour( scanner, 0 );
		else //Smooth it's edges if it's large enough
		{
			CvSeq* newC;
			if( poly1Hull0 ) //Polygonal approximation of the segmentation
				newC = cvApproxPoly( c, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 2, 0 );
			else //Convex Hull of the segmentation
				newC = cvConvexHull2( c, storage, CV_CLOCKWISE, 1 );
			cvSubstituteContour( scanner, newC );
			nContours++;
		}
	}
	contours = cvEndFindContours( &scanner );
	// paint the found regions back into the image
	cvZero( g_ForeImg );
	for( c=contours; c != 0; c = c->h_next )
		cvDrawContours( g_ForeImg, c, cvScalarAll(255), cvScalarAll(0), -1, CV_FILLED, 8,cvPoint(0,0));
	*/
	cvReleaseMemStorage(&storage); 
}

//清理生命周期之前的前景
void Vibe::ClearLongLifeForeground(int i_lifeLength/* =200 */)
{
	int i=0,j=0;
	double dVal=0;
	double dLife=0;
	int iR1,iR2=0;
	double dCurrVal=0;
	for (i=g_offset;i<g_Height-g_offset;i++)
	{
		for (j=g_offset;j<g_Width-g_offset;j++)
		{
			dVal=cvGetReal2D(g_ForeImg,i,j);
			dLife=LifeLength[i][j];
			if (dLife>i_lifeLength)
			{
				LifeLength[i][j]=0;
				dCurrVal=cvGetReal2D(g_Gray,i,j);
				// 更新背景模型
				iR1=GetRandom();
				iR2=GetRandom();
				g_Model[iR1][i][j]=dCurrVal;
				g_Model[iR2][i][j]=dCurrVal;
			}
			else
			{
				LifeLength[i][j]=dLife+1;
			}
		}
	}
}

void Vibe::Update()
{
	cvZero(g_UpdateMask);
}

void Vibe::PostProcess()
{
	cvZero(g_SegementMask);
	cvMorphologyEx(g_ForeImg,g_SegementMask,NULL,element,CV_MOP_OPEN,1);
}

//算颜色畸变
double Vibe::CalcuColorDist(CvScalar bkCs,CvScalar curCs)
{
	double r,g,b,br,bg,bb;
	r=curCs.val[0];
	g=curCs.val[1];
	b=curCs.val[2];
	
	br=bkCs.val[0];
	bg=bkCs.val[1];
	bb=bkCs.val[2];
	
	double curDist=r*r+g*g*b*b;
	double bkDist=br*br+bg*bg+bb*bb;
	double curBK=r*br+g*bg+b*bb;
	double curbkDist=curBK*curBK;
	double SquareP;
	if (bkDist==0.0)
	{
		SquareP=0;
	}
	else
	{
		SquareP=curbkDist/bkDist;
	}
	double dist=sqrtf(curDist-SquareP);
	return dist; 
}

double Vibe::CalcPixelDist(CvScalar bkCs,CvScalar curCs)
{
	double tmpDist=pow(bkCs.val[0]-curCs.val[0],2)+pow(bkCs.val[1]-curCs.val[1],2)+pow(bkCs.val[2]-curCs.val[2],2);
	return sqrtf(tmpDist);
}

int Vibe::GetRandom()
{
	int val = g_SampleNum * 1.0 * rand() / RAND_MAX;
	if( val == g_SampleNum )
		return val - 1;
	else
		return val;
}

int Vibe::GetRandom(int random)
{
	int val=random-g_offset+rand()%(2*g_offset);
	if (val<random-g_offset)
	{
		val=random-g_offset;
	}
	if (val>random+g_offset)
	{
		val=random+g_offset;
	}
	return val; 
}

int Vibe::GetRandom(int istart,int iend)
{
	int val=istart+rand()%(iend-istart);
	return val;
}


Vibe::~Vibe(void)
{
	if (g_ForeImg)
	{
		cvReleaseImage(&g_ForeImg);
	}
	if (g_SegementMask)
	{
		cvReleaseImage(&g_SegementMask);
	}
	if (g_UpdateMask)
	{
		cvReleaseImage(&g_UpdateMask);
	}
	if (g_Gray)
	{
		cvReleaseImage(&g_Gray);
	}
	
	if (g_Model!=NULL)
	{
		delete[]g_Model;
		g_Model=NULL;
	}
}

