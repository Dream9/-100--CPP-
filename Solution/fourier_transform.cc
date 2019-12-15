#include"Solution/fourier_transform.h"
#include"Solution/type_extension.h"
#include"Solution/grayscale_transfrom.h"
#include"Solution/geometric_transform.h"

#include<opencv2/imgproc.hpp>

//brief:���º������û�����
namespace {

typedef void(*FourierOperationType)(double*, double*&, double, int);

const double kPI = 3.14159265354;
const int COMPLEX_CHANNEL_NUMBER = 2;

//brief:���ʵ/������Ĳ�ͬ�������
//parameter:iter:�������ǰ����handler
//          cursor:ԭʼ����ǰ����handler
//          theta:����ŷ����ʽ����ɵĽǶ�ֵ
//          step:ָ��ƫ��������Ҫ�������м���ļ���
//becare:��ʱ����cursor��Ӧreal����
void __realFourier_real(double* iter, double*& cursor, double theta, int step) {
	*iter += *cursor * std::cos(theta);//���iter��Ӧreal
	cursor += step;
}
void __realFourier_complex(double* iter, double*& cursor, double theta,int step) {
	*iter += *cursor * std::cos(theta);//���iter��Ӧcomplex
	*(iter+1) += *cursor * std::sin(theta);		
	cursor += step;
}

//becare:��ʱ����cursor��Ӧcomplex����
void __complexFourier_complex(double* iter, double*& cursor, double theta, int step) {
	*iter += std::cos(theta)*(*cursor) - std::sin(theta)*(*(cursor + 1));//���iter��Ӧcomplex
	*(iter + 1) += std::cos(theta)*(*(cursor + 1)) + std::sin(theta)*(*cursor);
	cursor += step;
}
void __complexFourier_real(double* iter, double*& cursor, double theta, int step) {
	*iter += std::cos(theta)*(*cursor) - std::sin(theta)*(*(cursor + 1));//���iter��Ӧreal
	cursor += step;
}

//becare:Fourier 1D�У��У��任
//parameter:src:����Դ
//          tmp:�洢Ŀ�꣬Ҫ���Ѿ�����������ڴ棬��������ֻ����ִ�м��㣬�������ϲ㹦�ַܷ�
//          factor_inverse:������任��ʶ
//          scale:��������
//          op:ִ�еĲ�����ֻ������������
void __dft_row(cv::Mat& src, cv::Mat& tmp, double factor_inverse, double scale, FourierOperationType op);
void __dft_column(cv::Mat& src, cv::Mat& tmp, double factor_inverse, double scale, FourierOperationType op);

}

