//test file 

#include"Solution/test/solve_main.h"

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
					3,
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

#ifdef NO_023
	EqualizeHistogram eh023("imori_dark.jpg",
					true,
					true);
	Solve(eh023);
#endif

#ifdef NO_024
	GammaCorrection gc024("imori_gamma.jpg",
					1.5,
					2.5,
					true);
	Solve(gc024);
#endif

#ifdef NO_025
	NearestNeighborInterpolation gi025("imori.jpg",
					true);
	Solve(gi025);
#endif

#ifdef NO_026
	BilinearInterpolation bi026("imori.jpg",
					true);
	Solve(bi026);
#endif

#ifdef NO_027
	BicubicInterpolation bicubici027("imori.jpg",
					true); 
	Solve(bicubici027);
#endif

	//以下测试共用一个用例
#if (defined NO_028) || (defined NO_029) || (defined NO_030) || (defined NO_031)
	int ops = 0;
#endif
#ifdef NO_028
	ops = ops | AffineTransformation::TRANSLATION;
#endif
#ifdef NO_029
	ops = ops | AffineTransformation::SCALE;
#endif
#ifdef NO_030
	ops = ops | AffineTransformation::ROTATION;
#endif
#ifdef NO_031
	ops = ops | AffineTransformation::LEAN;
#endif
#if (defined NO_028) || (defined NO_029) || (defined NO_030) || (defined NO_031)
	AffineTransformation at028("imori.jpg",
					ops,
					true); 
	Solve(at028);
#endif


#ifdef NO_032
	Fourier fr032("rectangle.jpg",
					true); 
	Solve(fr032);
#endif

#ifdef NO_033
	FrequencyDomainFilterLowPass fdflp033("rectangle.jpg",
					FrequencyDomainFilterLowPass::BLPF,
					true); 
	Solve(fdflp033);
#endif

#ifdef NO_034
	FrequencyDomainFilterHighPass fdfhp034("rectangle.jpg",
					FrequencyDomainFilterHighPass::GHPF,
					true); 
	Solve(fdfhp034);
#endif

#ifdef NO_035
	FrequencyDomainFilterBand fdfb035("rectangle.jpg",
					FrequencyDomainFilterBand::BR,
					true); 
	Solve(fdfb035);
#endif

#ifdef NO_035
	coutInfo("不在100问之列，但是是一种很有必要了解的一种滤波手段");
	NotchFilter nf0XX("rectangle.jpg",
					true); 
	Solve(nf0XX);
#endif




#if (defined NO_041) || (defined NO_042) || (defined NO_043)
	coutInfo("三道题合并，如需查看过程，在base.cc编译时加入 -DSHOW_PROCESS,然后重新编译本测试文件");
	Canny cy041("house.jpg",
					100,
					300,
					3,
					true,
					true); 
	Solve(cy041);
#endif





}

#undef CHECK_ZERO