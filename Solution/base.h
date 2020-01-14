//brief:�ṩ�й�ͼ���˲����ܵķ�װ����Ҫ�����漰�����ڲ�����ͼ������
//      ��������/�������˲�����Ȼ�������

#ifndef _SOLUTION_BASE_H_
#define _SOLUTION_BASE_H_

#include<opencv2/highgui.hpp>

//#include<memory>
#include<vector>

namespace detail {

typedef std::function<void(uint8_t**, uint8_t*)> NonlinearOperationType;

//brief:�����ʽ
enum ConvolutionType {
	FULL = 0,
	SAME = 1,
	VALID = 2,
};

//brief:���ݴ���ȷ��sigma��С
inline double getSigma(int size) {
	assert(size > 1);

	return 0.3 * ((size - 1) * 0.5 - 1) + 0.8;
}

//brief:����sigma���ƴ��ڴ�С
inline int getWinSize(double sigma) {
	assert(sigma > 0);

	return static_cast<int>((sigma * 3) * 0.5 + 1);
}

//brief:��þ�ֵΪ1������Ϊ1�������
double randNorm();

//brief:����׳�
int factorial(int i);

//brief:��ô���Ϊwin��һά����ʽƽ������������
cv::Mat getSmoothKernel(int win);

//brief:��ô���Ϊwin��һάsobel�������������
cv::Mat getSobelDifference(int win);

//brief:�������
//paramter: src:����Դ
//          dst:�洢λ��
//          ddepth��Ŀ���������
//          kernel:�����
//          p:ê�㣬�μ�opencv
//          bordertype:����߽�����,�μ�opencv
void convolution2D(const cv::Mat& src,
	cv::Mat& dst,
	int ddepth,
	const cv::Mat& kernel,
	cv::Point p = cv::Point(-1, -1),
	int bordertype = cv::BORDER_DEFAULT);

//brief:��ά����ʽ�������
//paramter: src:����Դ
//          dst:�洢λ��
//          ddepth��Ŀ���������
//          kernel:�����
//          p:ê�㣬�μ�opencv
//          bordertype:����߽�����,�μ�opencv
void sepConvolution2D(const cv::Mat& src,
	cv::Mat& dst,
	int ddepth,
	const cv::Mat& kernelx, 
	const cv::Mat& kernely, 
	cv::Point p = cv::Point(-1, -1),
	int bordertype = cv::BORDER_DEFAULT);

//brief:��ȡ�ͱ�,�����Ǹ���depthȷ�������ͱ�
//becare;Ҫ��depth����ʱ��ȷ��
//       opencv�������ͷ����ʵ�֣����ǲ���������ʱif-else��ת����Ӧtemplate�ػ��ĺ�����
//       ����Ҫ�û���ʾָ�����ͣ���ҪΪ���п��ܵĵ������ɴ���
//       �������µ�Դ�룺
//       if (ddepth == CV_16S && sdepth == CV_32F)
//       return makePtr<ColumnFilter<Cast<float, short>, ColumnNoVec> >(kernel, anchor, delta);
//       if (ddepth == CV_16S && sdepth == CV_64F)
//       return makePtr<ColumnFilter<Cast<double, short>, ColumnNoVec> >(kernel, anchor, delta);
template<int depth>
class GetTypeFormDepth {
public:
	typedef int ValueType;
};
//����
//#define CV_8U   0
//#define CV_8S   1
//#define CV_16U  2
//#define CV_16S  3
//#define CV_32S  4
//#define CV_32F  5
//#define CV_64F  6
//#define CV_USRTYPE1 7
//����������
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

//brief:ͳһ��ɾ����,Ϊ��Ӧ��c++17֮ǰ��shared_ptr����delete[]����Ҫ��ʾ�����ȱ��
template<typename Ty>
void deleter(Ty* arr) {
	delete[] arr;
}

//brief:��ת�˲���
//parameter:arr��ʼ��ַ��N2�˲���ȫ��Ԫ�ش�С
//becare:����Ծ��������ڴ������
template<typename Ty>
void flipFilter(Ty* arr, int N2) {
	Ty* left = arr;
	Ty* right = arr + N2 - 1;

	while (left < right)
		std::swap(*left++, *right--);
}

//brief:��ά������
//parameter: src��ԭʼMat 
//         dst:Ŀ��Mat 
//         ddepth:Ŀ���������
//	       arr_filter:�˲�����������ά���˲�����Ҫת���ɾ��������ڴ�ֲ���һά��ʽ
//         win_c/win_r:���ڴ�С��
//         need_normalize�Ƿ���Ҫ��һ���˲�������
//         need_flip:�Ƿ���Ҫ��תfilter
//         Ky:��������������������ԭʼ��������
//becare:�ڴ�Ty���͵����ݲ����ڼ����������������缴MatΪuchar(Ky)�ģ�ʹ��uint16_t ���߸��ߵ�Ty
//       �������ṩ��ͳһ������㣬���û�����filter�����ص�����κ��Ż��������Ż�������������ʵ��
//       �������������ͨ������Ϊ4(rgba)
//TODO:���˲��˺ܴ�ʱ����DFT���㣬�ο�opencvԴ��
template<int sdepth, int ddepth, typename Ty = int>
void filter2D(cv::Mat& data, 
	cv::Mat& img, 
	Ty* arr_filter, //FIXME:Ӧ���ṩcv::Mat����չ
	int win_c, 
	int win_r, 
	bool need_normalize = true, 
	bool need_flip = true)
{
	//FIXME:ͨ��ʵ�ַ����DataType��ȡ��Ky����ʾ˵��
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

	//��չ�߽�
	cv::copyMakeBorder(data, data,offset_r, offset_r, offset_c, offset_c, cv::BORDER_DEFAULT);

	//ȷ����һ��ϵ��
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

	//�������
	auto cur = img.data;
	cv::Size size = data.size();
	for (int i = offset_r; i < size.height - offset_r; ++i) {
		auto cursor = data.ptr(i - offset_r, 0);
		for (int j = offset_c; j < size.width - offset_c; ++j) {

			//�������ڼ���
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
				//FIXME:��ֹ�������
				//cur_tmp[cn] = static_cast<Ky>(rgba[cn] * kFactor);
				cur_tmp[cn] = cv::saturate_cast<Dy>(rgba[cn] * kFactor);
			}
			cur += kImgElemSize;
			cursor += kElemSize;
		}
	}