namespace detail {

//brief:
void dft(cv::Mat& src, cv::Mat& dst, int flags) {
	//assert(src.depth() == CV_64F || src.depth() == CV_32F);//ֻ�����������
	assert(src.depth() == CV_64F);//ֻ����1����ȣ�TODO����չ
	assert(src.channels() <= 2);//Ҫô��˫ͨ��������Ҫô�ǵ�ͨ��ʵ��

	double factor_inverse = -1.0;//Ĭ�Ͻ���fourier forward�任
	int dst_channels = src.channels();//Ĭ����������srcһ��
	double scale = 1.;//Ĭ�ϲ����й�һ������
	cv::Size size = src.size();//Ĭ������ߴ粻��
	FourierOperationType op;//����flagȷ�����ε��ù���

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
	//ԭʼʵ�֣����Ӷ�ΪO(MNMN)
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
	int jump = src.channels();//��־��һ��Ԫ�ص�λ��

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
	//ͨ��2D�����1Dʵ�֣����Ӷ�ΪO(MN(M+N))
	if (flags & detail::DFT_INVERSE) {
		assert(src.channels() == 2);
		op = __complexFourier_complex;
	}
	else {
		assert(src.channels() == 1);
		op = __realFourier_complex;
	}

	//�����б任
	cv::Mat tmp(size, CV_MAKETYPE(CV_64F, COMPLEX_CHANNEL_NUMBER));
	if (flags & detail::DFT_SCALE) {
		scale = 1. / static_cast<double>(size.width);
	}

	__dft_row(src, tmp, factor_inverse, scale, op);

	//brief:ֻ������б任
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

	//�����б任
	dst.create(size, CV_MAKETYPE(CV_64F, dst_channels));
	//becare��������ֺ�,�����Ҫ�����б任�����Ȼһ������complex���м���tmp����֮����ɵ�
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

//brief:��任�ļ�ʵ�֣�ת��dft
//becare:Ĭ��flag��û��DFT_SCALE,��һ���Ǻ�opencvһ�µģ���1/MN�������������û���ʾ����
void idft(cv::Mat& src, cv::Mat& dst, int flags) {
	detail::dft(src, dst, flags | detail::DFT_INVERSE | detail::DFT_REAL_OUTPUT);
}

//brief:�Ӹ���Ҷ�任����еõ������ף�����Ҷ�ף�
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

//brief:��������
void getPhaseSpectrum(cv::Mat& src, cv::Mat& dst) {
	assert(src.channels() == COMPLEX_CHANNEL_NUMBER);

	dst.create(src.size(), CV_64FC1);
	assert(dst.isContinuous());
	double* iter = dst.ptr<double>(0, 0);

	auto get_phase = [&iter](uint8_t* cursor) {
		double* arr = reinterpret_cast<double*>(cursor);
		double tmp = std::atan2(arr[1], arr[0]);

		*iter = tmp < 0 ? 2 * kPI + tmp : tmp;//������Ϊ�˺�opencv�ļ���һ�£�С��0�Ľ�ȡ����

		//for test
		//digital::__printInfo("%f,%f,%f\r\n", *iter, arr[0], arr[1]);

		++iter;
	};

	//ת��grayscale_transfromģ��
	detail::grayscaleTransform(src, get_phase, true);
}

//brief:�õ����������任�ġ���һ������ͳһ���쵽8-bit�ռ��ͼ�񣬷���۲���ʾ
cv::Mat grayscaleAmplitudeSpctrum(cv::Mat& spectrum) {
	cv::Mat dst;
	cv::log(spectrum + 1, dst);
	cv::normalize(dst, dst, 0, 255, cv::NORM_MINMAX, CV_8U);
	return dst;
}


}//!namespace detail


namespace {

//brief:����Ҷ�б任,��ؽ��Ͳμ�������
void __dft_row(cv::Mat& src, cv::Mat& dst, double factor_inverse, double scale, FourierOperationType op) {
	assert(dst.isContinuous());
	assert(src.isContinuous());

	cv::Size size = src.size();
	double* iter = dst.ptr<double>(0, 0);
	int jump = src.channels();//��־��һ��Ԫ�ص�λ��
	bool is_complex = dst.channels() == 2;//�Ƿ��������

	//��Ե�ͨ��src,�������Ϊ˫ͨ��dst
	//����ı����ⷨ�����Ӷ�ΪO��M*N*M*N��
	for (int u = 0; u < size.height; ++u) {
		for (int v = 0; v < size.width; ++v) {
			assert(iter == dst.ptr<double>(u, v));
			*iter = 0.0;
			if(is_complex)
				*(iter + 1) = 0.0;

			//�з���
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

//brief:�����б任
void __dft_column(cv::Mat& src, cv::Mat& dst, double factor_inverse, double scale, FourierOperationType op) {
	assert(dst.isContinuous());
	assert(src.isContinuous());

	cv::Size size = src.size();
	double* iter = dst.ptr<double>(0, 0);
	int jump = src.cols * src.channels();
	bool is_complex = dst.channels() == 2;//�Ƿ��������

	for (int u = 0; u < size.height; ++u) {
		for (int v = 0; v < size.width; ++v) {
			assert(iter == dst.ptr<double>(u, v));
			*iter = 0.0;
			if(is_complex)
				*(iter + 1) = 0.0;

			//�з���
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