//提供有关几何形状匹配等相关的封装

#include<opencv2/core.hpp>

namespace detail{

//brief:霍夫标准直线检测
//parameter:src:输入图像
//          lines:输出结果，注意输出的是对应直线的(rho,theta)值，可以用std::vector<cv::vector2f>作为输出的容器
//          rho:霍夫直线空间中距离的step
//          theta:霍夫直线空间中角度的step,单位弧度
//          int threshold:判定为直线的阈值
void HoughLines(cv::InputArray src,
	cv::OutputArray lines,
	double rho,
	double theta, 
	int threshold);


//brief:概率霍夫直线检测
//parameter:min_line_length / max_line_length:最小/最大线段的长度限制，默认没有限制
//          其他参数参见HoughLines参数设置
//becare:概率判断存在一定的误差，基本原理和标准变换一致，只不过，在遍历过程中就会提前提取并剔除线段
void HoughLinesP(cv::InputArray src,
	cv::OutputArray lines,
	double rho,
	double theta,
	int threshold,
	double min_line_length = -1,
	double max_line_length = -1);

//brief:
//parameter: original:原始图像
//           mask:边界数据，通常是Canny等边缘检测的二值化结果
//           hough：houghLines结果
//           dst：输出
//           val:边界颜色
void overlapHoughImage(cv::InputArray original,
	cv::InputArray mask,
	cv::InputArray hough, 
	cv::OutputArray dst,
	cv::Scalar val = cv::Scalar(0,0,255));

}//!namespace detail