#include <ccv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

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

int main(int argc, char** argv)
{
	ccv_dense_matrix_t* image = 0;
	ccv_read(argv[1], &image, CCV_IO_RGB_COLOR | CCV_IO_ANY_FILE);
	ccv_scd_classifier_cascade_t* cascade = ccv_scd_classifier_cascade_read(argv[2]);
	ccv_array_t* faces = ccv_scd_detect_objects(image, &cascade, 1, ccv_scd_default_params);
	int i;

	IplImage* ddr = cvLoadImage(argv[1],CV_LOAD_IMAGE_COLOR);
	if(!ddr)
	{
		return -1;
	}

	for(i = 0; i < faces->rnum; i++)
	{
		ccv_comp_t* face = (ccv_comp_t*)ccv_array_get(faces, i);
		printf("%d %d %d %d\n",face->rect.x, face->rect.y, face->rect.width, face->rect.height);

		CvPoint topLeft, bottomRight;
		topLeft.x = face->rect.x;
		topLeft.y = face->rect.y;
		bottomRight.x = face->rect.x + face->rect.width;
		bottomRight.y = face->rect.y + face->rect.height;
		cvRectangle(ddr, topLeft, bottomRight, colors[i%8], 2, 8, 0);
	}
	cvShowImage("1", ddr);
	cvWaitKey(0);
	cvSaveImage("out.jpg", ddr, 0);
	cvReleaseImage(&ddr);
	ccv_array_free(faces);
	ccv_scd_classifier_cascade_free(cascade);
	ccv_matrix_free(image);

	return 0;
}

