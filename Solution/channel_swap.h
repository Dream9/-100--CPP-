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
	~ChannelSwap() override{}

	void operator()()override;

};

}//!namespace digital



#endif