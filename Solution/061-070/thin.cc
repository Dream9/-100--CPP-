#include"Solution/061-070/thin.h"
#include"Solution/morphology_transform.h"

#include<opencv2/highgui.hpp>

namespace digital {

//brief:
void Thin::operator()() {
	static_assert(detail::Hilditch == Hilditch, "compability error");
	static_assert(detail::ZhangSuen== ZhangSuen, "compability error");
	
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;
	detail::thin(data, img, op_);

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}


}//!namespace digital