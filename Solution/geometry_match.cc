#include"Solution/geometry_match.h"
#include"Solution/type_extension.h"

#include<opencv2/highgui.hpp>

namespace detail {

//brief:�����׼ֱ�߼��
//beacre:���ﴢ����ֻ����CV_64C2���͵�������������ʹ��vector<Vec2f>��¼
//       �������ݱ�����CV_8UC1�Ķ�ֵͼ�񣬰����е�ֵΪUINT8_MAX��Ϊǰ������
void HoughLines(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold) {
	cv::Mat in = src.getMat();
	assert(in.type() == CV_8UC1);
	assert(in.isContinuous());

	//��ʼ���ۼ���
	//becare:�����opencv����һ���Ĳ��죬��Ϊ�и�����������ʼ������
	cv::Size size = in.size();
	double max_distance = std::sqrt(size.height*size.height + size.width*size.width);
	int angle_number = static_cast<int>(std::ceil(CV_PI / theta));
	int rho_number = static_cast<int>(2 * max_distance / rho) + 4;
	double half_rho = rho * 0.5;
	double offset = rho_number * rho * 0.5 - half_rho;
	cv::Mat accumulator = cv::Mat::zeros(cv::Size(rho_number, angle_number), CV_32SC1);

	//���б�׼����任
	auto iter = in.data;
	for (int y = 0; y < size.height; ++y) {
		for (int x = 0; x < size.width; ++x) {
			if (*iter++ != UINT8_MAX)
				continue;
			double alpha = 0;
			for (int i = 0; i < angle_number; ++i) {
				double rho_current = x * std::cos(alpha) + y * std::sin(alpha);
				int pos = static_cast<int>((rho_current + offset) / rho);

				accumulator.at<int32_t>(i, pos) += 1;
				alpha += theta;
			}

		}
	}

#ifndef SHOW_PROCESS

	string name = "HoughSpace";
	cv::namedWindow(name, CV_WINDOW_NORMAL);
	cv::Mat accu_img;
	cv::normalize(accumulator, accu_img, 1, UINT8_MAX, cv::NORM_MINMAX, CV_8U);
	cv::imshow(name, accu_img);
	cv::waitKey(0);
	cv::destroyWindow(name);

#endif


	//ͳ��̽���ֱ��
	std::vector<cv::Vec2d> tmp;
	auto cur = accumulator.ptr<int>(0, 0);
	size = accumulator.size();
	double alpha = 0;
	for (int y = 0; y < size.height; ++y) {
		double distance = - offset + half_rho;
		for (int x = 0; x < size.width; ++x) {
			//if(fabs(distance +0.5*rho -232)<1e-5 && fabs(alpha-1.51844)<1e-5)


			//if (*cur++ <= threshold) {//FIXME:==ʱ��Ӧ������
			if (*cur++ < threshold) {
				distance += rho;
				continue;
			}

			//�ڻ���ֱ�߿ռ��г�����ֵ
			//tmp.emplace_back(alpha, distance);//Ϊ����opecv���ݣ��������Ⱦ����Ƕ�
			tmp.emplace_back(distance, alpha);
			distance += rho;
		}
		alpha += theta;
	}

	//�������䵽�û��ռ�
	int len = static_cast<int>(tmp.size());
	lines.create(cv::Size(1, len), CV_64FC2);
	cv::Mat out = lines.getMat();
	auto ptr = out.ptr<double>(0, 0);
	for (auto& val : tmp) {
		*ptr++ = val[0];
		*ptr++ = val[1];
	}
}

//brief:���ʻ���ֱ�߼��
void HoughLinesP(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold,
	double min_line_length, double max_line_length){
	//TODO

}

}//!namespace detail