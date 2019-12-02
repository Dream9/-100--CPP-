#include"Solution/011-020/differential_filter.h"

#include<opencv2/highgui.hpp>

#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#else
#include"Solution/base.h"
#endif

namespace digital {

//为了凑成奇数
const int DifferentialFilter::kWin = 3;

#define _SHOW(x,y) if(needShowOriginal())\
	{\
		show(&##x,&##y);\
	}\
	else{\
		show(&##y);\
	}

//becare:对x求导得到的是y方向的边沿，对y求导得到的是x方向的边沿
void DifferentialFilter::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

#ifdef USE_OPENCVLIB

	cv::Mat img;
	cv::Mat dx = (cv::Mat_<float>(3, 3) <<
		0, 0, 0,
		0, -1, 1,
		0, 0, 0);
	cv::filter2D(data, img, data.depth(), dx);
	//_SHOW(data, img);

	cv::Mat img_y;
	cv::Mat dy = (cv::Mat_<float>(3, 3) << 
		0, 0, 0,
		0, -1, 0,
		0, 1, 0);
	cv::filter2D(data, img_y, data.depth(), dy);
	//_SHOW(data, img);

	//计算梯度
	cv::Mat gradient;
	cv::addWeighted(img, 1.0, img_y, 1.0, 0.0, gradient);

#else

	//计算x方向的微分
	cv::Mat img;

	int filter_x[kWin * kWin] = {
		0, 0, 0,
		0,-1, 1,
		0, 0, 0 };
	//如果保留负值，然后重新标定，图像应该位于灰色调
	detail::filter2D<CV_8U, CV_16S, int>(data, img, filter_x, kWin, kWin, false, false);
	//重新calibration范围
	img.convertTo(img, CV_8UC1, 1.0, 128);
	//_SHOW(data, img);
	
	//计算y方向的微分
	cv::Mat img_y;
	int filter_y[kWin * kWin] = {
		0, 0, 0,
		0,-1, 0,
		0, 1, 0 };
	//如果不保留负值，图像应该位于黑色调
	detail::filter2D<CV_8U, CV_8U, int>(data, img_y, filter_y, kWin, kWin, false, false);
	//_SHOW(data, img_y);


	//近似计算梯度
	cv::Mat gradient = img + img_y;

#endif

	void*figs[4];
	figs[0] = &data;
	figs[1] = &img;
	figs[2] = &img_y;
	figs[3] = &gradient;
	if (needShowOriginal())
		show(figs, 4);
	else
		show(figs + 1, 3);
}

#undef _SHOW

}//!namespace digital
