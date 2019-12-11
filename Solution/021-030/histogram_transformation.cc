#include"Solution/021-030/histogram_transformation.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/highgui.hpp>

namespace digital {

//brief:
//becare:主要是lambda对象配合detail::grayscaleTransform
void HistogramTransformation::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;

#ifdef USE_OPENCVLIB

	auto tmp = data.reshape(1);

	cv::Scalar mean;
	cv::Scalar sigma;
	cv::meanStdDev(tmp, mean, sigma);

#ifndef NDEBUG
	__printInfo("before mean:%f\tstd:%f\r\n", mean[0], sigma[0]);
#endif

	double alpha = sigma_ / sigma[0];
	double beta = -mean[0] * alpha + mean_;

	cv::convertScaleAbs(data, img, alpha, beta);


#else

	double mean = 0.;
	double sigma = 0.;
	double N_inv = 1. / (data.total() * data.channels());

	//获得均值
	auto get_mean = [&](uint8_t* cursor) {
		mean += *cursor;
	};
	detail::grayscaleTransform(data, get_mean);
	mean *= N_inv;

	//获得sigma
	auto get_sigma = [&](uint8_t* cursor) {
		sigma += std::pow(*cursor - mean, 2);
	};
	detail::grayscaleTransform(data, get_sigma);
	sigma = std::sqrt(sigma * N_inv);
	
	double alpha = sigma_ / sigma;
	double beta = -mean * alpha + mean_;

	//重新赋值
	detail::convertScaleAbs(data, img, alpha, beta);

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

	
}

}//!namespace digital