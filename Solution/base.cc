#include"Solution/base.h"
#include"Solution/type_extension.h"

#include<opencv2/imgproc.hpp>

#include<random>
#include<algorithm>

//for test
#include<vector>

namespace detail {

static std::default_random_engine __global_eng;
static std::normal_distribution<> __global_dis(1, 1);
//brief:返回服从正态分布的随机数，E = 1 , D = 1
//becare:不是线程安全的
//      也可以采用RNG对象生成
double randNorm() {
	return __global_dis(__global_eng);
}

//breif:计算阶乘
//FIXME:尾递归实现
int factorial(int i) {
	assert(i > -1);

	if (i == 0 || i == 1)
		return 1;
	return i * factorial(i - 1);
}

//brief:计算单个Mat的指定通道的概率密度函数
void calcHistogram(const cv::Mat& data, cv::Mat& hist, int min, int max, int bins, int* select_channels, int len) {
	//assert(data.channels() == 1);
	assert(bins > 0);
	assert(data.depth() == CV_8U);
	assert(len == -1 || (len > 0 && len < data.channels()));

	hist = cv::Mat::zeros(bins, 1, CV_32F);

	int ranges = max - min + 1;
	double ratio = 1. * bins / ranges;
	float* arr = hist.ptr<float>(0, 0);
	cv::Size size = data.size();
	int channels = data.channels();	
	int step = static_cast<int>(data.step);

	if (data.isContinuous()) {
		size.width *= size.height;
		size.height = 1;
	}
	if (!select_channels || len == -1 || len == channels) {
		//以下为全部channel参与计算
		//连续性判断
		size.width *= channels;

		//根据灰度值分发即可
		auto cur = data.data;
		for (int i = 0; i < size.height; ++i) {
			auto tmp = cur;
			for (int j = 0; j < size.width; ++j) {
				int index = static_cast<int>(*tmp * ratio);
				++arr[index];
				++tmp;
			}
			cur += step;
		}
	}
	else {
		//以下为部分channels参与运算
		//先经过排序，就可以以顺序方式遍历筛选
		std::sort(select_channels, select_channels + len);
		auto cur = data.data;
		for (int i = 0; i < size.height; ++i) {
			auto tmp = cur;
			for (int j = 0; j < size.width; ++j) {
				//筛选通道
				int first = 0;
				for (int cn = 0; first < len; ++cn) {
					if (select_channels[first] != cn) {
						++tmp;
						continue;
					}

					int index = static_cast<int>(*tmp * ratio);
					++arr[index];
					++tmp;
					++first;
				}
			}
			cur += step;
		}
	}
}



//brief:根据hist数值生成对应的直方图图像Mat
//becare:期待hist为数据类型CV_32F的列向量
void fillHistogram(const cv::Mat& hist,
	cv::Mat& dst,
	const cv::Size& size,
	int type,
	const cv::Scalar& color) 
{
	assert(hist.cols == 1);
	assert(hist.depth() == CV_32F);

	dst = cv::Mat(size, type, cv::Scalar::all(255));
	//cv::Mat(size, type, cv::Scalar::all(255)).copyTo(dst);

	double step = 1. * size.width / hist.rows;//确定水平步长

	double max_val;
	cv::minMaxLoc(hist, nullptr, &max_val);
	double scale = 0.9 * size.height / max_val;//确定垂直伸缩比例，使得最大的灰度占据90%的全长

	double x_pos_d = 0;
	int x_pos = 0;
	int y = size.height;

	auto cursor = hist.ptr<float>(0, 0);
	auto end = hist.ptr<float>(hist.rows - 1, 0);
	while (cursor != end) {
		if (fabs(*cursor - 0.0) < digital::k_EPS) {
			x_pos_d += step;
			//FIXME:不要直接用int记录，否则存在舍入精度的损失,特别是step小于1时，总是被舍弃
			//x_pos = static_cast<int>(x_pos_d + step);
			++cursor;
			continue;
		}

		x_pos = static_cast<int>(x_pos_d);

#ifndef NDEBUG

		//for test
		digital::__printInfo("%d,%d--%d,%d\r\n", x_pos, y, x_pos, y - static_cast<int>(*cursor *scale));

#endif

		//brief；通过line表示出直方图分布
		cv::line(dst, cv::Point(x_pos, y), cv::Point(x_pos, y - static_cast<int>(*cursor * scale)), color, 1, 0);
		x_pos_d += step;
		++cursor;
	}
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

//brief:二维卷积
void convolution2D(const cv::Mat& src, 
	cv::Mat& dst, 
	int ddepth, 
	const cv::Mat& kernel, 
	cv::Point p , 
	int bordertype) 
{
	cv::Mat flip_k;
	cv::flip(kernel, flip_k, -1);

	//fort test
	//std::vector<float> vk = std::vector<float>(flip_k);

	cv::filter2D(src, dst, ddepth, flip_k, p, 0.0, bordertype);
}

//brief:分离的二维卷积
void sepConvolution2D(const cv::Mat& src, 
	cv::Mat& dst, 
	int ddepth,
	const cv::Mat& kernelx,
	const cv::Mat& kernely, 
	cv::Point p , 
	int bordertype) 
{
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