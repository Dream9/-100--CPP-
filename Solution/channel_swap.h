#ifndef _SOLUTION_CHANNEL_SWAP_H_
#define _SOLUTION_CHANNEL_SWAP_H_ 

#include"Solution/solution.h"

#include<opencv2/highgui.hpp>

#include<iostream>

namespace digital {

//brief:
class ChannelSwap :public Solution {
public:
	ChannelSwap(const string& path,
				bool show_original = false,
				const string& name = "ChannelSwap")
		:Solution(name, path, show_original)
	{
		Solution::getDescriptionHandler().assign("读取图像，然后将 RGB 通道替换成 BGR 通道。");
	}
	virtual ~ChannelSwap(){}


	virtual void operator()() {
		cv::Mat data = cv::imread(getPath(), 1);
		//assert(!data.empty());

		if (data.empty())
			return dealException(kFileError);

		cv::Mat img = needShowOriginal() ? data.clone() : data;

		int rows = img.rows;
		int cols = img.cols;
		int channels = img.channels();

		//brief:主要是熟悉一下Mat的内存结构
		//      这里的操作还是比较灵活的，只要清楚当前指针及数据结构
		for (int i = 0; i < rows; ++i) {
			//for (int j = 0; j < cols; ++j) {
			//  //通过指向cv::Vec3b的指针访问某个点数据
			//	//auto ptr = img.ptr<cv::Vec3b>(i, j);
			//	//std::swap((*ptr)[0], (*ptr)[2]);
			//
			//  //通过uint8_t*访问某个像素点数据
			//	auto ptr = img.ptr<uint8_t>(i,j);
			//	std::iter_swap(ptr, ptr + 2);

			//}

			//把整个一行三通道看作一行访问
			auto ptr = img.ptr<uint8_t>(i);
			for (int j = 0; j < cols; ++j) {
				auto tmp = ptr + j * channels;
				std::iter_swap(tmp, tmp + 2);
			}

			//此外也可以通过cv::Mat::at操作
			//或者借助data得到最初数据指针，然后手动计算偏移(step[0],step[1])操作
			//或则借助迭代器
			//本质都是一样的，关键是了解Mat如何组织数据的
		}

		//for test
		//__MatrixTest(&data, &img);
		
		if (needShowOriginal())
			show(&data, &img);
		else
			show(&img);
	}

};

}//!namespace digital



#endif