#include"Solution/gaussian_filter.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include<cmath>
#include<algorithm>

namespace digital {

//brief:��˹�˲�
void GaussianFilter::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//TODO:��չ�߽�
	cv::Size size = data.size();
	cv::Mat img = data.clone();

#ifdef USE_OPENCVLIB

	cv::GaussianBlur(data, img, { win_,win_ }, sigma_, sigma_);
	
#else

	int win = win_;
	std::vector<std::vector<double>> arr(win, std::vector<double>(win, 0.0));
	getGaussianFilter_(arr, sigma_);
	//reverseArray_(arr);//�����������Ϊ��˹�����ĶԳ�����ʹ�䷴ת��Ľ������

	const int radius = win >> 1;


	for (int i = radius; i < size.height - radius; ++i) {
		for (int j = radius; j < size.width - radius; ++j) {
			auto cur = img.ptr<uint8_t>(i, j);
			//���о������
			//becare:��ʱ���˲��˲����Ѿ���ת��180
			double sum_r = 0;
			double sum_g = 0;
			double sum_b = 0;

			//����ѭ��չ��
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

//brief:��ø�˹�˲��˲���coefficient
void GaussianFilter::getGaussianFilter_(std::vector<std::vector<double>>& arr, double sigma) {
	assert(!arr.empty());
	assert(!arr[0].empty());

	int m = static_cast<int>(arr.size());
	int n = static_cast<int>(arr[0].size());
	
#ifdef USE_OPENCVLIB
	//opencv�����˼���һά��˹��ϵ���ķ���
	cv::Mat ans = cv::getGaussianKernel(m, sigma_) * cv::getGaussianKernel(n, sigma_).t();
	//��ʱ����ͨ��cv::sepFilter2D���з�����˲��˵ı任
	
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
	//ͨ������ǰ��һ������һ�������Ͼ���ȡ������ɢ�ģ������ᵼ��ͼ������ֵ����ƫС
	//const double karea_factor = 1.0 / (2 * k_PI*sigma*sigma);
	for (int i = -cen_x; i <= cen_x; ++i) {
		for (int j = -cen_y; j <= cen_y; ++j) {
			double tmp = exp((i*i + j * j)*ksigma_2);
			sum += tmp;
			arr[i + cen_x][j + cen_y] = tmp;
			//arr[i + cen_x][j + cen_y] = tmp * karea_factor;
		}
	}

	//brief:��һ����������һ�������С
	for (std::vector<double>& vec : arr) {
		std::transform(vec.begin(), vec.end(), vec.begin(), [=](double val)
			{
				return val / sum; 
			});
	}

#endif

}

//brief:
//becare:���ڸ�˹�˲����ԣ���Ϊ���ǶԳƵģ�ʵ���Ͽ��Բ��÷�ת
//      ���;�����ԣ���ת�Ǳ��뾭����һ�����������������
//      ��һ����ʵ�������뵽���˲��˲���֮��
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