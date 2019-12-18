#include"Solution/031-040/fourier.h"
#include"Solution/fourier_transform.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

//brief:测试了正反变换，并计算了傅里叶谱和相角谱
//     具体实现为与fourier.[h/cc]
void Fourier::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat data_64f;
	data.convertTo(data_64f, CV_64F);

	//移频
	data_64f = detail::centralize(data_64f);

	cv::Mat img;
	cv::Mat idft_img;

	////////////////////////forward fourier transform
#ifdef USE_OPENCVLIB
	cv::Mat test;
	cv::dft(data_64f, test, cv::DFT_COMPLEX_OUTPUT);
	//img = test;
#else
	detail::dft(data_64f, img, detail::DFT_COMPLEX_OUTPUT);

#endif
	//for test
	//__MatrixTest(&test, &img);

	////////////////////////show Spectrum
	cv::Mat spectrum;
	detail::getAmplitudeSpectrum(img, spectrum);
	cv::Mat out = detail::grayscaleAmplitudeSpctrum(spectrum);

	if (needShowOriginal())
		show(&data, &out,"Amplitude Spectrum");
	else
		show(&out,"Amplitude Spectrum");

	/////////////////////////show phase
	cv::Mat phase;
#ifdef USE_OPENCVLIB
	std::vector<cv::Mat> vec;
	cv::split(img, vec);
	cv::phase(vec[0], vec[1], phase);
#else
	detail::getPhaseSpectrum(img, phase);
#endif
	//for test
	//__MatrixTest(&phase, &ttt);
	cv::normalize(phase, phase, 1, 255, cv::NORM_MINMAX, CV_8U);

	if (needShowOriginal())
		show(&data, &phase, "Phase Spectrum");
	else
		show(&phase,"Phase Spectrum");

	////////////////////////inverse fourier transform
#ifdef USE_OPENCVLIB
	cv::dft(img, idft_img, cv::DFT_SCALE | cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
#else
	detail::dft(img, idft_img, detail::DFT_INVERSE | detail::DFT_SCALE | detail::DFT_REAL_OUTPUT);
#endif
    //for test
	//__MatrixTest(&test, &idft_img);

	//移频
	idft_img = detail::centralize(idft_img);

	idft_img.convertTo(idft_img, CV_8U);

	if (needShowOriginal())
		show(&data, &idft_img,"右图为根据DFT结果重建图像");
	else
		show(&idft_img,"根据DFT结果重建图像");
}



}//!namespace digital