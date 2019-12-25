#include"Solution/001-010/otsu.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<vector>
#include<limits.h>
#include<cmath>
#include<numeric>

namespace digital {

//获取阈值
double Otsu::getOtsuThreshold_(void* input) {
	cv::Mat& img = *static_cast<cv::Mat*>(input);
	
	return detail::otsuThreshold(img);
	//以下为最初的实现//
	//std::vector<int> hist(256);
	//int rows = img.rows;
	//int cols = img.cols;
	//int64_t average = 0;
	//assert(INT64_MAX >= 255 * rows*cols);//通常不会溢出

	////统计直方图
	//int total = static_cast<int>(img.total());
	//auto iter = img.data;
	//auto end = img.data + total;
	//while (iter != end) {
	//	++hist[*iter];
	//	average += *iter;
	//	++iter;
	//}

	////类间最大方差法确定阈值
	////公式推导参见数字图像处理p481
	////这里利用了上一个值计算过程中间变量，减少重复计算
	//double global_v = 1.0*average / total;
	//std::vector<int> ans;//候选阈值
	//int k = 0;
	//int partial_sum = 0;
	//double last_mean = 0.0;
	//double max_v = 0.0;

	//while (k < 256) {
	//	partial_sum += hist[k];
	//	double cur_p = 1.0 * partial_sum / total;
	//	if (cur_p == 0 || cur_p == 1) {
	//		++k;
	//		continue;
	//	}

	//	double cur_mean = last_mean += 1.0*hist[k] / total * k;
	//	//简化后公式：(Mg*P1-M1)^2/(P1*(1-P1))
	//	double cur_v = ::pow(global_v*cur_p - cur_mean, 2) / (cur_p*(1 - cur_p));

	//	if (cur_v > max_v) {
	//		std::vector<int> tmp(1, k);
	//		tmp.swap(ans);
	//		max_v = cur_v;
	//	}
	//	else if (cur_v == max_v) {
	//		ans.emplace_back(k);
	//	}

	//	last_mean = cur_mean;
	//	++k;
	//}

	////当存在多个满足条件的值时，取其平均值
	//double threshold =  accumulate(ans.begin(), ans.end(), 0.0) / ans.size();

	//return threshold;
}

//brief:定制处理
void Otsu::operator()() {
	cv::Mat data = cv::imread(getPath(), 1);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//灰度化
	cv::Mat img;
	cv::cvtColor(data, img, cv::COLOR_RGB2GRAY);

	//确定阈值
#ifdef USE_OPENCVLIB
	//直接调用opencv库实现
	cv::threshold(data, img, 0, 255, CV_THRESH_OTSU);
#else
	double threshold = getOtsuThreshold_(&img);
	//二值化
	auto iter = img.data;
	auto end = iter + img.total();
	while (iter != end) {
		*iter = *iter > threshold ? 255 : 0;
		++iter;
	}
#endif

	//show
	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

}

}//！namespace digital