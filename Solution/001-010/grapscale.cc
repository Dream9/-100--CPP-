#include"Solution/001-010/grayscale.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

//brief:
void Grayscale::operator()()
{
	cv::Mat data = cv::imread(getPath(), 1);

	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	
	int rows = data.rows;
	int cols = data.cols;

	cv::Mat img(rows, cols, CV_8UC1);

#ifdef USE_OPENCVLIB
	//brief:使用自带库函数实现
	cv::cvtColor(data, img, cv::COLOR_RGB2GRAY);
#else
	for (int i = 0; i < rows; ++i) {
		auto ptr = data.ptr<uchar>(i);
		auto target = img.ptr<uchar>(i);
		for (int j = 0; j < cols; ++j) {
			//按照加权平均获得灰度值，rgb:0.3,0.59,0.11
			*target = static_cast<uint8_t>(0.11*(*ptr) + 0.59*(*(ptr + 1)) + 0.3*(*(ptr + 2)));

			++target;
			ptr += 3;
		}
	}
#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}


}//!namespace digital