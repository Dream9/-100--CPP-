#include"Solution/061-070/connect_number.h"
#include"Solution/morphology_transform.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

//brief£»
void ConnectNumber::operator()() {
	static_assert(cv::LINE_4 == LINE_4, "compability error");
	static_assert(cv::LINE_8 == LINE_8, "compability error");

	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;

	detail::setConnectNumber(data, img, line_type_);

	//cv::equalizeHist(img, img);

	
	cv::Scalar k[] = { {255,0,0},{0,255,0},{0,0,255},
						{128,128,128},{70,70,70},{200,200,200},
						{128,128,0},{0,128,128},{128,0,128} };

	cv::Mat img2 = cv::Mat::zeros(data.size(), CV_8UC3);
	auto iter = img2.data;
	size_t jump = img2.elemSize();
	auto set_color = [&k,&iter,jump](uint8_t* cur) {
		if (*cur == 0) {
			iter += jump;
			return;
		}
		auto count = *cur - 1;
		//coutInfo(std::to_string(count).c_str());

		iter[0] = uint8_t(k[count][0]);
		iter[1] = uint8_t(k[count][1]);
		iter[2] = uint8_t(k[count][2]);

		iter += jump;
	};
	detail::grayscaleTransform(img, set_color);

	if (needShowOriginal())
		show(&data, &img2);
	else
		show(&img2);
}

}