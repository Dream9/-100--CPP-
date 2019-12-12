#include"Solution/021-030/bicubic_interpolation.h"
#include"Solution/geometric_transform.h"

#include<opencv2/highgui.hpp>

namespace {

const double kScale = 1.5;

}

namespace digital {

//brief:
//becare:实现位于geometric_transform.[h/cc]
void BicubicInterpolation::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img;
	cv::Size size = data.size();
	size.width = static_cast<int>(kScale * size.width);
	size.height = static_cast<int>(kScale * size.height);

#ifdef USE_OPENCVLIB

	cv::Mat M = cv::getRotationMatrix2D(cv::Point(0, 0), 0, kScale);
	cv::warpAffine(data, img, M, size, cv::INTER_CUBIC);

#else

	cv::Mat M = detail::getRotationMatrix2D(cv::Point(0, 0), 0, kScale);
	detail::warpAffine(data, img, M, size, cv::INTER_CUBIC, cv::Scalar::all(0));

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

}