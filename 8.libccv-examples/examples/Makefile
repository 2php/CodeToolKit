
all:
	gcc -o imagecopy.exe imagecopy.c -I../libccv -L../libccv -lccv -lpng -lz -ljpeg -lgsl -lgslcblas -lws2_32
	gcc -o facedetect.exe facedetect.c -I../libccv -L../libccv -IF:/opencv/build/include -LF:/opencv/build/x86/vc10/lib -lccv -lpng -lz -ljpeg -lgsl -lgslcblas -lfftw3 -lfftw3f -lws2_32 -lopencv_highgui2411 -lopencv_core2411

