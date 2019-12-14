#include"Solution/fourier_transform.h"
#include"grayscale_transfrom.h"

#include"Solution/type_extension.h"

#include<opencv2/imgproc.hpp>

namespace {

const double kPI = 3.14159265354;

//becare:��ʱcursor��Ӧrealͼ��
void __forwardFourier_real(double* iter, double*& cursor, double theta) {
	*iter += *cursor * std::cos(theta);
	++cursor;
}
void __forwardFourier_complex(double* iter, double*& cursor, double theta) {
	*iter += *cursor * std::cos(theta);
	*(iter+1) += *cursor * std::sin(theta);		
	++cursor;
}

//becare:��ʱcursor��ӦcomplexƵ����
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
	//assert(src.depth() == CV_64F || src.depth() == CV_32F);//ֻ�����������
	assert(src.depth() == CV_64F);//ֻ����1�����

	double factor_inverse = -1.0;//Ĭ�Ͻ���fourier forward�任
	int dst_channels = src.channels();//Ĭ����������srcһ��
	double scale = 1.;//Ĭ�ϲ����й�һ������
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

	//����flagȷ�����ε��ù���
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

	//��Ե�ͨ��src,�������Ϊ˫ͨ��dst
	//����ı����ⷨ�����Ӷ�ΪO��M*N*M*N��
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

//brief:�Ӹ���Ҷ�任����еõ������ף�����Ҷ�ף�
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