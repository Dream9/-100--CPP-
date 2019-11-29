#include"Solution/median_blur.h"

#include<opencv2/highgui.hpp>

#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#endif


#include<vector>
#include<algorithm>

namespace digital {

const int MedianBlur::kThresholdForFastMedianBlur = 9;

//brief:��ֵ�˲��������ڽϴ�ʱ�Ż���ÿ�����ֵ�˲�
//     �߽���չ����BORDER_REPLICATE
void MedianBlur::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//��չ�߽�
	cv::Size old_size = data.size();
	const int offset = win_ >> 1;
	cv::copyMakeBorder(data, data, offset, offset, offset, offset, cv::BORDER_REPLICATE);
	cv::Mat img(old_size, data.type());

#ifdef USE_OPENCVLIB

	cv::medianBlur(data, img, win_);

#else

	if (win_ >= kThresholdForFastMedianBlur && data.depth() == CV_8U) {
		//������ֵ�˲�
		fastMedianBlur_(&data, &img, win_);
	}
	else {
		//����medium of medium����ֵ�˲�;
		cv::Size size = data.size();
		int step = static_cast<int>(data.step[0]);

		const int N = win_ * win_;
		const int kHalf = N >> 1;

		std::vector<int> arr_b(N,0);
		std::vector<int> arr_g(N,0);
		std::vector<int> arr_r(N,0);

		for (int i = offset; i < size.height - offset; ++i) {
			auto cur = img.ptr<uchar>(i - offset, 0);
			for (int j = offset; j < size.width - offset; ++j) {
				int n = 0;
				//����medium of medium��λ��n��Ԫ��
				for (int x = i - offset; x <= i + offset; ++x) {
					auto iter = data.ptr<uchar>(x, j - offset);
					for (int y = j - offset; y <= j + offset; ++y) {
						arr_b[n] = *iter++;
						arr_g[n] = *iter++;
						arr_r[n++] = *iter++;
					}
				}
				//��Ҫ����ȫ���������stlʵ���ǽ����ڿ��ţ�������KPRFT�㷨
				std::nth_element(arr_b.begin(), arr_b.begin() + kHalf, arr_b.end());
				std::nth_element(arr_g.begin(), arr_g.begin() + kHalf, arr_g.end());
				std::nth_element(arr_r.begin(), arr_r.begin() + kHalf, arr_r.end());

				cur[0] = arr_b[kHalf];
				cur[1] = arr_g[kHalf];
				cur[2] = arr_r[kHalf];

				cur += 3;
			}
		}
	}

#endif

	//img = img(cv::Rect(n, n, old_size.width, size.height));

	if (needShowOriginal()) {
		data = data(cv::Rect(offset, offset, old_size.width, old_size.height));
		show(&data, &img);
	}
	else
		show(&img);

}

//brief:������ֵ�˲�Ҫ�󴰿�Ҫ�Ƚϴ󣬷��򻹲�������Ч�ʸߣ����������Ȳ���̫��
//      �Ͼ�����pdf�ķ�ʽͳ����ֵ
//      �ӱ������潲�����㷨ֻ��Ͱ����������������������Ȼ�ǻ�������ģ�ֻ����
//      ����������һ�εļ���������
void MedianBlur::fastMedianBlur_(void* src, void* dst, int win) {
	cv::Mat& data = *static_cast<cv::Mat*>(src);
	cv::Mat& img = *static_cast<cv::Mat*>(dst);

	assert(data.depth() == CV_8U);
	const int N = 1 << 8;
	//std::shared_ptr<int> hist;
	int hist_r[N];
	int hist_g[N];
	int hist_b[N];

	cv::Size size = data.size();
	const int offset = win >> 1;
	const int kHalf = win_ * win_ >> 1;

	for (int i = offset; i < size.height - offset; ++i) {
		auto cur = img.ptr<uchar>(i - offset, 0);
		for (int j = offset; j < size.width - offset; ++j) {
			if (j == offset) {
				//����ȫ������hist
				memset(hist_r, 0, sizeof hist_r);
				memset(hist_g, 0, sizeof hist_g);
				memset(hist_b, 0, sizeof hist_b);

				for (int x = i - offset; x <= i + offset; ++x) {
					auto iter = data.ptr<uchar>(x, j - offset);
					for (int y = j - offset; y <= j + offset; ++y) {
						++hist_b[*iter++];
						++hist_g[*iter++];
						++hist_r[*iter++];
					}
				}
			}
			else {
				//��������
				auto iter = data.ptr<uchar>(i - offset, j - offset);
				auto last_iter = data.ptr<uchar>(i - offset, j + offset);
				for (int x = i - offset; x <= i + offset; ++x) {
					assert(iter == data.ptr<uchar>(x, j - offset));
					assert(last_iter == data.ptr<uchar>(x, j + offset));

					--hist_b[*iter];
					--hist_g[*(iter + 1)];
					--hist_r[*(iter + 2)];
					
					++hist_b[*last_iter];
					++hist_g[*(last_iter + 1)];
					++hist_r[*(last_iter + 2)];

					iter += data.step[0];
					last_iter += data.step[0];
				}
			}

			//��ֵ
			cur[0] = getMedianFromPdf_(hist_b, kHalf);
			cur[1] = getMedianFromPdf_(hist_g, kHalf);
			cur[2] = getMedianFromPdf_(hist_r, kHalf);

			cur += 3;
		}
	}
}

//brief:��pdf��ȡ��ֵ
//becare:����Ч�ʣ���˵�N(256)Զ�����˲����ڴ�Сʱ����ν�Ŀ�����ʵ������
int MedianBlur::getMedianFromPdf_(int* arr, int half) {
	int i = 255;
	for (; i >= 0; --i) {
		half -= arr[i];
		if (half < 0)
			return i;
	}

	dealException(kFatal);

	//��������������
	return -1;//error
}



}//!namespace dit