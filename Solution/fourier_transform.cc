#include"Solution/fourier_transform.h"
#include"Solution/type_extension.h"
#include"Solution/grayscale_transfrom.h"
#include"Solution/geometric_transform.h"

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<unordered_map>

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

//brief:以工厂模式替换if-else
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

//brief:用于初始化状态
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

//brief:从两个点中构建Rect
inline cv::Rect __getRectFromPoint(cv::Point p1, cv::Point p2) {
	int minx = MIN(p1.x,p2.x);
	int width = abs(p1.x - p2.x);
	
	int miny = MIN(p1.y,p2.y);
	int height = abs(p1.y - p2.y);

	return cv::Rect(minx, miny, width, height);
}

//brief:得到关于中心的点对称的另一个Rect，用于保证陷波滤波器的原点对称性
inline cv::Rect __getSymmetryRect(cv::Rect rect, cv::Point center) {
	return cv::Rect(2 * center.x - rect.x - rect.width, 2 * center.y - rect.y - rect.height, rect.width, rect.height);
}


}//!namespace

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

//brief:FFT
void fft(cv::Mat& src, cv::Mat& dst, int flags) {
	cv::Size size = src.size();
	size.width = cv::getOptimalDFTSize(size.width);
	size.height = cv::getOptimalDFTSize(size.height);

	//填充补零
	cv::Mat tmp;
	cv::copyMakeBorder(src, tmp, 0, size.height - src.rows, 0, size.width - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	cv::dft(tmp, dst, flags, src.rows);//节省计算

	//逆变换后需要用户自行去除补零部分！！！
	//dst = dst(cv::Rect(0, 0, src.cols, src. rows));
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

//brief:移频
//becare:要求必须未CV_64F
cv::Mat centralize(cv::Mat& src) {
	assert(src.depth() == CV_64FC1);
	assert(src.isContinuous());

	cv::Mat dst(src.size(), src.type());
	double* iter = dst.ptr<double>(0, 0);

	auto make_center = [&](int x, int y, uint8_t* cursor) {
		double cur = *reinterpret_cast<double*>(cursor);
		*iter = cur * (((x + y) & 0x1) ? -1. : 1.);//等价于pow(-1.,(x+y))
		++iter;
	};

	detail::geometricTriversal(src, make_center);

	return dst;
}

//brief:频率域常见滤波
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


#define CENTER_DISTANCE(x,x1,y,y1) std::sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1))

//brief：理想低通滤波，存在明显的振铃现象
void Ilpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double threshold = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全遍历一遍
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

//brief：巴特沃斯低通滤波，推荐n=2，n->无穷，则退化为理想低通
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

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全都遍历一遍
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

//brief：高斯低通滤波，在sigma处，频率下降到0.607，不存在振铃现象
void Glpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double sigma = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全都遍历一遍
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

//brief：理想高通滤波，存在明显的振铃现象
void Ihpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double threshold = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全遍历一遍
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

//brief：巴特沃斯高通滤波，推荐n=2，n->无穷，则退化为理想高通
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

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全都遍历一遍
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

//brief：高斯高通滤波，在sigma处，频率下降到0.607，不存在振铃现象
void Ghpf::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	dst.create(src.size(), src.type());
	double* ptr = dst.ptr<double>(0, 0);

	double sigma = *reinterpret_cast<double*>(data);
	double half_x = src.cols * 0.5;
	double half_y = src.rows * 0.5;

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全都遍历一遍
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

//brief：巴特沃斯带通滤波，推荐n=2，n->无穷，则退化为理想高通
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

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全都遍历一遍
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

//brief：巴特沃斯带阻滤波，推荐n=2，n->无穷，则退化为理想高通
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

	//FIXME:考虑到中心化后这是一个高度对称的结构，没有必要完全都遍历一遍
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

//brief:用户自定义的陷波滤波器
//     左键按下表示陷波的一点，左键抬起表示陷波的另一点
void Notch::operator()(cv::Mat& src, cv::Mat& dst, void* data) {
	assert(src.channels() == 2);
	assert(src.isContinuous());
	assert(src.depth() == CV_64F);

	bool need_symmetry = data == nullptr;//默认是对称的
	cv::Point center(src.cols >> 1, src.rows >> 1);
	cv::Rect TheBiggestRect(0, 0, src.cols, src.rows);

	cv::Mat spectrum;
	detail::getAmplitudeSpectrum(src, spectrum);
	spectrum = detail::grayscaleAmplitudeSpctrum(spectrum);

	src.copyTo(dst);
	cv::Point pts[3];//第三个point仅用来作为一个状态标识

	const string name = "select notch area, esc for accomplished";
	cv::namedWindow(name, CV_WINDOW_NORMAL);
	cv::setMouseCallback(name, Notch::__on_mouse, pts);

	//esc 完成自定义
	while (true) {
		if (pts[2].x == 2) {
			cv::Rect rect = __getRectFromPoint(pts[0], pts[1]) & TheBiggestRect;
			dst(rect).setTo(cv::Scalar::all(0));
			spectrum(rect).setTo(cv::Scalar::all(0));
			if (need_symmetry) {
				cv::Rect symmetry_rect = __getSymmetryRect(rect, center) & TheBiggestRect; //becare:防止溢出
				dst(symmetry_rect).setTo(cv::Scalar::all(0));
				spectrum(symmetry_rect).setTo(cv::Scalar::all(0));
			}
			pts[2].x = 0;//复原状态
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
		point[2].x = 1;//正在构建滤波器
		break;
	case CV_EVENT_LBUTTONUP:
		point[1].x = x;
		point[1].y = y;
		point[2].x = 2;//完成构建滤波器
		break;
	default:
		break;
	}
}

#undef CENTER_DISTANCE

}//!namespace