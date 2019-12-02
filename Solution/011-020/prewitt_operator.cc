#include"Solution/011-020/prewitt_operator.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

static const int kWin = 3;

//brief:
void PrewittOperator::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);
	cv::GaussianBlur(data, data, cv::Size(3, 3), 0);

	cv::Mat img_x;
	cv::Mat img_y;

#ifdef USE_OPENCVLIB

	cv::Mat kernel_x = (cv::Mat_<float>(kWin, kWin) <<
		-1, 0, 1,
		-1, 0, 1,
		-1, 0, 1);
	cv::filter2D(data, img_x, CV_16S, kernel_x);
	
	cv::Mat kernel_y = (cv::Mat_<float>(kWin, kWin) <<
		-1, -1, -1,
		0, 0, 0,
		1, 1, 1);
	cv::filter2D(data, img_y, CV_16S, kernel_y);

#else

	int filter_x[kWin*kWin] = {
		-1, 0, 1,
		-1, 0, 1,
		-1, 0, 1
	};
	detail::filter2D<CV_8U, CV_16S>(data, img_x, filter_x, kWin, kWin, false, false);

	int filter_y[kWin*kWin] = {
		-1, -1, -1,
		0, 0, 0,
		1, 1, 1,
	};
	detail::filter2D<CV_8U, CV_16S>(data, img_y, filter_y, kWin, kWin, false, false);

#endif

	//粗略的重新calibration
	cv::convertScaleAbs(img_x, img_x, 1, 128);
	cv::convertScaleAbs(img_y, img_y, 1, 128);
	cv::Mat gradient;
	cv::addWeighted(img_x, 0.5, img_y, 0.5, 0, gradient);

	//显示结果
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

}//！namespace digital