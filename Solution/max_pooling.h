#ifndef _SOLUTION_MAX_POOLING_H_
#define _SOLUTION_MAX_POOLING_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class MaxPooling :public Solution {
public:
	MaxPooling(const string& path, int win = 8, bool flag = false, const string& name ="MaxPooling")
		:Solution(name, path, flag),win_(win)
	{
		getDescriptionHandler().assign("使用最大值池化法对图像池化");
	}

	~MaxPooling() override {}

	void operator()()override;

private:
	//池化窗口大小
	int win_;
};


}

#endif