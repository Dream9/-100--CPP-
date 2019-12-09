//²âÊÔÎÄ¼þ

#include"Solution/solve_main.h"

#ifdef _WIN32
#include<direct.h>
#elif defined(__LINUX__)
#include<unistd.h>
#else
#error Unexpected os
#endif

using namespace digital;

const char* kDefaultDatapath ="F:/yzhh/VS2017pro/DigitalImageProcess/data/";

#define CHECK_ZERO(x,y) if((x))\
                 {\
                      perror("Fatal error:"#y);\
                      exit(EXIT_FAILURE);\
                 }
int main(int argc, char** argv) {
	const char* datapath;
	datapath = argc > 1 ? argv[1] : kDefaultDatapath;

#ifdef _WIN32
	CHECK_ZERO(_chdir(datapath), "change work directory failed.");
#elif defined(__LINUX__)
	CHECK_ZERO(chdir(datapath), "change work directory failed.");
#else
#error unexpected os
#endif

#ifdef NO_001
	ChannelSwap cs001("imori.jpg",
		              true);
	Solve(cs001);
#endif

#ifdef NO_002
	Grayscale gs002("imori.jpg",
		              true);
	Solve(gs002);
#endif

#ifdef NO_003
	Binarization bz003("imori.jpg",
					  true);
	Solve(bz003);
#endif

#ifdef NO_004
	Otsu otsu004("imori.jpg",
					  true);
	Solve(otsu004);
#endif

#ifdef NO_005
	ReverseHue rh005("imori.jpg",
					  true);
	Solve(rh005);
#endif

#ifdef NO_006
	ReduceColor rc006("imori.jpg",
					  true);
	Solve(rc006);
#endif

#ifdef NO_007
	MeanPooling mp007("imori.jpg", 
	                8,
					  true);
	Solve(mp007);
#endif

#ifdef NO_008
	MaxPooling maxp008("imori.jpg", 
	                8,
					  true);
	Solve(maxp008);
#endif

#ifdef NO_009
	GaussianFilter gf009("imori.jpg", 
	                1.5,
	                5,
					  true);
	Solve(gf009);
#endif

#ifdef NO_010
	MedianBlur mb010("imori_noise.jpg", 9,
					  true);
	Solve(mb010);
#endif

#ifdef NO_011
	MeanBlur meanb011("imori.jpg", 
					  5,
					  true);
	Solve(meanb011);
#endif

#ifdef NO_012
	MotionBlur motionb012("imori.jpg",
					  3,
					  0,
					  true);
	Solve(motionb012);
#endif

#ifdef NO_013
	MaxMinFilter mmf013("imori.jpg",
					 3,
		             true);
	Solve(mmf013);
#endif

#ifdef NO_014
	DifferentialFilter df014("imori.jpg",
            		true);
	Solve(df014);
#endif

#ifdef NO_015
	SobelOperator so015("imori.jpg",
					5,
            		true);
	Solve(so015);
#endif
	
#ifdef NO_016
	PrewittOperator po016("imori.jpg",
            		true);
	Solve(po016);
#endif

#ifdef NO_017
	Laplacian la017("imori.jpg",
            		true);
	Solve(la017);
#endif
	
#ifdef NO_018
	EmbossFilter ef018("imori.jpg",
            		true);
	Solve(ef018);
#endif
	
#ifdef NO_019
	LoG lg019("imori_noise.jpg",
					5,
            		true);
	Solve(lg019);
#endif
	
#ifdef NO_020
	Histogram hg020("imori_dark.jpg",
					223,
            		true);
	Solve(hg020);
#endif
	
#ifdef NO_021
	Normalization nl021("imori_dark.jpg",
				    true,
            		true);
	Solve(nl021);
#endif

#ifdef NO_022
	HistogramTransformation ht022("imori_dark.jpg",
					128.,
					50,
					true);
	Solve(ht022);
#endif

#ifndef NO_022
	EqualizeHistogram eh023("imori_dark.jpg",
					true,
					true);
	Solve(eh023);
#endif



}

#undef CHECK_ZERO