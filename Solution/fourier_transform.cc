#include"Solution/fourier_transform.h"
#include"Solution/type_extension.h"
#include"Solution/grayscale_transfrom.h"
#include"Solution/geometric_transform.h"

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<unordered_map>

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

//brief:�Թ���ģʽ�滻if-else
class FDF_factory;
std::unordered_map<int, FDF_factory*> dict;

class FDF_factory{
public:
	static FDF_factory* create(int type) {
		auto iter = dict.find(type);
		if (iter == dict.end())
			return nullptr;
		return iter->second;
	}
	virtual void operator()(cv::Mat&, cv::Mat&, void*) = 0;
};

class Ilpf:public FDF_factory{
public:
	void operator()(cv::Mat&, cv::Mat&, void*) override;
};

class Blpf:public FDF_factory{
public:
	void operator()(cv::Mat&, cv::Mat&, void*) override;
};

class Glpf:public FDF_factory{
public:
	void operator()(cv::Mat&, cv::Mat&, void*) override;
};

class Ihpf:public FDF_factory{
public:
	void operator()(cv::Mat&, cv::Mat&, void*) override;
};

class Bhpf:public FDF_factory{
public:
	void operator()(cv::Mat&, cv::Mat&, void*) override;
};

class Ghpf:public FDF_factory{
public:
	void operator()(cv::Mat&, cv::Mat&, void*) override;
};

class Bbpf :public FDF_factory {
public:
	void operator()(cv::Mat&, cv::Mat&, void*)override;
};

class Bbrf :public FDF_factory {
public:
	void operator()(cv::Mat&, cv::Mat&, void*)override;
};

class Notch :public FDF_factory {
public:
	void operator()(cv::Mat&, cv::Mat&, void*)override;
	static void __on_mouse(int event, int x, int y, int, void*);
};

//brief:���ڳ�ʼ��״̬
class Init {
public:
	Init() {
		dict.insert({ detail::ILPF,new Ilpf});
		dict.insert({ detail::BLPF,new Blpf});
		dict.insert({ detail::GLPF,new Glpf});
		
		dict.insert({ detail::IHPF,new Ihpf});
		dict.insert({ detail::BHPF,new Bhpf});
		dict.insert({ detail::GHPF,new Ghpf});


		dict.insert({ detail::BP,new Bbpf});
		dict.insert({ detail::BR,new Bbrf});

		dict.insert({ detail::NOTCH, new Notch });

	}
};

static Init _init_object;

//brief:���������й���Rect
inline cv::Rect __getRectFromPoint(cv::Point p1, cv::Point p2) {
	int minx = MIN(p1.x,p2.x);
	int width = abs(p1.x - p2.x);
	
	int miny = MIN(p1.y,p2.y);
	int height = abs(p1.y - p2.y);

	return cv::Rect(minx, miny, width, height);
}

//brief:�õ��������ĵĵ�ԳƵ���һ��Rect�����ڱ�֤�ݲ��˲�����ԭ��Գ���
inline cv::Rect __getSymmetryRect(cv::Rect rect, cv::Point center) {
	return cv::Rect(2 * center.x - rect.x - rect.width, 2 * center.y - rect.y - rect.height, rect.width, rect.height);
}


}//!namespace

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

