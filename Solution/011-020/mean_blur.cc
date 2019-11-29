#include"Solution/mean_blur.h"

#include<opencv2/highgui.hpp>

#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#endif

#include<vector>
#include<string>
namespace digital {

//brief:��ֵ�˲�
//     ����˿�����ֵ�˲���ֻ���״μ���ƽ��ֵʱ��Ҫȫ������
void MeanBlur::operator()() {
	auto data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

#ifdef USE_OPENCVLIB

	cv::Mat img;
	cv::blur(data, img, { win_,win_ });

#else

	cv::Size old_size = data.size();
	cv::Mat img = data.clone();
	int offset = win_ >> 1;
	cv::copyMakeBorder(data, data, offset, offset, offset, offset, cv::BORDER_DEFAULT);

	cv::Size size = data.size();
	auto cur = img.data;

	int sum_r = 0;
	int sum_g = 0;
	int sum_b = 0;

	const double kN2 = 1.0 / (win_*win_);

	//��ֵ�˲�
	for (int i = offset; i < size.width - offset; ++i) {
		auto cursor = data.ptr(i - offset, 0);
		for (int j = offset; j < size.height - offset; ++j) {
			if (j == offset) {
				//ȫ������
				sum_r = 0;
				sum_g = 0;
				sum_b = 0;
				
				//�����������ڸ���
				auto tmp = cursor;
				//for test
				assert(tmp == data.ptr(i - offset, j - offset));

				for (int x = 0; x < win_; ++x) {
					auto iter = tmp;
					for (int y = 0; y < win_; ++y) {
						sum_r += iter[2];
						sum_g += iter[1];
						sum_b += iter[0];

						iter += 3;
					}
					tmp += data.step[0];
				}
			}
			else {
				//��������
				auto iter = cursor - data.elemSize();
				auto last_iter = cursor + (win_ - 1)*data.elemSize();
				//for test
				assert(iter == data.ptr(i - offset, j - offset - 1));

				for (int x = 0; x < win_; ++x) {
					//for test
					//assert(iter == data.ptr(i - offset + x, j - offset - 1));
					
					//ȥ��һ�У�����һ��
					sum_r -= iter[2];
					sum_g -= iter[1];
					sum_b -= iter[0];

					sum_r += last_iter[2];
					sum_g += last_iter[1];
					sum_b += last_iter[0];

					iter += data.step;
					last_iter += data.step;
				}
			}

			cur[2] = static_cast<uint8_t>(sum_r * kN2);
			cur[1] = static_cast<uint8_t>(sum_g * kN2);
			cur[0] = static_cast<uint8_t>(sum_b * kN2);

			cur += 3;
			cursor += 3;
			assert(cursor == data.ptr(i - offset, j - offset + 1));
		}
	}

#endif

	if (needShowOriginal()) {

#ifndef USE_OPENCVLIB
		data = data(cv::Rect(offset, offset, old_size.width, old_size.height));
#endif
		show(&data, &img);
	}
	else
		show(&img);

}

}//!namespace digital