#include"Solution/solution.h"

//#include"Solution/001-010/channel_swap.h"
//#include"Solution/001-010/grayscale.h"
//#include"Solution/001-010/binarization.h"
//#include"Solution/001-010/otsu.h"
//#include"Solution/001-010/reverse_hue.h"
//#include"Solution/001-010/reduce_color.h"
//#include"Solution/001-010/mean_pooling.h"
//#include"Solution/001-010/max_pooling.h"
//#include"Solution/001-010/gaussian_filter.h"
//#include"Solution/001-010/median_blur.h"

//#include"Solution/011-020/mean_blur.h"
//#include"Solution/011-020/motion_blur.h"
//#include"Solution/011-020/max_min_filter.h"
//#include"Solution/011-020/differential_filter.h"
//#include"Solution/011-020/sobel_operator.h"
//#include"Solution/011-020/prewitt_operator.h"
//#include"Solution/011-020/laplacian.h"
//#include"Solution/011-020/emboss_filter.h"
//#include"Solution/011-020/LoG_filter.h"
//#include"Solution/011-020/histogram.h"

#include"Solution/021-030/normalization.h"

#ifdef _WIN32
#include<direct.h>
#else
#include<unistd.h>
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

	//ChannelSwap cs001("imori.jpg",
	//	              true);
	//Solve(cs001);

	//Grayscale gs002("imori.jpg",
	//	              true);
	//Solve(gs002);

	//Binarization bz003("imori.jpg",
	//				  true);
	//Solve(bz003);

	//Otsu otsu004("imori.jpg",
	//				  true);
	//Solve(otsu004);

	//ReverseHue rh005("imori.jpg",
	//				  true);
	//Solve(rh005);

	//ReduceColor rc006("imori.jpg",
	//				  true);
	//Solve(rc006);

	//MeanPooling mp007("imori.jpg", 
	//                8,
	//				  true);
	//Solve(mp007);

	//MaxPooling maxp008("imori.jpg", 
	//                8,
	//				  true);
	//Solve(maxp008);

	//GaussianFilter gf009("imori.jpg", 
	//                1.5,
	//                5,
	//				  true);
	//Solve(gf009);

	//MedianBlur mb010("imori_noise.jpg", 9,
	//				  true);
	//Solve(mb010);

	//MeanBlur meanb011("imori.jpg", 
	//				  5,
	//				  true);
	//Solve(meanb011);

	//MotionBlur motionb012("imori.jpg",
	//				  3,
	//				  0,
	//				  true);
	//Solve(motionb012);

	//MaxMinFilter mmf013("imori.jpg",
	//				 3,
	//	             true);
	//Solve(mmf013);

	//DifferentialFilter df014("imori.jpg",
    //        		true);
	//Solve(df014);
	
	//SobelOperator so015("imori.jpg",
	//				5,
    //        		true);
	//Solve(so015);
	
	//PrewittOperator po016("imori.jpg",
    //        		true);
	//Solve(po016);

	//Laplacian la017("imori.jpg",
    //        		true);
	//Solve(la017);
	
	//EmbossFilter ef018("imori.jpg",
    //        		true);
	//Solve(ef018);
	
	//LoG lg019("imori_noise.jpg",
	//				5,
    //        		true);
	//Solve(lg019);
	
	//Histogram hg020("imori_dark.jpg",
	//				223,
    //        		true);
	//Solve(hg020);
	
	Normalization nl021("imori_dark.jpg",
				    true,
            		true);
	Solve(nl021);



}

#undef CHECK_ZERO