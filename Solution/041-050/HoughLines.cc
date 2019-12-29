#include"Solution/041-050/hough_lines.h"
#include"Solution/geometry_match.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include<vector>


#include<iostream>
#include<algorithm>


namespace {

const int kUpper = 300;
const int kLower = 100;

}

namespace digital {

//brief:
void HoughLines::operator()() {
	cv::Mat original = cv::imread(getPath());
	if (original.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat data;
	cv::Mat img;
	cv::Mat binary;
	std::vector<cv::Vec2d> lines;

	cv::cvtColor(original, data, cv::COLOR_RGB2GRAY);
	cv::blur(data, data, cv::Size(3, 3));

#ifndef USE_OPENCVLIB
	cv::Canny(data, binary, kLower, kUpper, 5, false);
	show(&binary);
	cv::HoughLines(binary, lines, rho_, theta_, threshold_);//it should be the same except the 4 borders
#else
	detail::Canny(data, binary, kLower, kUpper, 5, false);
	show(&binary);
	detail::HoughLines(binary, lines, rho_, theta_, threshold_);
#endif

	//将探测的直线叠加到图像上
	img = original.clone();
	coutInfo(std::to_string(lines.size()).c_str());
	for (size_t i = 0; i < lines.size(); i++)
	{
		double rho = lines[i][0], theta = lines[i][1];
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		cv::line(img, pt1, pt2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}

	//与二值图像做一个交操作
	cv::Size size = binary.size();
	for(int y =0;y<size.height;++y){
		for (int x = 0; x < size.width; ++x) {
			if (*binary.ptr(y, x) != 255) {//其中非交集部分取原图像的值，从而使得直线受限于二值图像
				*img.ptr<cv::Vec3b>(y, x) = *original.ptr<cv::Vec3b>(y, x);
			}
		}
	}

	if (needShowOriginal())
		show(&original, &img);
	else
		show(&img);


}


}//！namespace digital