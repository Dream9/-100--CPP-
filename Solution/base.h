//brief:提供有关图像滤波功能的封装，主要包含涉及到窗口操作的图像处理功能
//      包含线性/非线性滤波运算等基础功能

#ifndef _SOLUTION_BASE_H_
#define _SOLUTION_BASE_H_

#include<opencv2/highgui.hpp>

//#include<memory>
#include<vector>

namespace detail {

typedef std::function<void(uint8_t**, uint8_t*)> NonlinearOperationType;

//brief:卷积方式
enum ConvolutionType {
	FULL = 0,
	SAME = 1,
	VALID = 2,
};

//brief:根据窗口确定sigma大小
inline double getSigma(int size) {
	assert(size > 1);

	return 0.3 * ((size - 1) * 0.5 - 1) + 0.8;
}

//brief:根据sigma反推窗口大小
inline int getWinSize(double sigma) {
	assert(sigma > 0);

	return static_cast<int>((sigma * 3) * 0.5 + 1);
}

//brief:获得均值为1，方差为1的随机数
double randNorm();

//brief:计算阶乘
int factorial(int i);

//brief:获得窗口为win的一维二项式平滑算子列向量
cv::Mat getSmoothKernel(int win);

//brief:获得窗口为win的一维sobel差分算子列向量
cv::Mat getSobelDifference(int win);

//brief:卷积运算
//paramter: src:数据源
//          dst:存储位置
//          ddepth：目标像素深度
//          kernel:卷积核
//          p:锚点，参见opencv
//          bordertype:扩充边界类型,参见opencv
void convolution2D(const cv::Mat& src,
	cv::Mat& dst,
	int ddepth,
	const cv::Mat& kernel,
	cv::Point p = cv::Point(-1, -1),
	int bordertype = cv::BORDER_DEFAULT);

//brief:二维分离式卷积运算
//paramter: src:数据源
//          dst:存储位置
//          ddepth：目标像素深度
//          kernel:卷积核
//          p:锚点，参见opencv
//          bordertype:扩充边界类型,参见opencv
void sepConvolution2D(const cv::Mat& src,
	cv::Mat& dst,
	int ddepth,
	const cv::Mat& kernelx, 
	const cv::Mat& kernely, 
	cv::Point p = cv::Point(-1, -1),
	int bordertype = cv::BORDER_DEFAULT);

//brief:萃取型别,作用是根据depth确定数据型别
//becare;要求depth编译时期确定
//       opencv关于类型反射的实现，他是采用了运行时if-else跳转到对应template特化的函数处
//       不需要用户显示指明类型，但要为所有可能的调用生成代码
//       比如如下的源码：
//       if (ddepth == CV_16S && sdepth == CV_32F)
//       return makePtr<ColumnFilter<Cast<float, short>, ColumnNoVec> >(kernel, anchor, delta);
//       if (ddepth == CV_16S && sdepth == CV_64F)
//       return makePtr<ColumnFilter<Cast<double, short>, ColumnNoVec> >(kernel, anchor, delta);
template<int depth>
class GetTypeFormDepth {
public:
	typedef int ValueType;
};
//参照
//#define CV_8U   0
//#define CV_8S   1
//#define CV_16U  2
//#define CV_16S  3
//#define CV_32S  4
//#define CV_32F  5
//#define CV_64F  6
//#define CV_USRTYPE1 7
//进行特例化
template<>
class GetTypeFormDepth<0> {
public:
	typedef uint8_t ValueType;
};

template<>
class GetTypeFormDepth<1> {
public:
	typedef int8_t ValueType;
};

template<>
class GetTypeFormDepth<2> {
public:
	typedef uint16_t ValueType;
};

template<>
class GetTypeFormDepth<3> {
public:
	typedef int16_t ValueType;
};

template<>
class GetTypeFormDepth<4> {
public:
	typedef int32_t ValueType;
};

template<>
class GetTypeFormDepth<5> {
public:
	typedef float ValueType;
};

template<>
class GetTypeFormDepth<6> {
public:
	typedef double ValueType;
};

//brief:统一的删除器,为了应对c++17之前的shared_ptr连个delete[]都需要显示传入的缺陷
template<typename Ty>
void deleter(Ty* arr) {
	delete[] arr;
}

//brief:翻转滤波器
//parameter:arr起始地址，N2滤波器全部元素大小
//becare:仅针对具有连续内存的容器
template<typename Ty>
void flipFilter(Ty* arr, int N2) {
	Ty* left = arr;
	Ty* right = arr + N2 - 1;

	while (left < right)
		std::swap(*left++, *right--);
}

//brief:二维矩阵卷积
//parameter: src：原始Mat 
//         dst:目标Mat 
//         ddepth:目标像素深度
//	       arr_filter:滤波器参数，二维的滤波器需要转换成具有连续内存分布的一维形式
//         win_c/win_r:窗口大小，
//         need_normalize是否需要归一化滤波器参数
//         need_flip:是否需要翻转filter
//         Ky:匿名变量，仅用来表征原始数据类型
//becare:期待Ty类型的数据不会在计算过程中溢出，例如即Mat为uchar(Ky)的，使用uint16_t 或者更高的Ty
//       本函数提供了统一卷积计算，因此没有针对filter数据特点进行任何优化，如需优化，需自行另外实现
//       本函数允许最大通道数量为4(rgba)
//TODO:当滤波核很大时采用DFT计算，参考opencv源码
template<int sdepth, int ddepth, typename Ty = int>
void filter2D(cv::Mat& data, 
	cv::Mat& img, 
	Ty* arr_filter, //FIXME:应该提供cv::Mat的扩展
	int win_c, 
	int win_r, 
	bool need_normalize = true, 
	bool need_flip = true)
{
	//FIXME:通过实现反向的DataType来取消Ky的显示说明
	typedef GetTypeFormDepth<ddepth>::ValueType Dy;
	typedef GetTypeFormDepth<sdepth>::ValueType Sy;

	Ty* filter = arr_filter;
	img.create(data.size(), CV_MAKETYPE(ddepth, data.channels()));

	//assert(data.type() == img.type());
	assert(data.size() == img.size());

	cv::Size old_size = data.size();
	const int offset_c = win_c >> 1;
	const int offset_r = win_r >> 1;
	const int kWinSize = win_c * win_r;
	const int kChannels = static_cast<int>(data.channels());
	const int kElemSize = static_cast<int>(data.elemSize());
	const int kImgElemSize = static_cast<int>(img.elemSize());
	const int kStep = static_cast<int>(data.step[0]);
	Ty rgba[4];

	assert(4 >= kChannels);

	//扩展边界
	cv::copyMakeBorder(data, data,offset_r, offset_r, offset_c, offset_c, cv::BORDER_DEFAULT);

	//确定归一化系数
	Ty tmp_f = 0;
	if (need_normalize) {
		for (int i = 0; i < kWinSize; ++i)
			tmp_f += filter[i];
	}
	else
		tmp_f = Ty(1);
	const double kFactor = 1.0 / tmp_f;

	if (need_flip)
		flipFilter(filter, kWinSize);

	//卷积运算
	auto cur = img.data;
	cv::Size size = data.size();
	for (int i = offset_r; i < size.height - offset_r; ++i) {
		auto cursor = data.ptr(i - offset_r, 0);
		for (int j = offset_c; j < size.width - offset_c; ++j) {

			//单个窗口计算
			memset(rgba, 0, sizeof rgba);

			auto iter = static_cast<Sy*>(cursor);
			int n = 0;
			for (int x = 0; x < win_r; ++x) {
				auto tmp = iter;
				for (int y = 0; y < win_c; ++y) {
					for (int cn = 0; cn < kChannels; ++cn) {
						//digital::__printInfo("%u,%d,ans=%d   ", tmp[cn], filter[n],tmp[cn]*filter[n]);
						rgba[cn] += cv::saturate_cast<Ty>(tmp[cn] * filter[n]);
					}

					tmp += kElemSize;
					++n;
				}
				iter += kStep;
			}

			Dy* cur_tmp = static_cast<Dy*>(static_cast<void*>(cur));

			//for test
			assert(cur == img.ptr<uchar>(i - offset_r, j - offset_c));

			for (int cn = 0; cn < kChannels; ++cn) {
				//FIXME:防止数据溢出
				//cur_tmp[cn] = static_cast<Ky>(rgba[cn] * kFactor);
				cur_tmp[cn] = cv::saturate_cast<Dy>(rgba[cn] * kFactor);
			}
			cur += kImgElemSize;
			cursor += kElemSize;
		}
	}

	//复原大小
	size = img.size();
	data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

//FIXME：提供更标准的接口形式
//brief:提供基于非线性操作的滤波接口，为每个窗口调用用户指定例程
//parameter:src:原始数据
//          dst:目标存储
//          ops:用户指定操作
//          win_c/win_r:列/行
//          Ky*:匿名对象，仅用于表明src数据类型
//becare:Op应该重载了operator()(Ky[] src,Ky* dst),其中src为窗口的每行起始地址（行数用win_r决定），dst为结果地址
template<typename Op, typename Ky>
void filter2DNonLinear(cv::Mat& src, 
	cv::Mat& img, 
	Op ops,
	int win_c, 
	int win_r, 
	Ky* =nullptr)
{
	img = cv::Mat(src.size(), src.type());

	assert(src.type() == img.type());
	assert(src.size() == img.size());

	cv::Size old_size = src.size();
	const int offset_c = win_c >> 1;
	const int offset_r = win_r >> 1;
	const int kElemSize = static_cast<int>(src.elemSize());
	const int kStep = static_cast<int>(src.step[0]);

	//FIXME:或者使用 vector ??
	//std::shared_ptr<Ky*> arr_dst(new Ky*[win_r] ,deleter<Ky>);//c++17之前的shared_ptr对于二维动态数组的扩展太差！差评
	std::vector<Ky*> arr_dst(win_r, nullptr);

	//扩展边界
	cv::Mat data;//becare:因为不是push_back,所以这里大部分情况下是重新分配内存
	cv::copyMakeBorder(src, data,offset_r, offset_r, offset_c, offset_c, cv::BORDER_DEFAULT);

	//滤波运算
	auto cur = img.data;
	cv::Size size = data.size();
	for (int i = offset_r; i < size.height - offset_r; ++i) {
		auto cursor = data.ptr(i - offset_r, 0);
		for (int j = offset_c; j < size.width - offset_c; ++j) {

			//单个窗口计算
			auto iter = static_cast<Ky*>(cursor);
			int n = 0;
			for (int x = 0; x < win_r; ++x) {
				//arr_dst.get()[x] = iter;
				arr_dst[x] = iter;
				iter += kStep;
			}

			//调用用户例程
			//ops(arr_dst.get(), static_cast<Ky*>(cur));
			ops(&arr_dst[0], static_cast<Ky*>(cur));

			cur += kElemSize;
			cursor += kElemSize;
		}
	}

	//复原大小
	//size = img.size();
	//data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

//brief:更标准的接口形式
//     参数参见cv::filter2D
//     其中convolution_type:卷积计算方式，默认采用SAME卷积，也就是最普遍的实现，在一些运算中(比如模式匹配)，
//     采用了VALID卷积，因为边界不需要计算
//becare:op为用户提提供的指定操作，通常完成非线性的变换
void filter2DNonLinear(cv::Mat& src,
	cv::Mat& dst,
	cv::Mat& kernel,
	int ddepth,
	NonlinearOperationType op,
	cv::Point anchor = cv::Point(-1, -1),
	//double delta = 0,
	int borderType = cv::BORDER_DEFAULT,
	int convolution_type = detail::SAME);

//brief:soble算子的封装
//parameter: src:目标图像
//           dst:存储目标
//           dx:x方向差分
//           dy:y方向差分
//           win:窗口大小
void Sobel(cv::Mat& src, 
	cv::Mat& dst, 
	int ddepth, 
	int dx, 
	int dy = 0,
	int win = 3);

//brief:Canny算子
//parameter：src:目标图像
//           dst:存储位置
//           threshold_1:低阈值
//           threshold_2:高阈值
//           apertureSize:内嵌Sobel的窗口大小
//           useL2gradient:默认为false,梯度通过L1范式计算，快且为线性算子；为true时，采用L2范式计算，更精确，但非线性算子
void Canny(cv::Mat& src,
	cv::Mat& dst,
	double threshold_1,
	double threshold_2,
	int apertureSize = 3,
	bool useL2gradient = false);

}//!namespace detail


#endif // !_SOLUTION_BASE_H_
