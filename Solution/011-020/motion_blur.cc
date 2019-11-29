#include"Solution/motion_blur.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#endif
#include<vector>

namespace digital {

void MotionBlur::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img = data.clone();

#ifdef USE_OPENCVLIB
	
	//becare:cv::filter2D传入的kernel必须时单通道浮点型变量，而且他不做flip，不做归一化，必须使用者在外部完成
	//       正是因为他不做归一化，因此期待的参数是浮点型
	cv::Mat kernel = (cv::Mat_<float>(win_, win_) << 1.0/3, 0, 0,
		0, 1.0/3, 0,
		0, 0, 1.0/3);
	cv::flip(kernel, kernel,-1);
	//for test
	//std::vector<uchar> sdfsdf(kernel.clone().reshape(1,1));
	cv::filter2D(data, img, data.depth(), kernel);

#else
	
	const int kWinSize = win_ * win_;
	std::vector<int>filter(kWinSize, 0);
	getMotionBlurCoefficient_(&filter[0]);

	//使用统一的卷积运算
	detail::filter2D<int, uchar>(&data, &img, &filter[0], win_, win_,true, false, nullptr);

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

//brief:得到指定角度的运动模糊算子
void MotionBlur::getMotionBlurCoefficient_(int* dst) {
	
	//TODO:实现其完整功能

	//提供了沿着45度方向的运动模糊
	//int i = win_ - 1;
	//for (; i >= 0; --i) {
	//	dst[i*win_ + i] = 1;
	//}

	//提供水品方向模糊
	int* med = dst + (win_ >> 1);
	for (int i = win_ - 1; i >= 0; --i) {
		med[i * win_] = 1;
	}
}

}//!namespace digital