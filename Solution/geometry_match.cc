#include"Solution/geometry_match.h"
#include"Solution/type_extension.h"

#include<opencv2/highgui.hpp>

namespace detail {

//brief:霍夫标准直线检测
//beacre:这里储存结果只返回CV_64C2类型的行向量，可以使用vector<Vec2f>记录
//       输入数据必须是CV_8UC1的二值图像，把其中的值为UINT8_MAX作为前景数据
void HoughLines(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold) {
	cv::Mat in = src.getMat();
	assert(in.type() == CV_8UC1);
	assert(in.isContinuous());

	//初始化累加器
	//becare:这里和opencv存在一定的差异，因为切割距离的坐标起始点问题
	cv::Size size = in.size();
	double max_distance = std::sqrt(size.height*size.height + size.width*size.width);
	int angle_number = static_cast<int>(std::ceil(CV_PI / theta));
	int rho_number = static_cast<int>(2 * max_distance / rho) + 4;
	double half_rho = rho * 0.5;
	double offset = rho_number * rho * 0.5 - half_rho;
	cv::Mat accumulator = cv::Mat::zeros(cv::Size(rho_number, angle_number), CV_32SC1);

	//进行标准霍夫变换
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


	//统计探测的直线
	std::vector<cv::Vec2d> tmp;
	auto cur = accumulator.ptr<int>(0, 0);
	size = accumulator.size();
	double alpha = 0;
	for (int y = 0; y < size.height; ++y) {
		double distance = - offset + half_rho;
		for (int x = 0; x < size.width; ++x) {
			//if(fabs(distance +0.5*rho -232)<1e-5 && fabs(alpha-1.51844)<1e-5)


			//if (*cur++ <= threshold) {//FIXME:==时不应被过滤
			if (*cur++ < threshold) {
				distance += rho;
				continue;
			}

			//在霍夫直线空间中超过阈值
			//tmp.emplace_back(alpha, distance);//为了与opecv兼容，返回是先距离后角度
			tmp.emplace_back(distance, alpha);
			distance += rho;
		}
		alpha += theta;
	}

	//将结果填充到用户空间
	int len = static_cast<int>(tmp.size());
	lines.create(cv::Size(1, len), CV_64FC2);
	cv::Mat out = lines.getMat();
	auto ptr = out.ptr<double>(0, 0);
	for (auto& val : tmp) {
		*ptr++ = val[0];
		*ptr++ = val[1];
	}
}

//brief:概率霍夫直线检测
void HoughLinesP(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold,
	double min_line_length, double max_line_length){
	//TODO

}

}//!namespace detail