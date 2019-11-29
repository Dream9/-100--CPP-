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

	const int N = 256 / 4;//���ٱ���
	const int bit = static_cast<int>(log2(N));//������λ����
	for (int i = 0; i < size.height; ++i) {
		auto iter = data.data + step * i;
		auto cur = img.data + step * i;

		for (int j = 0; j < size.width; ++j) {
			//Ҳ����ͨ��λ��������mod����֮ԭ����һ���ģ�����������ɫ��Χ���䣬������ɫ����
			//�޷���ѡ�����ʵ��ȥ���������е�λ���ֲ���
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