//brief:FFT
void fft(cv::Mat& src, cv::Mat& dst, int flags) {
	cv::Size size = src.size();
	size.width = cv::getOptimalDFTSize(size.width);
	size.height = cv::getOptimalDFTSize(size.height);

	//��䲹��
	cv::Mat tmp;
	cv::copyMakeBorder(src, tmp, 0, size.height - src.rows, 0, size.width - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	cv::dft(tmp, dst, flags, src.rows);//��ʡ����

	//��任����Ҫ�û�����ȥ�����㲿�֣�����
	//dst = dst(cv::Rect(0, 0, src.cols, src. rows));
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

//brief:��Ƶ
//becare:Ҫ�����δCV_64F
cv::Mat centralize(cv::Mat& src) {
	assert(src.depth() == CV_64FC1);
	assert(src.isContinuous());

	cv::Mat dst(src.size(), src.type());
	double* iter = dst.ptr<double>(0, 0);

	auto make_center = [&](int x, int y, uint8_t* cursor) {
		double cur = *reinterpret_cast<double*>(cursor);
		*iter = cur * (((x + y) & 0x1) ? -1. : 1.);//�ȼ���pow(-1.,(x+y))
		++iter;
	};

	detail::geometricTriversal(src, make_center);

	return dst;
}

//brief:Ƶ���򳣼��˲�
void frequencyDomainFilter(cv::Mat& src, cv::Mat& dst, int flags, void* data) {
	FDF_factory* lp = FDF_factory::create(flags);
	if (lp == nullptr) {
		dealException(digital::kParameterNotMatch);
		return;
	}

	lp->operator()(src, dst, data);
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


#define CENTER_DISTANCE(x,x1,y,y1) std::sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1))

//brief�������ͨ�˲����������Ե���������
void Ilpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double threshold = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ����һ��
	auto ilpf = [&ptr, threshold, half_x, half_y](int x, int y, uint8_t* cursor) {
		double* iter = reinterpret_cast<double*>(cursor);
		double D = CENTER_DISTANCE(x, half_x, y, half_y);
		if (D <= threshold) {
			*ptr = *iter;
			*(ptr + 1) = *(iter + 1);
			ptr += 2;
			return;
		}

		*ptr = 0.;
		*(ptr + 1) = 0.;
		ptr += 2;
	};

	detail::geometricTriversal(src, ilpf);
}

//brief��������˹��ͨ�˲����Ƽ�n=2��n->������˻�Ϊ�����ͨ
void Blpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double* arr = reinterpret_cast<double*>(data);
	double d = arr[0];
	double n = arr[1];
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ������һ��
	auto blpf = [&ptr, half_x, half_y, d, n](int x, int y, uint8_t* cursor) {
		double D = CENTER_DISTANCE(x, half_x, y, half_y);

		double h = 1 / (1 + std::pow(( D / d), n));
		double* iter = reinterpret_cast<double*>(cursor);

		*ptr = *iter * h;
		*(ptr + 1) =*(iter+1) * h;
		ptr += 2;
	};

	detail::geometricTriversal(src, blpf);
}

//brief����˹��ͨ�˲�����sigma����Ƶ���½���0.607����������������
void Glpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double sigma = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ������һ��
	auto blpf = [&ptr, half_x, half_y, sigma](int x, int y, uint8_t* cursor) {
		double D = CENTER_DISTANCE(x, half_x, y, half_y);

		double h = std::exp(-D*D*0.5/sigma/sigma);
		double* iter = reinterpret_cast<double*>(cursor);

		*ptr = *iter * h;
		*(ptr + 1) =*(iter+1) * h;
		ptr += 2;
	};

	detail::geometricTriversal(src, blpf);
}

//brief�������ͨ�˲����������Ե���������
void Ihpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double threshold = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ����һ��
	auto ilpf = [&ptr, threshold, half_x, half_y](int x, int y, uint8_t* cursor) {
		double* iter = reinterpret_cast<double*>(cursor);
		double D = CENTER_DISTANCE(x, half_x, y, half_y);
		if (D > threshold) {
			*ptr = *iter;
			*(ptr + 1) = *(iter + 1);
			ptr += 2;
			return;
		}

		*ptr = 0.;
		*(ptr + 1) = 0.;
		ptr += 2;
	};

	detail::geometricTriversal(src, ilpf);
}

//brief��������˹��ͨ�˲����Ƽ�n=2��n->������˻�Ϊ�����ͨ
void Bhpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double* arr = reinterpret_cast<double*>(data);
	double d = arr[0];
	double n = arr[1];
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ������һ��
	auto blpf = [&ptr, half_x, half_y, d, n](int x, int y, uint8_t* cursor) {
		double D = CENTER_DISTANCE(x, half_x, y, half_y);

		double h = 1 / (1 + std::pow(( d / D), n));
		double* iter = reinterpret_cast<double*>(cursor);

		*ptr = *iter * h;
		*(ptr + 1) =*(iter+1) * h;
		ptr += 2;
	};

	detail::geometricTriversal(src, blpf);
}

