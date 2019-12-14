#include"Solution/fourier_transform.h"
#include"grayscale_transfrom.h"

#include"Solution/type_extension.h"

#include<opencv2/imgproc.hpp>

namespace {

const double kPI = 3.14159265354;

//becare:此时cursor对应real图像
void __forwardFourier_real(double* iter, double*& cursor, double theta) {
	*iter += *cursor * std::cos(theta);
	++cursor;
}
void __forwardFourier_complex(double* iter, double*& cursor, double theta) {
	*iter += *cursor * std::cos(theta);
	*(iter+1) += *cursor * std::sin(theta);		
	++cursor;
}

//becare:此时cursor对应complex频率域
void __inverseFourier_complex(double* iter, double*& cursor, double theta) {
	*iter += std::cos(theta)*(*cursor) - std::sin(theta)*(*(cursor + 1));
	*(iter + 1) += std::cos(theta)*(*(cursor + 1)) + std::sin(theta)*(*cursor);
	cursor += 2;
}
void __inverseFourier_real(double* iter, double*& cursor, double theta) {
	*iter += std::cos(theta)*(*cursor) - std::sin(theta)*(*(cursor + 1));
	cursor += 2;
}
}

namespace detail {

//brief:
void dft(cv::Mat& src, cv::Mat& dst, int flags) {
	//assert(src.depth() == CV_64F || src.depth() == CV_32F);//只接受两种深度
	assert(src.depth() == CV_64F);//只接受1种深度

	double factor_inverse = -1.0;//默认进行fourier forward变换
	int dst_channels = src.channels();//默认输出结果与src一致
	double scale = 1.;//默认不进行归一化处理
	cv::Size size = src.size();

	if (flags & detail::DFT_INVERSE) {
		factor_inverse = 1.0;
	}
	if (flags & detail::DFT_REAL_OUTPUT) {
		dst_channels = 1;
	}
	if (flags & detail::DFT_COMPLEX_OUTPUT) {
		dst_channels = 2;
	}
	if (flags & detail::DFT_SCALE) {
		scale = 1. / static_cast<double>(size.width * size.height);
	}

	//根据flag确定本次调用功能
	void(*op)(double*, double*&, double);
	if (dst_channels == 2) {
		if (flags & detail::DFT_INVERSE)
			op = __inverseFourier_complex;
		else
			op = __forwardFourier_complex;
	}
	else {
		if (flags & detail::DFT_INVERSE)
			op = __inverseFourier_real;
		else
			op = __forwardFourier_real;
	}

	dst.create(size, CV_MAKETYPE(CV_64F, dst_channels));

	double* iter = dst.ptr<double>(0, 0);

	//针对单通道src,结果保存为双通道dst
	//最初的暴力解法，复杂度为O（M*N*M*N）
	for (int u = 0; u < size.height; ++u) {
		for (int v = 0; v < size.width; ++v) {
			assert(iter == dst.ptr<double>(u, v));
			*iter = 0.0;
			*(iter + 1) = 0.0;

			double* cursor = src.ptr<double>(0, 0);

			for (int n = 0; n < size.height; ++n) {
				for (int m = 0; m < size.width; ++m) {
					assert(cursor == src.ptr<double>(n, m));

					double theta = factor_inverse * 2 * kPI *( 1. * u * n / size.height + 1. * v * m / size.width);

					op(iter, cursor, theta);
					
					//for test
					//digital::__printInfo("theta:%f\r\n", theta);
				}
			}

			*iter++ *= scale;
			if (dst_channels == 2)
				*iter++ *= scale;
		}
	}
}

//brief:从傅里叶变换结果中得到幅度谱（傅里叶谱）
void getAmplitudeSpectrum(cv::Mat& src, cv::Mat& dst) {
	assert(src.channels() == 2);

	std::vector<cv::Mat> channels;
	cv::split(src, channels);

	cv::magnitude(channels[0], channels[1], dst);
	//or equal to
	//cv::Mat tmp1, tmp2;
	//cv::pow(channels[0], 2, tmp1);
	//cv::pow(channels[0], 2, tmp2);
	//cv::sqrt(tmp1 + tmp2, dst);
}

//brief:
cv::Mat grayscaleAmplitudeSpctrum(cv::Mat& spectrum) {
	cv::Mat dst;
	cv::log(spectrum + 1, dst);
	cv::normalize(dst, dst, 0, 255, cv::NORM_MINMAX, CV_8U);
	return dst;
}


}//!namespace detail