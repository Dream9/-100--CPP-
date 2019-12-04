#include"Solution/011-020/emboss_filter.h"
#include"Solution/base.h"

#include<opencv2/imgproc.hpp>

namespace digital {

const int EmbossFilter::kWin = 3;

//brief:采用了沿着主对角线的方向进行增强
void EmbossFilter::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);
	cv::Mat img;

	float filter[] = {
		-2, -1, 0,
		-1, 1, 1,
		0, 1, 2 };

#ifdef USE_OPENCVLIB

	cv::Mat kernel(cv::Size(3, 3),CV_32F,filter);
	cv::filter2D(data, img, CV_16S, kernel);

#else

	detail::filter2D<CV_8U, CV_16S, float>(data, img, filter, kWin, kWin, false, true);

#endif

	cv::convertScaleAbs(img, img);

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}


}