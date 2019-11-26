#include"Solution/median_blur.h"

#include<opencv2/highgui.hpp>

#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#endif


#include<vector>
#include<algorithm>

namespace digital {

const int MedianBlur::kThresholdForFastMedianBlur = 9;

//brief:中值滤波，当窗口较大时才会采用快速中值滤波
//     边界扩展采用BORDER_REPLICATE
void MedianBlur::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//扩展边界
	cv::Size old_size = data.size();
	const int offset = win_ >> 1;
	cv::copyMakeBorder(data, data, offset, offset, offset, offset, cv::BORDER_REPLICATE);
	cv::Mat img(old_size, data.type());

#ifdef USE_OPENCVLIB

	cv::medianBlur(data, img, win_);

#else

	if (win_ >= kThresholdForFastMedianBlur && data.depth() == CV_8U) {
		//快速中值滤波
		fastMedianBlur_(&data, &img, win_);
	}
	else {
		//基于medium of medium的中值滤波;
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
				//采用medium of medium定位第n个元素
				for (int x = i - offset; x <= i + offset; ++x) {
					auto iter = data.ptr<uchar>(x, j - offset);
					for (int y = j - offset; y <= j + offset; ++y) {
						arr_b[n] = *iter++;
						arr_g[n] = *iter++;
						arr_r[n++] = *iter++;
					}
				}
				//不要采用全排序，这里的stl实现是借助于快排，并不是KPRFT算法
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

//brief:快速中值滤波要求窗口要比较大，否则还不如排序效率高，其次像素深度不能太大
//      毕竟采用pdf的方式统计中值
//      从本质上面讲，本算法只是桶排序的特殊情况，本质上任然是基于排序的，只不过
//      便于利用上一次的计算结果而已
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
				//初次全量更新hist
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
				//增量更新
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

			//赋值
			cur[0] = getMedianFromPdf_(hist_b, kHalf);
			cur[1] = getMedianFromPdf_(hist_g, kHalf);
			cur[2] = getMedianFromPdf_(hist_r, kHalf);

			cur += 3;
		}
	}
}

//brief:从pdf获取中值
//becare:线性效率，因此当N(256)远大于滤波窗口大小时，所谓的快速其实并不快
int MedianBlur::getMedianFromPdf_(int* arr, int half) {
	int i = 255;
	for (; i >= 0; --i) {
		half -= arr[i];
		if (half < 0)
			return i;
	}

	dealException(kFatal);

	//不可能允许到这里
	return -1;//error
}



}//!namespace dit