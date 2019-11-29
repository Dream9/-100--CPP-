#ifndef _SOLUTION_MEDIAN_BLUR_H_
#define _SOLUTION_MEDIAN_BLUR_H_

#include"Solution/solution.h"

namespace digital {

//brief:中值滤波
//     作为基于排序的滤波方法，中值滤波适合处理脉冲噪声
class MedianBlur :public Solution {
public:
	//parameter: win:窗口大小，期待奇数
	MedianBlur(const string& path, int win=5, bool flag =false, const string& name="MedianBlur")
		:Solution(name,path,flag),win_(win)
	{
		assert(win_ > 0);
		assert((win_ & 0x1) == 0x1);

		getDescriptionHandler().assign("使用中值滤波处理图像");
	}

	~MedianBlur()override {}

	void operator()()override;

private:
	//快速中值滤波
	void fastMedianBlur_(void* src, void* dst, int win);
	int getMedianFromPdf_(int* arr, int half);

	static const int kThresholdForFastMedianBlur;

	int win_;
};


}//!namespace digital


#endif // !_SOLUTION_MEDIAN_BLUR_H_
