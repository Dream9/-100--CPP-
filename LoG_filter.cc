#include"Solution/011-020/LoG_filter.h"
#include"Solution/base.h"

#include<opencv2/imgproc.hpp>
#include<cmath>

namespace digital {

#define OUT_RANGE(x,y,s) (x<0 || y<0 || x>=s.height || y>=s.width)

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
	
#ifndef USE_OPENCVLIB

	cv::flip(filter, filter, -1);
	cv::filter2D(data, img, CV_16S, filter);

#else

	detail::convolution2D(data, img, CV_16S, filter);

#endif

	//下面是基于LoG的Marr-Hildreth边缘检测，进行交叉零点检测
	cv::Mat new_img=cv::Mat::zeros(img.size(), CV_8U);
	cv::Size size = img.size();
	for (int x = 0; x < size.height; ++x) {
		for (int y = 0; y < size.width; ++y) {
			int x1 = x, y1 = y - 1, x2 = x, y2 = y + 1;
			if (!OUT_RANGE(x1, y1, size) && !OUT_RANGE(x2, y2, size)) {
				if (img.at<short>(x1, y1)*img.at<short>(x2, y2) < 0)
					new_img.at<uchar>(x, y) = 255;
			}

			x1 = x-1, y1 = y , x2 = x+1, y2 = y ;
			if (!OUT_RANGE(x1, y1, size) && !OUT_RANGE(x2, y2, size)) {
				if (img.at<short>(x1, y1)*img.at<short>(x2, y2) < 0)
					new_img.at<uchar>(x, y) = 255;
			}
			
			x1 = x-1, y1 = y - 1, x2 = x+1, y2 = y + 1;
			if (!OUT_RANGE(x1, y1, size) && !OUT_RANGE(x2, y2, size)) {
				if (img.at<short>(x1, y1)*img.at<short>(x2, y2) < 0)
					new_img.at<uchar>(x, y) = 255;
			}
			
			x1 = x+1, y1 = y - 1, x2 = x-1, y2 = y + 1;
			if (!OUT_RANGE(x1, y1, size) && !OUT_RANGE(x2, y2, size)) {
				if (img.at<short>(x1, y1)*img.at<short>(x2, y2) < 0)
					new_img.at<uchar>(x, y) = 255;
			}
		}
	}


	cv::convertScaleAbs(img, img);

	show(&img, &new_img,"左侧为正常LoG,右侧为使用Marr-Hildreth进一步处理的结果");

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
		*cursor /= float(sum);
		--N;
	}
}


}//!namespace digital