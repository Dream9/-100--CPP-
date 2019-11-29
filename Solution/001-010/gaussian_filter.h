#ifndef _SOLUTION_GAUSSIAN_FILTER_H_
#define _SOLUTION_GAUSSIAN_FILTER_H_

#include"Solution/solution.h"

#include<vector>

namespace digital {

class GaussianFilter :public Solution {
public:
	//parameter: sigma:标准差，win:窗口大小，如果为负，则自动推导
	GaussianFilter(const string& path, double sigma = 1.3, int win = -1, bool flag = false, const string& name = "GaussianFilter")
		:Solution(name, path, flag), sigma_(sigma), win_(win)
	{
		getDescriptionHandler().assign("对图像进行高斯滤波");
		if (win_ < 0)
			win_ = getGaussianFilterSize_(sigma_);
	}
	~GaussianFilter()override {}

	void operator()()override;

private:

	//brief:根据sigma得到窗口大小
	//     根据3*sigma的积分面积已经占99.7%的原则
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