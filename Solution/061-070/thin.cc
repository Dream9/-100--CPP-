#include"Solution/061-070/thin.h"
#include"Solution/morphology_transform.h"

#include<opencv2/highgui.hpp>

namespace digital {

//brief:
void Thin::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;
	detail::thin(data, img, -1);

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}


}//!namespace digital