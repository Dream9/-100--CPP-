#include"Solution/011-020/max_min_filter.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>

#include<functional>

namespace digital {

//becare:����������ڻҶȿռ�����ɵģ���˻�ȥ��ԭ���Ķ�ͨ��
void MaxMinFilter::operator()() {
	//FIXME:����֮���ٻҶȻ�
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img;

	//FIXME:�Ժ���/Ƕ�������ʽ������ȥ����
	auto ops = [=](uint8_t** arr, uint8_t* dst) {
		uint8_t Max_gray = 0;
		
		uint8_t Min_gray = UINT8_MAX;
		int i = win_ - 1;

		while (i > 0) {
			uint8_t* iter = arr[i];
			int j = 0;
			while (j < win_) {
				Max_gray = MAX(Max_gray, iter[j+2]);
				Min_gray = MIN(Min_gray, iter[j+2]);

				++j;
			}
			--i;
		}
		*dst = Max_gray - Min_gray;
	};

	detail::filter2DNonLinear<decltype(ops), uint8_t>(data, img, ops, win_, win_, nullptr);

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

}