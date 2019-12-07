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
//brief:���ط�����̬�ֲ����������E = 1 , D = 1
//becare:�����̰߳�ȫ��
//      Ҳ���Բ���RNG��������
double randNorm() {
	return __global_dis(__global_eng);
}

//breif:����׳�
//FIXME:β�ݹ�ʵ��
int factorial(int i) {
	assert(i > -1);

	if (i == 0 || i == 1)
		return 1;
	return i * factorial(i - 1);
}

//brief:���㵥��Mat��ָ��ͨ���ĸ����ܶȺ���
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
		//����Ϊȫ��channel�������
		//�������ж�
		size.width *= channels;

		//���ݻҶ�ֵ�ַ�����
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
		//����Ϊ����channels��������
		//�Ⱦ������򣬾Ϳ�����˳��ʽ����ɸѡ
		std::sort(select_channels, select_channels + len);
		auto cur = data.data;
		for (int i = 0; i < size.height; ++i) {
			auto tmp = cur;
			for (int j = 0; j < size.width; ++j) {
				//ɸѡͨ��
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



//brief:����hist��ֵ���ɶ�Ӧ��ֱ��ͼͼ��Mat
//becare:�ڴ�histΪ��������CV_32F��������
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

	double step = 1. * size.width / hist.rows;//ȷ��ˮƽ����

	double max_val;
	cv::minMaxLoc(hist, nullptr, &max_val);
	double scale = 0.9 * size.height / max_val;//ȷ����ֱ����������ʹ�����ĻҶ�ռ��90%��ȫ��

	double x_pos_d = 0;
	int x_pos = 0;
	int y = size.height;

	auto cursor = hist.ptr<float>(0, 0);
	auto end = hist.ptr<float>(hist.rows - 1, 0);
	while (cursor != end) {
		if (fabs(*cursor - 0.0) < digital::k_EPS) {
			x_pos_d += step;
			//FIXME:��Ҫֱ����int��¼������������뾫�ȵ���ʧ,�ر���stepС��1ʱ�����Ǳ�����
			//x_pos = static_cast<int>(x_pos_d + step);
			++cursor;
			continue;
		}

		x_pos = static_cast<int>(x_pos_d);

#ifndef NDEBUG

		//for test
		digital::__printInfo("%d,%d--%d,%d\r\n", x_pos, y, x_pos, y - static_cast<int>(*cursor *scale));

#endif

		//brief��ͨ��line��ʾ��ֱ��ͼ�ֲ�
		cv::line(dst, cv::Point(x_pos, y), cv::Point(x_pos, y - static_cast<int>(*cursor * scale)), color, 1, 0);
		x_pos_d += step;
		++cursor;
	}
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

//brief:����Ķ�ά���
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