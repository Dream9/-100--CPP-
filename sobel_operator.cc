#include"Solution/011-020/sobel_operator.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#ifndef NDEBUG
//for test
#include<vector>
#endif

namespace digital {

//brief:Sobel 算子有两个，一个是检测水平边缘的 ；另一个是检测垂直边缘的 。
//     与Prewitt算子相比，Sobel算子对于象素的位置的影响做了加权，可以降低边缘模糊程度
//becare:使用更大的窗口，意味着采取更高的模糊
void SobelOperator::operator()() {
    //转为灰度并降噪
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if(data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::GaussianBlur(data, data, cv::Size(3, 3), 0.0, 0.0);
	cv::Mat img_x;
	cv::Mat img_y;

#ifdef USE_OPENCVLIB

	//此外内部还提供一种Scharr的实现，其提供更高精度的一阶微分近似，是对Sobel的优化
	cv::Sobel(data, img_x, CV_16S, 1, 0, win_);
	//or use cv::filter2D
	
	cv::Sobel(data, img_y, CV_16S, 0, 1, win_);

#else
	//最初采用固定的空间滤波核的实现方式
	//int filter_x[] = {
	//	-1, 0, 1,
	//	-2, 0, 2,
	//	-1, 0, 1 };
	//detail::filter2D<CV_8U, CV_16S, int>(data, img_x, filter_x, 3, 3, false, false);

	//int filter_y[] = {
	//	-1, -2, -1,
	//	0, 1, 0,
	//	1, 2, 1 };
	//detail::filter2D<CV_8U, CV_16S, int>(data, img_y, filter_y, 3, 3, false, true);

	detail::Sobel(data, img_x, CV_16S, 1, 0, win_);
	detail::Sobel(data, img_y, CV_16S, 0, 1, win_);
#endif

	//重新标定
	cv::convertScaleAbs(img_x, img_x, 1, 128);
	cv::convertScaleAbs(img_y, img_y, 1, 128);

	//近似梯度
	cv::Mat gradient;
	cv::addWeighted(img_x, 0.5, img_y, 0.5, 0, gradient);

	assert_imshow_type(gradient.depth());

	void* figs[4];
	figs[0] = &data;
	figs[1] = &img_x;
	figs[2] = &img_y;
	figs[3] = &gradient;

	if (needShowOriginal())
		show(figs, 4);
	else
		show(figs + 1, 3);
}


}//!namespace digital