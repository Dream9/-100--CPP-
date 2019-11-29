#include"Solution/reduce_color.h"

#include<opencv2/highgui.hpp>

namespace digital {

//brief:
void ReduceColor::operator()() {
	cv::Mat data = cv::imread(getPath(), 1);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img = needShowOriginal() ? cv::Mat::zeros(data.size(), data.type()) : data;
	
	cv::Size size = data.size();
	int step = static_cast<int>(data.step);
	if (data.isContinuous()) {
		size.width *= size.height;
		step = step * size.height;
		size.height = 1;
	}

	const int N = 256 / 4;//减少比例
	const int bit = static_cast<int>(log2(N));//舍弃低位部分
	for (int i = 0; i < size.height; ++i) {
		auto iter = data.data + step * i;
		auto cur = img.data + step * i;

		for (int j = 0; j < size.width; ++j) {
			//也可以通过位操作或则mod，总之原理都是一样的，保持总体颜色范围不变，减少颜色种类
			//无非是选择何种实现去丢弃数据中低位数字部分
			//cur[0] = *iter / N * N + (N >> 1);
			//cur[1] = *(iter+1) / N * N + (N >> 1);
			//cur[2] = *(iter+2) / N * N + (N >> 1);

			cur[0] = ((*iter >> bit) << bit) + (N >> 1);
			cur[1] = ((*(iter+1) >> bit) << bit) + (N >> 1);
			cur[2] = ((*(iter+2) >> bit) << bit) + (N >> 1);

			cur += 3;
			iter += 3;
		}
	}

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}


}//!namespace digital