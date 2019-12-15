#include"Solution/fourier_transform.h"
#include"Solution/type_extension.h"
#include"Solution/grayscale_transfrom.h"
#include"Solution/geometric_transform.h"

#include<opencv2/imgproc.hpp>

//brief:以下函数对用户隐藏
namespace {

typedef void(*FourierOperationType)(double*, double*&, double, int);

const double kPI = 3.14159265354;
const int COMPLEX_CHANNEL_NUMBER = 2;

//brief:针对实/复矩阵的不同计算操作
//parameter:iter:结果矩阵当前计算handler
//          cursor:原始矩阵当前运算handler
//          theta:根据欧拉公式换算成的角度值
//          step:指针偏移量，主要用于行列计算的兼容
//becare:此时输入cursor对应real矩阵
void __realFourier_real(double* iter, double*& cursor, double theta, int step) {
	*iter += *cursor * std::cos(theta);//输出iter对应real
	cursor += step;
}
void __realFourier_complex(double* iter, double*& cursor, double theta,int step) {
	*iter += *cursor * std::cos(theta);//输出iter对应complex
	*(iter+1) += *cursor * std::sin(theta);		
	cursor += step;
}

//becare:此时输入cursor对应complex矩阵
void __complexFourier_complex(double* iter, double*& cursor, double theta, int step) {
	*iter += std::cos(theta)*(*cursor) - std::sin(theta)*(*(cursor + 1));//输出iter对应complex
	*(iter + 1) += std::cos(theta)*(*(cursor + 1)) + std::sin(theta)*(*cursor);
	cursor += step;
}
void __complexFourier_real(double* iter, double*& cursor, double theta, int step) {
	*iter += std::cos(theta)*(*cursor) - std::sin(theta)*(*(cursor + 1));//输出iter对应real
	cursor += step;
}

//becare:Fourier 1D行（列）变换
//parameter:src:数据源
//          tmp:存储目标，要求已经分配了相关内存，即本函数只负责执行计算，不负责上层功能分发
//          factor_inverse:正反向变换标识
//          scale:缩放因子
//          op:执行的操作，只包括四种类型
void __dft_row(cv::Mat& src, cv::Mat& tmp, double factor_inverse, double scale, FourierOperationType op);
void __dft_column(cv::Mat& src, cv::Mat& tmp, double factor_inverse, double scale, FourierOperationType op);

}

namespace detail {

//brief:
void dft(cv::Mat& src, cv::Mat& dst, int flags) {
	//assert(src.depth() == CV_64F || src.depth() == CV_32F);//只接受两种深度
	assert(src.depth() == CV_64F);//只接受1种深度，TODO：扩展
	assert(src.channels() <= 2);//要么是双通道复数，要么是单通道实数

	double factor_inverse = -1.0;//默认进行fourier forward变换
	int dst_channels = src.channels();//默认输出结果与src一致
	double scale = 1.;//默认不进行归一化处理
	cv::Size size = src.size();//默认输出尺寸不变
	FourierOperationType op;//根据flag确定本次调用功能

	if (flags & detail::DFT_INVERSE) {
		factor_inverse = 1.0;
	}
	if (flags & detail::DFT_REAL_OUTPUT) {
		dst_channels = 1;
	}
	if (flags & detail::DFT_COMPLEX_OUTPUT) {
		dst_channels = 2;
	}
	
#ifdef USE_ORIGINAL_IMPLEMENT
	//原始实现，复杂度为O(MNMN)
	if (dst_channels == 2) {
		if (flags & detail::DFT_INVERSE) 
			op = __complexFourier_complex;
		else
			op = __realFourier_complex;
	}
	else {
		if (flags & detail::DFT_INVERSE) 
			op = __complexFourier_real;
		else
			op = __realFourier_real;
	}
	
	dst.create(size, CV_MAKETYPE(CV_64F, dst_channels));
	double* iter = tmp.ptr<double>(0, 0);
	int jump = src.channels();//标志下一个元素的位置

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
					op(iter, cursor, theta, jump);
					//for test
					//digital::__printInfo("theta:%f\r\n", theta);
				}
			}

			*iter++ *= scale;
			if (dst_channels == 2)
				*iter++ *= scale;
		}
	}

#else
	//通过2D分离成1D实现，复杂度为O(MN(M+N))
	if (flags & detail::DFT_INVERSE) {
		assert(src.channels() == 2);
		op = __complexFourier_complex;
	}
	else {
		assert(src.channels() == 1);
		op = __realFourier_complex;
	}

	//进行行变换
	cv::Mat tmp(size, CV_MAKETYPE(CV_64F, COMPLEX_CHANNEL_NUMBER));
	if (flags & detail::DFT_SCALE) {
		scale = 1. / static_cast<double>(size.width);
	}

	__dft_row(src, tmp, factor_inverse, scale, op);

	//brief:只需进行行变换
	if (flags & DFT_ROWS) {
		if (dst_channels == 2) {
			dst = tmp;
			return;
		}
		std::vector<cv::Mat> vec;
		cv::split(tmp, vec);
		dst = vec[0];
		return;
	}

	//进行列变换
	dst.create(size, CV_MAKETYPE(CV_64F, dst_channels));
	//becare：经过拆分后,如果还要进行列变换，则必然一定是在complex的中间结果tmp基础之上完成的
	if (dst_channels == 2)
		op = __complexFourier_complex;
	else
		op = __complexFourier_real;
	if (flags & detail::DFT_SCALE) {
		scale = 1. / static_cast<double>(size.height);
	}

	__dft_column(tmp, dst, factor_inverse, scale, op);

