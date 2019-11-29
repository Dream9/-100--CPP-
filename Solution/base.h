//brief:��Ϊ�ڲ�ʹ�ã�����Mat�����������

#ifndef _SOLUTION_BASE_H_

#include<opencv2/highgui.hpp>

namespace detail {

//brief:��ת�˲���
template<typename Ty>
void flipFilter(Ty* arr, int N2) {
	Ty* left = arr;
	Ty* right = arr + N2 - 1;

	while (left < right)
		std::swap(*left++, *right--);
}

//brief:��ά������
//parameter: src��ԭʼMat��ַ 
//         dst:Ŀ��Mat��ַ 
//	       arr_filter:�˲���������ַ 
//         win_c/win_r:���ڴ�С��
//         need_normalize�Ƿ���Ҫ��һ���˲�������
//         need_flip:�Ƿ���Ҫ��תfilter
//         Ky:��������������������ԭʼ��������
//becare:�ڴ�Ty���͵����ݲ����ڼ����������������缴MatΪuchar(Ky)�ģ�ʹ��uint16_t ���߸��ߵ�Ty
//       �������ṩ��ͳһ������㣬���û�����filter�����ص�����κ��Ż��������Ż�������������ʵ��
//       �������������ͨ������Ϊ4(rgba)
//TODO:���˲��˺ܴ�ʱ����DFT����
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

	//
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

	//��ԭ��С
	size = img.size();
	data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

}//!namespace detail


#endif // !_SOLUTION_BASE_H_
