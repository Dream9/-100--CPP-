#include"Solution/011-020/LoG_filter.h"
#include"Solution/base.h"

#include<opencv2/imgproc.hpp>
#include<cmath>

namespace digital {

//brief:根据窗口大小先对图像滤波
void LoG::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);

	cv::Mat filter;
	getLoGFilter_(&filter);

	cv::Mat img;
	
#ifdef USE_OPENCVLIB

	cv::flip(filter, filter, -1);
	cv::filter2D(data, img, CV_16S, filter);

#else

	detail::convolution2D(data, img, CV_16S, filter);

#endif

	cv::convertScaleAbs(img, img);

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

//brief；根据窗口大小获得sigma以及滤波核参数
void LoG::getLoGFilter_(void* dst) {
	cv::Mat& kernel = *static_cast<cv::Mat*>(dst);

	kernel.create(win_, win_, CV_32F);
	float* cursor = kernel.ptr<float>(0, 0);

	int right = win_ >> 1;
	int left = -right;

	double sigma = detail::getSigma(win_);
	double factor = 1. / (sigma * sigma);
	double factor2 = factor * 0.5;
	double sum = 0.;

	for (int i = left; i <= right; ++i) {
		for (int j = left; j <= right; ++j) {
			int cur = i * i + j * j;
			double tmp = (cur*factor - 2)*std::exp(-cur * factor2);
			*cursor++ = static_cast<float>(tmp);
			sum += tmp;
		}
	}

#ifndef NDEBUG
	__MatrixTest(&kernel);
#endif

	//归一化
	int N = win_ * win_ - 1 ;
	cursor = kernel.ptr<float>(0, 0);
	while (N >= 0) {
		*cursor /= sum;
		--N;
	}
}


}//!namespace digital