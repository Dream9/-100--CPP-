#include"Solution/base.h"

#include<opencv2/imgproc.hpp>

//for test
#include<vector>

namespace detail {

//breif:����׳�
//FIXME:β�ݹ�ʵ��
int factorial(int i) {
	assert(i > -1);

	if (i == 0 || i == 1)
		return 1;
	return i * factorial(i - 1);
}

//brief:�Զ���ʽչ��ʽ��Ϊ�ǹ�һ����˹ϵ���Ľ���
//return:������ӽ��Ϊ������
cv::Mat getSmoothKernel(int n)
{
	cv::Mat kernel = cv::Mat::zeros(cv::Size(n, 1), CV_32FC1);
	auto cur = kernel.ptr<float>(0, 0);
	for (int i = 0; i < n; i++){
		//���ö���ʽչ��ʽ��Ϊ��˹ƽ���Ľ���
		*cur++ = static_cast<float>(detail::factorial(n - 1) / (detail::factorial(i) * detail::factorial(n - 1 - i)));
	}
	return kernel;
}

//brief:�õ�soble�������,
//return:������ӽ��Ϊ������
cv::Mat getSobelDifference(int n)
{
	cv::Mat diff = cv::Mat::zeros(cv::Size(n, 1), CV_32FC1);
	//������ӵ�ǰ����n-2�׵Ķ���ʽչ��ʽ����ƽ���������1
	cv::Mat prev = getSmoothKernel(n - 1);
	//���ò�����ӵĻ�ù��̣��ı�Mat
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

//brief:��ά���
void convolution2D(cv::Mat& src, cv::Mat& dst, int ddepth, cv::Mat kernel, cv::Point p , int bordertype) {
	cv::Mat flip_k;
	cv::flip(kernel, flip_k, -1);

	//fort test
	//std::vector<float> vk = std::vector<float>(flip_k);

	cv::filter2D(src, dst, ddepth, flip_k, p, 0.0, bordertype);
}

//brief:����Ķ�ά���
void sepConvolution2D(cv::Mat& src, cv::Mat& dst, int ddepth, cv::Mat kernelx, cv::Mat kernely, cv::Point p , int bordertype) {
	cv::Mat flip_kx;
	cv::Mat flip_ky;
	cv::flip(kernelx, flip_kx, -1);
	cv::flip(kernely, flip_ky, -1);

	//fort test
	//std::vector<float> vx = std::vector<float>(kernelx);
	//std::vector<float> vy = std::vector<float>(flip_ky);

	cv::sepFilter2D(src, dst, ddepth, flip_kx, flip_ky, p, 0.0, bordertype);
}

//brief:
void colorInversion(cv::Mat& src, int max_value) {
	src = max_value - src;
}

}//!namespace detail