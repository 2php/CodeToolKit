CC := gcc
AR := ar
NVCC := 
CUDA_OBJS := 
DEFINE_MACROS := -D HAVE_LIBPNG -D HAVE_LIBJPEG -D HAVE_FFTW3 -D HAVE_SSE2 -D HAVE_GSL
prefix := /usr/local
exec_prefix := ${prefix}
CFLAGS := -msse2 $(DEFINE_MACROS) -I${prefix}/include
NVFLAGS := --use_fast_math -arch=sm_30 $(DEFINE_MACROS)
LDFLAGS := -L${exec_prefix}/lib -lm -lpng -ljpeg -lfftw3 -lfftw3f -lpthread -lgsl -lgslcblas
