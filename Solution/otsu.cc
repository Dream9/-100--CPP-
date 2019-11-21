#include"Solution/otsu.h"

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<vector>
#include<limits.h>
#include<cmath>
#include<numeric>

namespace {
//brief:�ο���opencv���ʵ��
//becare:ֻ�������Ϊuint8_t������
double __getOtsu(const cv::Mat& img) {
	int step = static_cast<int>(img.step);
	cv::Size size = img.size();

	//brief:��������ڴ���Ż�
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

		//brief:���ٲ���Ҫ���ڴ�����
		int end = size.width;
		//brief:����ѭ��չ��
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
	global_m *= scale;//ƽ���Ҷ�ֵ
	double prev_m1 = 0;
	double prev_p1 = 0;
	double max_sigma = 0;
	double out = 0;

	for (i = 0; i < N; ++i) {
		double pi = hist[i] * scale;
		double cur_p1 = prev_p1 + pi;
		double cur_m1 = (prev_m1*prev_p1 + i * pi) / cur_p1;
		
		//brief:��ֹ���ӽ�0����
		if (fabs(cur_p1) < 1e-8 || fabs(1 - cur_p1) < 1e-8)
			continue;

		double cur_p2 = 1 - cur_p1;
		double cur_m2 = (global_m - cur_p1 * cur_m1) / cur_p2;

		//becare:��������� P1*P2*(m1-m2)^2�ļ��㣬�������������һ����ʽ��ͬ
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

//��ȡ��ֵ
double Otsu::getOtsuThreshold_(const void* input) {
	const cv::Mat& img = *static_cast<const cv::Mat*>(input);
	
	return __getOtsu(img);

	//����Ϊ�����ʵ��
	std::vector<int> hist(256);
	int rows = img.rows;
	int cols = img.cols;
	int64_t average = 0;
	assert(INT64_MAX >= 255 * rows*cols);//ͨ���������

	//ͳ��ֱ��ͼ
	int total = static_cast<int>(img.total());
	auto iter = img.data;
	auto end = img.data + total;
	while (iter != end) {
		++hist[*iter];
		average += *iter;
		++iter;
	}

	//�����󷽲ȷ����ֵ
	//��ʽ�Ƶ��μ�����ͼ����p481
	//������������һ��ֵ��������м�����������ظ�����
	double global_v = 1.0*average / total;
	std::vector<int> ans;//��ѡ��ֵ
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
		//�򻯺�ʽ��(Mg*P1-M1)^2/(P1*(1-P1))
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

	//�����ڶ������������ֵʱ��ȡ��ƽ��ֵ
	double threshold =  accumulate(ans.begin(), ans.end(), 0.0) / ans.size();

	return threshold;
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