#ifndef _SOLUTION_OTSU_H_
#define _SOLUTION_OTSU_H_

#include"Solution/solution.h"''

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<vector>
#include<limits.h>
#include<cmath>
#include<numeric>

namespace digital {

//brief;���������󷽲�Զ�ȷ����ֵ������ֵ
class Otsu : public Solution {
public:
	Otsu(const string& path, bool flag, const string& name = "Otsu")
		:Solution(name, path, flag) 
	{
		getDescriptionHandler().assign("ʹ�ô���ֵ���㷨�������󷽲��㷨�����лҶȴ���");
	}
	virtual ~Otsu() {  }

	virtual void operator()() {
		cv::Mat data = cv::imread(getPath(), 1);
		if (data.empty()) {
			dealException(kFileError);
			return;
		}

		cv::Mat img;
		cv::cvtColor(data, img, cv::COLOR_RGB2GRAY);

		std::vector<int> hist(256);
		int rows = img.rows;
		int cols = img.cols;
		int64_t average = 0;
		assert(INT64_MAX >= 255 * rows*cols);//ͨ���������

		//ͳ��ֱ��ͼ
		int total = img.total();
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
		int threshold =  accumulate(ans.begin(), ans.end(), 0) / ans.size();

		//��ֵ��
		iter = img.data;
		while (iter != end) {
			*iter = *iter > threshold ? 255 : 0;
			++iter;
		}

		if (needShowOriginal())
			show(&data, &img);
		else
			show(&img);
	}
};

}//��namespace digital

#endif