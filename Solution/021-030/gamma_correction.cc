#include"Solution/021-030/gamma_correction.h"

#ifndef USE_OPENCVLIB
#include"Solution/base.h"
#include<functional>
#endif

#include<opencv2/highgui.hpp>

#include"Solution/geometric_transform.h"

namespace digital {

//brief:伽马校正是为了迎合显示器等电子设备的输出自带伽马编码而引入的
//becare:他把0-1范围内的数字进行了拉伸
void GammaCorrection::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;

#ifdef USE_OPENCVLIB

	//cv::multiply(data, 1./ c_, img, 1.0, CV_32F);
	//becare：务必转换到0-1范围
	cv::multiply(data, 1./ (c_ * UINT8_MAX), img, 1.0, CV_32F);
	cv::pow(img, 1. / gamma_, img);
	img.convertTo(img, data.type(), UINT8_MAX);

#else

	//img.create(data.size(), CV_MAKETYPE(CV_32F, data.channels()));
	img.create(data.size(), data.type());

	auto iter = img.data;
	double alpha = 1. / (c_ * UINT8_MAX);
	double gamma_inv = 1. / gamma_;

	//利用 lambda对象配合grayscaleTransform接口
	auto gamma_func = [&iter,alpha,gamma_inv](uint8_t* cursor) {
		*iter = cv::saturate_cast<uint8_t>(std::pow(*cursor * alpha, gamma_inv) * UINT8_MAX);
		++iter;
	};
	//一次性逐个像素转换
	detail::grayscaleTransform(data, gamma_func);
	
#endif

	//for test
	//__MatrixTest(&data, &img);


	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

}