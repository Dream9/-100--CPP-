#ifndef _SOLUTION_SOBEL_OPERATOR_H_
#define _SOLUTION_SOBEL_OPERATOR_H_

#include"Solution/solution.h"

namespace digital {

//brief:sobel算子,对中心像素增加权重的一阶微分算子
class SobelOperator : public Solution {
public:
	//becare：win必须位奇数，并且不大于11,如果小于等于1，自动取3
	SobelOperator(const string& path, int win, bool flag = false, const string& name = "SobelOperator")
		:Solution(name, path, flag), win_(win <= 1 ? 3 : win)
	{
		assert((win_ & 0x1) == 0x1);
		assert(win_ <= 11);

		getDescriptionHandler().assign("通过Sobel算子检测水平/垂直边沿，并获得梯度图像");
	}
	~SobelOperator()override {}

	void operator()()override;

private:
	//窗口大小代表不同阶数的sobel算子
	const int win_;
};
}
#endif // !_SOLUTION_SOBEL_OPERATOR_H_
