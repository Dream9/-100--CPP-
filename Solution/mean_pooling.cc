#include"Solution/mean_pooling.h"

#include<opencv2/highgui.hpp>
#include<opencv.hpp>

namespace digital {

//brief:mean-pooling�ܸ���ı���ͼ��ı�����Ϣ
void MeanPooling::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_COLOR);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	//becare:����ֻ�����˾�ֵ�ػ�����û�н��з��򴫲�
	//       ���򴫲���Ҫ��֤�в�䣬��˽�ÿ��ֵ���ַŻؼ���
	cv::Size size = data.size();
	cv::Size img_size;
	img_size.width = size.width / win_;
	img_size.height = size.height / win_;

	if (img_size.width * win_ != size.width || img_size.height * win_ != size.height) {
		dealException(kParameterNotMatch);
		//TODO:����ߴ粻ƥ��
		return;
	}

	cv::Mat img(img_size, data.type());
	const int kN2 = static_cast<int>(win_ * win_);

	auto cur = img.data;
	for (int i = 0; i < size.height; i += win_) {
		for (int j = 0; j < size.width; j += win_) {
			uint16_t r = 0;//��ֹ���
			uint16_t g = 0;
			uint16_t b = 0;

			//��ǰ������ȡ��ֵ
			for (int m = 0; m < win_; ++m) {
				auto iter = data.ptr<uchar>(i + m, j);
				for (int n = 0; n < win_; ++n) {
					r += iter[2];
					g += iter[1];
					b += iter[0];

					iter += 3;
				}
			}

			cur[0] = static_cast<uint8_t>(b / kN2);
			cur[1] = static_cast<uint8_t>(g / kN2);
			cur[2] = static_cast<uint8_t>(r / kN2);

			cur += 3;
		}
	}

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

}