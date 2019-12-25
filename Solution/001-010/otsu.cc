#include"Solution/001-010/otsu.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<vector>
#include<limits.h>
#include<cmath>
#include<numeric>

namespace digital {

//��ȡ��ֵ
double Otsu::getOtsuThreshold_(void* input) {
	cv::Mat& img = *static_cast<cv::Mat*>(input);
	
	return detail::otsuThreshold(img);
	//����Ϊ�����ʵ��//
	//std::vector<int> hist(256);
	//int rows = img.rows;
	//int cols = img.cols;
	//int64_t average = 0;
	//assert(INT64_MAX >= 255 * rows*cols);//ͨ���������

	////ͳ��ֱ��ͼ
	//int total = static_cast<int>(img.total());
	//auto iter = img.data;
	//auto end = img.data + total;
	//while (iter != end) {
	//	++hist[*iter];
	//	average += *iter;
	//	++iter;
	//}

	////�����󷽲ȷ����ֵ
	////��ʽ�Ƶ��μ�����ͼ����p481
	////������������һ��ֵ��������м�����������ظ�����
	//double global_v = 1.0*average / total;
	//std::vector<int> ans;//��ѡ��ֵ
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
	//	//�򻯺�ʽ��(Mg*P1-M1)^2/(P1*(1-P1))
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

	////�����ڶ������������ֵʱ��ȡ��ƽ��ֵ
	//double threshold =  accumulate(ans.begin(), ans.end(), 0.0) / ans.size();

	//return threshold;
}

//brief:���ƴ���
void Otsu::operator()() {
	cv::Mat data = cv::imread(getPath(), 1);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//�ҶȻ�
	cv::Mat img;
	cv::cvtColor(data, img, cv::COLOR_RGB2GRAY);

	//ȷ����ֵ
#ifdef USE_OPENCVLIB
	//ֱ�ӵ���opencv��ʵ��
	cv::threshold(data, img, 0, 255, CV_THRESH_OTSU);
#else
	double threshold = getOtsuThreshold_(&img);
	//��ֵ��
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

}//��namespace digital