#endif

}

//brief:逆变换的简化实现，转调dft
//becare:默认flag中没有DFT_SCALE,这一点是和opencv一致的，即1/MN的伸缩必须由用户显示调用
void idft(cv::Mat& src, cv::Mat& dst, int flags) {
	detail::dft(src, dst, flags | detail::DFT_INVERSE | detail::DFT_REAL_OUTPUT);
}

//brief:从傅里叶变换结果中得到幅度谱（傅里叶谱）
void getAmplitudeSpectrum(cv::Mat& src, cv::Mat& dst) {
	assert(src.channels() == COMPLEX_CHANNEL_NUMBER);

	std::vector<cv::Mat> channels;
	cv::split(src, channels);

	cv::magnitude(channels[0], channels[1], dst);
	//or equal to
	//cv::Mat tmp1, tmp2;
	//cv::pow(channels[0], 2, tmp1);
	//cv::pow(channels[0], 2, tmp2);
	//cv::sqrt(tmp1 + tmp2, dst);
}

//brief:获得相角谱
void getPhaseSpectrum(cv::Mat& src, cv::Mat& dst) {
	assert(src.channels() == COMPLEX_CHANNEL_NUMBER);

	dst.create(src.size(), CV_64FC1);
	assert(dst.isContinuous());
	double* iter = dst.ptr<double>(0, 0);

	auto get_phase = [&iter](uint8_t* cursor) {
		double* arr = reinterpret_cast<double*>(cursor);
		double tmp = std::atan2(arr[1], arr[0]);

		*iter = tmp < 0 ? 2 * kPI + tmp : tmp;//纯粹是为了和opencv的计算一致，小于0的角取另半边

		//for test
		//digital::__printInfo("%f,%f,%f\r\n", *iter, arr[0], arr[1]);

		++iter;
	};

	//转调grayscale_transfrom模块
	detail::grayscaleTransform(src, get_phase, true);
}

//brief:得到经过对数变换的、归一化后在统一拉伸到8-bit空间的图像，方便观察显示
cv::Mat grayscaleAmplitudeSpctrum(cv::Mat& spectrum) {
	cv::Mat dst;
	cv::log(spectrum + 1, dst);
	cv::normalize(dst, dst, 0, 255, cv::NORM_MINMAX, CV_8U);
	return dst;
}


}//!namespace detail


namespace {

//brief:傅里叶行变换,相关解释参见声明处
void __dft_row(cv::Mat& src, cv::Mat& dst, double factor_inverse, double scale, FourierOperationType op) {
	assert(dst.isContinuous());
	assert(src.isContinuous());

	cv::Size size = src.size();
	double* iter = dst.ptr<double>(0, 0);
	int jump = src.channels();//标志下一个元素的位置
	bool is_complex = dst.channels() == 2;//是否输出复数

	//针对单通道src,结果保存为双通道dst
	//最初的暴力解法，复杂度为O（M*N*M*N）
	for (int u = 0; u < size.height; ++u) {
		for (int v = 0; v < size.width; ++v) {
			assert(iter == dst.ptr<double>(u, v));
			*iter = 0.0;
			if(is_complex)
				*(iter + 1) = 0.0;

			//行方向
			double* cursor = src.ptr<double>(u, 0);
			for (int m = 0; m < size.width; ++m) {
				assert(cursor == src.ptr<double>(u, m));

				double theta = factor_inverse * 2 * kPI * v * m / size.width;
				op(iter, cursor, theta, jump);
			}

			*iter++ *= scale;
			if (is_complex)
				*iter++ *= scale;
		}
	}
}

//brief:进行列变换
void __dft_column(cv::Mat& src, cv::Mat& dst, double factor_inverse, double scale, FourierOperationType op) {
	assert(dst.isContinuous());
	assert(src.isContinuous());

	cv::Size size = src.size();
	double* iter = dst.ptr<double>(0, 0);
	int jump = src.cols * src.channels();
	bool is_complex = dst.channels() == 2;//是否输出复数

	for (int u = 0; u < size.height; ++u) {
		for (int v = 0; v < size.width; ++v) {
			assert(iter == dst.ptr<double>(u, v));
			*iter = 0.0;
			if(is_complex)
				*(iter + 1) = 0.0;

			//列方向
			double* cursor = src.ptr<double>(0, v);
			for (int n = 0; n < size.height; ++n) {
				assert(cursor == src.ptr<double>(n, v));

				double theta = factor_inverse * 2 * kPI * u * n / size.height;
				op(iter, cursor, theta, jump);
			}

			*iter++ *= scale;
			if (is_complex)
				*iter++ *= scale;
		}
	}

}


}//!namespace