#include"Solution/solution.h"

//#include"Solution/channel_swap.h"
//#include"Solution/grayscale.h"
//#include"Solution/binarization.h"
//#include"Solution/otsu.h"
//#include"Solution/reverse_hue.h"
//#include"Solution/reduce_color.h"
//#include"Solution/mean_pooling.h"
//#include"Solution/max_pooling.h"
//#include"Solution/gaussian_filter.h"
#include"Solution/median_blur.h"

#include"Solution/mean_blur.h"

#ifdef _WIN32
#include<direct.h>
#else
#include<unistd.h>
#endif

using namespace digital;

const char* datapath ="F:/yzhh/VS2017pro/DigitalImageProcess/data/";
#define CHECK_ZERO(x) if((x))\
                 {\
                      perror("Fatal error");\
                      exit(EXIT_FAILURE);\
                 }
int main(){

#ifdef _WIN32
	CHECK_ZERO(_chdir(datapath));
#else
	CHECK_ZERO(chdir(datapath));
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
	
	MeanBlur meanb011("imori.jpg", 
					  5,
					  true);
	Solve(meanb011);

}


#undef CHECK_ZERO