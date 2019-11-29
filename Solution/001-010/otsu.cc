#include"Solution/otsu.h"

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<vector>
#include<limits.h>
#include<cmath>
#include<numeric>

namespace {
//brief:参考了opencv库的实现
//becare:只针对类型为uint8_t的数据
double __getOtsu(const cv::Mat& img) {
	int step = static_cast<int>(img.step);
	cv::Size size = img.size();

	//brief:针对连续内存的优化
	if (img.isContinuous()) {
		size.width *= size.height;
		size.height = 1;
		step = size.width * sizeof(uchar);
	}

	const int N = 1 << (sizeof(uchar) * 8);
	//std::vector<int> hist(N);
	int hist[N] = { 0 };
	int i = 0;
	int j = 0;

	double global_m = 0;
	for (; i < size.height; ++i) {
		auto iter = img.ptr() + step * i;
		j = 0;

		//brief:减少不必要的内存引用
		int end = size.width;
		//brief:进行循环展开
		for (; j + 3 < end; j += 4) {
			++hist[iter[j]];
			++hist[iter[j+1]];
			++hist[iter[j+2]];
			++hist[iter[j+3]];

			global_m += iter[j];
			global_m += iter[j+1];
			global_m += iter[j+2];
			global_m += iter[j+3];
		}
	}

	double scale = 1.0 / size.width / size.height;
	global_m *= scale;//平均灰度值
	double prev_m1 = 0;
	double prev_p1 = 0;
	double max_sigma = 0;
	double out = 0;

	for (i = 0; i < N; ++i) {
		double pi = hist[i] * scale;
		double cur_p1 = prev_p1 + pi;
		double cur_m1 = (prev_m1*prev_p1 + i * pi) / cur_p1;
		
		//brief:防止除接近0的数
		if (fabs(cur_p1) < 1e-8 || fabs(1 - cur_p1) < 1e-8)
			continue;

		double cur_p2 = 1 - cur_p1;
		double cur_m2 = (global_m - cur_p1 * cur_m1) / cur_p2;

		//becare:这里采用了 P1*P2*(m1-m2)^2的计算，经过化简后与另一个公式相同
		double cur_sigma = cur_p1 * cur_p2*pow(cur_m1 - cur_m2, 2);

		if (cur_sigma > max_sigma) {
			max_sigma = cur_sigma;
			out = i;
		}

		prev_m1 = cur_m1;
		prev_p1 = cur_p1;
	}

	return out;
}

}

namespace digital {

//获取阈值
double Otsu::getOtsuThreshold_(const void* input) {
	const cv::Mat& img = *static_cast<const cv::Mat*>(input);
	
	return __getOtsu(img);

	//以下为最初的实现
	std::vector<int> hist(256);
	int rows = img.rows;
	int cols = img.cols;
	int64_t average = 0;
	assert(INT64_MAX >= 255 * rows*cols);//通常不会溢出

	//统计直方图
	int total = static_cast<int>(img.total());
	auto iter = img.data;
	auto end = img.data + total;
	while (iter != end) {
		++hist[*iter];
		average += *iter;
		++iter;
	}

	//类间最大方差法确定阈值
	//公式推导参见数字图像处理p481
	//这里利用了上一个值计算过程中间变量，减少重复计算
	double global_v = 1.0*average / total;
	std::vector<int> ans;//候选阈值
	int k = 0;
	int partial_sum = 0;
	double last_mean = 0.0;
	double max_v = 0.0;

	while (k < 256) {
		partial_sum += hist[k];
		double cur_p = 1.0 * partial_sum / total;
		if (cur_p == 0 || cur_p == 1) {
			++k;
			continue;
		}

		double cur_mean = last_mean += 1.0*hist[k] / total * k;
		//简化后公式：(Mg*P1-M1)^2/(P1*(1-P1))
		double cur_v = ::pow(global_v*cur_p - cur_mean, 2) / (cur_p*(1 - cur_p));

		if (cur_v > max_v) {
			std::vector<int> tmp(1, k);
			tmp.swap(ans);
			max_v = cur_v;
		}
		else if (cur_v == max_v) {
			ans.emplace_back(k);
		}

		last_mean = cur_mean;
		++k;
	}

	//当存在多个满足条件的值时，取其平均值
	double threshold =  accumulate(ans.begin(), ans.end(), 0.0) / ans.size();

	return threshold;
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