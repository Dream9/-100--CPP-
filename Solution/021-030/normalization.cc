#include"Solution/021-030/normalization.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

void Normalization::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img;

#ifdef USE_OPENCVLIB


	cv::normalize(data, img, 255, 0, cv::NORM_MINMAX, CV_8U);
	
	//normalize ����NORM_MINMAXʱ�ȼ�������Ĳ���
	double min = 0.;
	double max = 0.;
	//FIXME:�������ֵ��ȫ��ͨ���ģ��Ƿ�Ӧ����Բ�ͬͨ���ֱ���㣿
	cv::minMaxLoc(data, &min, &max);
	
	double alpha = (255 - 0) / (max - min);
	double beta = 0 - alpha * min;
	
	cv::convertScaleAbs(data, img, 255 / (max - min), beta);

#else

	double min = FLT_MAX;
	double max = 0.;
	
	auto get_min_max = [&](uint8_t* cursor) {
		min = MIN(min, *cursor);
		max = MAX(max, *cursor);
	};
	detail::grayscaleTransform(data, get_min_max);
	
	double alpha = (255 - 0) / (max - min);
	double beta = 0 - alpha * min;
	
	detail::convertScaleAbs(data, img, alpha, beta);

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

	if (show_pdf_) {
		//��һ��֮���ֱ��ͼ
		cv::Mat hist;
		cv::Mat distribution;
		detail::calcHistogram(img, hist, 0, 255, 255);
		detail::fillHistogram(hist, distribution, cv::Size(200, 200), CV_8U);
		show(&distribution);
	}
}


}