	//��ԭ��С
	size = img.size();
	data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

//FIXME���ṩ����׼�Ľӿ���ʽ
//brief:�ṩ���ڷ����Բ������˲��ӿڣ�Ϊÿ�����ڵ����û�ָ������
//parameter:src:ԭʼ����
//          dst:Ŀ��洢
//          ops:�û�ָ������
//          win_c/win_r:��/��
//          Ky*:�������󣬽����ڱ���src��������
//becare:OpӦ��������operator()(Ky[] src,Ky* dst),����srcΪ���ڵ�ÿ����ʼ��ַ��������win_r��������dstΪ�����ַ
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

	//FIXME:����ʹ�� vector ??
	//std::shared_ptr<Ky*> arr_dst(new Ky*[win_r] ,deleter<Ky>);//c++17֮ǰ��shared_ptr���ڶ�ά��̬�������չ̫�����
	std::vector<Ky*> arr_dst(win_r, nullptr);

	//��չ�߽�
	cv::Mat data;//becare:��Ϊ����push_back,��������󲿷�����������·����ڴ�
	cv::copyMakeBorder(src, data,offset_r, offset_r, offset_c, offset_c, cv::BORDER_DEFAULT);

	//�˲�����
	auto cur = img.data;
	cv::Size size = data.size();
	for (int i = offset_r; i < size.height - offset_r; ++i) {
		auto cursor = data.ptr(i - offset_r, 0);
		for (int j = offset_c; j < size.width - offset_c; ++j) {

			//�������ڼ���
			auto iter = static_cast<Ky*>(cursor);
			int n = 0;
			for (int x = 0; x < win_r; ++x) {
				//arr_dst.get()[x] = iter;
				arr_dst[x] = iter;
				iter += kStep;
			}

			//�����û�����
			//ops(arr_dst.get(), static_cast<Ky*>(cur));
			ops(&arr_dst[0], static_cast<Ky*>(cur));

			cur += kElemSize;
			cursor += kElemSize;
		}
	}

	//��ԭ��С
	//size = img.size();
	//data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

//brief:����׼�Ľӿ���ʽ
//     �����μ�cv::filter2D
//     ����convolution_type:������㷽ʽ��Ĭ�ϲ���SAME�����Ҳ�������ձ��ʵ�֣���һЩ������(����ģʽƥ��)��
//     ������VALID�������Ϊ�߽粻��Ҫ����
//becare:opΪ�û����ṩ��ָ��������ͨ����ɷ����Եı任
void filter2DNonLinear(cv::Mat& src,
	cv::Mat& dst,
	cv::Mat& kernel,
	int ddepth,
	NonlinearOperationType op,
	cv::Point anchor = cv::Point(-1, -1),
	//double delta = 0,
	int borderType = cv::BORDER_DEFAULT,
	int convolution_type = detail::SAME);

//brief:soble���ӵķ�װ
//parameter: src:Ŀ��ͼ��
//           dst:�洢Ŀ��
//           dx:x������
//           dy:y������
//           win:���ڴ�С
void Sobel(cv::Mat& src, 
	cv::Mat& dst, 
	int ddepth, 
	int dx, 
	int dy = 0,
	int win = 3);

//brief:Canny����
//parameter��src:Ŀ��ͼ��
//           dst:�洢λ��
//           threshold_1:����ֵ
//           threshold_2:����ֵ
//           apertureSize:��ǶSobel�Ĵ��ڴ�С
//           useL2gradient:Ĭ��Ϊfalse,�ݶ�ͨ��L1��ʽ���㣬����Ϊ�������ӣ�Ϊtrueʱ������L2��ʽ���㣬����ȷ��������������
void Canny(cv::Mat& src,
	cv::Mat& dst,
	double threshold_1,
	double threshold_2,
	int apertureSize = 3,
	bool useL2gradient = false);

}//!namespace detail


#endif // !_SOLUTION_BASE_H_
