#include"Solution/041-050/canny.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

void Canny::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;

#ifdef USE_OPENCVLIB
	cv::Canny(data, img, threshold_1_, threshold_2_, aperture_size_, use_L2gradient_);//it should be the same result(except the bordertype)
#else
	detail::Canny(data, img, threshold_1_, threshold_2_, aperture_size_, use_L2gradient_);
#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

}

}//!namespace digital