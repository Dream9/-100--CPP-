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
	
	//becare:cv::filter2D�����kernel����ʱ��ͨ�������ͱ���������������flip��������һ��������ʹ�������ⲿ���
	//       ������Ϊ��������һ��������ڴ��Ĳ����Ǹ�����
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

	//ʹ��ͳһ�ľ������
	detail::filter2D<int, uchar>(&data, &img, &filter[0], win_, win_,true, false, nullptr);

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

//brief:�õ�ָ���Ƕȵ��˶�ģ������
void MotionBlur::getMotionBlurCoefficient_(int* dst) {
	
	//TODO:ʵ������������

	//�ṩ������45�ȷ�����˶�ģ��
	//int i = win_ - 1;
	//for (; i >= 0; --i) {
	//	dst[i*win_ + i] = 1;
	//}

	//�ṩˮƷ����ģ��
	int* med = dst + (win_ >> 1);
	for (int i = win_ - 1; i >= 0; --i) {
		med[i * win_] = 1;
	}
}

}//!namespace digital