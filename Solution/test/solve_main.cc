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
	LoG lg019("house2.jpg",
					37,
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
	int affine_ops = 0;
#endif
#ifdef NO_028
	affine_ops = affine_ops | AffineTransformation::TRANSLATION;
#endif
#ifdef NO_029
	affine_ops = affine_ops | AffineTransformation::SCALE;
#endif
#ifdef NO_030
	affine_ops = affine_ops | AffineTransformation::ROTATION;
#endif
#ifdef NO_031
	affine_ops = affine_ops | AffineTransformation::LEAN;
#endif
#if (defined NO_028) || (defined NO_029) || (defined NO_030) || (defined NO_031)
	AffineTransformation at028("imori.jpg",
					affine_ops,
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

	//以下测试共用一个用例
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

	//以下测试共用一个用例
#if (defined NO_044) || (defined NO_045) || (defined NO_046)
	coutInfo("三道题合并，如需查看过程，在geometric_match.cc编译时加入 -DSHOW_PROCESS,然后重新编译本测试文件");
	HoughLines hl044 ("house2.jpg",
			    	1,
					3.14159265354 / 180,
					120,
					true);
	Solve(hl044);
#endif



	//以下测试共用一个用例
#if (defined NO_047) || (defined NO_048) || (defined NO_049) || (defined NO_050) || \
    (defined NO_051) || (defined NO_052) || (defined NO_053)
	int morph_ops = 0;
#endif
#ifdef NO_047
	morph_ops = morph_ops | Morphology::DILATE;
#endif
#ifdef NO_048
	morph_ops = morph_ops | Morphology::ERODE;
#endif
#ifdef NO_049
	morph_ops = morph_ops | Morphology::OPEN;
#endif
#ifdef NO_050
	morph_ops = morph_ops | Morphology::CLOSE;
#endif
#ifdef NO_051
	morph_ops = morph_ops | Morphology::GRADIENT;
#endif
#ifdef NO_052
	morph_ops = morph_ops | Morphology::TOPHAT;
#endif
#ifdef NO_053
	morph_ops = morph_ops | Morphology::BLACKHAT;
#endif

#if (defined NO_047) || (defined NO_048) || (defined NO_049) || (defined NO_050) || \
    (defined NO_051) || (defined NO_052) || (defined NO_053)

	Morphology md047("imori_uneven_shadow.jpg",
					2,
					morph_ops,
					true);
	Solve(md047);
#endif

#if (defined NO_054) || (defined NO_055) || (defined NO_056) || (defined NO_057)
	int match_ops;

#endif
#ifdef NO_054
	match_ops = MatchTemplate::TM_SQDIFF;
#endif
#ifdef NO_055
	coutInfo("055题就是用L1范数衡量相似性，在opencv的matchTemplate中并没有封装本功能。"
		"实现起来和054差不多，就是将L2范数部分改成绝对值累和即可");
	match_ops = MatchTemplate::TM_ABSDIFF;
#endif
#ifdef NO_056
	match_ops = MatchTemplate::TM_CCORR;
#endif
#ifdef NO_057
	match_ops = MatchTemplate::TM_CCOEFF;
#endif


#if (defined NO_054) || (defined NO_055) || (defined NO_056) || (defined NO_057)
	coutInfo("默认的处理方式都采用了归一化的处理，如果需要测试非归一化的方法，请在编译"
	"match_template.[h/cc]时，加入-DWITHOUT_NORMED，然后重新编译本测试文件");
	MatchTemplate mt054("imori.jpg",
					"imori_part.jpg",
					match_ops,
					true);
	Solve(mt054);
#endif



	//以下共享一个测试
#if (defined NO_058) || (defined NO_059)
	int op;
#ifdef NO_058
	op = ConnectedComponentLabel::LINE_4;
#endif
#ifdef NO_059
	op = ConnectedComponentLabel::LINE_8;
#endif
	ConnectedComponentLabel ccl058("seg.png",
					op,
					true);
	Solve(ccl058);
#endif

	//以下共享一个测试
#if (defined NO_061) || (defined NO_062)
	int op;
#ifdef NO_061
	op = ConnectNumber::LINE_4;
#endif
#ifdef NO_062
	op = ConnectNumber::LINE_8;
#endif
	ConnectNumber cn061("48neighbor.png",
					op,
					true);
	Solve(cn061);
#endif


#if (defined NO_063) || (defined NO_064) || (defined NO_065)
	int thin_op;
#endif
#if (defined NO_063)
	thin_op = Thin::Hilditch;
#endif
#if (defined NO_064)
	thin_op = Thin::Hilditch;
#endif
#if (defined NO_065)
	thin_op = Thin::ZhangSuen;
#endif

#if (defined NO_063) || (defined NO_064) || (defined NO_065)
	Thin tn063("g1.png",
				thin_op,
				true);
	Solve(tn063);
#endif

#if (defined NO_066) || (defined NO_067) || (defined NO_068)
	Hog hg066("imori.jpg",
		true);
	Solve(hg066);
#endif

	return 0;
}

#undef CHECK_ZERO