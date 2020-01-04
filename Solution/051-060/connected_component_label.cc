#include"Solution/051-060/connected_component_label.h"
#include"Solution/morphology_transform.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/highgui.hpp>

namespace digital {

void ConnectedComponentLabel::operator()() {
	//兼容性断言
	static_assert(detail::LINE_4 == LINE_4, "compability error");
	static_assert(detail::LINE_8 == LINE_8, "compability error");

	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;
	cv::Mat img2;

	auto len = detail::getConnectComponent(data, img, line_type_, false);
#ifdef USE_ORIGINAL_IMPLEMENT
	//任然输出灰度图像
	cv::Mat lut = cv::Mat::zeros(1, 256, CV_8UC1);
	int ratio = (UINT8_MAX - 30) / len;
	for (int i = 0; i < len; ++i)
		lut.at<uint8_t>(i+1) = ratio * i + 30;

	//for test
	//__MatrixTest(&lut);

	detail::LUT(img, img2, lut);
#else
	//为不同分量赋予颜色
	cv::Scalar k[] = { {255,0,0},{0,255,0},{0,0,255},
						{128,128,128},{70,70,70},{200,200,200},
						{128,128,0},{0,128,128},{128,0,128} };
	int LEN = sizeof k / sizeof(cv::Scalar);

	img2 = cv::Mat::zeros(data.size(), CV_8UC3);
	auto iter = img2.data;
	size_t jump = img2.elemSize();
	auto set_color = [&k,&iter,jump, LEN](uint8_t* cur) {
		if (*cur == 0) {
			iter += jump;
			return;
		}
		auto count = *cur % LEN;
		iter[0] = uint8_t(k[count][0]);
		iter[1] = uint8_t(k[count][1]);
		iter[2] = uint8_t(k[count][2]);

		iter += jump;
	};
	detail::grayscaleTransform(img, set_color);

#endif
	if (needShowOriginal())
		show(&data, &img2);
	else
		show(&img2);
}

}//!namespace digital