//brief:仅为内部使用，包含Mat运算基础功能

#ifndef _SOLUTION_BASE_H_

#include<opencv2/highgui.hpp>

namespace detail {

//brief:翻转滤波器
template<typename Ty>
void flipFilter(Ty* arr, int N2) {
	Ty* left = arr;
	Ty* right = arr + N2 - 1;

	while (left < right)
		std::swap(*left++, *right--);
}

//brief:二维矩阵卷积
//parameter: src：原始Mat地址 
//         dst:目标Mat地址 
//	       arr_filter:滤波器参数地址 
//         win_c/win_r:窗口大小，
//         need_normalize是否需要归一化滤波器参数
//         need_flip:是否需要翻转filter
//         Ky:匿名变量，仅用来表征原始数据类型
//becare:期待Ty类型的数据不会在计算过程中溢出，例如即Mat为uchar(Ky)的，使用uint16_t 或者更高的Ty
//       本函数提供了统一卷积计算，因此没有针对filter数据特点进行任何优化，如需优化，需自行另外实现
//       本函数允许最大通道数量为4(rgba)
//TODO:当滤波核很大时采用DFT计算
template<typename Ty, typename Ky>
void filter2D(void* src, void* dst, Ty* arr_filter, int win_c, int win_r,bool need_normalize = true, bool need_flip = true, Ky* =nullptr) {
	cv::Mat& data = *static_cast<cv::Mat*>(src);
	cv::Mat& img = *static_cast<cv::Mat*>(dst);
	Ty* filter = arr_filter;

	assert(data.type() == img.type());
	assert(data.size() == img.size());

	cv::Size old_size = data.size();
	const int offset_c = win_c >> 1;
	const int offset_r = win_r >> 1;
	const int kWinSize = win_c * win_r;
	const int kChannels = static_cast<int>(data.channels());
	const int kElemSize = static_cast<int>(data.elemSize());
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

	//
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

			auto iter = static_cast<Ky*>(cursor);
			int n = 0;
			for (int x = 0; x < win_r; ++x) {
				auto tmp = iter;
				for (int y = 0; y < win_c; ++y) {
					for (int cn = 0; cn < kChannels; ++cn) {
						rgba[cn] += tmp[cn] * filter[n];
					}

					tmp += kElemSize;
					++n;
				}
				iter += kStep;
			}

			Ky* cur_tmp = static_cast<Ky*>(cur);
			for (int cn = 0; cn < kChannels; ++cn) {
				cur_tmp[cn] = static_cast<Ky>(rgba[cn] * kFactor);
			}
			cur += kElemSize;
			cursor += kElemSize;
		}
	}

	//复原大小
	size = img.size();
	data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

}//!namespace detail


#endif // !_SOLUTION_BASE_H_
