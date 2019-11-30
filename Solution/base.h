//brief:��Ϊ�ڲ�ʹ�ã��û���Ӧֱ�ӵ��ñ��ļ�����������/�������˲�����Ȼ�������

#ifndef _SOLUTION_BASE_H_

#include<opencv2/highgui.hpp>

//#include<memory>
#include<vector>

//brief:��������
namespace detail {

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
//	       arr_filter:�˲�����������ά���˲�����Ҫת���ɾ��������ڴ�ֲ���һά��ʽ
//         win_c/win_r:���ڴ�С��
//         need_normalize�Ƿ���Ҫ��һ���˲�������
//         need_flip:�Ƿ���Ҫ��תfilter
//         Ky:��������������������ԭʼ��������
//becare:�ڴ�Ty���͵����ݲ����ڼ����������������缴MatΪuchar(Ky)�ģ�ʹ��uint16_t ���߸��ߵ�Ty
//       �������ṩ��ͳһ������㣬���û�����filter�����ص�����κ��Ż��������Ż�������������ʵ��
//       �������������ͨ������Ϊ4(rgba)
//TODO:���˲��˺ܴ�ʱ����DFT���㣬�ο�opencvԴ��
template<typename Ty, typename Ky>
void filter2D(cv::Mat& data, 
	cv::Mat& img, 
	Ty* arr_filter, 
	int win_c, 
	int win_r, 
	bool need_normalize = true, 
	bool need_flip = true, 
	Ky* =nullptr)
{
	//FIXME:ͨ��ʵ�ַ����DataType��ȡ��Ky����ʾ˵��
	Ty* filter = arr_filter;
	img.create(data.size(), data.type());

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
				//FIXME:��ֹ�������
				//cur_tmp[cn] = static_cast<Ky>(rgba[cn] * kFactor);
				cur_tmp[cn] = cv::saturate_cast<Ky>(rgba[cn] * kFactor);
			}
			cur += kElemSize;
			cursor += kElemSize;
		}
	}

	//��ԭ��С
	size = img.size();
	data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}

//brief:�ṩ���ڷ����Բ������˲��ӿڣ�Ϊÿ�����ڵ����û�ָ������
//parameter:src:ԭʼ����
//          dst:Ŀ��洢
//          ops:�û�ָ������
//          win_c/win_r:��/��
//          Ky*:�������󣬽����ڱ���src��������
//becare:OpӦ��������operator()(Ky[] src,Ky* dst),����srcΪ���ڵ�ÿ����ʼ��ַ��������win_r��������dstΪ�����ַ
template<typename Op, typename Ky>
void filter2DNonLinear(cv::Mat& data, 
	cv::Mat& img, 
	Op ops,
	int win_c, 
	int win_r, 
	Ky* =nullptr)
{
	img.create(data.size(), data.type());

	assert(data.type() == img.type());
	assert(data.size() == img.size());

	cv::Size old_size = data.size();
	const int offset_c = win_c >> 1;
	const int offset_r = win_r >> 1;
	const int kElemSize = static_cast<int>(data.elemSize());
	const int kStep = static_cast<int>(data.step[0]);

	//FIXME:����ʹ�� vector ??
	//std::shared_ptr<Ky*> arr_dst(new Ky*[win_r] ,deleter<Ky>);//c++17֮ǰ��shared_ptr���ڶ�ά��̬�������չ̫�����
	std::vector<Ky*> arr_dst(win_r, nullptr);

	//��չ�߽�
	cv::copyMakeBorder(data, data,offset_r, offset_r, offset_c, offset_c, cv::BORDER_DEFAULT);

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
	size = img.size();
	data(cv::Rect(offset_c, offset_r, size.width, size.height)).copyTo(data);
}



}//!namespace detail


#endif // !_SOLUTION_BASE_H_
