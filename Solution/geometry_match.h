//提供有关几何形状匹配等相关的封装

#include<opencv2/core.hpp>

namespace detail{

//brief:匹配方法，与opecv兼容
enum TemplateMatchModes {
    TM_SQDIFF        = 0, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')-I(x+x',y+y'))^2\f]
    TM_SQDIFF_NORMED = 1, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y')-I(x+x',y+y'))^2}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
    TM_CCORR         = 2, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')  \cdot I(x+x',y+y'))\f]
    TM_CCORR_NORMED  = 3, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y') \cdot I(x+x',y+y'))}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
    TM_CCOEFF        = 4, //!< \f[R(x,y)= \sum _{x',y'} (T'(x',y')  \cdot I'(x+x',y+y'))\f]
                          //!< where
                          //!< \f[\begin{array}{l} T'(x',y')=T(x',y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} T(x'',y'') \\ I'(x+x',y+y')=I(x+x',y+y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} I(x+x'',y+y'') \end{array}\f]
    TM_CCOEFF_NORMED = 5, //!< \f[R(x,y)= \frac{ \sum_{x',y'} (T'(x',y') \cdot I'(x+x',y+y')) }{ \sqrt{\sum_{x',y'}T'(x',y')^2 \cdot \sum_{x',y'} I'(x+x',y+y')^2} }\f]

	TM_ABSDIFF       = 6, //注意，这个是对opencv的扩展，cv::matchTemplate并不支持本参数
	TM_ABSDIFF_NORMED= 7, //注意，这个是对opencv的扩展，cv::matchTemplate并不支持本参数
};

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

//brief:根据标准霍夫变换结果以及原始图像以及边界掩码，填充检测的线段
//      主要是因为标准变换得到的结果是对直线描述的两个参数
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

//brief：模板匹配
//parameter：src:待匹配输入图像，只做了CV_8UC1的扩展
//           templ：模板图像，
//           dst:输出结果, type()为CV_32FC1
//           method:匹配模式，包括
void matchTemplate(cv::InputArray src,
	cv::InputArray templ,
	cv::OutputArray dst,
	int method);

}//!namespace detail