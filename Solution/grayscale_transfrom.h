//brief:提供有关灰度变换的功能封装，主要包括灰度反转，直方图统计匹配等仅涉及单个灰度值的图像处理功能

#ifndef _SOLUTION_GRAYSCALE_TRANSFORM_H_
#define _SOLUTION_GRAYSCALE_TRANSFORM_H_

#include<opencv2/core.hpp>
#include<functional>

namespace detail {

typedef std::function<void(uint8_t* cursor)> GrayScaleOperationType;

//brief:查找表
//     如果存在大量独立的重复计算，采用lut可以节省这些额外计算
//paramter:lut:参见cv::LUT,特别时关于通道数匹配方面
void LUT(cv::Mat& src, 
	cv::Mat& dst, 
	cv::Mat& lut);

//brief:
double otsuThreshold(cv::Mat& src);

//brief:计算单个Mat的概率密度函数
//parameter:src:带解析源
//          dst:存储目标，结果默认为CV_32F
//          min,max:数据源的范围
//          bins:分块数
//          *select_channels 和len用于说明参与计算的Mat的通道，默认时为全部参与运算，其组织形式参见cv::calcHist
//becare:Mat的depth必须是CV_8U,opencv中则还做了CV_16U和CV_32F的重载（通过if-else分发）
void calcHistogram(const cv::Mat& src,
	cv::Mat& dst,
	int min,
	int max,
	int bins,
	int* select_channels = nullptr,
	int len = -1);

//brief:根据灰度pdf绘制目标图像
//paramter: hist:r*1的pdf向量
//          dst:结果数据存储位置
//          size，type 目标类型
//          color:颜色
void fillHistogram(const cv::Mat& hist,
	cv::Mat& dst,
	const cv::Size& size,
	int type = CV_8UC1,
	const cv::Scalar& color = cv::Scalar::all(0));

//brief:灰度反转
//paramter:src:目标对象
//         max_value:图像最大值
void colorInversion(cv::Mat& src, int max_value = UINT8_MAX);

//brief:为每个像素值应用用户定义的操作ops
//paramter:dst:目标对象
//         ops:用户传入的可调用对象，类型参见定义
//          not_merge_channel:主要针对部分遍历需要把一个位置处的多个元素值一同处理(比如当作复数)，此时不能拆分他们单独遍历
//                            默认拆分处理，在计算相位是用当本函数
//becare:凡是仅涉及到单个像素的操作，基本上都可以通过本接口完成，必然gamma变换、灰度拉伸、均衡化等
void grayscaleTransform(cv::Mat& src, const GrayScaleOperationType& ops, bool not_merge_channel = false);

//brief:参见cv::convertScaleAbs实现
//     这里是利用了lambda对象配合grayscalTransform实现
//becare:由于grayscaleTransform只特化了CV_8U,因此本函数也只针对src为CV_8U类型的数据
void convertScaleAbs(cv::Mat& src, cv::Mat& dst, double alpha, double beta);

//brief:参见cv::equalizeHist实现
//becare:opencv中要求src必须是CV_8U1C的数据，这里做了一点小扩展，即多通道数据也可以参与运算
void equalizeHist(cv::Mat& src, cv::Mat& dst);

//brief:
//void addAbs(cv::InputArray first, cv::InputArray second, cv::OutputArray dst);

}//!namespace detail



#endif // !_SOLUTION_GRAYSCALE_TRANSFORM_H_
