#include"Solution/041-050/hough_lines.h"
#include"Solution/geometry_match.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include<vector>


#include<iostream>

namespace {

const int kUpper = 500;
const int kLower = 250;

const int kKernelSize = 5;

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
	cv::GaussianBlur(data, data, cv::Size(kKernelSize, kKernelSize), 2);
	
	//概率hough,存在一定的误差，但运行速度比标准hough快
	std::vector<cv::Vec4i> linesP;
	detail::Canny(data, binary, kLower, kUpper, 5, false);
#ifdef USE_OPENCVLIB
	cv::HoughLinesP(binary, linesP, rho_, theta_, threshold_, 20, 10);
#else
	detail::HoughLinesP(binary, linesP, rho_, theta_, threshold_, 20, 10);
#endif
	img = original.clone();
	for (size_t i = 0; i < linesP.size(); i++)
	{
		cv::Vec4i l = linesP[i];
		cv::line(img, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}
	if (needShowOriginal())
		show(&original, &img, "概率hough");
	else
		show(&img, "概率hough");


	//标准Hough
#ifdef USE_OPENCVLIB
	cv::Canny(data, binary, kLower, kUpper, 5, false);
	show(&binary);
	cv::HoughLines(binary, lines, rho_, theta_, threshold_);//it should be the same except the 4 borders
#else
	detail::Canny(data, binary, kLower, kUpper, 5, false);
	show(&binary);
	detail::HoughLines(binary, lines, rho_, theta_, threshold_);
#endif

	detail::overlapHoughImage(original, binary, lines, img);

	if (needShowOriginal())
		show(&original, &img);
	else
		show(&img);
}


}//！namespace digital