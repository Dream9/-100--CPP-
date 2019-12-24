//提供有关形态学变换的功能

#ifndef _SOLUTION_MORPHOLOGY_TRANSFORM_H_
#define _SOLUTION_MORPHOLOGY_TRANSFORM_H_

#include<opencv2/core.hpp>

namespace detail {

//与opencv兼容
enum MorphTypes {
	MORPH_ERODE = 0, //!< see #erode
	MORPH_DILATE = 1, //!< see #dilate
	MORPH_OPEN = 2, //!< an opening operation
	//!< \f[\texttt{dst} = \mathrm{open} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \mathrm{erode} ( \texttt{src} , \texttt{element} ))\f]
	MORPH_CLOSE = 3, //!< a closing operation
	//!< \f[\texttt{dst} = \mathrm{close} ( \texttt{src} , \texttt{element} )= \mathrm{erode} ( \mathrm{dilate} ( \texttt{src} , \texttt{element} ))\f]
	MORPH_GRADIENT = 4, //!< a morphological gradient
	//!< \f[\texttt{dst} = \mathrm{morph\_grad} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \texttt{src} , \texttt{element} )- \mathrm{erode} ( \texttt{src} , \texttt{element} )\f]
	MORPH_TOPHAT = 5, //!< "top hat"
	//!< \f[\texttt{dst} = \mathrm{tophat} ( \texttt{src} , \texttt{element} )= \texttt{src} - \mathrm{open} ( \texttt{src} , \texttt{element} )\f]
	MORPH_BLACKHAT = 6, //!< "black hat"
	//!< \f[\texttt{dst} = \mathrm{blackhat} ( \texttt{src} , \texttt{element} )= \mathrm{close} ( \texttt{src} , \texttt{element} )- \texttt{src}\f]
	MORPH_HITMISS = 7  //!< "hit or miss"
	//!<   .- Only supported for CV_8UC1 binary images. A tutorial can be found in the documentation
};

//brief:形态学膨胀，详情参见cv::dilate
//parameter：src:输入
//           dst输出
//           kernel:结构元SE
//           anchor:SE的锚点
//           iteration：迭代次数
void dilate(cv::Mat& src, 
	cv::Mat& dst,
	cv::Mat& kernel,
	cv::Point anchor = cv::Point(-1, -1),
	int iterations = 1);

//breif:形态学侵蚀
//parameter：src:输入
//           dst输出
//           kernel:结构元SE
//           anchor:SE的锚点
//           iteration：迭代次数
void erode(cv::Mat& src,
	cv::Mat& dst, 
	cv::Mat& kernel, 
	cv::Point anchor = cv::Point(-1, -1),
	int iterations = 1);


//brief:高级形态学变换
//parameter:src:输入
//          op:执行的操作，合法值参见MorphTypes
//          kernel:结构元SE
//          dst：输出
//          anchor:SE锚点
//          iteration：迭代次数
void morphologyEx(cv::Mat& src,
	cv::Mat& dst, 
	int op, 
	cv::Mat& kernel, 
	cv::Point anchor = cv::Point(-1, -1),
	int iteration = 1);


}





#endif !_SOLUTION_MORPHOLOGY_TRANSFORM_H_