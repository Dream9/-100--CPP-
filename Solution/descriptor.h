//本文件提供了有关特征描述符提取的类
#ifndef _SOLUTION_DESCRIPTOR_H_
#define _SOLUTION_DESCRIPTOR_H_

#include<opencv2/core.hpp>

namespace detail {
//brief:Histgram of Oriented Gradient Descriptor,
//      功能和实现参照了cv::HOGDescriptor
class HOGDescriptor {
public:
	HOGDescriptor(cv::Size win_size,
		cv::Size block_size,
		cv::Size block_stride,
		cv::Size cell_size,
		int bins)
		:win_size_(win_size),
		block_size_(block_size),
		block_stride_(block_stride),
		cell_size_(cell_size),
		bins_(bins)
	{
		;
	}

	//brief:获得单个窗口描述子大小
	size_t getDescriptorSize() const;

	//brief:计算图像的HOG特征
	//void compute(cv::InputArray src, cv::OutputArray descriptors);
	void compute(cv::InputArray src, std::vector<float>& descriptors);
private:

	//brief:计算单个cell的描绘性特征（未归一化）
	void _calc_cell_descriptor(float* angle,
		float* grad,
		size_t angle_step,
		size_t grad_step,
		float* dst,
		cv::Mat* test = nullptr,
		int row = 0,
		int col = 0);

	cv::Size win_size_;
	cv::Size block_size_;
	cv::Size block_stride_;
	cv::Size cell_size_;
	int bins_;
};

}//!namespace detail
#endif