//brief����˹��ͨ�˲�����sigma����Ƶ���½���0.607����������������
void Ghpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double sigma = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ������һ��
	auto blpf = [&ptr, half_x, half_y, sigma](int x, int y, uint8_t* cursor) {
		double D = CENTER_DISTANCE(x, half_x, y, half_y);

		double h = 1 - std::exp(-D*D*0.5/sigma/sigma);
		double* iter = reinterpret_cast<double*>(cursor);

		*ptr = *iter * h;
		*(ptr + 1) =*(iter+1) * h;
		ptr += 2;
	};

	detail::geometricTriversal(src, blpf);
}

//brief��������˹��ͨ�˲����Ƽ�n=2��n->������˻�Ϊ�����ͨ
void Bbpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double* arr = reinterpret_cast<double*>(data);
	double d = arr[0];
	double n = arr[1];
	double width = arr[2];

	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ������һ��
	auto blpf = [&ptr, half_x, half_y, d, n, width](int x, int y, uint8_t* cursor) {
		double D = CENTER_DISTANCE(x, half_x, y, half_y);

		double h = 1 - 1 / (1 + std::pow((width * D / (D*D - d*d) ), n));
		double* iter = reinterpret_cast<double*>(cursor);

		*ptr = *iter * h;
		*(ptr + 1) =*(iter+1) * h;
		ptr += 2;
	};

	detail::geometricTriversal(src, blpf);
}

//brief��������˹�����˲����Ƽ�n=2��n->������˻�Ϊ�����ͨ
void Bbrf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double* arr = reinterpret_cast<double*>(data);
	double d = arr[0];
	double n = arr[1];
	double width = arr[2];

	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:���ǵ����Ļ�������һ���߶ȶԳƵĽṹ��û�б�Ҫ��ȫ������һ��
	auto blpf = [&ptr, half_x, half_y, d, n, width](int x, int y, uint8_t* cursor) {
		double D = CENTER_DISTANCE(x, half_x, y, half_y);

		double h = 1 / (1 + std::pow((width * D / (D*D - d*d) ), n));
		double* iter = reinterpret_cast<double*>(cursor);

		*ptr = *iter * h;
		*(ptr + 1) =*(iter+1) * h;
		ptr += 2;
	};

	detail::geometricTriversal(src, blpf);
}

//brief:�û��Զ�����ݲ��˲���
//     ������±�ʾ�ݲ���һ�㣬���̧���ʾ�ݲ�����һ��
void Notch::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	bool need_symmetry = data == nullptr;//Ĭ���ǶԳƵ�
	cv::Point center(src.cols >> 1, src.rows >> 1);
	cv::Rect TheBiggestRect(0, 0, src.cols, src.rows);

	cv::Mat spectrum;
	detail::getAmplitudeSpectrum(src, spectrum);
	spectrum = detail::grayscaleAmplitudeSpctrum(spectrum);

	src.copyTo(dst);
	cv::Point pts[3];//������point��������Ϊһ��״̬��ʶ

	const string name = "select notch area, esc for accomplished";
	cv::namedWindow(name, CV_WINDOW_NORMAL);
	cv::setMouseCallback(name, Notch::__on_mouse, pts);

	//esc ����Զ���
	while (true) {
		if (pts[2].x == 2) {
			cv::Rect rect = __getRectFromPoint(pts[0], pts[1]) & TheBiggestRect;
			dst(rect).setTo(cv::Scalar::all(0));
			spectrum(rect).setTo(cv::Scalar::all(0));
			if (need_symmetry) {
				cv::Rect symmetry_rect = __getSymmetryRect(rect, center) & TheBiggestRect; //becare:��ֹ���
				dst(symmetry_rect).setTo(cv::Scalar::all(0));
				spectrum(symmetry_rect).setTo(cv::Scalar::all(0));
			}
			pts[2].x = 0;//��ԭ״̬
		}

		//memset(pts, 0, sizeof pts);

		cv::imshow(name, spectrum);
		if (cv::waitKey(10) == 27)
			break;
	}
}

void Notch::__on_mouse(int event, int x, int y, int, void* userdata) {
	cv::Point* point = static_cast<cv::Point*>(userdata);
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		point[0].x = x;
		point[0].y = y;
		point[2].x = 1;//���ڹ����˲���
		break;
	case CV_EVENT_LBUTTONUP:
		point[1].x = x;
		point[1].y = y;
		point[2].x = 2;//��ɹ����˲���
		break;
	default:
		break;
	}
}

#undef CENTER_DISTANCE

}//!namespace