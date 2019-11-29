#ifndef _SOLUTION_GAUSSIAN_FILTER_H_
#define _SOLUTION_GAUSSIAN_FILTER_H_

#include"Solution/solution.h"

#include<vector>

namespace digital {

class GaussianFilter :public Solution {
public:
	//parameter: sigma:��׼�win:���ڴ�С�����Ϊ�������Զ��Ƶ�
	GaussianFilter(const string& path, double sigma = 1.3, int win = -1, bool flag = false, const string& name = "GaussianFilter")
		:Solution(name, path, flag), sigma_(sigma), win_(win)
	{
		getDescriptionHandler().assign("��ͼ����и�˹�˲�");
		if (win_ < 0)
			win_ = getGaussianFilterSize_(sigma_);
	}
	~GaussianFilter()override {}

	void operator()()override;

private:

	//brief:����sigma�õ����ڴ�С
	//     ����3*sigma�Ļ�������Ѿ�ռ99.7%��ԭ��
	int getGaussianFilterSize_(double sigma) {
		return (static_cast<int>(3 * sigma) << 1) + 1;
	}

	void getGaussianFilter_(std::vector<std::vector<double>>& arr, double sigma);
	void reverseArray_(std::vector<std::vector<double>>& arr);

	double sigma_;
	int win_;
};
}



#endif