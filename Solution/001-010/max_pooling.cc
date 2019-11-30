#include"Solution/001-010/max_pooling.h"

#include<opencv2/highgui.hpp>

namespace digital {

//brief:最大值池化会更多的保留纹理信息，减少卷积层参数误差导致的均值偏移
//     其反向传播是将最大值放回原来的位置，其他数据置空
void MaxPooling::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Size size = data.size();
	int step = static_cast<int>(data.step[0]);

	//TODO:处理不整除的情况
	cv::Size img_size(size.width / win_, size.height / win_);
	if (img_size.width*win_ != size.width || img_size.height*win_ != size.height) {
		dealException(kParameterNotMatch);
		return;
	}

	//FIXME:使用detail::filter2DNonLinear完成以下操作

	cv::Mat img = cv::Mat::zeros(img_size, data.type());
	auto cur = img.data;

	for (int i = 0; i < size.height; i += win_) {
		for (int j = 0; j < size.width; j += win_) {
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;

			auto iter = data.data+ step * i + data.step[1] * j;
			//当前窗口内取最大值
			for (int m = 0; m < win_; ++m) {
				auto tmp = iter;
				for (int n = 0; n < win_; ++n) {
					r = MAX(r, tmp[2]);
					g = MAX(g ,tmp[1]);
					b = MAX(b, tmp[0]);

					tmp += 3;
				}
				iter += step;//移动到下一行
			}

			cur[0] = b;
			cur[1] = g;
			cur[2] = r;

			cur += 3;
		}
	}

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}


}//!namespace digital