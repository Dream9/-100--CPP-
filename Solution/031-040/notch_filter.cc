#include"Solution/031-040/notch_filter.h"
#include"Solution/fourier_transform.h"

#include<opencv2/highgui.hpp>

namespace{


}


namespace digital {

void NotchFilter::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat data_64f;
	cv::Mat fft_data;
	cv::Mat lp_fft_data;
	cv::Mat img;
	string title;

	data.convertTo(data_64f, CV_64F);
	data_64f = detail::centralize(data_64f);
	detail::dft(data_64f, fft_data, detail::DFT_COMPLEX_OUTPUT);

	//becare:关于这两个参数，只有采用巴特沃斯时，第二个参数才发挥作用
	//double threshold[] = { 50,2,40 };


	detail::frequencyDomainFilter(fft_data, lp_fft_data, detail::NOTCH);

	cv::Mat spectrum;
	detail::getAmplitudeSpectrum(lp_fft_data, spectrum);
	cv::Mat out = detail::grayscaleAmplitudeSpctrum(spectrum);

	if (needShowOriginal())
		show(&data, &out, "Amplitude Spectrum");
	else
		show(&out, "Amplitude Spectrum");

	detail::fft(lp_fft_data, data_64f, detail::DFT_INVERSE | detail::DFT_REAL_OUTPUT | detail::DFT_SCALE);
	data_64f = data_64f(cv::Rect(0, 0, data.cols, data.rows));//防止fft的副作用
	data_64f = detail::centralize(data_64f);
	data_64f.convertTo(img, CV_8U);

	if (needShowOriginal())
		show(&data, &img, title);
	else
		show(&img, title);

}

}//!namespace digital