#include"Solution/base.h"

namespace detail {

//breif:计算阶乘
//FIXME:尾递归实现
int factorial(int i) {
	assert(i > -1);

	if (i == 0 || i == 1)
		return 1;
	return i * factorial(i - 1);
}

//brief:以二项式展开式作为非归一化高斯系数的近似
//return:差分算子结果为列向量
cv::Mat getSmoothKernel(int n)
{
	cv::Mat kernel = cv::Mat::zeros(cv::Size(n, 1), CV_32FC1);
	auto cur = kernel.ptr<float>(0, 0);
	for (int i = 0; i < n; i++){
		//利用二项式展开式作为高斯平滑的近似
		*cur++ = static_cast<float>(detail::factorial(n - 1) / (detail::factorial(i) * detail::factorial(n - 1 - i)));
	}
	return kernel;
}

//brief:得到soble差分算子,
//return:差分算子结果为列向量
cv::Mat getSobelDifference(int n)
{
	cv::Mat diff = cv::Mat::zeros(cv::Size(n, 1), CV_32FC1);
	//差分算子的前身是n-2阶的二项式展开式，与平滑算子相差1
	cv::Mat prev = getSmoothKernel(n - 1);
	//利用差分算子的获得过程，改变Mat
	auto cur = diff.ptr<float>(0, 0);
	auto iter = prev.ptr<float>(0, 0);
	*cur++ = -1;
	for (int i = 1; i < n - 1; i++)
	{
		*cur++ = (*iter - *(iter + 1));
		++iter;
	}
	*cur = 1;
	return diff;
}

}//!namespace detail