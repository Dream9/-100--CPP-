#include"Solution/gaussian_filter.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include<cmath>
#include<algorithm>

namespace digital {

//brief:高斯滤波
void GaussianFilter::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//TODO:扩展边界
	cv::Size size = data.size();
	cv::Mat img = data.clone();

#ifdef USE_OPENCVLIB

	cv::GaussianBlur(data, img, { win_,win_ }, sigma_, sigma_);
	
#else

	int win = win_;
	std::vector<std::vector<double>> arr(win, std::vector<double>(win, 0.0));
	getGaussianFilter_(arr, sigma_);
	//reverseArray_(arr);//特殊情况，因为高斯函数的对称特性使其反转后的结果不变

	const int radius = win >> 1;


	for (int i = radius; i < size.height - radius; ++i) {
		for (int j = radius; j < size.width - radius; ++j) {
			auto cur = img.ptr<uint8_t>(i, j);
			//进行卷积运算
			//becare:此时的滤波核参数已经反转了180
			double sum_r = 0;
			double sum_g = 0;
			double sum_b = 0;

			//采用循环展开
			for (int x = 0; x < win; ++x) {
				auto iter = data.ptr<cv::Vec3b>(i + x - radius, j);
				for (int y = 0; y < win; ++y) {
					sum_r += arr[x][y] * iter[y - radius][2];
					sum_g += arr[x][y] * iter[y - radius][1];
					sum_b += arr[x][y] * iter[y - radius][0];
				}
			}

			cur[2] = static_cast<uint8_t>(sum_r);
			cur[1] = static_cast<uint8_t>(sum_g);
			cur[0] = static_cast<uint8_t>(sum_b);
		}
	}

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

//brief:获得高斯滤波核参数coefficient
void GaussianFilter::getGaussianFilter_(std::vector<std::vector<double>>& arr, double sigma) {
	assert(!arr.empty());
	assert(!arr[0].empty());

	int m = static_cast<int>(arr.size());
	int n = static_cast<int>(arr[0].size());
	
#ifdef USE_OPENCVLIB
	//opencv内置了计算一维高斯核系数的方法
	cv::Mat ans = cv::getGaussianKernel(m, sigma_) * cv::getGaussianKernel(n, sigma_).t();
	//此时可以通过cv::sepFilter2D进行分离的滤波核的变换
	
	auto iter = ans.data;
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			arr[i][j] = *reinterpret_cast<double*>(iter);
			iter += sizeof(double);
		}
	}
#else

	assert((m & 0x1) == 0x1);
	assert((n & 0x1) == 0x1);

	int cen_x = m >> 1;
	int cen_y = n >> 1;

	double sum = 0;
	const double ksigma_2 = -1.0 / (2 * sigma*sigma);
	//通过计算前归一化存在一定的误差，毕竟是取样是离散的，这样会导致图像像素值整体偏小
	//const double karea_factor = 1.0 / (2 * k_PI*sigma*sigma);
	for (int i = -cen_x; i <= cen_x; ++i) {
		for (int j = -cen_y; j <= cen_y; ++j) {
			double tmp = exp((i*i + j * j)*ksigma_2);
			sum += tmp;
			arr[i + cen_x][j + cen_y] = tmp;
			//arr[i + cen_x][j + cen_y] = tmp * karea_factor;
		}
	}

	//brief:归一化，计算后归一化误差最小
	for (std::vector<double>& vec : arr) {
		std::transform(vec.begin(), vec.end(), vec.begin(), [=](double val)
			{
				return val / sum; 
			});
	}

#endif

}

//brief:
//becare:对于高斯滤波而言，因为他是对称的，实际上可以不用反转
//      当就卷积而言，反转是必须经过的一步，否则是在求相关
//      这一步其实可以融入到求滤波核参数之中
void GaussianFilter::reverseArray_(std::vector<std::vector<double>>& arr) {
	assert(!arr.empty());
	assert(!arr[0].empty());
	
	int m = static_cast<int>(arr.size());
	int n = static_cast<int>(arr[0].size());

	for (int i = 0; i < m; ++i) {
		for (int j = i; j < n; ++j) {
			std::swap(arr[i][j], arr[m - i - 1][n - j - 1]);
		}
	}
}

}